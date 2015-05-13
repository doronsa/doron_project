/*
 * dcu_util.c
 *
 *  Created on: Mar 10, 2014
 *      Author: doronsa
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
#include "dcu_util.h"
#define DEBUG
#define LINUX
#ifdef LINUX
	#include <sys/signal.h>//for linux
#else
	#include <signal.h>//android
	#ifndef O_NONBLOCK
	# define O_NONBLOCK	  04000
	#endif
	#ifndef O_NDELAY
	# define O_NDELAY	O_NONBLOCK
	#endif
    #define FNDELAY	O_NDELAY
#endif
#include <errno.h>
#include <termios.h>
//protocol
#include "P_TwMtrBase.h"
#include "TW_K10232_p.h"
/* device name define*/
#define DCU_DRIVER_KEY "/dev/ttyACM7"
pthread_mutex_t lock;
/* global definition*/
#define FALSE 0
#define TRUE 1
#define OK 0

#define NPACK 10
#define MAX_BUFF_LEN 10
#define MARKER1 55
#define MARKER2 0xa5
#define MARKER_VAL 0xa5
#define TR_PACK_PREFIX_PAC __attribute__ ((packed))
#define BUFLEN 100
#define UDB_BUFFER 600
/* Function export */
int wait_dcu_flag=TRUE;
int AddDCUQue( St_ProtocolHeader *sSDUProtocolHeader, void *data );

extern int AddK10Que( St_ProtocolHeader *sProtocolHeader, void *data );
extern int InitK10uart( void );
void dcu_signal_handler_IO (int status);   /* definition of signal handler */
extern DataCommand SandDtataToSDU[10];

int dcu_connected = 0;
char send_buffer[BUFLEN];
static int old_port_number = 0;
int stopdcurec = 0;
/* Function */
//void dcu_signal_handler_IO (int status)
//{
//	//if (status == SIGTTIN)
//	{
//		printf("received signal from DCU UART. %d\n",status);
//		wait_dcu_flag = FALSE;
//	}
//}

char DCU_Port[20];
struct termios dcutermAttr;
struct sigaction dcusaio;

int DCU_SerialPortSearch(void)
{
	return 7;
}


int DCUData_Decode( int cmd, int TypeRespond, void *data,int AplicationStatus, int data_len)
{
	St_ProtocolHeader *sProtocolHeader = (St_ProtocolHeader*)malloc(sizeof(St_ProtocolHeader));
	sProtocolHeader->Address = 0;
	sProtocolHeader->Marker1 = 0xa5;
	sProtocolHeader->Marker2 = 0xa5;
	sProtocolHeader->Cmd = cmd;
	sProtocolHeader->StatusBit =1;
	sProtocolHeader->AplicationStatus = AplicationStatus;
	sProtocolHeader->RequestType = TypeRespond;

	if ( data_len > 0)
	{
		sProtocolHeader->DataSize = data_len;
		AddDCUQue( sProtocolHeader, data);

	}
	else
	{
		sProtocolHeader->DataSize = 0;
		AddDCUQue( sProtocolHeader, NULL);
	}
return 1;
}

int DCU_SerialPortInit(void)
{
	int fd,port_num = 0;
	port_num = DCU_SerialPortSearch();//find the last number for DCU port after modem port
	if (old_port_number != port_num)
	{
		sprintf(DCU_Port,"/dev/ttyACM%d",port_num);
		old_port_number = port_num;
	}
	else
	{
		sprintf(DCU_Port,"/dev/ttyACM%d",old_port_number);
		old_port_number = port_num;
	}

	fd = open(DCU_Port, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		printf("open_port: Unable to open %s\n",DCU_Port);
		//exit(1);
		//send_error("UDP server");
	}
//	dcusaio.sa_handler = dcu_signal_handler_IO;
//	sigemptyset(&dcusaio.sa_mask);   //dcusaio.sa_mask = 0;
//	dcusaio.sa_flags = 0;
//	dcusaio.sa_restorer = NULL;
//	sigaction(SIGIO,&dcusaio,NULL);
//	fcntl(fd, F_SETFL, FNDELAY);
	fcntl(fd, F_SETOWN, getpid());
	//fcntl(fd, F_SETFL,  O_ASYNC ); /**<<<<<<------This line made it work.**/
	fcntl(fd, F_SETFL, FNDELAY);
	tcgetattr(fd,&dcutermAttr);
	cfsetispeed(&dcutermAttr,B115200);
	cfsetospeed(&dcutermAttr,B115200);
	dcutermAttr.c_cflag &= ~PARENB;
	dcutermAttr.c_cflag &= ~CSTOPB;
	dcutermAttr.c_cflag &= ~CSIZE;
	dcutermAttr.c_cflag |= CS8;
	dcutermAttr.c_cflag |= (CLOCAL | CREAD);
	dcutermAttr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	dcutermAttr.c_iflag &= ~(IXON | IXOFF | IXANY);
	dcutermAttr.c_oflag &= ~OPOST;
	tcsetattr(fd,TCSANOW,&dcutermAttr);
	printf("DCU UART configured....\n");
	dcu_connected = 1;//Enable DCURecive message function
	stopdcurec = 1;
	return (fd);
}

int AddDCUQue( St_ProtocolHeader *sSDUProtocolHeader, void *data)
{
	int var=0,i=0;

	for ( var = 0; var < MAX_BUFF_LEN; ++var)
	{
		if (SandDtataToSDU[var].sendOk == 0)
		{
			//sProtocolHeader = (ProtocolHeader*)data;
			//chcink if the command is ok
			if( sSDUProtocolHeader->Cmd >= (unsigned char)e_CmdK10_CheckComm )
			{
				pthread_mutex_lock(&lock);
				SandDtataToSDU[var].ApplicationResult = sSDUProtocolHeader->StatusBit;
				SandDtataToSDU[var].ApplicationStatusBits = sSDUProtocolHeader->AplicationStatus;
				SandDtataToSDU[var].datalen = sSDUProtocolHeader->DataSize;
				SandDtataToSDU[var].e_IsRequesType = sSDUProtocolHeader->RequestType;
				SandDtataToSDU[var].i_cmd = sSDUProtocolHeader->Cmd;
				//if the data is on the end off the header
				if ( sSDUProtocolHeader->DataSize > 0)
				{
					memcpy( SandDtataToSDU[var].data,(unsigned char*)data , sSDUProtocolHeader->DataSize);
#ifdef DEBUG
					printf("Add DCU Que: ");
					for (i = 0; i < sSDUProtocolHeader->DataSize; i++)
					{
						printf(" %hhX ", SandDtataToSDU[var].data[i]);
					}
					printf(" end get %d\n ",i);
#endif
				}
				SandDtataToSDU[var].sendOk = 1;
				pthread_mutex_unlock(&lock);
				return (1);
			}

		}
		else
		{
			if (var == 9)
			{
				return (-1);
			}
		}
	}
	return 1;
}

static char state=0;
int OnDcuByteReceive( unsigned char uc_Character )
{
	switch(state)
	{
	case e_StateIdle:// start message
		// it is not marker character
		if(MARKER_VAL !=uc_Character)
		{
			return 0;
		}
		else
		{
			state = e_StateWaitMarker2;
			return 1;
		}
		break;

	case e_StateWaitMarker2:// the second marker
		// it is not marker character -> the message is not realy message
		if(MARKER_VAL !=uc_Character)
		{
			return 0;
		}
		else
		{
			state = e_StateReciveHeader;
			return 2;
		}
		break;
	default:
		return 0;
		break;
	}
	return 0;
}

int DcuByteReceive( unsigned char *p_Characters, int size )
{
	int i;
	int ans=0;
	//static char state=0;
	state=0;
	unsigned char*temp = p_Characters;
	// send all bytes to protocol handler
	for(i=0;i< size;i++)
	{
		ans = OnDcuByteReceive( temp[i] );
		if ( ans == 2)
		{
			i-=1;
			return i;
		}
		else
		{
			if (i>=size)
			{
				return -1;
			}
		}

	}
	return (0);
}

int EncodeDCUdata(  void *data , int data_len )
{
	int hsize= sizeof(St_ProtocolHeader);//14
	int index = 0;
	int var=0;
	int temp_data_len;
	unsigned char *pdata = data;
	unsigned char *ptempdata = (unsigned char *)malloc(data_len);
	//unsigned short len;
	void *senddata;
	memcpy( ptempdata, (unsigned char *)data, data_len );
	//St_UserData  *p = (St_UserData*)malloc(sizeof(St_UserData));
	if ( data_len >= hsize)
	{
		index = DcuByteReceive( pdata, data_len);
		if ( index == -1)
		{
			return -1;
		}

		pdata +=index;//move the data to the reel point
		St_ProtocolHeader *sProtocolHeader = (St_ProtocolHeader*)pdata;//(ProtocolHeader*)malloc(sizeof(ProtocolHeader));
		//free(pdata);
#ifdef DEBUG

		printf("**** data index :%d\n",index);
		printf("**** data cmd :%hhX\n",sProtocolHeader->Cmd);
		printf("**** data DataSize :%d\n",sProtocolHeader->DataSize);
		printf("**** data_len :%d\n",data_len);
#endif
		if ((sProtocolHeader->Marker1 == 0xa5)&&(sProtocolHeader->Marker2 == 0xa5))
		{
			{
				if ( sProtocolHeader->RequestType == e_ReqTypeRequest ||
					 sProtocolHeader->RequestType == e_ReqTypeRespond )
				{
					switch (sProtocolHeader->Cmd)
					{
						case 100:
							if ( (sProtocolHeader->DataSize + hsize) <= data_len )
							{
//								if ((data_len - 14) == sProtocolHeader->DataSize)
//									temp_data_len = sProtocolHeader->DataSize;
//								else
//									temp_data_len = data_len - 14;
								temp_data_len = sProtocolHeader->DataSize;
								sProtocolHeader->RequestType = e_ReqTypeRequest;
								sProtocolHeader->AplicationStatus = 1;
								senddata = (void *)malloc(temp_data_len);
								ptempdata += (hsize + index);//move to data point
								memcpy( senddata, ptempdata, temp_data_len);
								//AddDCUQue( sProtocolHeader, data);
								//int DCUData_Decode( int cmd, int TypeRespond, void *data,int AplicationStatus, int data_len)
								AddK10Que( sProtocolHeader, senddata);
								//for debug stop the DCU uart
								CloseDCUUart();
								InitK10uart();
								//CloseDCUUart();
							}
							break;
						default:
							break;
					}

				}

			}
		}
	}
	return 1;
}

void *DCURecive_message_function( void *ptr )
{
	unsigned char buff1[UDB_BUFFER];
	unsigned char buff2[UDB_BUFFER];//debug
	int get_error=0;
	int var;
	while( 1 )
	{
		while(dcu_connected == 1)
		{

			usleep(20000);//2000);
			if (wait_dcu_flag == FALSE)  //if input is available
			{

				// printf("DCU Recive_message_function \n");
				/* Read character from ABU */
				//usleep(200000);//50000);11
				memset( buff1,0, sizeof(buff1));
				memset( buff2,0, sizeof(buff1));//debug
				int n = read(DeviceFd.DCUKeyDrv, buff1, sizeof(buff1));
				memcpy(buff2,buff1,sizeof(buff1));
				if (n > 0)
				{
					//print_time();
	#ifdef DEBUG
					printf("DCU data receive n:%d\n",n);
					for (var = 0; var < n; ++var)
					{
						printf("%hhX ", buff2[var]);
					}
					printf("\n ");
	#endif
					//get_error = EncodeDCUdata( buff1, n);
					if (get_error==-1)
					{
						//log and send to gui com problem
						printf("Receive parsing error message \n");
					}
				}
				wait_dcu_flag = TRUE;

			}

		}
	}
	return NULL;
}

void CloseDCUUart(void)
{
	close(DeviceFd.DCUKeyDrv);
	stopdcurec = 0;
	dcu_connected = 0;
	wait_dcu_flag = TRUE;
	DeviceFd.K10Fd = InitK10uart();
	usleep(10000);
}
