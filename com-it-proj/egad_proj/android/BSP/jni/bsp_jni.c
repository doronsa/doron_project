#include <string.h>
#include <jni.h>
#include <sii_api.h>
#include <stdio.h>
#include <android/log.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <time.h>
#include "gpio_util.h"
#include "GPSDriver.h"

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


#include <time.h>
#include <stdbool.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <os_porting.h>

#include "Calypso/ClyAppApi.h"
#include "P_TwMtrBase.h"
#include "TW_K10232_P.h"
#include "BSPutil.h"

extern st_InitResource InitRes;
#define FALSE 0
#define TRUE 1
#define OK 0

//extern st_InitDCU Initdcu;
#define  LOG_TAG    "BSP"

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

int StartUDPThread = 0;
void *udp_message_function( void *ptr );
int StartUDPTrhad( void );
//pthread_t pUDPcomThread;
int UDPPort = 4444;
#define CLIENTPORT 4445
#define BUFLEN 600
#define SRV_IP "127.0.0.1"
#define NPACK 10
#define MAX_BUFF_LEN 10
#define MARKER1 55
#define UDB_BUFFER 1000
#define e_CmdK10_TimeOut_Error 40
#define BYTE 1
#define WORD 2
#define DWORD 4
unsigned char global_status;
//lock manage
pthread_mutex_t lock;
char send_buffer[BUFLEN];
int wait_flag=TRUE;
//void signal_handler_IO (int status);
pthread_t pUDPcomThread, pKeyBoardTrhad ,pK10threadRecive;

struct sigaction saio;
int connected;
char send_buffer[BUFLEN];
int uart_fd;
int load_version = 0;//global
St_K10_GetAppVersion getAppVersionGlobal;
typedef struct
{
	int PWMratio ;
	int current;
	int temp;
}LedSetup;

//typedef struct
//{
//	int gpio_num ;
//	int value;
//	int SendOK;
//	int cmd;
//}GPIOSetup;

typedef struct
{
	int SendOK;
	int time;
	int keynum;
}SandKeyToKGUI;


print_buffer(char *buffer, int len)
{
	int var = 0;
	for (var = 0; var < len; ++var)
	{
		LOGI("buffer %d %x",var,buffer[var]);
	}
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

int Sendudp(UDPcommand * dataGUI);

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
 typedef struct  __java_env{
	JNIEnv* jenv;
	jclass cls;
	jmethodID methodid;
	JavaVM* g_JavaVM ;
}   _java_env;

 _java_env java_env;

 typedef struct
 {
 	//PROCe_Tr1020ExchangeData Proc;
// 	CHANNEL_HANDLER p_handle;

 }st_PrimaryProtocolResource ;

 st_PrimaryProtocolResource Glblp_Resurce;
 int clypsoInit(void);
 int ReaderExchangeData(void* pHandlerr,//[IN]
 		int i_cmd,//[IN] the command
 		int i_TimeOutMs,//[IN] // the time out
 		int i_ObjectInSize,//[IN]// the data size to send
 		void *p_ObjectIn,//[IN] the data
 		int i_ObjectOutSizeReq,//[IN] // the data respond size except to
 		void *p_ObjectOut,//[OUT]// the data return
 		int *p_OutSizeArive,//[IN]// the size of data respond
 		unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
 		int *p_ApplicationError); //[OUT]// the application respond
// int RemoteCall(int Cmd,int Timeout,int OutDataSize, void *pOutData,int InExpectedSize,void *pInData);
 TR_BOOL TimeDataCallback(TR_St_DateTime* DateTime /*out*/);
 void SendMsgT(_java_env* java_env1, unsigned char cmd,int stat, int *sttypes, int typelen,void *value, int datalen);

 //void * demoUDP(void* msg );
 //void send_time_out(void);
 jmethodID methodid;
 JNIEnv* jenv;
 jclass cls;
 pthread_t tw ,Ttime;


 //#######################################################################
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
//########################################################################
 int ReaderExchangeData(void* pHandlerr,//[IN]
  		int i_cmd,//[IN] the command
  		int i_TimeOutMs,//[IN] // the time out
  		int i_ObjectInSize,//[IN]// the data size to send
  		void *p_ObjectIn,//[IN] the data
  		int i_ObjectOutSizeReq,//[IN] // the data respond size except to
  		void *p_ObjectOut,//[OUT]// the data return
  		int *p_OutSizeArive,//[IN]// the size of data respond
  		unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
  		int *p_ApplicationError) //[OUT]// the application respond
  {
		UDPcommand *udpcmd = (UDPcommand*)malloc(sizeof(UDPcommand));
	  	udpcmd->Marker1 = MARKER1;
	  	udpcmd->Marker2 = MARKER1;
	  	udpcmd->Cmd = i_cmd;
	  	udpcmd->DataLen = i_ObjectInSize ;

	  	if (i_ObjectInSize > 0)
	  	{
	  		memcpy(udpcmd->Data,p_ObjectIn,udpcmd->DataLen);
	  	}
	  	ExchangeDataCB.Cmd = i_cmd;
	  	ExchangeDataCB.seand_data = 1;
	  	Sendudp(udpcmd);
	  	//i_TimeOutMs = 10;
  	    start_timer(i_TimeOutMs);
	  	LOGI("*******  From ExchangeData get cmd =%d Size =%d  Return Size %d App ret%d TimeOut:%d************\n",
	  	  			                      i_cmd,udpcmd->DataLen,ExchangeDataCB.datalen ,*p_ApplicationError,i_TimeOutMs);

  	//usleep(100);
  	free(udpcmd);
  	while(1)
  	{
  	  		if (ExchangeDataCB.recive_data == 1)
  	  		{
				LOGI("*******  ReaderExchangeData recive_data cmd =%d Send Size =%d  Return Size %d App ret%d************",
												i_cmd,i_ObjectInSize,ExchangeDataCB.datalen ,*p_ApplicationError);
				pthread_mutex_lock(&lock);
				if (ExchangeDataCB.datalen>0)
				{
					memcpy(p_ObjectOut,(void*)ExchangeDataCB.Data,ExchangeDataCB.datalen);
					//print_buffer(p_ObjectOut,ExchangeDataCB.datalen);
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
					LOGI("*******  ReaderExchangeData TimeOut cmd =%d ************\n", i_cmd);
					pthread_mutex_lock(&lock);
					ExchangeDataCB.recive_data = 0;
					ExchangeDataCB.seand_data = 0;
					ExchangeDataCB.Cmd = 0;
					*p_ApplicationError = 0;
					pthread_mutex_unlock(&lock);
					stop_timer();
					return 0;
				}
			}
  	  		usleep(100);
  	  	}
  	return 0;
  }

//#############################################################################
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
 //#########################################################################
// void send_time_out( void )
// {
//	 unsigned char cmd = e_CmdK10_TimeOut_Error;
//	 _java_env* env2 = ( _java_env*)(msg);
//	 int stat = 0;
//	 int type_len = 0;
//	 int data_len = 0;
//	 SendDataToJava(env2, cmd, stat, type, type_len, NULL, data_len);
// }

//#########################################################################
 void timer_handler ( int sig )
 {
 	//static int count = 0;
	pthread_mutex_lock(&lock);
 	stop_timer();
 	if (ExchangeDataCB.seand_data == 1)
 	{
 		//LOGI("timer expired %d times\n", sig);
 		ExchangeDataCB.TimeOut = 1;
 	}
 	pthread_mutex_unlock(&lock);
 	LOGI("****** timer expired times ********");
 	//send_time_out( msg );

 }
 //########################################################################
 int clypsoInit(void)
 {
	int res = 0;
	void (*ExchangeData)(int);
	void (*TimeDataCB)(int);
	ExchangeData = &ReaderExchangeData;
	TimeDataCB = &TimeDataCallback;
	typedef int (*some_func)(char *param);
	InitRes.ProtocolCB = ExchangeData;
	InitRes.TimeAndDateCB = TimeDataCB;
	InitRes.pHandler = 1;
	res = TR_InitReader( &InitRes );
	LOGI("TR_InitReader res:%d",res);
	return res;
 }
 //

 ///////////////////////////////////////////////////////////////////////
 JNIEXPORT jobjectArray JNICALL
 Java_com_commit_bsp_BSPUtils_getGPSData(JNIEnv *env, jobject jobj)
 {
	 LOGE("GET_GPS_DATA ");
	 char* strs[6],name[50];
	 int data_len = 6,cmd = 250,type[6];
	 GPSdata gData;
	 memset(&gData,0,sizeof(gData));
	 GetGPSLocation( &gData);
	 //LOGE("GET_GPS_DATA_NEW lat:%s lat_s:%s lng:%s lng_s:%s date:%s st:%s", gData.lat, gData.lat_s, gData.lng, gData.lng_s, gData.date, gData.st_num);
	 jobjectArray ret;
     int i;
     int len = 6;
     jclass objectClass =  (*env)->FindClass(env, "java/lang/Object");
      jobjectArray data =  (*env)->NewObjectArray(env, len, objectClass, 0);

     ret= (jobjectArray)(*env)->NewObjectArray(env,len,
              (*env)->FindClass(env,"java/lang/String"),
              (*env)->NewStringUTF(env,""));
     (*env)->SetObjectArrayElement(env, ret,0,(*env)->NewStringUTF(env,gData.lat));
     (*env)->SetObjectArrayElement(env, ret,1,(*env)->NewStringUTF(env,gData.lat_s));
     (*env)->SetObjectArrayElement(env, ret,2,(*env)->NewStringUTF(env,gData.lng));
     (*env)->SetObjectArrayElement(env, ret,3,(*env)->NewStringUTF(env,gData.lng_s));
     (*env)->SetObjectArrayElement(env, ret,4,(*env)->NewStringUTF(env,gData.date));
     (*env)->SetObjectArrayElement(env, ret,5,(*env)->NewStringUTF(env,gData.st_num));
     return(ret);
   }

 //########################################################################
JNIEXPORT int Java_com_commit_bsp_BSPUtils_sendCMD(JNIEnv *env, jobject object, jobjectArray prdctini)
{
	    const char *param[20];
        jstring stringArray[10];
        const char send_buffer[600];
        UDPcommand *udpcmd = (UDPcommand*)malloc(sizeof(UDPcommand));
		udpcmd->Marker1= MARKER1;
		udpcmd->Marker2= MARKER1;
        jsize stringCount = (*env)->GetArrayLength(env,prdctini);
        int i;
        int stat = -2;



		 if(stringCount == 0)
		 {
		    return -1;
		 }

         for ( i=0; i<stringCount; i++)
         {
                 stringArray[i] = (jstring) (*env)->GetObjectArrayElement(env, prdctini, i);
                 param[i] = (*env)->GetStringUTFChars(env, stringArray[i], NULL);
         }

         if(!strcmp(param[0], "PRINTER_INIT"))
         {
        	     LOGE("PRINTER_INIT");
        	     char **argv = (char *[]){"util", "0"};
                 stat = util(2,argv);
                 LOGE("PRINTER_INIT (%x)",stat);
         }

         else if(!strcmp(param[0], "PRINT_IMAGE"))
         {
         	  char* strs[3];
              strs[0] = "util";
	          strs[1] = "3";
              strs[2] = param[1];
              LOGE("PRINT_IMAGE %s" ,strs[2]);
              stat = util(3,strs);
              LOGE("PRINT_IMAGE (%x)",stat);
         }
         else if(!strcmp(param[0], "GPIO_VALUE"))
		 {

			  int gpio = atoi(param[1]);
			  LOGE("GET_GPIO_VAL %d" ,gpio);
			  stat = GetGpioValue(gpio);
			  LOGE("GET_GPIO_VAL (%d) value (%d)",gpio,stat);
		 }
         else if(!strcmp(param[0], "GET_GPS_DATA"))
		 {
        	  LOGE("GET_GPS_DATA ");
        	  char* strs[6],name[50];
        	  int data_len = 6,cmd = 250,type[6];
        	  GPSdata gData;
        	  memset(&gData,0,sizeof(gData));
        	  GetGPSLocation( &gData);
        	  LOGE("GET_GPS_DATA lat:%s lat_s:%s lng:%s lng_s:%s date:%s st:%s", gData.lat, gData.lat_s, gData.lng, gData.lng_s, gData.date, gData.st_num);




		 }
         else if(!strcmp(param[0], "K10_TEST_COM"))
		 {

			  LOGE("K10_TEST_COM ");
			  udpcmd->Cmd = e_CmdK10_CheckComm;
			  udpcmd->DataLen = 0;
			  Sendudp(udpcmd);
		 }
         else if(!strcmp(param[0], "PRINTER_CUT"))
         {
        	     LOGE("PRINTER_CUT");
        	     char **argv = (char *[]){"util", "4"};
                 stat = util(2,argv);
                 LOGE("PRINTER_CUT (%x)",stat);
         }

         else if(!strcmp(param[0], "PRINTER_PARTIAL_CUT"))
		 {
				  LOGE("PRINTER_PARTIAL_CUT");
				  char **argv = (char *[]){"util", "4" , "1" , "100"};
				  stat = util(2,argv);
				  LOGE("PRINTER_CUT (%x)",stat);
         }

         else if(!strcmp(param[0], "PRINTER_TEST"))
         {
        	     LOGE("PRINTER_TEST");
        	     char **argv = (char *[]){"util", "1"};
                 stat = util(2,argv);
                 LOGE("END PRINTER_TEST (%x)",stat);
         }
         else if(!strcmp(param[0], "PRINTER_STATUS"))
		 {
				 LOGE("PRINTER_STATUS");
				 char **argv = (char *[]){"util", "5"};
				 stat = util(2,argv);
				 LOGE("END PRINTER_STATUS (%x)",stat);
		 }

         else if(!strcmp(param[0], "MODEM_CONNECT"))
         {
        	   LOGE("IN MODEM_CONNECT");
               stat = system("/system/bin/sh /etc/ppp/ppp-connect");
			   LOGE("MODEM_CONNECT (%d)",stat);
         }
         else if(!strcmp(param[0], "MODEM_DISCONNECT"))
         {
        	  LOGE("IN MODEM_DISCONNECT");
        	  stat = system("/system/bin/sh /etc/ppp/ppp-disconnect");
        	  LOGE("MODEM_DISCONNECT (%d)",stat);
         }
         else if(!strcmp(param[0], "MODEM_STATUS"))
         {
        	  LOGE("MODEM_STATUS");
			  stat = system("/system/bin/sh  /etc/ppp/ppp-check");
			  LOGE("MODEM_STATUS (%d)",stat);
         }

         else if(!strcmp(param[0], "MODEM_RSSI"))
		 {
			  LOGE("MODEM_STATUS");
			  stat = system("/system/bin/sh  /etc/ppp/getRssi");
			  LOGE("MODEM_STATUS (%d)",stat);
         }

         else if(!strcmp(param[0], "DRIVER_MODULE_INIT"))
         {
                stat = 0;
         }
         else if(!strcmp(param[0], "GO_TO_LOW_MODE"))
		 {
			 LOGE("GO_TO_LOW_MODE");
			 stat = system("/system/bin/sh  /data/bsp/go_to_low_mod.sh");
			 LOGE("GO_TO_LOW_MODE (%d)",stat);
		 }
         else if(!strcmp(param[0], "GO_TO_HI_MODE"))
		 {
			 LOGE("GO_TO_HI_MODE");
			 stat = system("/system/bin/sh  /data/bsp/go_to_hi_mode.sh");
			 LOGE("GO_TO_HI_MODE (%d)",stat);
		 }

         for ( i=0; i<stringCount; i++)
         {
                  (*env)->ReleaseStringUTFChars(env, stringArray[i], param[i]);
         }

       if(udpcmd)
    	   free(udpcmd);
    return stat;
}



//#######################################################################
JNIEXPORT
	jstring
	JNICALL
	Java_com_example_hellojni_HelloJni_getMessageFromNative(
			JNIEnv *env,
			jobject callingObject)
	{
		return (*jenv)->NewStringUTF(jenv, "Native code rules!");
	}
//########################################################################




jint JNI_OnLoad(JavaVM* vm, void* reserved)
{

  if ((*vm)->GetEnv(vm, (void**) &jenv, JNI_VERSION_1_6) != JNI_OK)
    return -1;

  java_env.g_JavaVM=vm;
	GetUDPPortFromFile();
  return JNI_VERSION_1_6;
}

//########################################################################
jint JNI_UnLoad(JavaVM* vm, void* reserved){
	pthread_kill(tw, 0);
	//timeout_done();
	//pthread_kill(Ttime, 0);
	(*jenv)->DeleteLocalRef(jenv, methodid);
	(*vm)->DetachCurrentThread(vm);
	return 0;
}
void GetUDPPortFromFile(void)
{
    FILE *fileSave = fopen("/dev/daemon_soc.port", "r");
    char buf[10],str[10];
    if (fileSave == NULL)
    {
        printf("Error opening daemon_soc.txt file!\n");
        exit(1);
    }
    fgets( buf, 10, fileSave);
    fclose(fileSave);
    memcpy(str,buf,10);
    UDPPort = strtol(str,NULL,10);
    LOGI("port number :%d \n",UDPPort);
}

//########################################################################
int Sendudp(UDPcommand * dataGUI)//for gui data
{
	struct sockaddr_in si_other;

	int sockfd, i,n, slen=sizeof(si_other);
	//	printf("Start UDP send command to GUI \n");
	//	LOGE("Start UDP send command to GUI");
	LOGI("Sendudp cmd = %d timestamp = %ld",dataGUI->Cmd,(long)time(NULL));
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
		LOGE("UDP Sendudp - socket() error");
	}
	memset((char *) &si_other, sizeof(si_other), 0);
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(CLIENTPORT);
	if (inet_aton(SRV_IP, &si_other.sin_addr)==0)
	{
	  fprintf(stderr, "inet_aton() failed\n");
	  exit(1);
	}


	if (sendto(sockfd , dataGUI, BUFLEN, 0,(struct sockaddr *)&si_other, slen)< 0)
	{
		LOGE("UDP error sending command to GUI ");
	}
	else
	{
		LOGI("UDP sending command to GUI ");
	}

	close(sockfd);
	return 0;

}
//########################################################################
void SendDataToJava(void* msg, unsigned char cmd, void *data, unsigned int data_len , char *name)
{

	 int type_len = 0;
	 int type [200];
	 int var,stat=0;
	 _java_env* env2 = ( _java_env*)(msg);
	 if (StartUDPThread == 0)
	 {
		 StartUDPThread = 1;
		 (*(env2->g_JavaVM))->AttachCurrentThread(env2->g_JavaVM,&env2->jenv, NULL);
	 }
	 LOGI("command name :%s cmd:%d",name,cmd);
	 switch (cmd)
	 {
		case e_CmdK10_CheckComm:
		{
			type_len = 200;
			for (var = 0; var < type_len; ++var)
			{
				type[var] = BYTE;
			}

		}
			break;
		case e_CmdK10_SelfTest:
		{
			type_len = sizeof(St_K10_BITResult);
			for (var = 0; var < type_len; ++var)
			{
				type[var] = BYTE;
			}
		}
		break;
//		case e_CmdK10_KeyboardCommand:
//		{
//			type_len = 1;
//			type[0] = BYTE;
//		}
//		break;
//		case e_CmdK10_KeyboardGetKey:
//		{
//			type_len = sizeof(St_CmdK10_KeyboardGetKey);
//			for (var = 0; var < type_len; ++var)
//			{
//				type[var] = BYTE;
//			}
//		}
//		break;
		case e_CmdK10_PowerCommandGet:
		{
			int typetmp[]={DWORD,BYTE,BYTE,BYTE,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,BYTE,DWORD,BYTE};//St_K10_PowerCommandGet
			type_len = sizeof(St_K10_PowerCommandGet);
			for (var = 0; var < type_len; ++var)
			{
				if (typetmp[var] >DWORD )
					typetmp[var]=BYTE;
				else
				    type[var] = typetmp[var];
			}
		}
		break;
		case e_CmdK10_SensorCommandGet:
		{
			int typetmp[]={BYTE,BYTE,BYTE,BYTE,DWORD};
			type_len = 5;
			for (var = 0; var < type_len; ++var)
			{
				if (typetmp[var] >DWORD )
					typetmp[var]=BYTE;
				else
				    type[var] = typetmp[var];
			}
		}
		break;
		case e_CmdK10_PeriodicMonitorEvent:
		case e_CmdK10_PeriodicMonitorPoll:
		{

			int typetmp[220];
			type_len = 220;
			LOGI("e_CmdK10_PeriodicMonitorPoll %d ",type_len);
			///typetmp[0] = DWORD;
			for (var = 0; var < type_len; ++var)
			{
				//typetmp[var] = DWORD;
				typetmp[var]=BYTE;
				//var++;

			}
		}
		break;
		case e_CmdK10_GetAppVersion:
		{
			int typetmp[]={BYTE,WORD,WORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,BYTE,BYTE};
			St_K10_GetAppVersion *GetAppVersion = data;
			type_len = 15;
			for (var = 0; var < type_len; ++var)
			{
			    type[var] = typetmp[var];
			}
		}
		break;
		case e_CmdK10_GetRTCTime:
		{
			int typetmp[]={DWORD,BYTE,BYTE,BYTE,BYTE,BYTE};
			type_len = 6;
			for (var = 0; var < type_len; ++var)
			{
				if (typetmp[var] >DWORD )
					typetmp[var]=BYTE;
				else
				    type[var] = typetmp[var];
			}
		}
		break;

		case e_CmdK10_EventSensorChange:
		{
			int typetmp[]={BYTE,BYTE};
			type_len = 2;
			for (var = 0; var < type_len; ++var)
			{
				type[var] = typetmp[var];
			}
		}
		break;
//		case e_CmdK10_KeyBoardEvent:
//		{
//			int typetmp[]={BYTE,BYTE};
//			type_len = 2;
//			for (var = 0; var < type_len; ++var)
//			{
//				if (typetmp[var] >DWORD )
//					typetmp[var]=BYTE;
//				type[var] = typetmp[var];
//			}
//		}
//		break;
		case e_CmdK10_TimeOut_Error:
		{
			int typetmp[]={BYTE,BYTE};
			type_len = 0;
			data_len = 0;

		}
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
		{
			//test debug
			int typetmp[]={BYTE,BYTE,BYTE,BYTE,BYTE,BYTE,BYTE,BYTE,BYTE,BYTE,BYTE,BYTE,BYTE,BYTE};
			type_len = 14;
			data_len = 14;

		}
		break;
		case e_CmdK10_KeyBoardEvent:
		{
			int typetmp[]={DWORD,DWORD,DWORD};
			LOGI("e_CmdK10_KeyBoardEvent %d (%d %d %d)",type_len ,((int*)data)[0],((int*)data)[1],((int*)data)[2]);
			type_len = 3;
			data_len = type_len;
			for (var = 0; var < type_len; ++var)
			{
//				if (typetmp[var] >DWORD )
//					typetmp[var]=BYTE;
//				else
					type[var] = typetmp[var];

			}

		}
			break;
		case e_BSPGPIO_1:
		{
			int typetmp[]={DWORD,DWORD,DWORD,DWORD};
			LOGI("e_BSPGPIO_1 %d ",type_len);
			type_len = 4;
			data_len = type_len;
			for (var = 0; var < type_len; ++var)
			{
				if (typetmp[var] >DWORD )
					typetmp[var]=BYTE;
				else
					type[var] = typetmp[var];
			}

		}
			break;
		case e_BSP_GET_VERSION:
		{
			int typetmp[]={BYTE,BYTE,BYTE};
			LOGI("e_BSP_GET_VERSION %d ",type_len);
			type_len = 3;
			data_len = type_len;
			for (var = 0; var < type_len; ++var)
			{
				if (typetmp[var] >DWORD )
					typetmp[var]=BYTE;
				else
					type[var] = typetmp[var];
			}
		}
		break;
		default:
			return;
			
	}
	 type_len = data_len;
	LOGI("SendMsgT cmd %d len:%d",cmd,data_len);
	SendMsgT(env2, cmd, stat, type, type_len, data, data_len);
}
//########################################################################
int EncodeUDPData(void* msg, unsigned char *buffer)
{
	 UDPcommand *data =( UDPcommand*)buffer;
	 unsigned char var=0;
	 char* buf_ptr;
	 char name[100];
	 if (data->Marker1==MARKER1)
     {
		 if (data->Marker2 == MARKER1)
		 {
			 //pthread_mutex_lock(&lock);
			 switch (data->Cmd)
			 {
				case e_CmdK10_CheckComm:
				{
					St_K10_TestComm *TestComm = (St_K10_TestComm*)data->Data;
					sprintf(name,"e_CmdK10_CheckComm");
					buf_ptr = (St_K10_TestComm*)malloc(sizeof(St_K10_TestComm));
					for (var = 0; var < sizeof(St_K10_TestComm); ++var)
					{
						buf_ptr += sprintf(buf_ptr,"%hhu",TestComm->EchoTest[var]);
					}
					SendDataToJava(msg, data->Cmd,TestComm,ECHO_SIZE_TEST, name);
					free(TestComm);
				}
					break;

				break;
				case e_CmdK10_SetParam:
					break;
				case e_CmdK10_DisplayCommandGet:
				{
					St_K10_DisplayGet *DisplayGet = (St_K10_DisplayGet*)data->Data;
					sprintf(name,"e_CmdK10_DisplayCommandGet");
					SendDataToJava(msg, data->Cmd,DisplayGet, 70, name);
					free(DisplayGet);
				}
				break;

//				case e_CmdK10_KeyboardCommand://St_K10_KeyboardCommand//St_K10_KeyboardCommandResponse
//				{
//					St_K10_KeyboardCommandResponse *KeyboardCommandResponse = (St_K10_KeyboardCommandResponse*)data->Data;
//					sprintf(name,"e_CmdK10_KeyboardCommand ");
//					LOGI("Get St_K10_KeyboardCommandResponse %d ",KeyboardCommandResponse->Qsize);
//					SendDataToJava(msg, data->Cmd,KeyboardCommandResponse, sizeof(St_K10_KeyboardCommandResponse), name);
//					free(KeyboardCommandResponse);
//				}
//				break;
//				case e_CmdK10_KeyboardGetKey:
//				{
//					St_CmdK10_KeyboardGetKey *KeyboardGetKey = (St_CmdK10_KeyboardGetKey*)data->Data;
//					sprintf(name,"e_CmdK10_KeyboardGetKey");
//					SendDataToJava(msg, data->Cmd,KeyboardGetKey, sizeof(St_CmdK10_KeyboardGetKey), name);
//					free(KeyboardGetKey);
//				}
//				break;
				case e_CmdK10_PowerCommandGet:
				{
					St_K10_PowerCommandGet *PowerCommandGet = (St_K10_PowerCommandGet*)data->Data;
					sprintf(name,"e_CmdK10_PowerCommandGet");
					SendDataToJava(msg, data->Cmd,PowerCommandGet, 14, name);
					free(PowerCommandGet);
				}
				break;
				case e_CmdK10_SensorCommandGet:
				{
					St_K10_SensorCommandGet *SensorCommandGet = (St_K10_SensorCommandGet*)data->Data;
					sprintf(name,"e_CmdK10_SensorCommandGet");
					SendDataToJava(msg, data->Cmd,SensorCommandGet, 5, name);
					free(SensorCommandGet);
				}
				break;
				case e_CmdK10_PeriodicMonitorEvent:
				case e_CmdK10_PeriodicMonitorPoll:
				{
					St_K10_PeriodicMonitorPoll *PeriodicMonitorPoll = (St_K10_PeriodicMonitorPoll*)data->Data;
					sprintf(name,"e_CmdK10_PeriodicMonitorPoll");
					SendDataToJava(msg, data->Cmd,PeriodicMonitorPoll, 218, name);
					free(PeriodicMonitorPoll);
				}
				break;
				case e_CmdK10_GetAppVersion:
				{
					int var;
					int tmp[15];
					unsigned char buffer[50];
					St_K10_GetAppVersion *GetAppVersion = (St_K10_GetAppVersion*)data->Data;
					sprintf(name,"e_CmdK10_GetAppVersion");
					load_version = GetAppVersion->AppOrLoader;
					tmp[0] = GetAppVersion->AppOrLoader;
					tmp[1] = GetAppVersion->LoaderVersion;
					tmp[2] = GetAppVersion->AppVersion;
					tmp[3] = GetAppVersion->AppActualCrc32;
					tmp[4] = GetAppVersion->LoaderAppHeader.CRC32;
					tmp[5] = GetAppVersion->LoaderAppHeader.Length;
					tmp[6] = GetAppVersion->LoaderAppHeader.Offset;
					tmp[7] = GetAppVersion->LoaderAppHeader.Version;
					tmp[8] = GetAppVersion->LoaderAppHeader.Reserved[0];
					tmp[9] = GetAppVersion->LoaderAppHeader.Reserved[1];
					tmp[10] = GetAppVersion->LoaderAppHeader.Reserved[2];
					tmp[11] = GetAppVersion->LoaderAppHeader.LRC;
					tmp[12] = GetAppVersion->ProrotocolDataVer;




					SendDataToJava(msg, data->Cmd,tmp, 15, name);
					free(GetAppVersion);
				}
				break;
				case e_CmdK10_GetRTCTime:
				{
					St_K10_RTCTime *RTCTime = (St_K10_RTCTime*)data->Data;
					sprintf(name,"e_CmdK10_GetRTCTime");
					SendDataToJava(msg, data->Cmd,RTCTime, 6, name);
					free(RTCTime);
				}
				break;
				case e_CmdK10_EventSensorChange:
				{
					char tmp[2];
					int ver;
					St_K10_EventSensorChange *EventSensorChange = (St_K10_EventSensorChange*)data->Data;
					tmp[0] = EventSensorChange->SensorThatChane;
					tmp[0] = EventSensorChange->SensorState;
					sprintf(name,"e_CmdK10_EventSensorChange");
					SendDataToJava(msg, data->Cmd,tmp, 2, name);
					free(EventSensorChange);
				}
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
				{
					LOGI("*************** udp recive_data cmd:%d******************",data->Cmd);
					pthread_mutex_lock(&lock);
					if (ExchangeDataCB.Cmd == data->Cmd)
					{
						ExchangeDataCB.datalen = data->DataLen;
						if (data->DataLen > 0)
						{
							memcpy(ExchangeDataCB.Data,data->Data,data->DataLen);
						}
						ExchangeDataCB.recive_data = 1;
						LOGI("*************** recive_data ******************");
							//p_ApplicationError = &ExchangeDataCB.p_ApplicationError;
					}
					pthread_mutex_unlock(&lock);
				}
					break;
				case e_CmdK10_KeyBoardEvent:
				{

					SandKeyToKGUI *sSandKeyToKGUI = (SandKeyToKGUI*)data->Data;
					char tmp[3];
					LOGI("e_CmdK10_KeyBoardEvent cmd:%d key%d",data->Cmd,sSandKeyToKGUI->keynum);
					sprintf(name,"e_CmdK10_KeyBoardEvent");
					tmp[0]  = sSandKeyToKGUI->SendOK;
					tmp[1]  = sSandKeyToKGUI->time;
					tmp[2]  = sSandKeyToKGUI->keynum;
					SendDataToJava(msg, data->Cmd,tmp, 3, name);
					free(sSandKeyToKGUI);
				}
					break;
				case e_BSPGPIO_1:
				{
					char tmp[3];
					GPIOSetup *sSandGPIOGUI = (GPIOSetup*)data->Data;
					tmp[0] = sSandGPIOGUI->gpio_num;
					tmp[1] = sSandGPIOGUI->value;
					tmp[2] = sSandGPIOGUI->SendOK;
					tmp[3] = sSandGPIOGUI->cmd;
					LOGI("sSandGPIOGUI cmd:%d key%d",data->Cmd,sSandGPIOGUI->gpio_num);
					sprintf(name,"GPIOSetup");
					SendDataToJava(msg, data->Cmd,tmp, 4, name);
					free(sSandGPIOGUI);
				}
					break;
				case e_BSP_GET_VERSION:
				{
					char tmp[3];
					tmp[0] = data->Data[0];
					tmp[1] = data->Data[1];
					tmp[2] = data->Data[2];
					LOGI("e_BSP_GET_VERSION cmd:%d version %s",data->Cmd,tmp);
					sprintf(name,"BSP Get VERSION");
					SendDataToJava(msg, data->Cmd,tmp, 3, name);
				}
   				    break;
				default:
					break;
				}
 		 }
     }
	 else
	 {
		 LOGE("Get no valid Command from GUI to K10 Command \n");
	 }
	 if (data)
		 free(data);
	 return (1);
}

//########################################################################

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
	char addr[2];//="85170";
	char GuiCmd,DataGui;
	/* get a socket descriptor */

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		LOGE("UDP Sendudp - socket() error %d",sd);
	}
	else
		LOGD("UDP server - socket() is OK");

	LOGD("UDP server - try to bind...");
	/* bind to address */
	memset(&serveraddr, 0x00, serveraddrlen);
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(UDPPort);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if((rc = bind(sd, (struct sockaddr *)&serveraddr, serveraddrlen)) < 0)
	{
		LOGE("UDP server - bind() error");
	    close(sd);
	}
	else
		LOGD("UDP server - bind() is OK");
    usleep(2000);
    LOGD("UDP server - Listening...");
	/* Wait on client requests. */
	while (1)
	{

		 rc = recvfrom(sd, bufptr, buflen, 0, (struct sockaddr *)&clientaddr,(socklen_t*) &clientaddrlen);
		 if(rc < 0)
		 {
			 LOGE("UDP Server - recvfrom() error");
			 close(sd);
			 return (1);
		 }
		 else
		 {
			if (bufptr != NULL)
			{
				EncodeUDPData( ptr, (unsigned char*)bufptr );
			}
			else
			{
			 LOGE(" UDP Server - recvfrom() Get NULL pointer");
			}
	     }
	}

   close(sd);
   return (1);
}


//########################################################################
Java_com_commit_bsp_BSPUtils_initJNI( JNIEnv* env,jobject thiz )
{
	    __android_log_write(ANDROID_LOG_INFO, "BRIDGE", "START here");//Or ANDROID_LOG_INFO, ...
		  cls = (*env)->FindClass(env, "com/commit/bsp/BSPUtils");
		  if(!cls)
		  {
			  __android_log_write(ANDROID_LOG_ERROR, "BRIDGE", "NO CLASS FOUND!!");
			  return -1;
		  }
		  methodid = (*env)->GetStaticMethodID(env, cls, "GetMsg","(II[I[Ljava/lang/Object;)V");
		  if(!methodid) {
			  __android_log_write(ANDROID_LOG_ERROR, "BRIDGE", "NO METHOD FOUND!!");
			  return -1;
		  }

		  jenv = env;
		  java_env.jenv = env;
		  java_env.cls = cls;
		  java_env.methodid = methodid;

		  if (pthread_mutex_init(&lock, NULL) != 0)
		  {
			printf("\n mutex init failed\n");
			return (1);
		  }
/*************init cylipso *************/

		  void (*ExchangeData)(int);
		  void (*TimeDataCB)(int);
		  ExchangeData = &ReaderExchangeData;
		  TimeDataCB = &TimeDataCallback;
		  typedef int (*some_func)(char *param);
		  InitRes.ProtocolCB = ExchangeData;
		  InitRes.TimeAndDateCB = TimeDataCB;
		  InitRes.pHandler = 1;
		 // int res = TR_InitReader(&InitRes);
/*******************init DCU ******************************/
//		  void (*RemoteCallBeck)(int);
//		  RemoteCallBeck = &RemoteCall;
//		  typedef int (*some_func)(char *param);
//		  Initdcu.ProtocolCB = RemoteCallBeck;
/*************************************************/
		  int status;
		  pthread_create(&tw,NULL,udp_message_function,(void *)&java_env);
		  //start timer signal
		  signal(SIGALRM, timer_handler);
		  init_timer();
		 // pthread_create(&Ttime,NULL,timer_function,(void *)&java_env);
		  __android_log_write(ANDROID_LOG_ERROR, "BRIDGE", "Create Thread");

		  return 0 ;

}

/////////////////////////////////////////////////////////////////////////
int DownloadK10(void)
{
	UDPcommand *udpcmd = (UDPcommand*)malloc(sizeof(UDPcommand));
	udpcmd->Marker1= MARKER1;
	udpcmd->Marker2= MARKER1;
	udpcmd->Cmd = 17;//e_CmdK10_Jump2Loader;
	udpcmd->DataLen=0;
	Sendudp(udpcmd);
    sleep(1);
	udpcmd->Cmd = e_CmdK10_GetAppVersion;
	udpcmd->DataLen = 0;
	Sendudp(udpcmd);
	sleep(1);
	if (load_version==0xbb)
	{
		udpcmd->Cmd = 16;//e_CmdK10_DownLoad;
		udpcmd->DataLen=0;
		Sendudp(udpcmd);
	}
	else return 2;

	sleep(3);
	udpcmd->Cmd = 18;//e_CmdK10_Jump2App;
	udpcmd->DataLen=0;
	Sendudp(udpcmd);
	sleep(1);
	udpcmd->Cmd = e_CmdK10_GetAppVersion;
	udpcmd->DataLen = 0;
	Sendudp(udpcmd);
	sleep(1);
	if (load_version==0xaa)
		return 0;
	else return 5;


}


/////////////////////////////////////////////////////////////////////
void SendMsgT(_java_env* java_env1, unsigned char cmd,int stat, int *sttypes, int typelen,void *value, int datalen)
{
	 static int acc = 0;
	 acc++;
	 int var,var_data;
     char buffer [610];
     char *buf_ptr = value;
     int *ptypes = sttypes;
     jstring jstr;

     LOGI("Entering to SendMsgT type len:%d data len %d",typelen,datalen);
     jintArray types =(jintArray)(*(java_env1->jenv))->NewIntArray(java_env1->jenv,typelen);
     jint *_types = calloc(typelen, sizeof(jint));
     for (var = 0; var < typelen; ++var)
     {
    	 _types[var] = *ptypes;
    	 ptypes++;
	 }
     // stop timer
     stop_timer();

     (*(java_env1->jenv))->SetIntArrayRegion(java_env1->jenv,types,0,typelen,(jint*)_types);

     jclass objectClass = (*(java_env1->jenv))->FindClass(java_env1->jenv, "java/lang/Object");
     jobjectArray data = (*(java_env1->jenv))->NewObjectArray(java_env1->jenv, datalen, objectClass, 0);

     if ( typelen == datalen)
     {
    	 memset(buffer,0,10);
		 for (var = 0; var < datalen; ++var)
		 {
			 switch ( _types[var] )
			 {
				case BYTE:
					sprintf (buffer, "%hhx " , (unsigned char*)(*buf_ptr));
					break;
				case WORD:
					sprintf (buffer, "%hu " ,(unsigned short*) *buf_ptr);
					break;
				case DWORD:
					sprintf (buffer, "%d " , (unsigned long*)*buf_ptr);
					break;
				default:
					break;
			}

			 jstr = (*(java_env1->jenv))->NewStringUTF(java_env1->jenv, buffer);
			(*(java_env1->jenv))->SetObjectArrayElement(java_env1->jenv, data, var, jstr);
			//Clean buffer
			(*(java_env1->jenv))->DeleteLocalRef(java_env1->jenv,jstr);
			//LOGI("jni index:%d type :%d buffer:%s value: %hhx",var,_types[var],buffer,*buf_ptr);
			buf_ptr++;
		 }

     }
     else//for char array only
     {
    	 LOGE("SendMsgT error");
     }

    // LOGI("End Build string Msg ");

     (*(java_env1->jenv))->CallStaticVoidMethod(java_env1->jenv, java_env1->cls, java_env1->methodid,(int)cmd, stat,types,data);


     //Clean buffer
      (*(java_env1->jenv))->DeleteLocalRef(java_env1->jenv,types);
      (*(java_env1->jenv))->DeleteLocalRef(java_env1->jenv,data);
      (*(java_env1->jenv))->DeleteLocalRef(java_env1->jenv,objectClass);
//      free(buf_ptr);
//      free(ptypes);

  }
//########################################################################

JNIEXPORT int Java_com_commit_bsp_BSPUtils_SetMsg(JNIEnv *env, jobject object,jint id ,jint len ,jlongArray vals)
{
	int msg_id = (int) id;
	int msg_len = (int) len;
	jlong *data_array;
	long tmp1,tmp2;
	int stat=0,res=0;
	static int PWMratio = 0;
    UDPcommand *udpcmd = (UDPcommand*)malloc(sizeof(UDPcommand));
	udpcmd->Marker1= MARKER1;
	udpcmd->Marker2= MARKER1;

	// get a pointer to the array
	data_array = (*env)->GetIntArrayElements(env, vals, NULL);


	if (len > 0 && data_array == NULL)
	{
	return -1; /* exception occurred */
	}
    //LOGD("start timeout timer");
    //start_timer(300);//start time out timer
	switch (msg_id)
	{
		case e_CmdK10_GetAppVersion:
			LOGI("IN CMD_GET_APP_VERSION");
			tmp1 = (long)data_array[0] ;
			tmp2 = (long)data_array[1] ;
			LOGI("GET_APP_VERSION debug  cmd:%d len:%d val1:%d val2:%d",id,msg_len,tmp1,tmp2);
			udpcmd->Cmd = e_CmdK10_GetAppVersion;
			udpcmd->DataLen = 0;
			Sendudp(udpcmd);
			break;
		case e_CmdK10_CheckComm:
			LOGI("IN CMD_CHECKCOMM");
			udpcmd->Cmd = e_CmdK10_CheckComm;
			udpcmd->DataLen = 0;
			Sendudp(udpcmd);
			break;
		case e_CmdK10_PeriodicMonitorPoll:
        	 LOGI("IN e_CmdK10_PeriodicMonitorPoll");
			 udpcmd->Cmd = e_CmdK10_PeriodicMonitorPoll;
			 udpcmd->DataLen = 0;
			 Sendudp(udpcmd);
			break;
		case e_CmdK10_SetParam:
		{
			LOGI("IN CMD_SETPARAM msg_len:%d",sizeof(St_K10Param));
			udpcmd->Cmd = e_CmdK10_SetParam;
			udpcmd->DataLen = sizeof(St_K10Param);
			St_K10Param *K10Param =(St_K10Param*) malloc(sizeof(St_K10Param));
			K10Param->MonitorPolling = (K_BOOL)data_array[0] ;
			K10Param->MonitorPeriodInSec = (K_DWORD)data_array[1] ;
			K10Param->WatchDogTimeInSec = (K_DWORD)data_array[2] ;
			K10Param->AutoLowPowerManage = (K_BOOL)data_array[3] ;
			K10Param->IdleTimeForLowPowerInSec = (K_DWORD)data_array[4] ;
			K10Param->Time2ResetIfNoComunicationInSec = (K_DWORD)data_array[5] ;
			K10Param->SendAutomatEvent_DriverModule = (K_BOOL)data_array[6] ;
			K10Param->SendAutomatEvent_Switch = (K_BOOL)data_array[7] ;
			K10Param->SendAutomatEvent_PaperLevel = (K_BOOL)data_array[8] ;
			K10Param->SendAutomatEvent_PrinterDoor = (K_BOOL)data_array[9] ;
			K10Param->SupprtFanFunctions = (K_BOOL)data_array[10] ;
			K10Param->SupprtDriveModuleFunction = (K_BOOL)data_array[11] ;
			K10Param->SupprtKeyboardFunction = (K_BOOL)data_array[12] ;
			K10Param->SupprtBataryAndChargerMonitoring = (K_BOOL)data_array[13] ;
			K10Param->SupprtBataryAndChargerCommand = (K_BOOL)data_array[14] ;
			memcpy(udpcmd->Data, K10Param, udpcmd->DataLen);
			Sendudp(udpcmd);
			free(K10Param);
		}
		break;
		case e_CmdK10_DisplayCommandGet:
        	 LOGI("IN CMD_DISPLAYCOMMANDGET");
			 udpcmd->Cmd = e_CmdK10_DisplayCommandGet;
			 udpcmd->DataLen = 0;
			 Sendudp(udpcmd);
			 break;
		case e_CmdK10_DisplayCommandSet:
		{
			LOGI("IN CMD_SET Display");
#if 1
			//for testing
			//set the back Light
			St_K10_DisplaySet pSt_K10_DisplaySet;
			pSt_K10_DisplaySet.K10_DisplayId = e_K10_Dsp_ID_Passanger;
			pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_Set_BackGround;
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);
			usleep(100000);
			//set led Arrow
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			udpcmd->DataLen = sizeof(pSt_K10_DisplaySet);
			pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_SetArrowLed;
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);
			usleep(100000);
			//set led Ambient
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			pSt_K10_DisplaySet.DisplayCommand =e_K10_Dsp_SetAmbient;
			udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);
			usleep(100000);

			//printf("input GPIO:%d\n",86);
			GPIOSetup SandDataGPIO;
			udpcmd->Cmd= e_BSPGPIO_1;
			SandDataGPIO.gpio_num = 86;
			SandDataGPIO.value = 1;
			udpcmd->DataLen = sizeof(GPIOSetup);
			memcpy( udpcmd->Data, &SandDataGPIO, udpcmd->DataLen);
			Sendudp(udpcmd);
/*
			//clear led Arrow
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_ClrArrowLed;
			udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);
			usleep(100000);
			//clear led Ambient
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_ClrAmbient;
			udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);
			usleep(100000);


*/
			//set on the Display "test"
			//on position x=0 y=0
			char char_buff[]={0xA0,0};//0xA0 starting the Hebrew
			pSt_K10_DisplaySet.K10_DisplayId = e_K10_Dsp_ID_Passanger;
			pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_Set_WriteStringXY;
			//memcpy(pSt_K10_DisplaySet.StringXY,"test",5);
			//memcpy(pSt_K10_DisplaySet.StringXY,"test",5);
			memcpy(pSt_K10_DisplaySet.StringXY,char_buff,2);
			pSt_K10_DisplaySet.x = 0;
			pSt_K10_DisplaySet.y = 0;
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);
#else
			int var;
			udpcmd->DataLen = sizeof(St_K10_DisplaySet);
			St_K10_DisplaySet *DisplaySet =(St_K10_DisplaySet*) malloc(sizeof(St_K10_DisplaySet));
			DisplaySet->K10_DisplayId = (K_BYTE)data_array[0] ;
			DisplaySet->DisplayCommand = (K_BYTE)data_array[1] ;
			DisplaySet->CursorMode = (K_BYTE)data_array[2] ;
			for (var = 0; var < DSP_ROW_SIZE; ++var)
			{
				DisplaySet->StringXY[var] = (char)data_array[3+var] ;
			}
			DisplaySet->x = (K_BYTE)data_array[var] ;
			DisplaySet->y = (K_BYTE)data_array[var++] ;
			udpcmd->Cmd = e_CmdK10_DisplayCommandSet;
			memcpy(udpcmd->Data, DisplaySet, udpcmd->DataLen);
			Sendudp(udpcmd);
			free(DisplaySet);
#endif
		}
		break;
		//ther is no keyboard
//		case e_CmdK10_KeyboardCommandConfig://ther is no keyboard
//		{
//			LOGI("IN CMD_KEYBOARDCOMMANDCONFIG");
//			St_K10_KeyboardCommandConfig *KeyboardCommandConfig =(St_K10_KeyboardCommandConfig*) malloc(sizeof(St_K10_KeyboardCommandConfig));
//			udpcmd->DataLen = sizeof(St_K10_KeyboardCommandConfig);
//			udpcmd->Cmd = e_CmdK10_KeyboardCommandConfig;
//			KeyboardCommandConfig->KeyBoardMethodGet = (K_BYTE)data_array[0] ;
//			KeyboardCommandConfig->KeyBoardDebounc = (K_BYTE)data_array[1] ;
//			KeyboardCommandConfig->KeyBufferSize = (K_BYTE)data_array[2] ;
//			KeyboardCommandConfig->BytesPerSecondInBertsMode = (K_WORD)data_array[3] ;
//			KeyboardCommandConfig->DelayBitweenPress = (K_WORD)data_array[4] ;
//			memcpy(udpcmd->Data, KeyboardCommandConfig, udpcmd->DataLen);
//			Sendudp(udpcmd);
//		}
//		break;
//		case e_CmdK10_KeyboardCommand:
//		{
//			LOGI("IN CMD_KEYBOARDCOMMANDCONFIG");
//			St_K10_KeyboardCommand *KeyboardCommand =(St_K10_KeyboardCommand*) malloc(sizeof(St_K10_KeyboardCommand));
//	       	udpcmd->DataLen = sizeof(St_K10_KeyboardCommand);
//			udpcmd->Cmd = e_CmdK10_KeyboardCommand;
//			KeyboardCommand->KeyCmd = (K_BYTE)data_array[0] ;
//			memcpy(udpcmd->Data, KeyboardCommand, udpcmd->DataLen);
//			Sendudp(udpcmd);
//			free(KeyboardCommand);
//		}
//		break;
//		case e_CmdK10_KeyboardGetKey:
//		{
//			LOGI("IN CMD_KEYBOARDGETKEY");
//			udpcmd->Cmd = e_CmdK10_KeyboardGetKey;
//			udpcmd->DataLen = 0;
//			Sendudp(udpcmd);
//		}
//		break;
		case e_CmdK10_PowerCommandGet:
		{
			LOGI("IN CMD_POWERCOMMANDGET");
			udpcmd->Cmd = e_CmdK10_PowerCommandGet;
			udpcmd->DataLen = 0;
			Sendudp(udpcmd);
		}
		break;
		case e_CmdK10_PowerCommandSet:
		{
			LOGI("IN CMD_POWERCOMMANDSET");
//			St_K10_PowerCommandSet *PowerCommandSet =(St_K10_PowerCommandSet*) malloc(sizeof(St_K10_PowerCommandSet));
//			udpcmd->DataLen = sizeof(St_K10_PowerCommandSet);
//			udpcmd->Cmd = e_CmdK10_PowerCommandSet;
//			PowerCommandSet->Channel = (K_BYTE)data_array[0] ;
//			PowerCommandSet->Command = (K_BYTE)data_array[1] ;
//			//example
//			PowerCommandSet.Channel = e_BataryChannel;
//			PowerCommandSet.Command = e_ChannelOn;
			St_K10_PowerCommandSet pSt_K10_PowerCommandSet;
			pSt_K10_PowerCommandSet.Channel = e_BataryChannel;
			pSt_K10_PowerCommandSet.Command = e_ChannelOn;
			memcpy(udpcmd->Data, &pSt_K10_PowerCommandSet, udpcmd->DataLen);
			Sendudp(udpcmd);
			//free(PowerCommandSet);
		}
		break;
		case e_CmdK10_SensorCommandGet:
		{
        	 LOGI("IN CMD_SENSORCOMMANDGET");
			 udpcmd->Cmd = e_CmdK10_SensorCommandGet;
			 udpcmd->DataLen = 0;
			 Sendudp(udpcmd);
		}
		break;
		case e_CmdK10_SolenoidCommandSet:
		{
			LOGI("IN CMD_SOLENOIDCOMMANDSET");
			St_K10_SolenoidCommandSet *SolenoidCommandSet =(St_K10_SolenoidCommandSet*) malloc(sizeof(St_K10_SolenoidCommandSet));
			udpcmd->DataLen = sizeof(St_K10_SolenoidCommandSet);
			udpcmd->Cmd = e_CmdK10_SolenoidCommandSet;
			SolenoidCommandSet->SolenoidId = (K_BYTE)data_array[0] ;
			SolenoidCommandSet->SolenoidOperation = (K_BYTE)data_array[1] ;
			memcpy(udpcmd->Data, SolenoidCommandSet, udpcmd->DataLen);
			Sendudp(udpcmd);
			free(SolenoidCommandSet);

		}
		break;
//		case e_CmdK10_PrinterCommandSet:
//		{
//			LOGI("IN CMD_PRINTERCOMMANDSET");
//			St_PrinterCommandSet *PrinterCommandSet =(St_PrinterCommandSet*) malloc(sizeof(St_PrinterCommandSet));
//			udpcmd->DataLen = sizeof(St_PrinterCommandSet);
//			PrinterCommandSet->PrinterCommand = (K_BYTE)data_array[0] ;
//			PrinterCommandSet->CommandValue = (K_BYTE)data_array[1] ;
//			udpcmd->Cmd = e_CmdK10_PrinterCommandSet;
//			memcpy(udpcmd->Data, PrinterCommandSet, udpcmd->DataLen);
//			Sendudp(udpcmd);
//			free(PrinterCommandSet);
//		}
//		break;
		case e_CmdK10_LedCommandSet:
		{
			LOGI("IN CMD_e_CmdK10_LedCommandSet");
			St_K10_LedCommandSet pSt_K10_LedCommandSet;
			//pSt_K10_LedCommandSet.LedBlockId = (K_BYTE)data_array[0] ;
			//pSt_K10_LedCommandSet.Red = (K_BYTE)data_array[1] ;
			//pSt_K10_LedCommandSet.Green = (K_BYTE)data_array[2] ;
			//pSt_K10_LedCommandSet.Yellow = (K_BYTE)data_St_K10_LedCommandSet pSt_K10_LedCommandSet;
			pSt_K10_LedCommandSet.LedBlockId = e_BothLed;
			pSt_K10_LedCommandSet.Red = e_LedOn;
			pSt_K10_LedCommandSet.Green = e_LedOn;
			pSt_K10_LedCommandSet.Yellow = e_LedOn;
			udpcmd->Cmd= e_CmdK10_LedCommandSet;
			udpcmd->DataLen=sizeof(pSt_K10_LedCommandSet);
			memcpy(udpcmd->Data,&pSt_K10_LedCommandSet,sizeof(pSt_K10_LedCommandSet));
			Sendudp(udpcmd);
		}
		break;
//		case e_CmdK10_PeriodicMonitorPoll:
//		{
//	       	 LOGI("IN CMD_PERIODICMONITORPOLL");
//			 udpcmd->Cmd = e_CmdK10_PeriodicMonitorPoll;
//			 udpcmd->DataLen = 0;
//			 Sendudp(udpcmd);
//		}
		break;
		case e_CmdK10_DownLoad:
		{
			int var;
			udpcmd->DataLen = sizeof(St_K10_DownLoad);
			St_K10_DownLoad *K10_DownLoad =(St_K10_DownLoad*) malloc(sizeof(St_K10_DownLoad));
			LOGI("IN CMD_LEDCOMMANDSET");
			udpcmd->Cmd = e_CmdK10_DownLoad;
			K10_DownLoad->AppVersion = (K_WORD)data_array[0];
			K10_DownLoad->AppCRC32 = (K_DWORD)data_array[1];
			K10_DownLoad->PacketNum = (K_DWORD)data_array[2];
			K10_DownLoad->Offset = (K_DWORD)data_array[3];
			K10_DownLoad->PacketSize = (K_WORD)data_array[4];
			K10_DownLoad->IsLast = (K_BYTE)data_array[5];
			for (var = 0; var < MAX_BUFFER_SIZE_FOR_DOWNLOAD; ++var)
			{
				K10_DownLoad->Buffer[var] = (K_BYTE)data_array[5 + var];
			}

			memcpy(udpcmd->Data, K10_DownLoad, udpcmd->DataLen);
			Sendudp(udpcmd);
			free(K10_DownLoad);

		}
		break;
		case e_CmdK10_Jump2Loader:
		{
			LOGI("IN CMD_JUMP2LOADER");
			udpcmd->Cmd = e_CmdK10_Jump2Loader;
			udpcmd->DataLen = 0;
			Sendudp(udpcmd);
		}
		break;
		case e_CmdK10_Jump2App:
		{
			LOGI("IN CMD_JUMP2APP");
			udpcmd->Cmd = e_CmdK10_Jump2App;
			udpcmd->DataLen = 0;
			Sendudp(udpcmd);
		}
		break;
		case e_CmdK10_Set2LowPower:
		{
			LOGI("IN CMD_SETLOWPOWER");
			udpcmd->Cmd = e_CmdK10_Set2LowPower;
			udpcmd->DataLen = 0;
			Sendudp(udpcmd);
		}
		break;
		case e_CmdK10_ResetApp:
		{
			LOGI("IN CMD_RESETAPP");
			udpcmd->Cmd = e_CmdK10_ResetApp;
			udpcmd->DataLen = 0;
			Sendudp(udpcmd);
		}
		break;
		case e_CmdK10_GetRTCTime:
		{
			LOGI("IN CMD_GETTIME");
			udpcmd->Cmd = e_CmdK10_GetRTCTime;
			udpcmd->DataLen = 0;
			Sendudp(udpcmd);
		}
		break;
		case e_CmdK10_SetRTCTime:
		{
			LOGI("IN CMD_SETRTCTIME");
			St_K10_RTCTime *K10_RTCTime =(St_K10_RTCTime*) malloc(sizeof(St_K10_RTCTime));
			udpcmd->DataLen = sizeof(St_K10_RTCTime);
			udpcmd->Cmd = e_CmdK10_SetRTCTime;
			LOGI("IN CMD_LEDCOMMANDSET cmd %d",udpcmd->Cmd);
			K10_RTCTime->Year = (K_BYTE)data_array[0] ;
			K10_RTCTime->Month = (K_BYTE)data_array[1] ;
			K10_RTCTime->Day = (K_BYTE)data_array[2] ;
			K10_RTCTime->Hour = (K_BYTE)data_array[3] ;
			K10_RTCTime->Minutes = (K_BYTE)data_array[4] ;
			K10_RTCTime->Secondes = (K_BYTE)data_array[5] ;
			memcpy(udpcmd->Data, K10_RTCTime, udpcmd->DataLen);
			Sendudp(udpcmd);
			free(K10_RTCTime);
		}
		break;
		case 60:
		{
			int res = 0;
			LOGI("IN CMD_InitInterface");
			res = TR_InitReader( &InitRes );
			LOGI("TR_InitReader res:%d",res);

		}
		break;
		case 61:
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
		}
		break;
		case 62:
		{
			while(1)
			{
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
				sleep(3);
			}
			break;
		}

		case 63:
		{
			//TR_ForgetCard(void);
			res = TR_ForgetCard();
			printf(" res = %d \n", res);
			break;
		}

		case 64:
		{
			//TR_GetEnvironmentData(TR_st_EnvironmentData* pEnv)
			TR_st_EnvironmentData Env;
			res = TR_GetEnvironmentData(&Env);
			printf("res = %d\n", res);
			break;
		}

		case 65:
		{
			//TR_GetListForReportAndReload(TR_st_AllContracts* array/*out*/)
			TR_st_AllContracts array;
			res = TR_GetListForReportAndReload(&array);
			printf("res = %d\n", res);
			break;
		}

		case 66:
		{
			//TR_GetListForUse(TR_st_ContractsForUseResponse *ContractData);
			TR_st_ContractsForUseResponse ContractData;
			res = TR_GetListForUse(&ContractData);
			printf("res = %d\n", res);
			break;
		}

		case 67:
		{
			//TR_IsPossibleLoad(const TR_St_LoadContract* pLoadData);
			TR_St_LoadContract LoadData = {0};
			FillContractForLoad(&LoadData);
			res = TR_IsPossibleLoad(&LoadData);
			printf("res = %d\n", res);
			break;
		}

		case 68:
		{

			//TR_Load(const TR_St_LoadContract*, TR_St_LoadContractResponse*);
			TR_St_LoadContract LoadData = {0};
			TR_St_LoadContractResponse loadRes = {0};
			FillContractForLoad(&LoadData);
			res = TR_Load(&LoadData, &loadRes);
			printf("res = %d\n", res);
			break;
		}

		case 90:
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
		//todo
		//TR_IsPossibleCancel(const IsPossibleCancel* pIsPossible);
		//TR_CancelOp(const TR_St_CancelData*, TR_St_TransactionData*);
		//TR_GetWhiteContractList(TR_St_WhiteContractList* pStContracts);
		//TR_GetInspectorCardData(TR_st_UserData* pUserData);
		//TR_GetAllEventsForReport(TR_St_EventForReport EventsArr[MAX_EVENT_FOR_REPORT]);
		//TR_GetSamData(TR_St_SamData* pSamCounter);
		//TR_ReadAllCardBinData(TR_CardBinData* pBuf);


		case 92://LED on off
		{

			LedSetup SandDataLed;
			udpcmd->Cmd= e_BSPKeyBoardled;
			SandDataLed.PWMratio = PWMratio;//(int)data_array[0];
			udpcmd->DataLen = sizeof(LedSetup);
			memcpy( udpcmd->Data, &SandDataLed, udpcmd->DataLen);
			Sendudp(udpcmd);
			//**** for test  *****
			PWMratio+=50;
			if (PWMratio >= 255)
			{
				PWMratio=0;
			}
			LOGI("IN CMD Set Key board LED val  %d",PWMratio);

		}
		break;
		case 93://GPIO on off
		{
			GPIOSetup SandDataGPIO;
			udpcmd->Cmd= e_BSPGPIO_1;
			SandDataGPIO.gpio_num = (char)data_array[0];
			SandDataGPIO.value = (char)data_array[1];
			udpcmd->DataLen = sizeof(GPIOSetup);
			memcpy( udpcmd->Data, &SandDataGPIO, udpcmd->DataLen);
			Sendudp(udpcmd);
		}
		break;
		case 94://get gpio val
		{
			int data,type;
			data = GetGpioValue((char)data_array[0]);
			LOGI("IN CMD_Get GPIO %d Value %d",data , data_array[0]);

		}
			break;
		case 95:
		{
			LOGI("Start Download file to k10");
			int ret = DownloadK10();
			LOGI("Downlaod end ret:%d",ret);

		}
		break;
		case 100:
		{
			LOGI("IN CMD_SET Display Set_BackGround");
			//for testing
			//set the back Light
			St_K10_DisplaySet pSt_K10_DisplaySet;
			pSt_K10_DisplaySet.K10_DisplayId = e_K10_Dsp_ID_Passanger;
			pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_Set_BackGround;
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);

		}
		break;
		case 101:
		{
			//set led Arrow
			LOGI("IN CMD_SET Display Set led Arrow");
			St_K10_DisplaySet pSt_K10_DisplaySet;
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			udpcmd->DataLen = sizeof(pSt_K10_DisplaySet);
			pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_SetArrowLed;
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);

		}
		break;
		case 102:
		{
			//set led Ambient
			LOGI("IN CMD_SET Display Set led Ambient");
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			St_K10_DisplaySet pSt_K10_DisplaySet;
			pSt_K10_DisplaySet.DisplayCommand =e_K10_Dsp_SetAmbient;
			udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);
			usleep(100000);
		}
		break;
		case 103:
		{
			//clear led Arrow
			LOGI("IN CMD_SET Display clear led Arrow");
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			St_K10_DisplaySet pSt_K10_DisplaySet;
			pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_ClrArrowLed;
			udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);

			usleep(100000);
			//clear led Ambient
			LOGI("IN CMD_SET Display clear led Ambient");
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			//St_K10_DisplaySet pSt_K10_DisplaySet;
			pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_ClrAmbient;
			udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);
			usleep(100000);
			//
			GPIOSetup SandDataGPIO;
			udpcmd->Cmd= e_BSPGPIO_1;
			SandDataGPIO.gpio_num = 86;
			SandDataGPIO.value = 0;
			udpcmd->DataLen = sizeof(GPIOSetup);
			memcpy( udpcmd->Data, &SandDataGPIO, udpcmd->DataLen);
			Sendudp(udpcmd);

		}
		break;
		case 105:
		{
			//set on the Display "test"
			//on position x=0 y=0
			LOGI("IN CMD_SET Display set on the Display test");
			St_K10_DisplaySet pSt_K10_DisplaySet;
			pSt_K10_DisplaySet.K10_DisplayId = e_K10_Dsp_ID_Passanger;
			pSt_K10_DisplaySet.DisplayCommand = e_K10_Dsp_Set_WriteStringXY;
			memcpy(pSt_K10_DisplaySet.StringXY,"test",5);
			pSt_K10_DisplaySet.x = 0;
			pSt_K10_DisplaySet.y = 0;
			udpcmd->Cmd= e_CmdK10_DisplayCommandSet;
			udpcmd->DataLen=sizeof(pSt_K10_DisplaySet);
			memcpy(udpcmd->Data,&pSt_K10_DisplaySet,sizeof(pSt_K10_DisplaySet));
			Sendudp(udpcmd);

		}
		break;
		case 106:
			//
			udpcmd->Cmd= e_BSP_GET_VERSION;
			udpcmd->DataLen = 0;
			Sendudp(udpcmd);
		default:
			break;
	}
	// release the memory
	free(udpcmd);
	(*env)->ReleaseIntArrayElements(env, vals, data_array, 0);
	__android_log_write(ANDROID_LOG_ERROR, "BSP", "SetMsg");

    return 0;
}
