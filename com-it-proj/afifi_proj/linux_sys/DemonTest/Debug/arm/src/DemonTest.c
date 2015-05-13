 /* Demon program
  * get massage from the K10 parsing and send to android GUI throw UDP
  * get massage from GUI throw UDP and parsing and sending to the K10
  * date 17-01-14
  * programer:doron sandroy
  * version 1.00
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
//#include <linux/i2c-dev.h>
#include "KeyBoard.h"
#include "gpio_util.h"

void print_time ()
{
 struct timeval tv;
 struct tm* ptm;
 char time_string[40];
 long milliseconds;

 /* Obtain the time of day, and convert it to a tm struct. */
 gettimeofday (&tv, NULL);
 ptm = localtime (&tv.tv_sec);
 /* Format the date and time, down to a single second. */
 strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
 /* Compute milliseconds from microseconds. */
 milliseconds = tv.tv_usec / 1000;
 /* Print the formatted time, in seconds, followed by a decimal point
   and the milliseconds. */
 printf ("%s.%03ld\n", time_string, milliseconds);
}


#define DEBUG
//#define LINUX
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
//#define K10PORT "/dev/ttyUSB2"// "/dev/ttyUSB0"
//for test
#define K10PORT "/dev/ttyUSB0"
#define MODEM "/dev/ttyACM0"
#define PRINTER "/dev/usb/lp0"
#define KEYBOARD "/dev/i2c-0"
/* global definition*/
#define FALSE 0
#define TRUE 1
#define OK 0
#define SERVPORT 4445
#define CLIENTPORT 4444
#define BUFLEN 100
#define SRV_IP "127.0.0.1"
#define NPACK 10
#define MAX_BUFF_LEN 10
#define MARKER1 55
#define MARKER2 0xa5
#define MARKER_VAL 0xa5
#define TR_PACK_PREFIX_PAC __attribute__ ((packed))
#define UDB_BUFFER 600
/* Function export */
int wait_flag=TRUE;
void signal_handler_IO (int status);   /* definition of signal handler */

/* Structure definition */
/*htons()
host to network short
htonl()
host to network long
ntohs()
network to host short
ntohl()
network to host long
 * */
struct termios termAttr;
struct sigaction saio;
//extern struct St_UserData;
typedef struct St_ProtocolHeader
{
	unsigned char Marker1;// a5
	unsigned char Marker2;// a5
	unsigned char Address;// must be 0
	unsigned char Cmd;// the application command
	unsigned char RequestType;//e_RequestType  request / respond/event
	unsigned short StatusBit;// 16 bits flags free for aplication using
	unsigned char ProtocolStatus;//e_ProtocoleError protocol error handling field   (relavnt in respond state)
	unsigned char AplicationStatus; // application error handler field
	unsigned short RequestCount;// Request counter for link bitween respond & request
	unsigned short DataSize;// the data size of fields
	unsigned char LrcCheckSum;// xoring of all Checksum= XOR(Header+Data)XOR 0xaa  the LrcCheckSum field at calculate must be ZERO
}((packed));//St_ProtocolHeader;// protocol header

typedef struct St_ProtocolHeader ProtocolHeader ;

typedef struct
{
/* GUI command send/receive */
	unsigned char Marker1;
	unsigned char Marker2;
	unsigned char Cmd;
	unsigned char DataLen;
	unsigned char TimeOut;
	unsigned char Data[UDB_BUFFER];
}UDPcommand ;

typedef struct
{
	int SendOK;
	int time;
	int keynum;
}SandKeyToKGUI;

DataCommand SandDtataToK10[10];
DataCommand SandDataToKGUI[10];
SandKeyToKGUI KeyToKGUI[50];

//sDeviceFd DeviceFd;//global File descriptor
pthread_t pK10threadRecive, pUDPcomThread, pKeyBoardTrhad;
pthread_mutex_t lock;
void *print_message_function( void *ptr );
int AddK10Que( St_ProtocolHeader *sProtocolHeader, void *data );
int AddGUIQue( St_ProtocolHeader *sProtocolHeader, void *data , int data_len);
int AddKeyBoarQue( SandKeyToKGUI KeyToKGUI);
/*  init variable*/
int connected;
char send_buffer[BUFLEN];
/* Function */
void signal_handler_IO (int status)
{
    //printf("received data from UART.\n");
    wait_flag = FALSE;
}


void CloseK10Uart(void)
{
	close(DeviceFd.K10Fd);
}
/* BIT
 * Testing all devises if connected
 * the return code :
 * 1 = modem problem
 * 2 = k10 problem
 * 4 = printer problem
 * 8 = keyboard problem
 */
int TestModules( sDeviceFd Fd)
{
	int ModuleError = 0;
	int ret = 0;
	struct stat info;
	ret = stat( MODEM, &info);
	if( ret == OK )
	{
		ModuleError = ret;
	}
	else
	{
		ModuleError+=1;
	}
	ret = stat( K10PORT, &info);
	if( ret < 0 )
	{
		ModuleError|=2;
	}
	else
	{
		if( ret != 0 )
		{
			ModuleError++;
		}
	}
	ret = stat( PRINTER, &info);//nedd to bey lp0 on target
	if( ret < 0 )
	{
		ModuleError|=4;
	}
	else
	{
		if( ret != 0 )
		{
			ModuleError++;
		}
	}
	ret = stat( KEYBOARD, &info);// i2c-1, i2c-2
	if( ret < 0 )
	{
		ModuleError|=8;
	}
	else
	{
		if( ret != 0 )
		{
			ModuleError++;
		}
	}
	return (ModuleError);
}



int InitK10uart( void )
{
	int fd ;
	fd = open(K10PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		perror("open_port: Unable to open /dev/ttyUSB0\n");
		exit(1);
	}
	saio.sa_handler = signal_handler_IO;
	saio.sa_flags = 0;
	saio.sa_restorer = NULL;
	sigaction(SIGIO,&saio,NULL);

	fcntl(fd, F_SETFL, FNDELAY);
	fcntl(fd, F_SETOWN, getpid());
	fcntl(fd, F_SETFL,  O_ASYNC ); /**<<<<<<------This line made it work.**/

	tcgetattr(fd,&termAttr);
	//baudRate = B115200;          /* Not needed */
	cfsetispeed(&termAttr,B115200);
	cfsetospeed(&termAttr,B115200);
	termAttr.c_cflag &= ~PARENB;
	termAttr.c_cflag &= ~CSTOPB;
	termAttr.c_cflag &= ~CSIZE;
	termAttr.c_cflag |= CS8;
	termAttr.c_cflag |= (CLOCAL | CREAD);
	termAttr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	termAttr.c_iflag &= ~(IXON | IXOFF | IXANY);
	termAttr.c_oflag &= ~OPOST;
	tcsetattr(fd,TCSANOW,&termAttr);
	printf("UART1 configured....\n");
	return (fd);
}


void error(char *msg)
{
    perror(msg);
    exit(1);
}
void LaodK10File(int cmd, void *data, int len)
{
	e_ComResult ComResult=0;
	unsigned char e_IsRequesType=0;
	unsigned short ApplicationResult=0;
	unsigned short ApplicationStatusBits=0;
	int end=1;
	char tcmd=0;
	//char data[500];
	/* need to upload new version to k10
	 * this is state machine
	 * 1. Sending to k10 go to download :e_CmdK10_Jump2Loader
	 * 2. If ok start Sending the bin file :e_CmdK10_DownLoad
	 * 3. Sending :e_CmdK10_Jump2App
	 * 4. Sending :e_CmdK10_ResetApp
	 * if need to load the bin file and the data is the name of the file
	 * */
	if ( cmd == e_CmdK10_DownLoad)
	{
		pthread_mutex_lock(&lock);
		//First step;
		cmd = 0;
		while(end)
		{
			switch (cmd)
			{
				case 0:
					tcmd = e_CmdK10_Jump2Loader;
					cmd = 1;
					break;
				case 1:
					tcmd = e_CmdK10_DownLoad;
					cmd = 2;
					break;
				case 2:
					tcmd = e_CmdK10_Jump2App;
					cmd = 3;
					break;
				case 3:
					tcmd = e_CmdK10_ResetApp;
					cmd = 4;
					end = 0;
					break;
				default:
					break;
			}
			ComResult = e_SendMessage( DeviceFd.K10Fd, tcmd, e_IsRequesType, ApplicationResult,
												 ApplicationStatusBits, len, data,
												 (len * 10));
			if (ComResult != e_ComOk)
			{
				printf(" erroro sendig to K10\n");
			}
		}
		pthread_mutex_unlock(&lock);
	}
}

void keyEventQue( ProtocolHeader *sProtocolHeader, void *data )
{

}
static char state=0;
int OnByteReceive( unsigned char uc_Character )
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

}

int ByteReceive( unsigned char *p_Characters, int size )
{
	int i;
	int ans=0;
	//static char state=0;
	state=0;
	unsigned char*temp = p_Characters;
	// send all bytes to protocol handler
	for(i=0;i< size;i++)
	{
		ans = OnByteReceive( temp[i] );
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

int Data_encode( void *data , int data_len)
{
	int hsize= sizeof(St_ProtocolHeader);//14
	int index = 0;
	int var=0;
	unsigned char *pdata = data;
	unsigned char *ptempdata = (unsigned char *)malloc(data_len);
	//unsigned short len;
	void *senddata;
	memcpy( ptempdata, (unsigned char *)data, data_len );
	//St_UserData  *p = (St_UserData*)malloc(sizeof(St_UserData));
	if ( data_len >= hsize)
	{
		index = ByteReceive( pdata, data_len);
		if ( index == -1)
		{
			return -1;
		}

		pdata +=index;//move the data to the reel point
		St_ProtocolHeader *sProtocolHeader = (St_ProtocolHeader*)pdata;//(ProtocolHeader*)malloc(sizeof(ProtocolHeader));
		free(pdata);
#ifdef DEBUG
		printf("data index :%d\n",index);
		printf("data cmd :%hhX\n",sProtocolHeader->Cmd);
		printf("data DataSize :%d\n",sProtocolHeader->DataSize);
#endif
		if ((sProtocolHeader->Marker1 == 0xa5)&&(sProtocolHeader->Marker2 == 0xa5))
		{
			if ( sProtocolHeader->Address == 0)
			{
				if ( sProtocolHeader->RequestType == e_ReqTypeRequest ||
					 sProtocolHeader->RequestType == e_ReqTypeRespond )
				{
					switch (sProtocolHeader->Cmd)
					{
						case e_CmdK10_CheckComm:
						case e_CmdK10_SelfTest:
						case e_CmdK10_SetParam:
						case e_CmdK10_DisplayCommandGet:
						case e_CmdK10_DisplayCommandSet:
						case e_CmdK10_KeyboardCommandConfig:
						case e_CmdK10_KeyboardCommand:
						case e_CmdK10_KeyboardGetKey:
						case e_CmdK10_PowerCommandGet:
						case e_CmdK10_PowerCommandSet:
						case e_CmdK10_SensorCommandGet:
						case e_CmdK10_SolenoidCommandSet:
						case e_CmdK10_PrinterCommandSet:
						case e_CmdK10_LedCommandSet:
						case e_CmdK10_PeriodicMonitorPoll:
						case e_CmdK10_GetAppVersion:
						case e_CmdK10_DownLoad:
						case e_CmdK10_Jump2Loader:
						case e_CmdK10_Jump2App:
						case e_CmdK10_Set2LowPower:
						case e_CmdK10_ResetApp:
						case e_CmdK10_GetRTCTime:
						case e_CmdK10_SetRTCTime:
						case e_CmdK10_EventSensorChange:
						case e_CmdK10_KeyBoardEvent:
							if ( (sProtocolHeader->DataSize + hsize) <= data_len )
							{
								senddata = (void *)malloc(sProtocolHeader->DataSize);
								ptempdata += (hsize + index);//move to data point
								memcpy( senddata, ptempdata, sProtocolHeader->DataSize);
								AddK10Que( sProtocolHeader, senddata);
							}
							break;
						default:
							break;
					}

				}
				else
				{
					if ( sProtocolHeader->RequestType == e_ReqTypeEvent )
					{
						if ( sProtocolHeader->Cmd == e_CmdK10_PeriodicMonitorEvent)
						{
							senddata = (void *)malloc(sProtocolHeader->DataSize);
							ptempdata += (hsize + index);//move to data point
							memcpy( senddata, ptempdata, sProtocolHeader->DataSize);
#ifdef DEBUG
							printf("Data_encode: ");
							for (var = 0; var < sProtocolHeader->DataSize; var++)
							{
								printf(" %hhX ", ptempdata[var]);
							}
							printf("\n ");
#endif
							AddK10Que( sProtocolHeader, senddata);
						}
						else
						{
							if ( sProtocolHeader->Cmd == e_CmdK10_KeyBoardEvent)
							{
								keyEventQue((ProtocolHeader*) sProtocolHeader, (unsigned char*)data);
							}
						}
					}
				}
			}
		}
	}
//	if(senddata)
//		free(senddata);
//	if (ptempdata)
//		free(ptempdata);
	return 1;
}

int AddKeyBoarQue( SandKeyToKGUI Key )
{
	int var=0;
	for ( var = 0; var < MAX_BUFF_LEN; ++var)
	{
		if (KeyToKGUI[var].SendOK == 0)
		{
			pthread_mutex_lock(&lock);
			KeyToKGUI[var].keynum = Key.keynum;
			KeyToKGUI[var].time = Key.time;
			KeyToKGUI[var].SendOK = 1;
			pthread_mutex_unlock(&lock);
		}
	}
}

int Datda_Decode( int cmd, int TypeRespond, void *data, int data_len)
{
	St_ProtocolHeader *sProtocolHeader = (St_ProtocolHeader*)malloc(sizeof(St_ProtocolHeader));
	sProtocolHeader->Address = 0;
	sProtocolHeader->Marker1 = 0xa5;
	sProtocolHeader->Marker2 = 0xa5;
	sProtocolHeader->Cmd = cmd;
	sProtocolHeader->StatusBit =1;
	sProtocolHeader->AplicationStatus = 1;
	sProtocolHeader->RequestType = TypeRespond;
	if ( data_len > 0)
	{
		sProtocolHeader->DataSize = data_len;
		AddGUIQue( sProtocolHeader, data, data_len);

	}
	else
	{
		sProtocolHeader->DataSize = 0;
		AddGUIQue( sProtocolHeader, NULL, data_len);
	}
return 1;
}



int AddK10Que( St_ProtocolHeader *sProtocolHeader, void *data )
{
	int var=0,i=0;

	for ( var = 0; var < MAX_BUFF_LEN; ++var)
	{
		if (SandDataToKGUI[var].sendOk == 0)
		{
			//sProtocolHeader = (ProtocolHeader*)data;
			//chcink if the command is ok
			if( sProtocolHeader->Cmd >= (unsigned char)e_CmdK10_CheckComm )
			{
				pthread_mutex_lock(&lock);
				SandDataToKGUI[var].ApplicationResult = sProtocolHeader->StatusBit;
				SandDataToKGUI[var].ApplicationStatusBits = sProtocolHeader->AplicationStatus;
				SandDataToKGUI[var].datalen = sProtocolHeader->DataSize;
				SandDataToKGUI[var].e_IsRequesType = sProtocolHeader->RequestType;
				SandDataToKGUI[var].i_cmd = sProtocolHeader->Cmd;
				//if the data is on the end off the header
				if ( sProtocolHeader->DataSize > 0)
				{
					memcpy( SandDataToKGUI[var].data,(unsigned char*)data , sProtocolHeader->DataSize);
#ifdef DEBUG
					printf("AddK10Que: ");
					for (i = 0; i < sProtocolHeader->DataSize; i++)
					{
						printf(" %hhX ", SandDataToKGUI[var].data[i]);
					}
					printf(" end get %d\n ",i);
#endif
				}
				SandDataToKGUI[var].sendOk = 1;
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


//int AddGUIDtataCommand( char *data , int data_len)
int AddGUIQue(St_ProtocolHeader *sProtocolHeader, void *data , int data_len)
{
	int var=0;
	//ProtocolHeader *sProtocolHeader;
	for ( var = 0; var < MAX_BUFF_LEN; ++var)
	{
		if (SandDtataToK10[var].sendOk == 0)
		{
			//sProtocolHeader = (ProtocolHeader*)data;

			//chcink if the command is ok
			if(sProtocolHeader->Cmd >= (unsigned char)e_CmdK10_CheckComm)
			{
				pthread_mutex_lock(&lock);
				SandDtataToK10[var].ApplicationResult = 0;//sProtocolHeader->AplicationStatus;
				SandDtataToK10[var].ApplicationStatusBits = sProtocolHeader->AplicationStatus;
				SandDtataToK10[var].datalen = data_len;//sProtocolHeader->DataSize;
				SandDtataToK10[var].e_IsRequesType = sProtocolHeader->RequestType;
				SandDtataToK10[var].i_cmd = (unsigned char)sProtocolHeader->Cmd;
				SandDtataToK10[var].StatusBit = sProtocolHeader->StatusBit;
				//if the data is on the end off the header
				if ( SandDtataToK10[var].datalen > 0)
				{
					memcpy( SandDtataToK10[var].data, (unsigned char*)data, sProtocolHeader->DataSize);
				}
				SandDtataToK10[var].sendOk = 1;
				pthread_mutex_unlock(&lock);
			}
			return (1);
		}
		else
		{
			if (var == 9)
			{
				return (-1);
			}
		}
	}
	return (0);
}

void *Recive_message_function( void *ptr )
{
	unsigned char buff1[UDB_BUFFER];
	unsigned char buff2[UDB_BUFFER];//debug
	int get_error=0;
	int var;
	while(connected == 1)
    {

		usleep(200);//2000);
		if (wait_flag == FALSE)  //if input is available
		{

			 printf("Recive_message_function \n");
			/* Read character from ABU */
			usleep(200000);//50000);
			memset( buff1,0, sizeof(buff1));
			memset( buff2,0, sizeof(buff1));//debug
			int n = read(DeviceFd.K10Fd, buff1, sizeof(buff1));
			memcpy(buff2,buff1,sizeof(buff1));
			if (n > 0)
			{
				print_time();
#ifdef DEBUG
				printf("data receive n:%d\n",n);
				for (var = 0; var < n; ++var)
				{
					printf("%hhX ", buff2[var]);
				}
				printf("\n ");
#endif
				get_error = Data_encode( buff1, n);
				if (get_error==-1)
				{
					//log and send to gui com problem
					printf("Receive parsing error message \n");
				}
			}
			wait_flag = TRUE;

		}

	}
	return NULL;
}


//int Sendudp(char * dataGUI)//for gui data
int Sendudp(DataCommand SandDataToKGUI)
{
	char dataGUI[UDB_BUFFER];
	struct sockaddr_in si_other;
	int sockfd, i, slen=sizeof(si_other);
	//char buf[BUFLEN];
	printf("Start UDP send command to GUI \n");
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
		perror("UDP Sendudp - socket() error");
	}
	  //diep("socket");

	memset((char *) &si_other, sizeof(si_other), 0);
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(CLIENTPORT);
	if (inet_aton(SRV_IP, &si_other.sin_addr)==0)
	{
	  fprintf(stderr, "inet_aton() failed\n");
	  exit(1);
	}
    //build command for send to GUI
	memset(dataGUI,0,UDB_BUFFER);
	dataGUI[0]=MARKER1;
	dataGUI[1]=MARKER1;
	dataGUI[2]=SandDataToKGUI.i_cmd;
	dataGUI[3]=SandDataToKGUI.datalen;
	memcpy( &dataGUI[4], SandDataToKGUI.data, SandDataToKGUI.datalen);
	if (sendto(sockfd , dataGUI, UDB_BUFFER, 0,(struct sockaddr *)&si_other, slen)< 0)
	{
		printf("UDP error sending command to GUI %s\n",dataGUI);
	}
	else
	{
#ifdef DEBUG
		printf("UDP sending command to GUI \n");
		printf("  cmd %hhx ",dataGUI[2]);
		printf("  len %hhx ",dataGUI[3]);
		 for (i=0;i<SandDataToKGUI.datalen;i++)
		 {
			 printf("%hhx ",dataGUI[i]);
		 }
		printf("send %hu \n",i);
#endif
	}

	close(sockfd);
	return 0;

}

int EncodeUDPData(unsigned char *buffer)
{
	 UDPcommand *data =( UDPcommand*)buffer;
	 unsigned char *tempbuf=(unsigned char*)malloc(UDB_BUFFER);
	 memcpy(tempbuf,buffer,UDB_BUFFER);
	 if (data->Marker1==MARKER1)
     {
		 if (data->Marker2 == MARKER1)
		 {
#ifdef DEBUG
			 printf("Get Command from GUI to K10 Command %d\n",tempbuf[3]);
#endif
			 switch ( data->Cmd )
			 {
				case e_CmdK10_DownLoad:
					LaodK10File( data->Cmd, data->Data, data->DataLen);
					break;
				case e_CmdKeyBoardSetLedOn:
				case e_CmdKeyBoardSetLedOff:
					SetLED( data->Data[0], data->Cmd);
					break;
				case e_CmdGPIOSetOn:
				case e_CmdGPIOSetOff:
					SetGpioValue(data->Data[0], data->Cmd);
					break;
				default:
					if( data->DataLen == 0)
					{
						Datda_Decode( data->Cmd, e_ReqTypeRequest, NULL, 0);
					 //AddGUIDtataCommand( send_buffer, BUFLEN );
					}
					else
					{
						Datda_Decode( data->Cmd, e_ReqTypeRequest, data->Data, data->DataLen);
					}
					break;
			}

		 }
     }
	 else
	 {
		 printf("Get no valid Command from GUI to K10 Command \n");
	 }
	 return (1);
}




void *udp_message_function( void *ptr )
{
	/* Variable and structure definitions. */
	int sd, rc;
	struct sockaddr_in serveraddr, clientaddr;
	int clientaddrlen = sizeof(clientaddr);
	int serveraddrlen = sizeof(serveraddr);
	char buffer[UDB_BUFFER];
	char *bufptr = buffer;
	int buflen = sizeof(buffer);
	/* get a socket descriptor */

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
	 perror("UDP server - socket() error");
	 //exit(-1);
	}
	else
	 printf("UDP server - socket() is OK\n");

	printf("UDP server - try to bind...\n");
	/* bind to address */
	memset(&serveraddr, 0x00, serveraddrlen);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVPORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if((rc = bind(sd, (struct sockaddr *)&serveraddr, serveraddrlen)) < 0)
	{
	 perror("UDP server - bind() error");
	 close(sd);
	 /* If something wrong with socket(), just exit lol */
	 //exit(-1);
	}
	else
	 printf("UDP server - bind() is OK\n");

	printf("UDP server - Listening...\n");

	/* Wait on client requests. */
	while (1)
	{

		 rc = recvfrom(sd, bufptr, buflen, 0, (struct sockaddr *)&clientaddr,(socklen_t*) &clientaddrlen);
		 if(rc < 0)
		 {
			 perror("UDP Server - recvfrom() error");
			 close(sd);
			 //exit(-1);
		 }
		 else
		 {
#ifdef DEBUG
			 printf("UDP Server - recvfrom() is OK...\n");
#endif
			 //print_time();
			 EncodeUDPData( (unsigned char*)bufptr );
		 }


	}

   close(sd);
   return (NULL);
}


void *print_message_function( void *ptr )
{
     char *message;
     message = (char *) ptr;
     printf("%s \n", message);
     return (NULL);
}


void *keyboar_message_function( void *ptr )
{
	int key = 0,oldkey = 0;
	int time = 0;
	SandKeyToKGUI sendkey;
	while (1)
	{
		key = GetKey();
		if ((key>=0 )&& (key<0xff))
		{
			printf("Key board get function key:%x\n",key);
		}
		if (oldkey != key)
		{
			printf("Key board get function key:%x\n",key);
			key = oldkey;
			sendkey.SendOK = 0;
			sendkey.keynum = key;
			sendkey.time = time;
			AddKeyBoarQue( sendkey );
		}
		usleep(2000);
	}
    return (NULL);
}

int StartK10ReciveTrhad( void )
{
	const char *message1 = "Thread StartK10ReciveTrhad";
	int  iret1;
	iret1 = pthread_create( &pK10threadRecive, NULL, Recive_message_function, (void*) message1);
	//pthread_join( pK10threadRecive, NULL);
	printf("Thread StartK10ReciveTrhad returns: %d\n",iret1);
	return 1;
}

int StartUDPTrhad( void )
{
	const char *message2 = "Thread StartUDPTrhad";
	int  iret1;
	iret1 = pthread_create( &pUDPcomThread, NULL, udp_message_function, (void*) message2);
	//pthread_join( pUDPcomThread, NULL);
	printf("Thread StartUDPTrhad returns: %d\n",iret1);
	return 1;
}

int StartKeyBoardTrhad ()
{
    const char *message2 = "Thread StartKeyBoardTrhad";
    int  iret1;
    iret1 = pthread_create( &pKeyBoardTrhad, NULL, keyboar_message_function, (void*) message2);
    //pthread_join( pUDPcomThread, NULL);
    printf("Thread StartUDPTrhad returns: %d\n",iret1);
    return 1;
}

int SendKeyBoardDataToUDP(int data)
{
	/*
	 * if get form i2c new data
	 * Store the data in buffer
	 * then send the data to UDP
	 * */
	return 1;
}

//int SendCommandToK10(char *data)
CoreReturnVal SendCommandToK10 ( int UARTId, void *Bytes, int Length, int Timeout)
{
	int var;
	unsigned char* data = Bytes;
	/*
	 * if get command from the UDP for the K10
	 * Store the command in buffer
	 * then send it to K10 if OK delete from the buffer
	 * */
#ifdef DEBUG
	printf("SendCommandToK10: ");
	for (var = 0; var < Length; ++var)
	{
		printf("%hhX ",data[var]);
	}
	printf(" end\n");
#endif
	write( DeviceFd.K10Fd, data, Length);
	return 1;
}


int main(void )
{
	int GlobalError = 0;
	int var = 0;
	e_ComResult ComResult;
	sleep(3);
	//char buff[10]="AT\n\r";//for modem test
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init failed\n");
		return (1);
	}

/*
 * Start Test modules
 * if the modules is not up run inituser.sh
 * script for start the modules.
 */
	GlobalError = TestModules( DeviceFd );
    if ( GlobalError != 0)
	{
		/* init critical error
		 * if need reboot system
		 * the return code :
		 * 1 = modem problem
		 * 2 = k10 problem
		 * 4 = printer problem
		 * 8 = keyboard problem
		 */
		printf("There is Problem loading Driver\n\r");
		if ( (GlobalError&1) == 1 )
		{
			printf("Modem Driver not loading\n\r");
		}
		if ( (GlobalError&2) == 2 )
		{
			printf("K10 Driver not loading\n\r");
		}
		if ( (GlobalError&4) == 4 )
		{
			printf("Printer Driver not loading\n\r");
		}
		if ( (GlobalError&8) == 8 )
		{
			printf("keyboard Driver not loading\n\r");
		}
	}
 //for debug need to remove if all divests is connected
#ifndef DEBUG
//	else
#endif
	{
//		DeviceFd.K10Fd = InitK10uart();/* Get the Modem File descriptor */
		DeviceFd.KeyBoard = InitKeyBoard();/* Get the Key Board File descriptor */
		connected = 1;//enable port
//		StartK10ReciveTrhad();/* Start K10 Demon */
//		StartUDPTrhad();/* Start UDP Demon */
		StartKeyBoardTrhad();/* Start Key Board Demon */
		//gpioTrhad
		//uart2Trhad
	}
//main loop
 while (1)
 {
	for ( var = 0; var < MAX_BUFF_LEN; ++var)
	{
		if (SandDataToKGUI[var].sendOk == 1)
		{
			Sendudp(SandDataToKGUI[var]);
			pthread_mutex_lock(&lock);
			SandDataToKGUI[var].sendOk = 0;
			pthread_mutex_unlock(&lock);
		}
		if (SandDtataToK10[var].sendOk == 1)
		{
			print_time();
			pthread_mutex_lock(&lock);
			ComResult = e_SendMessage( DeviceFd.K10Fd, SandDtataToK10[var].i_cmd,
					                     SandDtataToK10[var].e_IsRequesType,
					                     SandDtataToK10[var].ApplicationResult,
					                     SandDtataToK10[var].ApplicationStatusBits,
					                     SandDtataToK10[var].datalen, SandDtataToK10[var].data,
					                     (SandDtataToK10[var].datalen * 10));

			SandDtataToK10[var].sendOk = 0;
			pthread_mutex_unlock(&lock);
			if (ComResult != e_ComOk)
			{
				printf(" erroro sendig to K10\n");
			}
		}

	}
   usleep(20000);
 }


   /* Close the K10 serial port */
  //close(DeviceFd.K10Fd);
  CloseK10Uart();
  close(DeviceFd.KeyBoard);

 }
