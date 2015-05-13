
#include "ProtoLink.h"      // Protocol implimentation h
#include <pthread.h>
#include <unistd.h>
////////////////////////////////////////////////////////////////////////////////////
//
// Module: ProtoBase.h / c a Win32 Protocol implimintation
//
////////////////////////////////////////////////////////////////////////////////////


typedef void (*Protocol_Timer)(CHANNEL_HANDLER Handler);
static void Protocol_SET_MUTEX(e_OSLock eLockType);
extern CoreReturnVal SendCommandToK10 ( int UARTId, void *Bytes, int Length, int Timeout);
///////////////////////////////////////////////////////////////////////////////////////
//
// Internal session type
//
///////////////////////////////////////////////////////////////////////////////////////

struct tag_ProtoCommSession
{                    
    CoreUARTConfig          UARTConfig;
    COM_HANDLER             p_Handler;
    unsigned char           *UARTCmdBuffer;
    int                     UARTCmdBufferLen;
    unsigned char           *ReceivedDataBuffer;
    unsigned char           *QueueBuffer;
    unsigned long           WriteTimeout;
    CHANNEL_HANDLER         ChannelHandle;
    CHANNEL_HANDLER         ChannelToWakeUp;
    PROC_ON_MSG_RECEIVED    MsgRecvd;
    PROC_ON_MSG_ERR         MsgErr;
    int                     TimerPeriodInMillisec;                          
    int                     TimerID;
    Protocol_Timer          TimerToExec;
    Protocol_TXHook         TXHook;
    Protocol_RXHook         RXHook;
};
typedef struct tag_ProtoCommSession ProtoCommSession;

ProtoCommSession *pSession = 0;

///////////////////////////////////////////////////////////////////////////////////////
//
// Timer struct
//
///////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
//
// Function: Protocol_OPEN_COM_PROC
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

COM_HANDLER  Protocol_OPEN_COM_PROC(int i_ComId,    // [IN]
    e_ComResult *p_ComRes)  //[OUT]
{

    if(!pSession)
    {
        *p_ComRes = e_ComUnKnwon;
        return 0;
    }
    else   // Success
    {
        *p_ComRes = e_ComOk;
        return (COM_HANDLER)&pSession->UARTConfig.UARTId;
    }
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: Protocol_CLOSE_COM_PROC
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

void  Protocol_CLOSE_COM_PROC(COM_HANDLER p_Handler) //[in]
{

    if(!pSession)
        return;

    //CoreUARTClose(pSession->UARTConfig.UARTId);
    close(DeviceFd.K10Fd);

}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: Protocol_TICK_SYSTEM_PROC
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

int Protocol_TICK_SYSTEM_PROC(void)
{
    return 1;//(int)CoreGetTickCount();

}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: Protocol_TIMER_THREAD
// Description: Timer thread (for timout events)
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

void Protocol_TIMER_THREAD(void)
{

    if(!pSession)
        return;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: Protocol_USER_CREATE_TIMER
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

TW_TIMER_HANDLER Protocol_USER_CREATE_TIMER (
    int PeriodInMillisec,
    CHANNEL_HANDLER ChannelToWakeUp,
    Protocol_Timer TimerToExec
    )
{

    if(!pSession)
        return 0;

    pSession->TimerPeriodInMillisec = PeriodInMillisec;
    pSession->ChannelToWakeUp       = ChannelToWakeUp;
    pSession->TimerToExec           = TimerToExec;

    // Dummy , timer will be executed from the main task
    return (TW_TIMER_HANDLER)1;

}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: Protocol_USER_CLOSE_TIMER
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

void Protocol_USER_CLOSE_TIMER(TW_TIMER_HANDLER TimerHandler)
{  
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: Protocol_START_ASYNC_LISTENING 
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

e_ComResult Protocol_START_ASYNC_LISTENING(COM_HANDLER p_handler)
{

    return e_ComUnKnwon; // Not supported under this system
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: func_USER_SYSTEM_DELAY
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

void Protocol_USER_SYSTEM_DELAY(unsigned long ms)
{
#ifndef CORE_WORK_WITH_OS    
    usleep(ms);
#else
  os_dly_wait(ms);
#endif
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: Protocol_COM_PURGE
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

void Protocol_COM_PURGE(COM_HANDLER p_handler)
{

    if(!pSession)
        return;

    //CoreUARTPurge(pSession->UARTConfig.UARTId,eCoreUARTRxAndTx);

}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: func_SEND_COM_PROC
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

e_ComResult Protocol_SEND_COM_PROC(COM_HANDLER p_handler,   // [IN]
    void *p_Data,                                           // [IN]	stream of bytes
    unsigned short us_size                                  // [IN]		
    )
{

    CoreReturnVal   Err = 0;
    e_ComResult     ComRes = e_ComUnKnwon;

//    if(!pSession)
//        ComRes = e_ComUnKnwon;
//
//    else
//    {
        Err = SendCommandToK10( uart_fd, p_Data, us_size, (us_size * 10)); // Write to the UART
        if(Err > 0)
            ComRes =  e_ComOk;
//    }

//    if(pSession->TXHook)
//        pSession->TXHook(Err,p_Data,us_size);

    return ComRes;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: func_RECEIVE_COM_PROC 
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

e_ComResult Protocol_RECEIVE_COM_PROC(COM_HANDLER p_Handler, //[IN]
    void *p_InData,                 // [IN]	
    unsigned short us_size,         // [IN]
    int *p_BytesRead,               // [OUT]
    int TimeOutMs)
{

    CoreReturnVal   Err;
    e_ComResult     ComRes = e_ComUnKnwon;
    unsigned int    nRead = *p_BytesRead;

    if(!pSession)
        return e_ComUnKnwon;

    do
    {
        if (PROTO_IS_WORKING_WITH_BYTE_EVENT == 1)
        {
            // In this case all job will be performed in the callback: func_StartAsyncListening
            ComRes =  e_ComInvalidArgument;
            break;
        }
        else
        {

            Err = CoreUARTRead (pSession->UARTConfig.UARTId,p_InData,&nRead,TimeOutMs);   // UART Read
            if ((nRead != (unsigned long)us_size) || (Err != (unsigned char)eCoreOK))
            {
                ComRes = e_ComUnKnwon;
                break;
            }
            if (p_BytesRead)
                *p_BytesRead = nRead;

            else
            {
                ComRes = e_ComInvalidArgument;
                break;
            }

            ComRes = e_ComOk;
        }
    }while(0);

    if(pSession->RXHook)
        pSession->RXHook(Err,p_InData,nRead);
    return ComRes;

}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: Proto_DUMMY_ON_MSG_RECEIVED
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

static void Proto_DUMMY_ON_MSG_RECEIVED(void *p_data, unsigned short us_size, int i_RequestID,int i_command, int i_AppStatus,e_RequestType RequestType)     
{
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: Proto_DUMMY_ON_MSG_ERR
// Description: 
// Note: a function router
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

static void Proto_DUMMY_ON_MSG_ERR(int i_RequestId, int i_command,e_ProtocoleError e_ErrorType)  
{
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ProtocolSetHook 
// Description: Set a hook api for RX and TX logging
// Return:  void  
//
////////////////////////////////////////////////////////////////////////////////////

void ProtocolSetHook(Protocol_TXHook TxHook, Protocol_RXHook Rxhook)
{
    if(!pSession)
        return;

    if(TxHook)
        pSession->TXHook = TxHook;     

    if(TxHook)
        pSession->RXHook = Rxhook; 
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ProtocolRelease
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

void ProtocolRelease(void)
{

    if(!pSession)
        return;

    // Cleanup
    if (pSession->ChannelHandle)
    {
        e_ClosePrtocolChannel(pSession->ChannelHandle);
        pSession->ChannelHandle = NULL;
    }

    if (pSession->QueueBuffer)
    {
        free(pSession->QueueBuffer);
        pSession->QueueBuffer = 0;

    }
    if(pSession->ReceivedDataBuffer)
    {
        free(pSession->ReceivedDataBuffer);
        pSession->ReceivedDataBuffer = 0;
    }

    pSession = 0;

}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ProtoGetChannele
// Description:
// Params: 
// Return: 
//
////////////////////////////////////////////////////////////////////////////////////

CHANNEL_HANDLER ProtoGetChannele(void)
{
    if(!pSession)
        return 0;

    return pSession->ChannelHandle;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ProtoUARTPolling 
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

void ProtoUARTPolling(void)
{

//    unsigned        int     RxLen = 0;
//    CoreReturnVal   Err;
//
//    if(!pSession)
//        return;
//
//    if(pSession->TimerToExec)
//        pSession->TimerToExec(pSession->ChannelToWakeUp);
//
//    // Poll the Uart for bytes
//    RxLen = CoreUARTGetRxSize(pSession->UARTConfig.UARTId);
//
//    if(RxLen && (RxLen < pSession->UARTCmdBufferLen))
//    {
//        Err  = CoreUARTRead(pSession->UARTConfig.UARTId,(void*)pSession->UARTCmdBuffer,&RxLen,0);
//        if(pSession->RXHook)
//            pSession->RXHook(Err,pSession->UARTCmdBuffer,RxLen);
//
//        if(RxLen > 0)
//            v_OnByteReceive(&pSession->UARTConfig.UARTId,pSession->UARTCmdBuffer,RxLen);
//    }
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ProtocolStart 
// Description:
// Parameters:
// Return:      
//
////////////////////////////////////////////////////////////////////////////////////

PROTO_HANDLE ProtocolStart(int UARTId,unsigned char *pUARTBufferPtr, int UARTBufferLen, PROC_ON_MSG_RECEIVED MsgRecvFunc,PROC_ON_MSG_ERR MsgErrFunc)
{

    int             i_ProtocolPeriodTimer           = 10;
    int             i_IsMasterSlaveNotSupported     = 1; 
    int             i_IsMaster                      = 0;
    int             i_MaxDataRecivedBuffer          = PROTO_BUFFER_SIZE;

    unsigned char   QueCount                        = 0;
    int             QBufferLength                   = 0;
    St_UserCallback UserCallbackFunction            = {0};
    St_UserEvent    UserEventCallback               = {0};
    St_HandShake    HandShake;

    // Allocate the session
    if(!pSession)
    {
        pSession = (ProtoCommSession*)calloc(sizeof(ProtoCommSession),1);
        if(!pSession)
            return 0; // Malloc error

        if(!pSession->UARTCmdBuffer)
        {
            pSession->UARTCmdBuffer = (unsigned char*)calloc(UARTBufferLen,1);
            if(!pSession->UARTCmdBuffer)
                return 0; // Malloc error
            pSession->UARTCmdBufferLen = UARTBufferLen; 
        }
    }

    // Init Core UART 
    pSession->UARTConfig.Baud             = eCoreUARTBaud115200;
    pSession->UARTConfig.Parity           = eCoreUARTParityNone;
    pSession->UARTConfig.RXBuffer         = pUARTBufferPtr;
    pSession->UARTConfig.RxEchoEnabled    = 0;
    pSession->UARTConfig.RxRTS            = 0;
    pSession->UARTConfig.TxRTS            = 0;
    pSession->UARTConfig.RXBufferLength   = UARTBufferLen;
    pSession->UARTConfig.UARTId           = UARTId;

    if(InitK10uart() == 0)
    {
        // Uart init error 
        //free(pSession);
    	CloseK10Uart();
        pSession = 0;
        return 0;

    }

    if(pSession->ChannelHandle)
        return 0; // Chunnel was initialized before

    do
    {

        pSession->ChannelHandle  = NULL;

        if(!MsgRecvFunc)
            pSession->MsgRecvd = Proto_DUMMY_ON_MSG_RECEIVED;
        else
            pSession->MsgRecvd = MsgRecvFunc;

        if(!MsgErrFunc)
            pSession->MsgErr = Proto_DUMMY_ON_MSG_ERR;
        else
            pSession->MsgErr = MsgErrFunc;

        UserCallbackFunction.ProcOpen               = Protocol_OPEN_COM_PROC;	    // mandatory 			
        UserCallbackFunction.ComClose               = Protocol_CLOSE_COM_PROC;      // mandatory 
        UserCallbackFunction.ProcSend               = Protocol_SEND_COM_PROC;       // mandatory 
        UserCallbackFunction.ProcReceive            = Protocol_RECEIVE_COM_PROC;    // mandatory  if i_IsWorkingWithByteEvent=0 if i_IsWorkingWithByteEvent=1 the driver ignore this value
        UserCallbackFunction.ProcTick               = Protocol_TICK_SYSTEM_PROC;    // mandatory  (Ms)
        UserCallbackFunction.UserCreateTimerProc    = Protocol_USER_CREATE_TIMER;   // mandatory
        UserCallbackFunction.CloseTimerProc         = Protocol_USER_CLOSE_TIMER;    // optional 
        UserCallbackFunction.GetLengthProc          = NULL;                         // mandatory  if i_IsWorkingWithByteEvent=0 if i_IsWorkingWithByteEvent=1 the driver ignore this value
        UserCallbackFunction.StartAsyncListening    = Protocol_START_ASYNC_LISTENING;
        UserCallbackFunction.PurgeCom               = Protocol_COM_PURGE;
				// add nutex fo send 
				UserCallbackFunction.SetMutex            = Protocol_SET_MUTEX;
        UserEventCallback.On_ProcError              = pSession->MsgErr;
        UserEventCallback.On_ProcRecived            = pSession->MsgRecvd;
			
				

        memset(&HandShake, 0, sizeof(HandShake));

        HandShake.HandShakeUserFunction             = NULL;
        HandShake.i_HandShakeTimeBitweenMessage     = 10;
        HandShake.i_HandShakeTimeOutMS              = 1000;
        HandShake.Method                            = e_HandshakeGuardTime;         // Async connection
        HandShake.ProcDelay                         = Protocol_USER_SYSTEM_DELAY;
        HandShake.ProcGetHandShakeState             = NULL;	

        // Allocate memory 
        if(i_MaxDataRecivedBuffer)
        {
            pSession->ReceivedDataBuffer = (unsigned char*)malloc(i_MaxDataRecivedBuffer);
            if(!pSession->ReceivedDataBuffer)
                break;
        }

        if(QueCount)
        {        
            QBufferLength = i_ProtocolCalcAlocateSizeForQ(QueCount);
            pSession->QueueBuffer = (unsigned char*)malloc (QBufferLength);	
            if(!pSession->QueueBuffer)
                break;
        }

        pSession->ChannelHandle  = InitProtocolChannel(pSession->UARTConfig.UARTId,            // [IN]
            &UserCallbackFunction                               // [IN]
            ,&UserEventCallback,                                // [IN]
            &HandShake,                                         // [IN] in mater /slaver  case this parmeter shuld be NULL  
            i_IsMasterSlaveNotSupported,                        // [IN]
            PROTO_IS_WORKING_WITH_BYTE_EVENT,
            i_ProtocolPeriodTimer,                              // [IN] using for time create parameter
            i_IsMaster,
            i_MaxDataRecivedBuffer,
            pSession->ReceivedDataBuffer,
            QBufferLength,                                      // [IN]the user alocation size in byte for Queue (the size=i_ProtocolCalcAlocateSizeForQ(Q_cnt))
            pSession->QueueBuffer                               // [IN] // the user buffer that alocate for Queue 
            );	
    }while(0);

    if(pSession->ChannelHandle == NULL)
    {
        // Free memory on error
        if(pSession->QueueBuffer)
        {
            free(pSession->QueueBuffer);
            pSession->QueueBuffer = 0;
        }
        if(pSession->ReceivedDataBuffer)
        {
            free(pSession->ReceivedDataBuffer);
            pSession->ReceivedDataBuffer = 0;
        }
        return 0;
    }

    return (PROTO_HANDLE)&pSession;
}

//static os_mut_init (PMUSession.OSData.HoppersMutex); // Init hoppers mutex
pthread_mutex_t M_e_OSMutexLock;



static void Protocol_SET_MUTEX(e_OSLock eLockType)
{
	//static HANDLE hndlSerialCommPortMutex = NULL;
	static int in=0;
	if(in==0)
	{
		pthread_mutex_init ( &M_e_OSMutexLock, NULL); // Init hoppers mutex
		in=1;
	}
	

	switch(eLockType)
	{
	case e_OSMutexInit:
	
		break;
	case e_OSMutexLock:
		//os_mut_wait (M_e_OSMutexLock, 0xffff);


		break;
    case e_OSMutexRelease:
			
    	pthread_mutex_unlock (&M_e_OSMutexLock);


		break;
	case e_OSMutexUnInit:
		break;
		}

}

