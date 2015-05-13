/*
 * P_TwMtrBase.h
 *
 *  Created on: Jan 20, 2014
 *      Author: doronsa
 */

#ifndef P_TWMTRBASE_H_
#define P_TWMTRBASE_H_

//-----------------------------------------
//		enums
//-----------------------------------------

#define MAX_CHANNELS 4
//#define MAX_Q_SIZE   5
// Protocole errors
typedef enum e_ProtocoleError
{
       eRT_OK = 0,				// correct
       eRT_TIMEOUT = 1,			// timeout
       eRT_WRONG_LEN_PARAM = 2, // len field is out of range
       eRT_WRONG_CHECKSUM=3,		// checksum is incorerct
	   eRT_UNKNOWN=4		// checksum is incorerct

} e_ProtocoleError;

// COM errors
typedef enum e_ComResult
{
 e_ComOk,
 e_ComTimeOut,
 e_ComInvalid,
 e_ComNotInit,
 e_ComCallbackMissing,
 e_ComInvalidHandler,
 e_ComTheFunctionNotFitForThisMode,
 e_ComLinkNack,
 eCom_Q_ReqFull,
 e_ComHandShakeTimeOut,
 e_ComDataLengthToBig,
 e_ComDataLengthMoreThenAllocatedSize,
 e_ComUnKnwon,
 e_ComInvalidArgument,
 e_ComNotImpl,
 e_ComInvalidArgument_Use_e_SendResponse_Instead
} e_ComResult;

typedef enum
{
	e_ReqTypeRequest=1,// the outher side must respond
    e_ReqTypeRespond=0, // respond to the request
    e_ReqTypeEvent=3,// the outher side does not must respond ,this request is not relavant for master/slave mode
}e_RequestType;
// Communication mode
typedef enum e_CommType				//+++approve this
{
	e_CommTransmit,
	e_CommReceive,
} e_CommType;


// COM errors
typedef enum e_OSMutex
{
    e_OSMutexInit,
    e_OSMutexLock,
    e_OSMutexRelease,
	e_OSMutexUnInit
} e_OSLock;

typedef enum
{
    eCoreError = 0,
    eCoreOK,

}CoreReturnVal;
//-----------------------------------------
//		vars
//-----------------------------------------
typedef void *COM_HANDLER;		// reference to the data
typedef void *CHANNEL_HANDLER;	// reference to the channel



//-----------------------------------------
//		callback functions
//-----------------------------------------

// open com user function
typedef COM_HANDLER  (*OPEN_COM_PROC)(int i_ComId,//[in]
						    e_ComResult *p_ComRes);//[out]

typedef e_ComResult (*START_ASYNC_LISTENING)(COM_HANDLER p_handler //[in]
	                                        );

// close com user function
typedef void  (*CLOSE_COM_PROC)(COM_HANDLER p_Handler); //[in]


// send data user function
typedef  e_ComResult (*SEND_COM_PROC)(COM_HANDLER p_handler,//[IN]
									  void *p_Data,//[IN]					//stream of bytes
									  unsigned short us_size//[IN]
									  );//[IN]

// receive data user function
typedef  e_ComResult (*RECEIVE_COM_PROC)(COM_HANDLER p_Handler,//[IN]
										void *p_InData,//[IN]				//stream of bytes
										unsigned short us_size,//[IN]
										int *p_BytesRead,//[OUT]
										int TimeOutMs);

// purge com user function
typedef void  (*PURGE_COM_PROC)(COM_HANDLER p_Handler); //[in]

// get Q input size user function
typedef  e_ComResult (*USER_GET_COM_LEGTH_PROC)(COM_HANDLER p_Handler,//[IN]
										   int *p_QLength);//[OUT]


// Set os mutex as the protocol is not reentrant
typedef  void (*USER_OS_MUTEX_PROC)(e_OSLock eLockType);//[IN]

typedef  enum
{
 e_Hand_SH_Permit2Send,
 e_Hand_SH_Transmit_ForBidden,
}e_HandShakeCommand;

// handshake command user function
typedef void  (*HANDSHAKE_COMMAND)(COM_HANDLER p_Handler,//[in]
								   e_HandShakeCommand Command);


typedef e_HandShakeCommand  (*HANDSHAKE_GET_OTHER_SIDE_STATE)(COM_HANDLER p_Handler//[in]
								  );



// get system tick time
typedef int  (*TICK_SYSTEM_PROC)(void);

typedef void* TW_TIMER_HANDLER;

// 1 success 0 fail  this function is not necessary  for master mode
typedef TW_TIMER_HANDLER (*USER_CREATE_TIMER)(
											int PeriodInMillisec,                            //[IN]
											CHANNEL_HANDLER ChannelToWakeUp,
											void (*protocol_timer)(CHANNEL_HANDLER ChannelToWakeUp)
											);

// close timer
// 1 success 0 fail
typedef void (*USER_CLOSE_TIMER)(TW_TIMER_HANDLER TimerHandler);

typedef void (*USER_SYSTEM_DELAY)(unsigned long ms);





typedef enum
{
 e_HandshakeNon,// for master slave mode
 e_HandshakeGuardTime,
 e_HandshakeCommand,

}e_HandshakeMethod;

/* doron */
typedef struct
{
	int K10Fd;
	int PrinterFd;
	int ModemFd;
	int KeyBoard;
	int DCUKeyDrv;
}sDeviceFd;
sDeviceFd DeviceFd;//global File descriptor
typedef struct
{
	int i_cmd;
	int sendOk;
	int datalen;
	unsigned char   e_IsRequesType;
	unsigned short  ApplicationResult;
	unsigned short  ApplicationStatusBits;
	char data[500];
}DataCommand;

typedef struct
{
 e_HandshakeMethod  Method;// if i_IsMasterSlaveNotSupported=1 the value must be or e_HandshakeTimeOut or e_HandshakeCommand
 int i_HandShakeTimeOutMS;
 int i_HandShakeTimeBitweenMessage;
 HANDSHAKE_COMMAND  HandShakeUserFunction;// if  Method is e_HandshakeCommand the HandShakeUserFunction must be valid
 USER_SYSTEM_DELAY ProcDelay;// mandatory in case of HandShakeUserFunction is not NULL
 HANDSHAKE_GET_OTHER_SIDE_STATE ProcGetHandShakeState;// in case of e_HandshakeCommand just if other the other side need Handshake support
}St_HandShake;


// all the fields of this shuld be valid
typedef struct
{
	OPEN_COM_PROC ProcOpen;	// mandatory
	CLOSE_COM_PROC ComClose;// mandatory
	SEND_COM_PROC ProcSend;// mandatory
	RECEIVE_COM_PROC ProcReceive;// mandatory  if i_IsWorkingWithByteEvent=0 if i_IsWorkingWithByteEvent=1 the driver ignore this value
	TICK_SYSTEM_PROC ProcTick;//mandatory  (Ms)
	USER_CREATE_TIMER UserCreateTimerProc; //mandatory
	USER_CLOSE_TIMER CloseTimerProc;//optional
	USER_GET_COM_LEGTH_PROC  GetLengthProc;//mandatory  if i_IsWorkingWithByteEvent=0 if i_IsWorkingWithByteEvent=1 the driver ignore this value
	START_ASYNC_LISTENING StartAsyncListening; // mandatory if i_IsWorkingWithByteEvent=1
    PURGE_COM_PROC PurgeCom;
  USER_OS_MUTEX_PROC SetMutex;
	USER_SYSTEM_DELAY UserSystemDelay;
	int DelayTimeOnTask;

}St_UserCallback;

//-----------------------------------------
//		events for user
//-----------------------------------------
// When message is received not use for master mode
typedef void (*PROC_ON_MSG_RECEIVED)(void *p_data ,//[IN]
									unsigned short us_size, //[IN]
									int i_RequestId, //[IN]
									int i_command,//[IN]
									int i_AppStatus, // [IN]
									e_RequestType RequestType); // [IN]

// When error message is received  not use for master mode
typedef void (*PROC_ON_MSG_ERR)(int i_RequestId, //[IN]
                                int i_command,//[IN]
								e_ProtocoleError e_ErrorType); //[IN]


typedef struct
{
	PROC_ON_MSG_RECEIVED On_ProcRecived;// mandatory in all cases excluding  master mode
	PROC_ON_MSG_ERR On_ProcError;//not mandatory
} St_UserEvent;

// to add external function that will execute from the main task of protocol ,
typedef void (*PRTCOLOL_TASK_EXETNTION_PROC)(void);

#ifdef __cplusplus

extern "C"
{
#endif


	// return the number of bytes that the driver need to mannage Queue with  i_QitemCount items
	// the caller must call to this function befor InitProtocolChannel for now how many bytes he need to  alocate for Queue
	int  i_ProtocolCalcAlocateSizeForQ(int i_QitemCount);

	// Initialize the channel
	CHANNEL_HANDLER InitProtocolChannel(int i_ComId,//[IN]
		St_UserCallback *p_UserFunction//[IN]
		,St_UserEvent *UserEvent,//[IN]
		St_HandShake  *UserHandShake,//[IN] in mater /slaver  case this parmeter shuld be NULL
		int i_IsMasterSlaveNotSupported, //[IN]
		int i_IsWorkingWithByteEvent,/*[IN] if true the client 232 driver have to call to
				 v_OnByteReceive every byte that recived*/
         int i_ProtocolPeriodTimer,//[IN] using for time create parameter
		 int i_IsMaster,
		 int i_MaxDataRecivedBuffer,
		 unsigned char *cp_ReceivedDataBuffer,
		 int i_AlocatedSizeFoeQueue,//[IN]the user alocation size in byte for Queue (the size=i_ProtocolCalcAlocateSizeForQ(Q_cnt))
		 void * QueueBuffer//[IN] // the user buffer that alocate for Queue

		 );

	// register user function to be execute in main task
	int i_RegisterExetnationUserFunction(CHANNEL_HANDLER h ,PRTCOLOL_TASK_EXETNTION_PROC userproc);

	COM_HANDLER GetCOMHndlrByChannel(CHANNEL_HANDLER h);

	e_ComResult e_ClosePrtocolChannel(CHANNEL_HANDLER h);
    /* in master slave protocol Only for the slave
	*/
	e_ComResult e_SendMessage(CHANNEL_HANDLER p_handler, //[IN]
						int i_cmd, //[IN]
						e_RequestType e_IsRequesType,//[IN]
						unsigned short ApplicationResult,// in case of i_IsRequest=1 the function  e_SendMessage ignore this value
                        unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
						int i_size,//[IN]
						void *p_data,//[IN]
						int i_TimeOutMillSec //[IN]
						);

	e_ComResult e_SendResponse(CHANNEL_HANDLER p_handler, //[IN]
						int i_cmd, //[IN]
						int i_RequestID,//[IN]
						unsigned short ApplicationResult,// in case of i_IsRequest=1 the function  e_SendMessage ignore this value
                        unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
						int i_size,//[IN]
						void *p_data,//[IN]
						int i_TimeOutMillSec //[IN]
						);

	//use only for master
	e_ComResult e_Tr1020ExchangeData(CHANNEL_HANDLER p_handler,//[IN]
									  int i_cmd,//[IN]
                                      int i_TimeOutMs,//[IN]
                                      int i_ObjectInSize,//[IN]
                                      void *p_ObjectIn,//[IN]
                                      int i_ObjectOutSizeReq,//[IN]
                                      void *p_ObjectOut,//[OUT]
									  int *p_OutSizeArive,//[IN]

									  unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
                                      int *p_ApplicationError); //[OUT]// the reader error



	// Client 232 driver call to this function at receve byte (i_IsWorkingWithByteEvent=1)
 void v_OnByteReceive(CHANNEL_HANDLER p_handler , //[IN]
	                    unsigned char *p_Characters, // [IN]
						int size//[IN]
							);
void v_Proc_protocol_timer_Task(CHANNEL_HANDLER p_handler);

//doron
int InitK10uart( void );
void CloseK10Uart(void);
int uart_fd;
#ifdef __cplusplus
}
#endif
#endif
