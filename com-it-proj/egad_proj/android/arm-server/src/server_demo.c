/*
 ============================================================================
 Name        : server_demo.c
 Author      : doron
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
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

#include "BSPutil.h"
#include <time.h>


#include <stdbool.h>
#include <sys/time.h>



#include "P_TwMtrBase.h"
#include "TW_K10232_P.h"
#include <dlfcn.h>
#include "ClyAppApi.h"
#include "os_porting.h"

extern st_InitResource InitRes;
#define FALSE 0
#define TRUE 1
#define OK 0
#define SERVPORT 4444
#define CLIENTPORT 4445
#define BUFLEN 100
#define SRV_IP "127.0.0.1"//"10.0.0.6"
#define NPACK 10
#define MARKER1 55

long int UDPPort = 0;

pthread_mutex_t lock;
char send_buffer[BUFLEN];
int wait_flag=TRUE;
#define UDB_BUFFER 2000
void signal_handler_IO (int status);
pthread_t pUDPcomThread, pKeyBoardTrhad ,pGPSthreadRecive;
struct termios termAttr;
struct sigaction saio;
/*  init variable*/
int connected;
char send_buffer[BUFLEN];
int uart_fd;

extern st_InitDCU Initdcu;
/* Function */



/******************************************************************
 *  Function name: signal_handler_IO
 *  Description get intrupt fromHW
 *  in :status
 *  out :set the global flag wait_flag to FALSE
 ******************************************************************/
void signal_handler_IO (int status)
{
    //printf("received data from UART.\n");
    wait_flag = FALSE;
}


typedef struct
{
	int gpio_num ;
	int value;
}GPIOSetup;


typedef struct
{
	int PWMratio ;
	int current;
	int temp;
}LedSetup;

typedef enum
{
	e_BSPKeyBoardled = 234,//befor enum off calypso=60 and after events k10  K10 send event =33
	e_BSPKeyBoardSetup,
	e_BSPGPIO_1,
	e_BSPGPIO_2,
	e_BSPGPIO_3,
	e_BSPGPIO_4,
	e_BSPGPIO_5,
	e_BSPGPIO_6,
	e_BSPGPIO_7,
	e_BSPGPIO_8,
	e_BSPGPIO_9,
	e_BSP_GET_VERSION,
	e_BSPLaset,//  Mast be last

}e_BSPcommand;


/******************************************************************
 *  Function name: print_buffer
 *  Description: print the contain off the input buffer
 *  in : buff pointer , buffer len
 *  out
 ******************************************************************/
void print_buffer(char * buff ,int len)
{
	int var;
	printf("print_buffer: ");
	for (var = 0; var < len; ++var)
	{
		printf("%x ",(unsigned char)buff[var]);
	}
	printf(" end \n");
}

/******************************************************************
 *  Function name: print_time
 *  Description: print system time
 *  in:
 *  out:
 ******************************************************************/
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
	//PROCe_Tr1020ExchangeData Proc;
	CHANNEL_HANDLER p_handle;

}st_PrimaryProtocolResource ;

typedef struct
{
	unsigned char Cmd;
	unsigned char TimeOut;
	unsigned char seand_data;
	unsigned char recive_data;
	int datalen;
	int p_ApplicationError;
	unsigned char Data[UDB_BUFFER];
}ExchangeData;
ExchangeData ExchangeDataCB;
ExchangeData DCUDataCB;
st_PrimaryProtocolResource Glblp_Resurce;
TR_BOOL TimeDataCallback(TR_St_DateTime* DateTime /*out*/);

/******************************************************************
 *  Function name: timer_handler
 *  Description: if no receive data from the K10 the timer expired
 *  And exit from the ReaderExchangeData loop
 *  in
 *  out
 ******************************************************************/
void timer_handler ( int sig )
{
 	//static int count = 0;

 	stop_timer();
 	if (ExchangeDataCB.seand_data == 1)
 	{
 		//printf("timer expired %d times\n", sig);
 		ExchangeDataCB.TimeOut = 1;
 	}
 }


TR_PACK_PREFIX struct TAG_RESPONSE_OBJ
{
	unsigned char data[110];
	unsigned char sw1_sw2[2];
	//unsigned char temp1[2];
	short int Len;
	long int  ReaderErr;
};
typedef TR_PACK_PREFIX struct TAG_RESPONSE_OBJ  RESPONSE_OBJ;


TR_PACK_PREFIX struct TAG_St_ClySam_ResetResp
{
	unsigned char p_Atr[20];
	unsigned char iAtrLen;
	//K_DWORD iAtrLen;
	RESPONSE_OBJ obj;
};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_ResetResp St_ClySam_ResetResp;

/******************************************************************
 *  Function name: ReaderExchangeData
 *  Description: Get From the Calypso Lib the exact  Function for RPC
 *  in: i_cmd,//[IN] the command
 *  	i_TimeOutMs,//[IN] // the time out
 *  	int i_ObjectInSize,//[IN]// the data size to send
 *  	void *p_ObjectIn,//[IN] the data
 *  	int *p_OutSizeArive,//[IN]// the size of data respond
 *  	unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the
 *  	                                        function  e_SendMessage ignore this value
 *  	int *p_ApplicationError) //[OUT]// the application respond
 *
 *  out: void *p_ObjectOut,//void *p_ObjectOut,//[OUT]// the data return
 ******************************************************************/

int ReaderExchangeData(void* pHandlerr,//[IN]
		int i_cmd,//[IN] the command
		int i_TimeOutMs,//[IN] // the time out
		int i_ObjectInSize,//[IN]// the data size to send
		void *p_ObjectIn,//[IN] the data
		int i_ObjectOutSizeReq,//[IN] // the data respond size except to
		void *p_ObjectOut,//void *p_ObjectOut,//[OUT]// the data return
		int *p_OutSizeArive,//[IN]// the size of data respond
		unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
		int *p_ApplicationError) //[OUT]// the application respond
{

	UDPcommand *udpcmd = (UDPcommand*)malloc(sizeof(UDPcommand));
  	udpcmd->Marker1 = MARKER1;
  	udpcmd->Marker2 = MARKER1;
  	udpcmd->Cmd = i_cmd;
  	udpcmd->DataLen = i_ObjectInSize;

  	if (i_ObjectInSize > 0)
  	{
  		memcpy(udpcmd->Data,p_ObjectIn,udpcmd->DataLen);
  	}
  	ExchangeDataCB.Cmd = i_cmd;
  	ExchangeDataCB.seand_data = 1;
  	Sendudp(udpcmd);
  	start_timer(1200);
	print_time();
  	printf(" From Exchange Data get cmd =%d Size =%d  Return Size %d App ret%d\n",
  	  			                                i_cmd,i_ObjectInSize,ExchangeDataCB.datalen ,*p_ApplicationError);
  	usleep(100);
  	free(udpcmd);
  	while(1)
  	{
  		if (ExchangeDataCB.recive_data == 1)
  		{
  			printf("* From Exchange Data recive_data cmd =%d datalen:%d\n", i_cmd,ExchangeDataCB.datalen);
  			pthread_mutex_lock(&lock);
  			if (ExchangeDataCB.datalen>0)
  			{
   				memcpy(p_ObjectOut,(void*)(ExchangeDataCB.Data),ExchangeDataCB.datalen);
  			}
  			else
  			{
  				p_ObjectOut = NULL;
  			}
  			ExchangeDataCB.p_ApplicationError = 1;
  			*p_OutSizeArive = ExchangeDataCB.datalen;
  			*p_ApplicationError = ExchangeDataCB.p_ApplicationError;
  			ApplicationStatusBits = 1;
  			ExchangeDataCB.recive_data = 0;
  			ExchangeDataCB.seand_data = 0;
  			pthread_mutex_unlock(&lock);
  			stop_timer();
  			return 0;
  		}
  		else
  		{
  			if (ExchangeDataCB.TimeOut == 1)
  			{
  				printf("From Exchange Data TimeOut cmd =%d \n", i_cmd);
  				pthread_mutex_lock(&lock);
  				ExchangeDataCB.recive_data = 0;
				ExchangeDataCB.seand_data = 0;
				ExchangeDataCB.Cmd = 0;
				*p_ApplicationError = 0;
				pthread_mutex_unlock(&lock);
				//stop_timer();
				return 0;
  			}
  		}
  	}

}




//fixed by Yoni
/******************************************************************
 *  Function name: TimeDataCallback
 *  Description: Time Data Callback
 *  in
 *  out: TR_St_DateTime
 ******************************************************************/
TR_BOOL TimeDataCallback(TR_St_DateTime* DateTime /*out*/)
{
	 time_t t = time(NULL);
	 struct tm tm = *localtime(&t);
	 printf("************** TimeDataCallback: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900,
			            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	 DateTime->Year = tm.tm_year + 1900;
	 DateTime->Month = tm.tm_mon + 1;
	 DateTime->Day = tm.tm_mday;
	 DateTime->Hour = tm.tm_hour;
	 DateTime->Minute = tm.tm_min;
	 DateTime->Second = tm.tm_sec;

	 return TR_TRUE;//added by Yoni

}

/******************************************************************
 *  Function name: SendCommandToK10
 *  Description: Send to K10 data
 *  in: UARTId //UART handle
 *      Bytes//buffer to send
 *      Length//buffer Length
 *      Timeout//not used
 *  out: sendnum //Bytes send
 ******************************************************************/
int SendCommandToK10 ( int UARTId, void *Bytes, int Length, int Timeout)
{
	int sendnum = 0;
	/*
	 * if get command from the UDP for the K10
	 * Store the command in buffer
	 * then send it to K10 if OK delete from the buffer
	 * */
	sendnum = write( UARTId, Bytes, Length);
	return sendnum;
}

/******************************************************************
 *  Function name: Sendudp
 *  Description: Send data from UDP
 *  in: UDPcommand //data struct to send
 *  out
 ******************************************************************/
int Sendudp(UDPcommand * dataGUI)//for gui data
{
	struct sockaddr_in si_other;

	int sockfd, slen=sizeof(si_other);
	printf("Start UDP send command to GUI \n");
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
		perror("UDP Sendudp - socket() error");
	}

	memset((char *) &si_other, sizeof(si_other), 0);
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(UDPPort);
	if (inet_aton(SRV_IP, &si_other.sin_addr)==0)
	{
	  fprintf(stderr, "inet_aton() failed\n");
	  exit(1);
	}

	if (sendto(sockfd , dataGUI, UDB_BUFFER, 0,(struct sockaddr *)&si_other, slen)< 0)
	{
		printf("UDP error sending command to GUI %s\n",dataGUI);
	}
	else
	{
		printf("UDP sending command to GUI marker:%d marker:%d\n",dataGUI->Marker1 ,dataGUI->Marker2);
		printf("UDP sending command to GUI cmd:%d data len:%d\n",dataGUI->Cmd ,dataGUI->DataLen);
	}


	close(sockfd);
	return 0;

}

/******************************************************************
 *  Function name: udp_message_function
 *  Description: UDP Thread
 *  in: ptr
 *  out
 ******************************************************************/
void *udp_message_function( void *ptr )
{
	/* Variable and structure definitions. */
	int sd, rc, i=0;
	struct sockaddr_in serveraddr, clientaddr;
	int clientaddrlen = sizeof(clientaddr);
	int serveraddrlen = sizeof(serveraddr);
	char buffer[UDB_BUFFER];
	char *bufptr = buffer;
	int buflen = UDB_BUFFER;//sizeof(buffer);
	char addr[2];//="85170";
	char GuiCmd,DataGui;
	unsigned char len;
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
	}
	else
	 printf("UDP server - bind() is OK\n");
     printf("UDP server - Listening...\n");

	/* Wait on client requests. */
	while (1)
	{

		 fflush(stdout);

		 rc = recvfrom(sd, bufptr, buflen, 0, (struct sockaddr *)&clientaddr,(socklen_t*) &clientaddrlen);
		 memcpy(addr,bufptr , 2);
		 GuiCmd = bufptr[2];
		 DataGui = bufptr[3];
		 if(rc < 0)
		 {
			 perror("UDP Server - recvfrom() error");
			 close(sd);
			 //exit(-1);
		 }
		 else
		 {
			printf("UDP Server - recvfrom() is OK...\n");
			 //printf("data : %s\n",bufptr);
			printf(" ***********************************************************\n");
			printf(" UDP data :");
			UDPcommand *data =( UDPcommand*)buffer;
			printf("  len: %hhx, %hu \n",data->DataLen,data->DataLen);
			printf("  cmd: %hhx ",data->Cmd);
			printf(" data: ");
			for (i=0;i< data->DataLen;i++)
		    {
					 printf("%hhX ",data->Data[i]);
		    }
			printf(" end count %d\n",i);
			printf(" ***********************************************************\n");
			//printf("****  DataLen: %hu \n",data->DataLen);
			pthread_mutex_lock(&lock);
			if (ExchangeDataCB.Cmd == data->Cmd)
			{
				ExchangeDataCB.datalen = data->DataLen;
				if (data->DataLen > 0)
				{
					memcpy(ExchangeDataCB.Data,data->Data,data->DataLen);
				}
				ExchangeDataCB.recive_data = 1;
			  		//p_ApplicationError = &ExchangeDataCB.p_ApplicationError;
			}
			pthread_mutex_unlock(&lock);
		 }
	}

	close(sd);
}

/******************************************************************
 *  Function name: InitK10uart
 *  Description: Init K10 UART
 *  in
 *  out
 ******************************************************************/
int InitK10uart( void )
{
	uart_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
	//uart_fd = open(K10PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	if (uart_fd == -1)
	{
		perror("open_port: Unable to open /dev/ttyUSB0\n");
		exit(1);
	}
	saio.sa_handler = signal_handler_IO;
	saio.sa_flags = 0;
	saio.sa_restorer = NULL;
	sigaction(SIGIO,&saio,NULL);

	fcntl(uart_fd, F_SETFL, FNDELAY);
	fcntl(uart_fd, F_SETOWN, getpid());
	fcntl(uart_fd, F_SETFL,  O_ASYNC ); /**<<<<<<------This line made it work.**/

	tcgetattr(uart_fd,&termAttr);
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
	tcsetattr(uart_fd,TCSANOW,&termAttr);
	printf("UART1 configured....\n");
	return (uart_fd);
}

/******************************************************************
 *  Function name: Recive_message_function
 *  Description: UART Thread
 *  in:ptr
 *  out
 ******************************************************************/
void *Recive_message_function( void *ptr )
{
	while(connected == 1)
    {

		usleep(200);
		if (wait_flag == FALSE)  //if input is available
		{
			 wait_flag = TRUE;
			 printf("Recive_message_function \n");
			/* Read character from ABU */
			char buff1[UDB_BUFFER];
			memset( buff1,0, sizeof(buff1));
			int n = read(DeviceFd.K10Fd, buff1, sizeof(buff1));
			if (n > 0)
			{
				//Data_encode( buff1, sizeof(buff1));
				//Parsing the incoming data
				printf("Got %s.\n", buff1);
//				if (strncmp(buff1,"cmd2",4)==0 )
//				{
//					//sprintf(buff1,"cmd2");
//					AddK10DtataCommand( buff1, sizeof(buff1));
//					//Sendudp(buff1);
//				}
			}

		}

	}
	return NULL;
}


/******************************************************************
 *  Function name: StartUDPTrhad
 *  Description: Start UDP Trhad
 *  in
 *  out
 ******************************************************************/
int StartUDPTrhad( void )
{
	     const char *message2 = "Thread StartUDPTrhad";
	     int  iret1;
	     iret1 = pthread_create( &pUDPcomThread, NULL, udp_message_function, (void*) message2);
	     //pthread_join( pUDPcomThread, NULL);
	     printf("Thread StartUDPTrhad returns: %d\n",iret1);
	     return 1;
}

/******************************************************************
 *  Function name: CloseK10Uart
 *  Description: Close K10Uart
 *  in
 *  out
 ******************************************************************/
void CloseK10Uart(void)
{
	close(DeviceFd.K10Fd);
}

/******************************************************************
 *  Function name: FillContractForLoad
 *  Description
 *  in
 *  out
 ******************************************************************/
static void FillContractForLoad(TR_St_LoadContract* pLoadData /*out*/)
{
	//kartisia
	//pLoadData->IsRestore = 0;
	pLoadData->sFareCode = 2;
	pLoadData->sPredefinedCode = 0;
	pLoadData->sZoneBitmap = 0;
	pLoadData->st_ContractValidityStartDate.Day = 22;
	pLoadData->st_ContractValidityStartDate.Month = 3;
	pLoadData->st_ContractValidityStartDate.Year = 2014;
	pLoadData->ucEtt = 14;
	pLoadData->ucInterchangeType = 0;
	pLoadData->ucRestrictTimeCode = 0;
	pLoadData->ucSpatialType = 1;
	pLoadData->uc_ContractCustomerProfile = 0;
	pLoadData->usCluster = 127;
	pLoadData->usSaleNumberDaily = 500;
}

/******************************************************************
 *  Function name: StartGPSReciveTrhad
 *  Description: Start GPS Receive Trhad
 *  in
 *  out
 ******************************************************************/
int StartGPSReciveTrhad( void )
{
	const char *message1 = "Thread StartGPSReciveTrhad";
	int  iret1;
	iret1 = pthread_create( &pGPSthreadRecive, NULL, GPS_Recive_message_function, (void*) message1);
	//pthread_join( pK10threadRecive, NULL);
	printf("Thread StartGPSReciveTrhad returns: %d\n",iret1);
	return 1;
}


void GetUDPPortFromFile(void)
{
	FILE *fileSave = fopen("/dev/daemon_soc.port", "r");
	char buf[10],str[10];
	if (fileSave == NULL)
	{
		printf("Error opening daemon_soc.port file!\n");
		exit(1);
	}
	fgets( buf, 10, fileSave);
	fclose(fileSave);
	memcpy(str,buf,10);
	UDPPort = strtol(str,NULL,10);
	printf("port number :%d  str:%s\n",UDPPort,str);
}

/******************************************************************
 *  Function name: main
 *  Description: main loop
 *  in
 *  out
 ******************************************************************/

int main( void )
{
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init failed\n");
		return (1);
	}
    //get udp port from file
	GetUDPPortFromFile();

	char PWMratio = 0;
	signal(SIGALRM, timer_handler);
	init_timer();

	void (*ExchangeData)(int);
	void (*TimeDataCB)(int);
	ExchangeData = &ReaderExchangeData;
	TimeDataCB = &TimeDataCallback;
	int res=0;
	typedef int (*some_func)(char *param);

	InitRes.ProtocolCB = ExchangeData;
	InitRes.TimeAndDateCB = TimeDataCB;
	InitRes.pHandler = 1;

	int  ComResult,uartfd,datalen=0;
	char cmd;
	UDPcommand *udpcmd = (UDPcommand*)malloc(sizeof(UDPcommand));
	char buffer[100];
	StartUDPTrhad();
	usleep(2000);
	connected = 1;
	while (1)
	{

		printf("Enter Command to send 1 = e_CmdK10_GetAppVersion\n"
			   "                      2 = e_CmdK10_CheckComm\n"
			   "                      3 = e_CmdK10_PeriodicMonitorPoll\n"
			   "                      4 = e_CmdK10_SetParam\n"
			   "                      5 = e_CmdK10_DisplayCommandSet back Light\n"
			   "                      6 = e_CmdK10_DisplayCommandSet\n"
			   "                      7 = e_CmdK10_PowerCommandSet\n"
			   "                      8 = e_CmdK10_PowerCommandGet\n"
			   "                      k = e_CmdK10_Jump2Loader\n"
			   "                      l = e_CmdK10_Jump2App\n"
			   "                      m = e_CmdK10_DownLoad\n"
			   "                      r = e_CmdK10_SensorCommandGet\n"
			   "                      a = Init Cylipso cb \n"
			   "                      b = Set params \n"
			   "                      c = TR_IsCardIn \n"
			   "                      d = TR_Forget \n"
			   "                      e = TR_GetEnvironmentData \n"
			   "                      f = TR_GetListForReportAndReload \n"
			   "                      g = TR_GetListForUse \n"
			   "                      h = TR_IsPossibleLoad \n"
			   "                      i = TR_Load \n"
			   "                      j = TR_Use \n"
			   "					  t = Use loop \n"
			   "                      v = Test Key Board LED \n"
			   "                      q = Test GPIO \n"
			   "                      99 = K10 response for GetAppVersion\n");
		cmd = getchar();
		udpcmd->Marker1= MARKER1;
		udpcmd->Marker2= MARKER1;
//		sleep(3);
//		cmd = '1';
		switch (cmd)
		{
			case '1':
				udpcmd->Cmd= e_CmdK10_GetAppVersion;
				udpcmd->DataLen=0;
				//udpcmd.Data=NULL;
				print_time();
				Sendudp(udpcmd);
				sleep(2);
				udpcmd->Cmd= e_BSP_GET_VERSION;
				udpcmd->DataLen = 0;
				Sendudp(udpcmd);
				break;
			case '2':
				udpcmd->Cmd= e_CmdK10_CheckComm;
				udpcmd->DataLen=0;
				Sendudp(udpcmd);
				break;
			case '3':
				udpcmd->Cmd= e_CmdK10_PeriodicMonitorPoll;//e_CmdK10_SelfTest;
				udpcmd->DataLen=0;
				Sendudp(udpcmd);
				break;

			case '4':
			{
				St_K10_LedCommandSet pSt_K10_LedCommandSet;
				pSt_K10_LedCommandSet.LedBlockId = e_BothLed;
				pSt_K10_LedCommandSet.Red = e_LedOn;
				pSt_K10_LedCommandSet.Green = e_LedOn;
				pSt_K10_LedCommandSet.Yellow = e_LedOn;
				udpcmd->Cmd= e_CmdK10_LedCommandSet;
				udpcmd->DataLen=sizeof(pSt_K10_LedCommandSet);
				memcpy(udpcmd->Data,&pSt_K10_LedCommandSet,sizeof(pSt_K10_LedCommandSet));
				Sendudp(udpcmd);
//				udpcmd->Cmd= e_CmdK10_SetParam;
//				udpcmd->DataLen=0;
//				Sendudp(udpcmd);
			}
				break;
			case '5':
			{
				St_K10_DisplaySet pSt_K10_DisplaySet;
				pSt_K10_DisplaySet.K10_DisplayId = e_K10_Dsp_ID_Passanger;
				pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_Set_BackGround;
				udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
				udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
				memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
				Sendudp(udpcmd);
				usleep(100000);

				udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
				udpcmd->DataLen = sizeof(pSt_K10_DisplaySet);
				pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_SetArrowLed;
				memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
				Sendudp(udpcmd);
				usleep(100000);

				udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
				pSt_K10_DisplaySet.DisplayCommand =e_K10_Dsp_SetAmbient;
				udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
				memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
				Sendudp(udpcmd);
				usleep(1000000);
				GPIOSetup sGPIOSetup;
				printf("input GPIO:%d\n",86);
				udpcmd->Cmd = e_BSPGPIO_1;//for input
				sGPIOSetup.gpio_num = 86;
				sGPIOSetup.value = 1;
				udpcmd->DataLen = sizeof(sGPIOSetup);
				memcpy(udpcmd->Data , &sGPIOSetup,udpcmd->DataLen);
				Sendudp(udpcmd);
				sleep(4);

				udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
				pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_ClrArrowLed;
				udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
				memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
				Sendudp(udpcmd);
				usleep(100000);

				udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
				pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_ClrAmbient;
				udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
				memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
				Sendudp(udpcmd);
				usleep(100000);
				//GPIOSetup sGPIOSetup;
				printf("input GPIO:%d\n",86);
			    udpcmd->Cmd = e_BSPGPIO_1;//for input
			    sGPIOSetup.gpio_num = 86;
			    sGPIOSetup.value = 0;
			    udpcmd->DataLen = sizeof(sGPIOSetup);
			    memcpy(udpcmd->Data , &sGPIOSetup,udpcmd->DataLen);
			    Sendudp(udpcmd);

			}
				break;
			case '6':
			{
				char char_buff[]={0xA0,0};//0xA0 starting the Hebrew
				St_K10_DisplaySet pSt_K10_DisplaySet;
				pSt_K10_DisplaySet.K10_DisplayId = e_K10_Dsp_ID_Passanger;
				pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_Set_WriteStringXY;
				//memcpy(pSt_K10_DisplaySet.StringXY,"Passanger test",15);
				memcpy(pSt_K10_DisplaySet.StringXY,char_buff,2);
				//pSt_K10_DisplaySet.StringXY[] = {"test"};
				pSt_K10_DisplaySet.x = 0;
				pSt_K10_DisplaySet.y = 0;
				udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
				udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
				memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
				Sendudp(udpcmd);
			}
				break;
			case '7':
			{
				St_K10_PowerCommandSet pSt_K10_PowerCommandSet;
				pSt_K10_PowerCommandSet.Channel = e_BataryChannel;
				pSt_K10_PowerCommandSet.Command = e_ChannelOn;
				udpcmd->Cmd= e_CmdK10_PowerCommandSet;
				udpcmd->DataLen = sizeof(pSt_K10_PowerCommandSet);
				memcpy(udpcmd->Data,&pSt_K10_PowerCommandSet,sizeof(pSt_K10_PowerCommandSet));
				Sendudp(udpcmd);
			}
				break;
			case '8':
			{
				udpcmd->Cmd= e_CmdK10_PowerCommandGet;
				udpcmd->DataLen = 0;
				Sendudp(udpcmd);
			}
				break;
//			case '9':
//				udpcmd->Cmd= e_CmdK10_KeyboardGetKey;
//				udpcmd->DataLen=0;
//				Sendudp(udpcmd);
//				break;
			case 'k':
				udpcmd->Cmd = e_CmdK10_Jump2Loader;
				udpcmd->DataLen=0;
				Sendudp(udpcmd);
				break;
			case 'l':
				udpcmd->Cmd = e_CmdK10_Jump2App;
				udpcmd->DataLen=0;
				Sendudp(udpcmd);
				break;
			case 'm':
				udpcmd->Cmd = e_CmdK10_DownLoad;
				udpcmd->DataLen=0;
				Sendudp(udpcmd);
				break;
			case 'r':
				udpcmd->Cmd = e_CmdK10_SensorCommandGet;
				udpcmd->DataLen=0;
				Sendudp(udpcmd);
				break;
			case 'a':
			{
				res = TR_InitReader(&InitRes);
				printf("TR_InitReader res = %d\n", res);
				break;
			}
			case 'b':
			{
				//TR_SetParam(const TR_st_Parameters* pParams)
				TR_st_Parameters Params = {0};
				Params.lv_DeviceNumber = 111;
				Params.lv_StoredValueCeiling = 40000;
				Params.lv_time_zone_bias_minutes = 120;
				Params.uc_ProviderId = 14;
				//Params.us_BusLineNumber = 12345;
				Params.us_EndOfServiceHour = 0;
				Params.us_EveningStartHour = 0;
				Params.us_MorningEndHour = 0;
				Params.us_ReuseLimitInMinutes = 0;
				Params.us_StoredValuePredefineCode = 200;
				res = TR_SetParam(&Params);
				printf(" res = %d\n", res);

				break;
			}

			case 'c':
			{
				int var;
				//TR_IsCardIn(TR_BOOL* cardIn, TR_st_CardInfo* pInfo);
				TR_st_CardInfo Info = {0};
				TR_BOOL cardIn = TR_FALSE;
				res = TR_IsCardIn(&cardIn, &Info);
				printf(" res = %d  CardIn=%d \n", res, cardIn);
				if(cardIn)
				{
					char sn[21]= {0};
					memcpy(sn, Info.m_serialNumber, sizeof(Info.m_serialNumber));
					printf("IsEnvOk=%d sn=%s\n", Info.IsEnvOk, sn);
				}

				break;
			}

			case 'd':
			{
				//TR_ForgetCard(void);
				res = TR_ForgetCard();
				printf(" res = %d \n", res);
				break;
			}

			case 'e':
			{
				//TR_GetEnvironmentData(TR_st_EnvironmentData* pEnv)
				TR_st_EnvironmentData Env;
				res = TR_GetEnvironmentData(&Env);
				printf("res = %d\n", res);
				break;
			}

			case 'f':
			{
				//TR_GetListForReportAndReload(TR_st_AllContracts* array/*out*/)
				TR_st_AllContracts array;
				res = TR_GetListForReportAndReload(&array);
				printf("res = %d\n", res);
				break;
			}

			case 'g':
			{
				//TR_GetListForUse(TR_st_ContractsForUseResponse *ContractData);
				TR_st_ContractsForUseResponse ContractData;
				res = TR_GetListForUse(&ContractData);
				printf("res = %d\n", res);
				break;
			}

			case 'h':
			{
				//TR_IsPossibleLoad(const TR_St_LoadContract* pLoadData);
				TR_St_LoadContract LoadData = {0};
				FillContractForLoad(&LoadData);
				res = TR_IsPossibleLoad(&LoadData);
				printf("res = %d\n", res);
				break;
			}

			case 'i':
			{

				//TR_Load(const TR_St_LoadContract*, TR_St_LoadContractResponse*);
				TR_St_LoadContract LoadData = {0};
				TR_St_LoadContractResponse loadRes = {0};
				FillContractForLoad(&LoadData);
				res = TR_Load(&LoadData, &loadRes);
				printf("res = %d\n", res);
				break;
			}

			case 'j':
			{
				TR_St_UseContractData in = {0};
				TR_St_UseContractResponse out={0};
				//in.m_ClusterNumber = 140;
				in.m_Code = 1;
				in.m_Contract_index_on_the_card = 0;
				in.m_CurrAzmash = 255;
				in.m_IsFirstTrip = 1;
				in.m_PassengerCount = 1;
				in.m_SegmentForHemshech = 0;
				in.m_StationNumber = 50123;
				in.m_StoredValueSum = 0;
				in.m_TokensCounter = 1;//1 if kartisia
				res = TR_Use(&in, &out);
				printf("res = %d\n", res);
				break;
			}

			case 't':
			{
				int i;
				TR_St_UseContractData in = {0};
				TR_St_UseContractResponse out={0};
				//in.m_ClusterNumber = 140;
				in.m_Code = 1;
				in.m_Contract_index_on_the_card = 0;
				in.m_CurrAzmash = 255;
				in.m_IsFirstTrip = 1;
				in.m_PassengerCount = 1;
				in.m_SegmentForHemshech = 0;
				in.m_StationNumber = 50123;
				in.m_StoredValueSum = 0;
				in.m_TokensCounter = 0;//1 if kartisia
				for(i=0;i<10000;i++){
					TR_BOOL cardIn;
					TR_st_CardInfo Info = {0};

					St_K10_LedCommandSet pSt_K10_LedCommandSet;
					printf("iteration %d\n, i");

					pSt_K10_LedCommandSet.LedBlockId = e_BothLed;
					pSt_K10_LedCommandSet.Red = e_LedOff;
					pSt_K10_LedCommandSet.Green = e_LedOff;
					pSt_K10_LedCommandSet.Yellow = e_LedOff;
					udpcmd->Cmd= e_CmdK10_LedCommandSet;
					udpcmd->DataLen=sizeof(pSt_K10_LedCommandSet);
					memcpy(udpcmd->Data,&pSt_K10_LedCommandSet,sizeof(pSt_K10_LedCommandSet));
					Sendudp(udpcmd);
                    usleep(100000);
					res = TR_IsCardIn(&cardIn, &Info);
					printf(" res = %d  CardIn=%d \n", res, cardIn);
					cardIn = 1;//doron for test
					if(cardIn)
					{
						udpcmd->Cmd= e_CmdK10_CheckComm;
						udpcmd->DataLen=0;
						Sendudp(udpcmd);
						usleep(100000);
						pSt_K10_LedCommandSet.LedBlockId = e_BothLed;
						pSt_K10_LedCommandSet.Red = e_LedOff;
						pSt_K10_LedCommandSet.Green = e_LedOff;
						pSt_K10_LedCommandSet.Yellow = e_LedOn;
						udpcmd->Cmd= e_CmdK10_LedCommandSet;
						udpcmd->DataLen=sizeof(pSt_K10_LedCommandSet);
						memcpy(udpcmd->Data,&pSt_K10_LedCommandSet,sizeof(pSt_K10_LedCommandSet));
						Sendudp(udpcmd);
						usleep(100000);

						res = TR_Use(&in, &out);
						printf("res = %d\n", res);

						pSt_K10_LedCommandSet.LedBlockId = e_BothLed;
						if(res == 1)
						{
							pSt_K10_LedCommandSet.Red = e_LedOff;
							pSt_K10_LedCommandSet.Green = e_LedOn;
							pSt_K10_LedCommandSet.Yellow = e_LedOff;
						}
						else
						{
							pSt_K10_LedCommandSet.Red = e_LedOn;
							pSt_K10_LedCommandSet.Green = e_LedOff;
							pSt_K10_LedCommandSet.Yellow = e_LedOff;
						}
						udpcmd->Cmd= e_CmdK10_LedCommandSet;
						udpcmd->DataLen=sizeof(pSt_K10_LedCommandSet);
						memcpy(udpcmd->Data,&pSt_K10_LedCommandSet,sizeof(pSt_K10_LedCommandSet));
						Sendudp(udpcmd);
						usleep(100000);
					}
					res = TR_ForgetCard();
					sleep(1);

				}
				printf("res = %d\n", res);
				break;
			}
			//todo
			//TR_IsPossibleCancel(const IsPossibleCancel* pIsPossible);
			//TR_CancelOp(const TR_St_CancelData*, TR_St_TransactionData*);
			//TR_GetWhiteContractList(TR_St_WhiteContractList* pStContracts);
			//TR_GetInspectorCardData(TR_st_UserData* pUserData);
			//TR_GetAllEventsForReport(TR_St_EventForReport EventsArr[MAX_EVENT_FOR_REPORT]);
			//TR_GetSamData(TR_St_SamData* pSamCounter);
			//TR_ReadAllCardBinData(TR_CardBinData* pBuf);
			case 'x':
				udpcmd->Cmd= 101;
				udpcmd->DataLen=0;
				Sendudp(udpcmd);
				break;
			case 'z':
				res =  DEV_InitDCU( &Initdcu );

				DCUDataCB.p_ApplicationError = res;

				break;
			case 'v'://LED on off
			{

				LedSetup SandDataLed;
				udpcmd->Cmd= e_BSPKeyBoardled;
				SandDataLed.PWMratio = PWMratio;
				udpcmd->DataLen = sizeof(LedSetup);
				memcpy( udpcmd->Data, &SandDataLed, udpcmd->DataLen);
				Sendudp(udpcmd);
				PWMratio+=50;
				if (PWMratio >= 255)
				{
					PWMratio=0;
				}

			}
				break;
			case 'q'://GPIO test
			{
				int c,gpio=0,value=0;
				GPIOSetup sGPIOSetup;
				udpcmd->Marker1= MARKER1;
				udpcmd->Marker2= MARKER1;
				int n;
				for(;;)
				{
					printf("input GPIO for Test  q for exit\n\r");
					n = scanf("%d", &c);
					if (n == 1)
					{
					    printf("GPIO: %d\n\r", c);
					    gpio = c;
					    printf("input :1  outpot :2\n\r");
					    n = scanf("%d", &c);
					    if (n == 1)
					    {
					    	if (c==1)
					    	{
					    	   printf("input GPIO:%d\n",gpio);
					    	   udpcmd->Cmd = e_BSPGPIO_9;//for input
							   sGPIOSetup.gpio_num = gpio;
							   sGPIOSetup.value = 0;
							   udpcmd->DataLen = sizeof(sGPIOSetup);
							   memcpy(udpcmd->Data , &sGPIOSetup,udpcmd->DataLen);
							   Sendudp(udpcmd);
					    	}
					    	if (c==2)
					    	{
					    	   printf("outpot \n\r");
					    	   printf("input value\n");
							   n = scanf("%d", &c);
							   value = c;
							   if (n == 1)
							   {
								   printf("GPIO: %d value: %d \n\r",gpio,value);
								   udpcmd->Cmd = e_BSPGPIO_1;
								   sGPIOSetup.gpio_num = gpio;
								   sGPIOSetup.value = value;
								   udpcmd->DataLen = sizeof(sGPIOSetup);
								   memcpy(udpcmd->Data , &sGPIOSetup,udpcmd->DataLen);
								   Sendudp(udpcmd);
							   }
					    	}
					    }
					}
					else
					{
						if (n == 0)
						{
							break;
						}
					}
				}

			}
			break;
			case '99':
				datalen = sprintf(buffer,"version 1.2.3.4.5.6");
				Glblp_Resurce.p_handle = uartfd;
				ComResult = e_SendMessage( Glblp_Resurce.p_handle, 15, e_ReqTypeRespond,0,0, datalen+1, buffer,(datalen * 10));
//				ComResult = e_SendMessage(Glblp_Resurce.p_handle, //[IN]
//										e_CmdK10_ClyApp_Test, //[IN]
//										e_ReqTypeRequest,//[IN]
//										0,
//										0,//[IN]
//										sizeof(event_data),//[IN]
//										event_data,//[IN]
//										0) != 0 )


				break;
			default:
				break;
		}
		usleep(20000);
	}


   return EXIT_SUCCESS;
}
