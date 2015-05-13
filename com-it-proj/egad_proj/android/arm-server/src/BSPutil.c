/*
 ============================================================================
 Name        : BSPutil.c.c
 Author      : doron sandroy
 Version     :
 Copyright   : Your copyright notice
 Description : timer util
 ============================================================================
 */
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include "BSPutil.h"
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
//#include <hardware/gps.h>

/* for Atheros GPS devices*/
#define ATHR_GPS
#define ATHR_GPSXtra
#define ATHR_GPSNi
//#include <android/log.h>
#define GPSPORT "/dev/ttymxc1"
#define DelayForLongData 1
#define UDB_BUFFER 2000
#define FALSE 0
#define TRUE 1
int wait_gps_flag=TRUE;
int connected = 0;
#define  LOG_TAG    "BSP"
int GPSfd;
timer_t gTimerid;

struct termios termAttr;
struct sigaction saio;

/* Nmea Parser stuff */
#define  NMEA_MAX_SIZE  83

enum {
  STATE_QUIT  = 0,
  STATE_INIT  = 1,
  STATE_START = 2
};


/******************************************************************
 *  Function name: init_timer
 *  Description
 *  in
 *  out
 ******************************************************************/
void init_timer(void)
{
	timer_create (CLOCK_REALTIME, NULL, &gTimerid);
}

/******************************************************************
 *  Function name: start_timer
 *  Description : start timer healer
 *  in: time
 *  out
 ******************************************************************/
void start_timer(long time)
{
	long time_temp = time * 1000*1000;
	struct itimerspec value;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = time_temp;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = time_temp;
	timer_settime (gTimerid, 0, &value, NULL);
}

/******************************************************************
 *  Function name: stop_timer
 *  Description
 *  in
 *  out
 ******************************************************************/
void stop_timer(void)
{
	struct itimerspec value;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 0;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;
	timer_settime (gTimerid, 0, &value, NULL);
	//timer_delete(gTimerid);
}

/******************************************************************
 *  Function name: signal_gps_handler_IO
 *  Description: get the intrupt from GPS UART
 *  in
 *  out
 ******************************************************************/
void signal_gps_handler_IO (int status)
{
    //printf("received data from UART.\n");
    wait_gps_flag = FALSE;
}

/******************************************************************
 *  Function name : InitGPIOuart
 *  Description: init the GPS uart
 *  in
 *  out
 ******************************************************************/
int InitGPIOuart( void )
{
	int fd ;
	GPSfd = open(GPSPORT, O_RDWR | O_NOCTTY );//| O_NDELAY);
	if (GPSfd == -1)
	{
		perror("open_port: Unable to open /dev/ttymxc2\n");
		//exit(1);
		//send_error("K10 init error");
	}
	saio.sa_handler = signal_gps_handler_IO;
	sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
	saio.sa_flags = 0;
	saio.sa_restorer = NULL;
	//sigfillset(&saio.sa_mask);
	sigaction(SIGIO,&saio,NULL);
	//fcntl(fd, F_SETFL, FNDELAY);
	fcntl(fd, F_SETOWN, getpid());
	fcntl(fd, F_SETFL,  O_ASYNC ); /**<<<<<<------This line made it work.**/
	//fcntl(fd, F_SETFL, FNDELAY|FASYNC);
	//fcntl(fd, F_SETFL, FASYNC);
	tcgetattr(fd,&termAttr);
	cfsetispeed(&termAttr,B4800);
	cfsetospeed(&termAttr,B4800);
	termAttr.c_cflag &= ~PARENB;
	termAttr.c_cflag &= ~CSTOPB;
	termAttr.c_cflag &= ~CSIZE;
	termAttr.c_cflag |= CS8 ;//| CRTSCTS;
	termAttr.c_cflag |= (CLOCAL | CREAD);
	termAttr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	termAttr.c_iflag &= ~(IXON | IXOFF | IXANY);
	termAttr.c_oflag &= ~OPOST;
	tcsetattr(fd,TCSANOW,&termAttr);
	printf("GPS configured....\n");
	connected = 1;
	return (GPSfd);
}



/******************************************************************
 *  Function name: prefix
 *  Description: get the prefix from the data string
 *  in
 *  out
 ******************************************************************/
int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

/******************************************************************
 *  Function name: GPS_Recive_message_function
 *  Description
 *  in
 *  out
 ******************************************************************/
void *GPS_Recive_message_function( void *ptr )
{
	unsigned char buff1[UDB_BUFFER];
	unsigned char buff2[UDB_BUFFER];
	unsigned char *buffp = buff1;//debug
	int n,acc;
	int data_len;
	int get_error=0;
	int var,nn,totalzise=0,datared=0;
	char* lat = (char*)malloc(10);
	char* lat_s = (char*)malloc(10);
	char* lng = (char*)malloc(10);
	char* lng_s = (char*)malloc(10);
	char* date = (char*)malloc(10);
	char* st_num = (char*)malloc(10);
	GPSfd = InitGPIOuart();
	if (GPSfd < 0)
	{
		printf("port not open \n");
		return 0;
	}
	printf("port is open \n");
	while(connected == 1)
    {

		usleep(200);//2000);
		if (wait_gps_flag == FALSE)  //if input is available
		{
			printf("Recive_message_function \n");
			/* Read character from ABU */
			usleep(140000*1);
			//DelayForLongData = 1;
			memset( buff1,0, sizeof(buff1));
			memset( buff2,0, sizeof(buff1));//debug
			totalzise = 0;
			ioctl(GPSfd, FIONREAD, &datared);
			if(!(datared > 0))
				continue;
			while(1)
			{
			n = read(GPSfd , buffp + acc, 1);
				acc ++;
			if(buff1[acc] == '\r')
					break;
			}

			n = read(GPSfd , buff1, datared);
			if (n > 0)
				printf("get msg*********************  %d  %s\n",n,buff1);


	   }
    }
	return 0;
}
