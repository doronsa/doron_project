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
#include "KeyBoard.h"
#include "dcu_util.h"
#include "demon_util.h"
#include <sys/ioctl.h>
#define LINUX
#ifndef LINUX
#include <linux/i2c-dev.h>
#endif
#include "gpio_util.h"
extern int GetPackeOk;
sigset_t mskvar_1  ;                 //Variable of signal bitfieldtype
//struct sigaction sigio_action

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
#define K10PORT "/dev/ttyUSB0"//ttymxc2"//ttyUSB0"// "/dev/ttyUSB0"
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
#define UDB_BUFFER 2000
/* Function export */
int wait_flag=TRUE;
void signal_handler_IO (int status);   /* definition of signal handler */
//extern int dcu_connected;
int AddGUIQue(St_ProtocolHeader *sProtocolHeader, void *data , int data_len);
int InitK10uart( void );
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


print_buffer(char *buffer, int len)
{
	int var = 0;
	printf("buffer %d:",len);
	for (var = 0; var < len; ++var)
	{
		printf(" %x",buffer[var]);
	}
	printf("\n");
}

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
typedef struct St_ProtocolHeader SDUProtocolHeader ;

typedef struct
{
/* GUI command send/receive */
	unsigned char Marker1;
	unsigned char Marker2;
	unsigned char Cmd;
	int DataLen;
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
DataCommand SandDtataToSDU[10];
DataCommand SandDataToKGUI[10];
SandKeyToKGUI KeyToKGUI[50];

//sDeviceFd DeviceFd;//global File descriptor
pthread_t pK10threadRecive, pUDPcomThread, pKeyBoardTrhad, pDCUthreadRecive;
pthread_mutex_t lock;
void *print_message_function( void *ptr );
int AddK10Que( St_ProtocolHeader *sProtocolHeader, void *data );
//int AddDCUQue( St_ProtocolHeader *sSDUProtocolHeader, void *data );
int AddGUIQue( St_ProtocolHeader *sProtocolHeader, void *data , int data_len);
int AddKeyBoarQue( SandKeyToKGUI KeyToKGUI);
/*  init variable*/
int connected;
char send_buffer[BUFLEN];
/* Function */
void signal_handler_IO (int status)
{
    //printf("received data from UART. %d\n",status);
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
	fd = open(K10PORT, O_RDWR | O_NOCTTY );//| O_NDELAY);
	if (fd == -1)
	{
		perror("open_port: Unable to open /dev/ttymxc2\n");
		//exit(1);
		//send_error("K10 init error");
	}

	saio.sa_handler = signal_handler_IO;
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
    //exit(1);
    //send_error("error");
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
	return 0;
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
	int temp_data_len;
	unsigned char tmpdata[2000];
	unsigned char *pdata = data;
	unsigned char *ptempdata = &tmpdata;

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
		//free(pdata);
#ifdef DEBUG
		printf("**** Data_encode *******\n");
		printf("**** data index :%d\n",index);
		printf("**** data cmd :%hhX\n",sProtocolHeader->Cmd);
		printf("**** data DataSize :%d\n",sProtocolHeader->DataSize);
		printf("**** data_len :%d\n",data_len);
		printf("**** heder len :%d\n",hsize);
#endif
		if ( data_len < (hsize + index))
		{
			printf("**** Bade Data_encode *******\n");
			return -1;
		}
		if ( data_len < sProtocolHeader->DataSize)
		{
			printf("**** Bade DataSize Data_encode *******\n");
			sProtocolHeader->DataSize = data_len - hsize;
		}
		if ((sProtocolHeader->Marker1 == 0xa5)&&(sProtocolHeader->Marker2 == 0xa5))
		{
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
						case e_CmdK10_Jump2Loader:
						case e_CmdK10_Jump2App:
						case e_CmdK10_Set2LowPower:
						case e_CmdK10_ResetApp:
						case e_CmdK10_GetRTCTime:
						case e_CmdK10_SetRTCTime:
						case e_CmdK10_EventSensorChange:
						case e_CmdK10_KeyBoardEvent:
						case e_CmdK10_ClySam_InitInterface:
						case e_CmdK10_ClySam_DetectCard:
						case e_CmdK10_ClySam_Reset:
						case e_CmdK10_ClySam_GetType:
						case e_CmdK10_ClySam_GetSerNum:
						case e_CmdK10_ClySam_Unlock:
						case e_CmdK10_ClySam_ReadData:
						case e_CmdK10_ClyCard_Init:
						case e_CmdK10_ClyCard_GetSerNum:
						case e_CmdK10_ClyCard_DetectCard:
						case e_CmdK10_ClyCard_EjectCard:
						case e_CmdK10_ClyCard_Reset:
						case e_CmdK10_ClyCard_StartWorkWithCard:
						case e_CmdK10_ClyCard_ReadRecord:
						case e_CmdK10_ClyCard_WriteRecord:
						case e_CmdK10_ClyCard_UpdateRecord:
						case e_CmdK10_ClyCard_IncreaseDecrease:
						case e_CmdK10_ClyCard_Invalidate:
						case e_CmdK10_ClyCard_TestReadWrite:
						case e_CmdK10_7816_ContactlessDetect:
						case e_CmdK10_7816_GetCardResetInfo:
						case e_CmdK10_7816_ContactlessForgetCard:
						case e_CmdK10_7816_CheckCardComm:
						case e_CmdK10_ClySession_OpenSecureSession:
						case e_CmdK10_ClySession_CloseSecureSession:
						case e_CmdK10_ClyApp_PingTest:
						case e_CmdK10_ClyApp_TxDataTest:
						case e_CmdK10_ClyApp_IsCardIn:
						case e_CmdK10_ClyLast:
						{
								temp_data_len = sProtocolHeader->DataSize;
								sProtocolHeader->RequestType = e_ReqTypeRequest;
								sProtocolHeader->AplicationStatus = 1;
								if ( senddata == NULL )
									free(senddata);
								senddata = (void *)malloc(temp_data_len);
								ptempdata += (hsize + index);//move to data point
								memcpy( senddata, ptempdata, temp_data_len);
								AddK10Que( sProtocolHeader, senddata);
								free(senddata);
						}
							break;
						case e_CmdK10_DownLoad:
						{
							if(sProtocolHeader->AplicationStatus == 0)
								GetPackeOk = 1;
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
						if ( sProtocolHeader->Cmd == e_CmdK10_PeriodicMonitorEvent || sProtocolHeader->Cmd == e_CmdK10_EventSensorChange)
						{
							if ( senddata == NULL )
								free(senddata);
							senddata = (void *)malloc(sProtocolHeader->DataSize);
							ptempdata += (hsize + index);//move to data point
							memcpy( senddata, ptempdata, sProtocolHeader->DataSize);
							AddK10Que( sProtocolHeader, senddata);
#ifdef DEBUG
							printf("Data_encode: ");
							for (var = 0; var < sProtocolHeader->DataSize; var++)
							{
								printf(" %hhX ", ptempdata[var]);
							}
							printf("\n ");
#endif

							free(senddata);

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

int encode_heder( void *data , int data_len)
{
	int hsize= sizeof(St_ProtocolHeader);//14
	int index = 0;
	int var=0;
	int temp_data_len;
	unsigned char *pdata = data;

	if ( data_len >= hsize)
	{
		index = ByteReceive( pdata, data_len);
		if ( index == -1)
		{
			return -1;
		}

		pdata +=index;//move the data to the reel point
		St_ProtocolHeader *sProtocolHeader = (St_ProtocolHeader*)pdata;
		if ((sProtocolHeader->Marker1 == 0xa5)&&(sProtocolHeader->Marker2 == 0xa5))
		{
			return sProtocolHeader->DataSize;
		}
	}
	return -1;
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
			return (1);

		}
	}
}

int Datda_Decode( int cmd, int TypeRespond, void *data,int AplicationStatus, int data_len)
{
	St_ProtocolHeader sProtocolHeader;
	sProtocolHeader.Address = 0;
	sProtocolHeader.Marker1 = 0xa5;
	sProtocolHeader.Marker2 = 0xa5;
	sProtocolHeader.Cmd = cmd;
	sProtocolHeader.StatusBit =1;
	sProtocolHeader.AplicationStatus = AplicationStatus;
	sProtocolHeader.RequestType = TypeRespond;

	if ( data_len > 0)
	{
		sProtocolHeader.DataSize = data_len;
		AddGUIQue( &sProtocolHeader, data, data_len);

	}
	else
	{
		sProtocolHeader.DataSize = 0;
		AddGUIQue( &sProtocolHeader, NULL, data_len);
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
				SandDtataToK10[var].datalen = sProtocolHeader->DataSize;
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
	unsigned char buff2[UDB_BUFFER];
	unsigned char *buffp = buff1;//debug
	int n;
	int data_len;
	int get_error=0;
	int var,totalzise=0,datared=0;
	while(connected == 1)
    {

		usleep(200);//2000);
		if (wait_flag == FALSE)  //if input is available
		{
			printf("Recive_message_function \n");
			/* Read character from ABU */
			usleep(70000);
			memset( buff1,0, sizeof(buff1));
			memset( buff2,0, sizeof(buff1));//debug
			totalzise = 0;
			//int n = read(DeviceFd.K10Fd, buff1, sizeof(buff1));
			ioctl(DeviceFd.K10Fd, FIONREAD, &datared);
			printf("ioctl receive data read:%d\n",datared);
			if (datared >= 14)
			{
				n = read(DeviceFd.K10Fd, buff1, datared);
				printf("data receive n:%d data read%d\n",n,datared);
				if (n >= 14)
				{
					print_time();
					data_len = encode_heder(buff1,datared);
					printf("data_len receive  data_len%d\n",data_len);
					if (data_len >= 0)
					{
						//buffp+=n;
						if ( data_len > 0)
						{
							totalzise = datared;
							if ( datared != (data_len + 14))
							{
								if(data_len > 400)
								{
									usleep(20000);
								}
								buffp+= totalzise;
								while(totalzise <= (data_len) )
								{

									n = read(DeviceFd.K10Fd, buffp, data_len);
									if (n>0)
									{
										totalzise+=n;
										buffp+= totalzise;
										//printf("while receive n:%d totalzise:%d data len%d\n",n,totalzise,data_len);
									}
									else
									{
										break;
									}

								}
								//printf("while receive n:%d totalzise:%d data len%d\n",n,totalzise,data_len);

							}
							memcpy(buff2,buff1,totalzise);
							get_error = Data_encode( buff2, totalzise);
							printf("data receive n:%d totalzise:%d data len%d\n",n,totalzise,data_len);
							for (var = 0; var < n; ++var)
							{
								printf("%hhX ", buff2[var]);
							}
							printf("\n ");

						}
						else
						{
							totalzise = datared;
							memcpy(buff2,buff1,totalzise);
							get_error = Data_encode( buff2, datared);
						}
					}
				}//
				else
				{
					//usleep(2000);
					printf("break receive n:%d totalzise:%d data len%d\n",n,totalzise,data_len);
					tcflush(DeviceFd.K10Fd, TCIOFLUSH);
					break;
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
	UDPcommand udpcmd;
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
	  //exit(1);
	  //send_error("Sendudp");
	}
    //build command for send to GUI
	memset(dataGUI,0,UDB_BUFFER);
	udpcmd.Marker1 = MARKER1;
	udpcmd.Marker2 = MARKER1;
	udpcmd.Cmd = SandDataToKGUI.i_cmd;
	udpcmd.DataLen = SandDataToKGUI.datalen;

	memcpy( udpcmd.Data, SandDataToKGUI.data, SandDataToKGUI.datalen);
	if (sendto(sockfd , &udpcmd, UDB_BUFFER, 0,(struct sockaddr *)&si_other, slen)< 0)
	{
		printf("UDP error sending command to GUI %s\n",udpcmd);
	}
	else
	{
#ifdef DEBUG
		printf("UDP sending command to GUI \n");
		printf("  cmd %hhx ",udpcmd.Cmd);
		printf("  len %hhx ",udpcmd.DataLen);
		 for (i=0;i<udpcmd.DataLen;i++)
		 {
			 printf("%hhx ",udpcmd.Data[i]);
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
	 //unsigned char *tempbuf=(unsigned char*)malloc(UDB_BUFFER);
	// memcpy(tempbuf,buffer,UDB_BUFFER);
	 if (data->Marker1==MARKER1)
     {
		 if (data->Marker2 == MARKER1)
		 {
#ifdef DEBUG
			 printf("Get Command from GUI to K10 Command %d\n",data->Cmd);
#endif
			 switch ( data->Cmd )
			 {
				case e_CmdK10_DownLoad:
					K10_Download();
					break;
				case e_CmdKeyBoardSetLedOn:
				case e_CmdKeyBoardSetLedOff:
					SetLED( data->Data[0], data->Cmd);
					break;
				case e_CmdGPIOSetOn:
				case e_CmdGPIOSetOff:
					SetGpioValue(data->Data[0], data->Cmd);
					break;
				case e_CmdK10_ClySam_InitInterface:
				case e_CmdK10_ClySam_DetectCard:
				case e_CmdK10_ClySam_Reset:
				case e_CmdK10_ClySam_GetType:
				case e_CmdK10_ClySam_GetSerNum:
				case e_CmdK10_ClySam_Unlock:
				case e_CmdK10_ClySam_ReadData:
				case e_CmdK10_ClyCard_Init:
				case e_CmdK10_ClyCard_GetSerNum:
				case e_CmdK10_ClyCard_DetectCard:
				case e_CmdK10_ClyCard_EjectCard:
				case e_CmdK10_ClyCard_Reset:
				case e_CmdK10_ClyCard_StartWorkWithCard:
				case e_CmdK10_ClyCard_ReadRecord:
				case e_CmdK10_ClyCard_WriteRecord:
				case e_CmdK10_ClyCard_UpdateRecord:
				case e_CmdK10_ClyCard_IncreaseDecrease:
				case e_CmdK10_ClyCard_Invalidate:
				case e_CmdK10_ClyCard_TestReadWrite:
				case e_CmdK10_7816_ContactlessDetect:
				case e_CmdK10_7816_GetCardResetInfo:
				case e_CmdK10_7816_ContactlessForgetCard:
				case e_CmdK10_7816_CheckCardComm:
				case e_CmdK10_ClySession_OpenSecureSession:
				case e_CmdK10_ClySession_CloseSecureSession:
				case e_CmdK10_ClyApp_PingTest:
				case e_CmdK10_ClyApp_TxDataTest:
				case e_CmdK10_ClyApp_IsCardIn:
				case e_CmdK10_ClyLast:
				case e_CmdK10_Jump2Loader:
				case e_CmdK10_Jump2App:
					if( data->DataLen == 0)
					{
						Datda_Decode( data->Cmd, e_ReqTypeRequest, NULL,1, 0);
					}
					else
					{
						Datda_Decode( data->Cmd, e_ReqTypeRequest, data->Data,1, data->DataLen);
					}
					break;
				case 101:
					DeviceFd.DCUKeyDrv = DCU_SerialPortInit();
					break;
				case e_CmdDCUinit:
					if( data->DataLen == 0)
					{
						DCUData_Decode( data->Cmd, e_ReqTypeRequest, NULL,1, 0);
					}
					else
					{
						DCUData_Decode( data->Cmd, e_ReqTypeRequest, data->Data,1, data->DataLen);
					}
					break;
				default:
					if( data->DataLen == 0)
					{
						Datda_Decode( data->Cmd, e_ReqTypeRequest, NULL,1, 0);
					 //AddGUIDtataCommand( send_buffer, BUFLEN );
					}
					else
					{
						Datda_Decode( data->Cmd, e_ReqTypeRequest, data->Data,1, data->DataLen);
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
	 //send_error("UDP server");
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
	 //send_error("UDP server");
	}
	else
	{
	 printf("UDP server - bind() is OK\n");

	 printf("UDP server - Listening...\n");
	}
	/* Wait on client requests. */
	while (1)
	{

		 rc = recvfrom(sd, bufptr, buflen, 0, (struct sockaddr *)&clientaddr,(socklen_t*) &clientaddrlen);
		 if(rc < 0)
		 {
			 perror("UDP Server - recvfrom() error");
			 close(sd);
			 //exit(-1);
			 //send_error("UDP server");
		 }
		 else
		 {
#ifdef DEBUG
			 printf("UDP Server - recvfrom() is OK...\n");
#endif
			 //print_time();
			 //pthread_mutex_lock(&lock);
			 EncodeUDPData( (unsigned char*)bufptr );
			 //pthread_mutex_unlock(&lock);
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
	int getkey=0;
	SandKeyToKGUI sendkey;
	while (1)
	{
		getkey = GetKey();
		switch (getkey)
		{
			case 0:
					key = 1;
				break;
			case 0x08:
					key = 2;
				break;
			case 0x10:
					key = 3;
				break;
			case 0x18:
					key = 4;
				break;
			case 0x20:
					key = 5;
				break;
			case 0x28:
					key = 6;
				break;
			case 0x30:
					key = 7;
				break;
			case 0x38:
					key = 8;
				break;
			case 39:
					key = 9;
				break;
			case 0x31:
					key = 10;
				break;
			case 0x29:
					key = 11;
				break;
			case 0x21:
					key = 12;
				break;
			case 0x19:
					key = 13;
				break;
			case 0x11:
					key = 14;
				break;
			case 0x09:
					key = 15;
				break;
			case 0x01:
					key = 16;
				break;
			default:
				break;
		}


		if ((oldkey != key) && (key > 0))
		{
			printf("Get Key board SW :%x index:%d\n",key,time);
			oldkey = key;
			sendkey.SendOK = 0;
			sendkey.keynum = key;
			sendkey.time = time;
			AddKeyBoarQue( sendkey );
			//for debug
			time++;
		}
		usleep(200000);
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


int StartDCUReciveTrhad( void )
{
	const char *message1 = "Thread StartDCUReciveTrhad";
	int  iret1;
	iret1 = pthread_create( &pDCUthreadRecive, NULL, DCURecive_message_function, (void*) message1);
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
	printf("from SendCommand ID:%d: ",UARTId);
	if ( Length > 0)
	{
		for (var = 0; var < Length; ++var)
		{
			printf("%hhX ",data[var]);
		}
		printf(" end\n");
	}
#endif
	tcflush(DeviceFd.K10Fd, TCIOFLUSH);
	write( UARTId, data, Length);
	return 1;
}

extern void dcu_signal_handler_IO (int status);

int main(void )
{
	int GlobalError = 0;
	int var = 0;
	e_ComResult ComResult;
	//char buff[10]="AT\n\r";//for modem test
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init failed\n");
		return (1);
	}
//	if (signal(SIGIO, signal_handler_IO) == SIG_ERR)
//	    printf("\ncan't catch SIGINT\n");
//	if (signal(SIGIO, dcu_signal_handler_IO) == SIG_ERR)
//	    printf("\ncan't catch SIGINT\n");

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
		DeviceFd.K10Fd = InitK10uart();/* Get the Modem File descriptor */
		//DeviceFd.KeyBoard = InitKeyBoard();/* Get the Key Board File descriptor */
		connected = 1;//enable port
		StartK10ReciveTrhad();/* Start K10 Demon */
		//StartDCUReciveTrhad();/* Start DCU Demon */
		StartUDPTrhad();/* Start UDP Demon */
		//StartKeyBoardTrhad();/* Start Key Board Demon */
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
		if (SandDtataToSDU[var].sendOk == 1)
		{
			print_time();
			pthread_mutex_lock(&lock);
			//debug
			if (SandDtataToSDU[var].i_cmd == 63)
				printf(" for test K10\n");
			ComResult = e_SendMessage( DeviceFd.DCUKeyDrv, SandDtataToSDU[var].i_cmd,
											SandDtataToSDU[var].e_IsRequesType,
											SandDtataToSDU[var].ApplicationResult,
											SandDtataToSDU[var].ApplicationStatusBits,
											SandDtataToSDU[var].datalen, SandDtataToSDU[var].data,
					                        (SandDtataToSDU[var].datalen * 10));

			SandDtataToSDU[var].sendOk = 0;
			pthread_mutex_unlock(&lock);
			if (ComResult != e_ComOk)
			{
				printf(" erroro sendig to K10\n");
			}
		}
	}
   usleep(2000);
 }


   /* Close the K10 serial port */
  //close(DeviceFd.K10Fd);
  CloseK10Uart();
 // CloseDCUUart();
  close(DeviceFd.KeyBoard);

 }
