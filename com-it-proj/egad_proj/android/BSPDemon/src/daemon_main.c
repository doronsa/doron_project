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
#include <android/log.h>
#include <netdb.h>      /* gethostbyname */

#include <time.h>
//#include <linux/i2c-dev.h>
#include "KeyBoard.h"
//#include "dcu_util.h"
#include "daemon_util.h"
#include <sys/ioctl.h>
#include "gpio_util.h"

#define  LOG_TAG    "BSPdeamon"

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

extern int GetPackeOk;
sigset_t mskvar_1  ;                 //Variable of signal bitfieldtype

typedef struct
{
	int PWMratio ;
	int current;
	int temp;
}LedSetup;

#define BSPVERSION 31

GPIOSetup SandGPIOGUI[NUM_OFF_GPIO];


void print_time (int command)
{
 struct timeval tv;
 struct tm* ptm;
 static struct tm* temp_ptm;
 char time_string[40];
 long milliseconds;
 /* Obtain the time of day, and convert it to a tm struct. */
 gettimeofday (&tv, NULL);
 ptm = localtime (&tv.tv_sec);
 /* command = 0 do nun
  * = 1 start cuont
  * = 2 print dif */
 switch (command)
 {
	case 1:
	{
		temp_ptm = ptm;
	}
		break;
	case 2:
	{
		 strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", temp_ptm);
		 milliseconds = tv.tv_usec / 1000;
		 printf ("start time : %s.%03ld\n", time_string, milliseconds);
		 strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
		 milliseconds = tv.tv_usec / 1000;
		 printf ("end time :%s.%03ld\n", time_string, milliseconds);
	}
	default:
	{

		 /* Format the date and time, down to a single second. */
		 strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
		 /* Compute milliseconds from microseconds. */
		 milliseconds = tv.tv_usec / 1000;
		 /* Print the formatted time, in seconds, followed by a decimal point
		   and the milliseconds. */
		 printf ("%s.%03ld\n", time_string, milliseconds);
	}
		break;
}

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
#define K10PORT "/dev/ttymxc4"
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
int DelayForLongData = 1;
void signal_handler_IO (int status);   /* definition of signal handler */

int AddGUIQue(St_ProtocolHeader *sProtocolHeader, void *data , int data_len);
/* Structure definition */
/*htons()
 * */
#ifdef DEBUG
/******************************************************************
 *  Function name: print_buffer
 *  Description:
 *  in
 *  out
 ******************************************************************/
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
#endif
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
pthread_t pGPIOthread;
pthread_mutex_t lock;
void *print_message_function( void *ptr );
int AddK10Que( St_ProtocolHeader *sProtocolHeader, void *data );
int AddGUIQue( St_ProtocolHeader *sProtocolHeader, void *data , int data_len);
int AddKeyBoarQue( SandKeyToKGUI KeyToKGUI);
/*  init variable*/
int connected;
char send_buffer[BUFLEN];
/* Function */
/******************************************************************
 *  Function name: SendVarsion
 *  Description: Send Demon Version
 *  in
 *  out
 ******************************************************************/
int SendVarsion(void)
{
	int var=0,i=0;
	printf("SendVarsion\n");
	for ( var = 0; var < MAX_BUFF_LEN; ++var)
	{
		if (SandDataToKGUI[var].sendOk == 0)
		{
			pthread_mutex_lock(&lock);
			SandDataToKGUI[var].ApplicationResult = 1;
			SandDataToKGUI[var].ApplicationStatusBits = 1;
			SandDataToKGUI[var].datalen = 3;
			SandDataToKGUI[var].e_IsRequesType = 1;
			SandDataToKGUI[var].i_cmd = e_BSP_GET_VERSION;
			//if the data is on the end off the header
			sprintf(SandDataToKGUI[var].data,"%d",BSPVERSION);
			SandDataToKGUI[var].sendOk = 1;
			pthread_mutex_unlock(&lock);
			return (1);
		}
	}

	return (-1);
}

/******************************************************************
 *  Function name: signal_handler_IO
 *  Description: UARD Intrupt
 *  in
 *  out
 ******************************************************************/
void signal_handler_IO (int status)
{
    //printf("received data from UART.\n");
    wait_flag = FALSE;
}

/******************************************************************
 *  Function name: CloseK10Uart
 *  Description: Close K10 Uart
 *  in
 *  out
 ******************************************************************/
void CloseK10Uart(void)
{
	close(DeviceFd.K10Fd);
}

/******************************************************************
 *  Function name: TestModules
 *  Description: Testing all devises if connected
 *  in: FD
 *  out :the return code :
 * 		1 = modem problem
 * 		2 = k10 problem
 * 		4 = printer problem
 * 		8 = keyboard problem
 ******************************************************************/
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


/******************************************************************
 *  Function name: InitK10uart
 *  Description: Init K10 UART
 *  in
 *  out
 ******************************************************************/
int InitK10uart( void )
{
	int fd ;
	fd = open(K10PORT, O_RDWR | O_NOCTTY );//| O_NDELAY);
	if (fd == -1)
	{
		perror("open_port: Unable to open /dev/ttymxc2\n");
		exit(1);
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
	termAttr.c_cflag |= CS8 ;//| CRTSCTS;
	termAttr.c_cflag |= (CLOCAL | CREAD);
	termAttr.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	termAttr.c_iflag &= ~(IXON | IXOFF | IXANY);
	termAttr.c_iflag &= ~IGNCR;  // turn off ignore \r
	termAttr.c_iflag &= ~INLCR;  // turn off translate \n to \r
	termAttr.c_iflag &= ~ICRNL;  // turn off translate \r to \n
	termAttr.c_oflag &= ~ONLCR;  // turn off map \n  to \r\n
	termAttr.c_oflag &= ~OCRNL;  // turn off map \r to \n
	termAttr.c_oflag &= ~OPOST;  // turn off implementation defined output processing
	termAttr.c_oflag &= ~OPOST;
	tcsetattr(fd,TCSANOW,&termAttr);
	printf("UART1 configured....\n");
	return (fd);
}

/******************************************************************
 *  Function name: error
 *  Description
 *  in
 *  out
 ******************************************************************/
void error(char *msg)
{
    perror(msg);
}

/******************************************************************
 *  Function name: keyEventQue
 *  Description
 *  in
 *  out
 ******************************************************************/
void keyEventQue( ProtocolHeader *sProtocolHeader, void *data )
{

}
static char state=0;

/******************************************************************
 *  Function name: OnByteReceive
 *  Description: Handle UART Byte Receive Protocol Header
 *  in : uc_Character
 *  out : Status
 ******************************************************************/
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

/******************************************************************
 *  Function name: ByteReceive
 *  Description: Handle UART Byte Receive Protocol Header
 *  in :p_Characters , size
 *  out
 ******************************************************************/
int ByteReceive( unsigned char *p_Characters, int size )
{
	int i;
	int ans=0;
	//static char state=0;
	state=0;
	unsigned char*temp = p_Characters;
	// send all bytes to protocol handler
	if(p_Characters == NULL)
	{
		LOGI(" ByteReceive NULL ");
		return (-1);
	}
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

/******************************************************************
 *  Function name: Data_encode
 *  Description: Encode the in Receive data from K10
 *  in: data , data_len
 *  out
 ******************************************************************/
int Data_encode( void *data , int data_len)
{
	int hsize= sizeof(St_ProtocolHeader);//14
	int index = 0;
	int var=0;
	int temp_data_len;
	if( data == NULL)
	{
		LOGI(" Data_encode NULL ");
		return -1;
	}
	unsigned char tmpdata[2000];
	unsigned char *pdata = data;
	unsigned char *ptempdata = &tmpdata;

	void *senddata;
	memcpy( ptempdata, (unsigned char *)data, data_len );

	if ( data_len >= hsize)
	{
		index = ByteReceive( pdata, data_len);
		if ( index == -1)
		{
			return -1;
		}

		pdata +=index;//move the data to the reel point
		St_ProtocolHeader *sProtocolHeader = (St_ProtocolHeader*)pdata;

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
					if ( sProtocolHeader->RequestType == e_ReqTypeEvent ||
						 sProtocolHeader->RequestType ==e_ReqTypeRespond )
					{
						if ( sProtocolHeader->Cmd == e_CmdK10_PeriodicMonitorEvent ||
							 sProtocolHeader->Cmd == e_CmdK10_EventSensorChange ||
							 sProtocolHeader->Cmd == e_CmdK10_PeriodicMonitorPoll)
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
	return 1;
}

/******************************************************************
 *  Function name: encode_heder
 *  Description: Get the Header from the data Receive from K10
 *  in: data , data_len
 *  out
 ******************************************************************/
int encode_heder( void *data , int data_len)
{
	int hsize= sizeof(St_ProtocolHeader);//14
	int index = 0;
	int var=0;
	int temp_data_len;
	if(data == NULL)
	{
		LOGI(" encode_heder NULL ");
		return -1;
	}
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


/******************************************************************
 *  Function name: AddKeyBoarQue
 *  Description: key buffer que
 *  in
 *  out
 ******************************************************************/
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


/******************************************************************
 *  Function name: Datda_Decode
 *  Description: build the data for send
 *  in: cmd, TypeRespond, data, AplicationStatus, data_len
 *  out:
 ******************************************************************/
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


/******************************************************************
 *  Function name: AddK10Que
 *  Description: data buffer que
 *  in: St_ProtocolHeader , data
 *  out:
 ******************************************************************/
int AddK10Que( St_ProtocolHeader *sProtocolHeader, void *data )
{
	int var=0,i=0;
	if (data == 0)
	{
		LOGI(" AddK10Que NULL ");
	    return -1;
	}

	for ( var = 0; var < MAX_BUFF_LEN; ++var)
	{
		if (SandDataToKGUI[var].sendOk == 0)
		{
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

/******************************************************************
 *  Function name: AddGUIQue
 *  Description: add the data to que
 *  in:St_ProtocolHeader,data,data_len
 *  out:
 ******************************************************************/
int AddGUIQue(St_ProtocolHeader *sProtocolHeader, void *data , int data_len)
{
	int var=0;
	for ( var = 0; var < MAX_BUFF_LEN; ++var)
	{
		if (SandDtataToK10[var].sendOk == 0)
		{
			//chcink if the command is ok
			if(sProtocolHeader->Cmd >= (unsigned char)e_CmdK10_CheckComm)
			{
				pthread_mutex_lock(&lock);
				SandDtataToK10[var].ApplicationResult = 0;
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

/******************************************************************
 *  Function name: Recive_message_function
 *  Description: Receive trhed
 *  in
 *  out
 ******************************************************************/
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
			//printf("Recive_message_function \n");
			/* Read character from ABU */
			usleep(23000*DelayForLongData);
			DelayForLongData = 1;
			memset( buff1,0, sizeof(buff1));
			memset( buff2,0, sizeof(buff1));//debug
			totalzise = 0;
			ioctl(DeviceFd.K10Fd, FIONREAD, &datared);
			if (datared >= 14)
			{
				n = read(DeviceFd.K10Fd, buff1, datared);
				if (n >= 14)
				{
					print_time(2);
					data_len = encode_heder(buff1,datared);
					if (data_len >= 0)
					{
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

									}
									else
									{
										break;
									}

								}


							}
							memcpy(buff2,buff1,totalzise);
							get_error = Data_encode( buff2, totalzise);
//							printf("data receive n:%d totalzise:%d data len%d\n",n,totalzise,data_len);
//							for (var = 0; var < n; ++var)
//							{
//								printf("%hhX ", buff2[var]);
//							}
//							printf("\n ");

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

					printf("break receive n:%d totalzise:%d data len%d\n",n,totalzise,data_len);
					tcflush(DeviceFd.K10Fd, TCIOFLUSH);
					break;
				}
			}
			pthread_mutex_lock(&lock);
			wait_flag = TRUE;
			pthread_mutex_unlock(&lock);

		}

	}
	return NULL;
}


/******************************************************************
 *  Function name: Sendudp
 *  Description:Send data to UDP
 *  in:DataCommand
 *  out:
 ******************************************************************/
int Sendudp(DataCommand SandDataToKGUI)
{
	char dataGUI[UDB_BUFFER];
	UDPcommand udpcmd;
	struct sockaddr_in si_other;
	int sockfd, i, slen=sizeof(si_other);

	printf("Start UDP send command to GUI \n");
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
		perror("UDP Sendudp - socket() error");
	}

	memset((char *) &si_other, sizeof(si_other), 0);
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(CLIENTPORT);
	if (inet_aton(SRV_IP, &si_other.sin_addr)==0)
	{
	  fprintf(stderr, "inet_aton() failed\n");
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

/******************************************************************
 *  Function name: EncodeUDPData
 *  Description: Get Command from GUI to K10 Command
 *  in: buffer
 *  out:
 ******************************************************************/
int EncodeUDPData(unsigned char *buffer)
{
	if (buffer == 0)
	{
		LOGI(" EncodeUDPData NULL ");
	    return -1;
	}
	 UDPcommand *data =( UDPcommand*)buffer;
	 if (data->Marker1==MARKER1)
     {
		 if (data->Marker2 == MARKER1)
		 {
#ifdef DEBUG
			 printf("Get Command from GUI to K10 Command %d\n",data->Cmd);
#endif
			 switch ( data->Cmd )
			 {
			 	case e_BSP_GET_VERSION:
			 		SendVarsion();
			 		break;
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
				case e_BSPKeyBoardled:
				{
					LedSetup sLedSetup;
					memcpy(&sLedSetup, data->Data,data->DataLen);
					SetLED(sLedSetup.current,sLedSetup.PWMratio);

				}
				break;
				case e_BSPGPIO_1:
				{
					GPIOSetup sGPIOSetup;
					memcpy(&sGPIOSetup, data->Data,data->DataLen);
					SetGpioValue(sGPIOSetup.gpio_num , sGPIOSetup.value);
				}
				break;
				case e_BSPGPIO_9:
				{
					GPIOSetup sGPIOSetup;
					memcpy(&sGPIOSetup, data->Data,data->DataLen);
					GetGpioValue(sGPIOSetup.gpio_num , &sGPIOSetup.value);
					printf("Get GPIO %d value %d\n",sGPIOSetup.gpio_num,sGPIOSetup.value);
				}
				break;
//				case 101:
//					DeviceFd.DCUKeyDrv = DCU_SerialPortInit();
//					break;
//				case e_CmdDCUinit:
//					if( data->DataLen == 0)
//					{
//						DCUData_Decode( data->Cmd, e_ReqTypeRequest, NULL,1, 0);
//					}
//					else
//					{
//						DCUData_Decode( data->Cmd, e_ReqTypeRequest, data->Data,1, data->DataLen);
//					}
//
//					break;
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
	 free(data);
	 return (1);
}

int sd;

void sig_handler(int signo)
{
	if (signo == SIGUSR1)
	{
		printf("received SIGUSR1\n");
		LOGI("received SIGUSR1");
		close(sd);
		exit(1);
	}
	else if (signo == SIGKILL)
	{
		printf("received SIGKILL\n");
		LOGI("received SIGKILL");
		close(sd);
		exit(1);
	}
	else if (signo == SIGSTOP)
	{
		printf("received SIGSTOP\n");
		LOGI("received SIGSTOP");
		close(sd);
		exit(1);
	}
}



void save_add_to_file( int number)
{
	//save socket port to file /tmp/daemon_soc.txt
	FILE *fileSave = fopen("/dev/daemon_soc.port", "w+");       
	char buf[10];
	snprintf(buf,10,"%d",number);
	if (fileSave == NULL)
	{
	    printf("Error opening daemon_soc.txt file!\n");
	    exit(1);
	}
	fputs(buf, fileSave);
	fflush(fileSave);
	fclose(fileSave);
        sleep(1);
        chmod("/dev/daemon_soc.port", 0644);
        system("chmod 0644 /dev/daemon_soc.port");
        //system("/data/bsp/port.sh");

}

/******************************************************************
 *  Function name:udp_message_function
 *  Description:UDP Trhad
 *  in:
 *  out:
 ******************************************************************/
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
	int length;
	/* create a socket
	     IP protocol family (PF_INET)
	     UDP protocol (SOCK_DGRAM)
    */

	  if ((sd = socket( PF_INET, SOCK_DGRAM, 0 )) < 0) {
	    printf("Problem creating socket\n");
	    LOGI("UDP no port find go to reboot");
	    system("reboot");
	  }

	  /* establish our address
	     address family is AF_INET
	     our IP address is INADDR_ANY (any of our IP addresses)
	     the port number is assigned by the kernel
	  */

	  serveraddr.sin_family = AF_INET;
	  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	  serveraddr.sin_port = htons(0);

	  if (bind(sd, (struct sockaddr *) &serveraddr, sizeof(serveraddr))<0) {
		 LOGI("UDP no bind go to reboot");
		 system("reboot");/* If something wrong with socket()reboot system */
	  }


	  /* find out what port we were assigned and print it out */

	  length = sizeof( serveraddr );
	  if (getsockname(sd, (struct sockaddr *) &serveraddr, &length)<0) {
		  LOGI("UDP no get sockname go to reboot");
		  system("reboot");/* If something wrong with socket()reboot system */
	  }
	  else
	  {
		  /* port number's are network byte order, we have to convert to
		     host byte order before printing !
		  */
		  LOGI("The server UDP port number is %d\n",ntohs(serveraddr.sin_port));
		  save_add_to_file(ntohs(serveraddr.sin_port));
	  }

	/* Wait on client requests. */
	while (1)
	{

		 rc = recvfrom(sd, bufptr, buflen, 0, (struct sockaddr *)&clientaddr,(socklen_t*) &clientaddrlen);
		 if(rc < 0)
		 {
			 //perror("UDP Server - recvfrom() error");
			 close(sd);

		 }
		 else
		 {
#ifdef DEBUG
			 printf("UDP Server - recvfrom() is OK...\n");
#endif
			 printf("Get Mssg from UDP\n");
			 print_time(1);
			 if (bufptr != NULL)
			 {
			 	 EncodeUDPData( (unsigned char*)bufptr );
			 }
			 else
			 {
				 LOGE(" UDP Server - recvfrom() Get NULL pointer");
			 }
		 }


	}

   close(sd);
   return (NULL);
}

/******************************************************************
 *  Function name:print_message_function
 *  Description
 *  in
 *  out
 ******************************************************************/
void *print_message_function( void *ptr )
{
     char *message;
     message = (char *) ptr;
     printf("%s \n", message);
     return (NULL);
}

/******************************************************************
 *  Function name:keyboar_message_function
 *  Description: KeyBoard Trhad
 *  in
 *  out
 ******************************************************************/
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
			case 0x39:
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
					key = 0;
					oldkey = 0;
				break;
		}


		if ((oldkey != key) && (key > 0))
		{
			//printf("Get Key board SW :%x index:%d\n",key,time);
			LOGI(" Get Key board SW :%x index:%d\n",key,time);
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

/******************************************************************
 *  Function name: StartK10ReciveTrhad
 *  Description
 *  in
 *  out
 ******************************************************************/
int StartK10ReciveTrhad( void )
{
	const char *message1 = "Thread StartK10ReciveTrhad";
	int  iret1;
	iret1 = pthread_create( &pK10threadRecive, NULL, Recive_message_function, (void*) message1);
	//pthread_join( pK10threadRecive, NULL);
	//printf("Thread StartK10ReciveTrhad returns: %d\n",iret1);
	LOGI(" Thread StartK10ReciveTrhad returns %d ",iret1);
	return 1;
}


/******************************************************************
 *  Function name: StartUDPTrhad
 *  Description
 *  in
 *  out
 ******************************************************************/
int StartUDPTrhad( void )
{
	const char *message2 = "Thread StartUDPTrhad";
	int  iret1;
	iret1 = pthread_create( &pUDPcomThread, NULL, udp_message_function, (void*) message2);
	//printf("Thread StartUDPTrhad returns: %d\n",iret1);
	LOGI(" Thread StartUDPTrhad returns %d ",iret1);
	return 1;
}

/******************************************************************
 *  Function name: StartGPIOTrhad
 *  Description
 *  in
 *  out
 ******************************************************************/
int StartGPIOTrhad( void )
{
	const char *message2 = "Thread StartUDPTrhad";
	int  iret1;
	iret1 = pthread_create( &pGPIOthread, NULL, gpio_message_function, (void*) message2);
	//printf("Thread StartUDPTrhad returns: %d\n",iret1);
	LOGI(" Thread StartGPIOTrhad returns %d ",iret1);
	return 1;
}

/******************************************************************
 *  Function name: StartKeyBoardTrhad
 *  Description
 *  in
 *  out
 ******************************************************************/
int StartKeyBoardTrhad ()
{
    const char *message2 = "Thread StartKeyBoardTrhad";
    int  iret1;
    iret1 = pthread_create( &pKeyBoardTrhad, NULL, keyboar_message_function, (void*) message2);
    //printf("Thread StartUDPTrhad returns: %d\n",iret1);
    LOGI(" Thread StartKeyBoardTrhad returns %d ",iret1);
    return 1;
}

/******************************************************************
 *  Function name: SendKeyBoardDataToUDP
 *  Description
 *  in
 *  out
 ******************************************************************/
int SendKeyBoardDataToUDP(int data)
{
	/*
	 * if get form i2c new data
	 * Store the data in buffer
	 * then send the data to UDP
	 * */
	return 1;
}

/******************************************************************
 *  Function name: SendCommandToK10
 *  Description: Send Command to K10
 *  in: UARTId, Bytes, Length, Timeout
 *  out
 ******************************************************************/
CoreReturnVal SendCommandToK10 ( int UARTId, void *Bytes, int Length, int Timeout)
{
	int var;
	unsigned char* data = Bytes;
	/*
	 * if get command from the UDP for the K10
	 * Store the command in buffer
	 * then send it to K10 if OK delete from the buffer
	 * */
	if ((Length == 0)&&((data==0 )))
	{
		printf("****************from SendCommand  Length and data problem *******************\n");
		return -1;
	}
#ifdef DEBUG
	printf("from SendCommand ID:%d: ",UARTId);
	if ( Length > 0)
	{
		for (var = 0; var < Length; ++var)
		{
			//printf("%hhX ",data[var]);
			printf("%X ",var);
		}
		printf(" end\n");
	}
#endif
	tcflush(DeviceFd.K10Fd, TCIOFLUSH);
	write( DeviceFd.K10Fd, data, Length);
	return 1;
}

extern void dcu_signal_handler_IO (int status);

/******************************************************************
 *  Function name: main
 *  Description: main loop
 *  in
 *  out
 ******************************************************************/
int main(void )
{
	int GlobalError = 0;
	int var = 0;
	e_ComResult ComResult;
	DataCommand SendKeyBoardData;
	DataCommand SendGPIOData;
	GPIOSetup TempGPIO;
	sleep(3);//wait setup udp port

	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init failed\n");
		return (1);
	}
	//init intrust from system
	if (signal(SIGUSR1, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGUSR1\n");
	if (signal(SIGKILL, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGKILL\n");
	if (signal(SIGSTOP, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGSTOP\n");
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

		DeviceFd.K10Fd = InitK10uart();/* Get the Modem File descriptor */
		connected = 1;//enable port
		StartK10ReciveTrhad();/* Start K10 Demon */
		StartUDPTrhad();/* Start UDP Demon */
		StartGPIOTrhad();
		DeviceFd.KeyBoard = InitKeyBoard();/* Get the Key Board File descriptor */
		StartKeyBoardTrhad();/* Start Key Board Demon */
		LOGI(" BSP Deamon Version %d stat OK ",BSPVERSION);
//main loop
 while (1)
 {
	for (var = 0; var < NUM_OFF_GPIO; ++var)
	{
		SendGPIOData.i_cmd = SandGPIOGUI[var].cmd;
		SendGPIOData.datalen = sizeof(GPIOSetup);
		TempGPIO.SendOK = SandGPIOGUI[var].SendOK;
		TempGPIO.cmd = SandGPIOGUI[var].cmd;
		TempGPIO.gpio_num = SandGPIOGUI[var].gpio_num;
		TempGPIO.value = SandGPIOGUI[var].value;
		memcpy(SendGPIOData.data,&TempGPIO , SendGPIOData.datalen);
		if (SandGPIOGUI[var].SendOK == 1)
			{
				Sendudp(SendGPIOData);
				pthread_mutex_lock(&lock);
				SandGPIOGUI[var].SendOK = 0;
				pthread_mutex_unlock(&lock);
			}
	}
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
			printf("********DelayForLongData %d\n",DelayForLongData);
			if (SandDtataToK10[var].i_cmd == e_CmdK10_ClyApp_IsCardIn)
			{
				DelayForLongData = 7;//(data->DataLen) / 100 +1;
			}
			printf("Send mssg to K10\n");
			print_time(0);
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
			print_time(0);
			pthread_mutex_lock(&lock);
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
		if (KeyToKGUI[var].SendOK == 1)
		{
				pthread_mutex_lock(&lock);
				SendKeyBoardData.i_cmd = e_CmdK10_KeyBoardEvent;
				SendKeyBoardData.datalen = sizeof(SandKeyToKGUI);
				memcpy(SendKeyBoardData.data, &KeyToKGUI[var], sizeof(SandKeyToKGUI));
				Sendudp(SendKeyBoardData);
				KeyToKGUI[var].SendOK = 0;
				pthread_mutex_unlock(&lock);
		}
	}
   usleep(20000);
 }


   /* Close the K10 serial port */
  //close(DeviceFd.K10Fd);
  CloseK10Uart();
  CloseDCUUart();
	pthread_mutex_unlock(&lock);
	if (ComResult != e_ComOk)
  close(DeviceFd.KeyBoard);

 }
