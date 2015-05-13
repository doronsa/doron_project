/*
 ============================================================================
 Name        : GPSDriver.c

 Author      : doron Sandroy
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <android/log.h>
#include "GPSDriver.h"

#define  LOG_TAG    "GPS"

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#define BAUDRATE B4800
#define MODEMDEVICE "/dev/ttymxc1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
int globalstatus = 0;
int fd;
volatile int STOP=FALSE;
int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

int OpenGPSport(void)
{
	int c, res;
	struct termios oldtio,newtio;
	char buf[1024];
	//LOGE("GetGPSLocation");
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
	if (fd <0) {perror(MODEMDEVICE); exit(-1); }
	//LOGE("Open GPS port fd:%d",fd);
	tcgetattr(fd,&oldtio); /* save current port settings */

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
	return fd;

}
void GetGPSLocation( GPSdata *data)
{

	int c, res;
	char buf[3030];
	int acc = 0, number_off_count=0;
	char* token;
	//if(globalstatus==0)
	//{
		fd = OpenGPSport();
	//	globalstatus=1;
	//}
	//else LOGE("port is open fd:%d",fd);

	//struct termios oldtio,newtio;

	memset(buf,0,3024);
    STOP = FALSE;
    number_off_count = 0;

	while (STOP==FALSE)
	{       /* loop for input */
	  res = read(fd,buf+acc,255);   /* returns after 5 chars have been input */
	  //usleep(10000);
      if ((res > 0)&&(acc < 3025))
      {
		  if(buf[acc] == '\n')
		  {
			  {
				  if(strstr(buf , "$GPGGA" ))
				  {
					//LOGE("\n\n****** master get n:%d char %s ******\n",acc, buf);
					token = strtok(buf, ",");
					acc = 0;
					while (acc < 7)
					{
						token = strtok(0, ",");

						if(acc == 0)
						{
							strcpy(data->date,token);
						}
						if(acc == 1)
						{
							strcpy(data->lat,token);
						}
						if(acc == 2)
						{
							strcpy(data->lat_s,token);
						}
						if(acc == 3)
						{
							strcpy(data->lng,token);
						}
						if(acc == 4)
						{
							strcpy(data->lng_s,token);
						}
						if(acc == 6)
						{
							strcpy(data->st_num,token);
						}

						acc++;

					}
					number_off_count++;
					//LOGE("date: %s lat :%s lat_s:%s lng:%s lng_s:%s num off st:%s\n",data->date, data->lat, data->lat_s, data->lng, data->lng_s, data->st_num);
				  }
			  }
			  acc = 0;
		  }//buf
		  acc += res;
		  if (acc > 3024)
		  {
			  acc = 0;
			  //break;
		  }

		  if (number_off_count==1)
		  {
			 STOP=TRUE;
		  }
      }

	}

	//tcsetattr(fd,TCSANOW,&oldtio);
	//LOGE("close GPS port");
	tcflush(fd, TCIFLUSH);
	close(fd);
}
