/*
 * P_TwMtrBase.c
 *
 *  Created on: Jan 20, 2014
 *      Author: doronsa
 */

#include<stdlib.h>
#include<string.h>

#include "P_TwMtrBase.h"
//#include <Core.h>
//#define TR_PACK_PREFIX_PP __attribute__ ((packed))


// to do add pragma for packet

////////////////////  defines &ENUM ///////////////////////////////////////////////////////////

#define OPEN_CHANNEL 0xaa  // aa mean channel open

#define MIN(a,b) ((a)<(b)?(a):(b))

#define MARKER_VAL 0xa5 // marker value of protocol

#define INITLIZE_LRC_VAL 0xAA // lrc initialize value

#define MARKER2_INDEX 1  // marker index

#define MAX_TIME_STREAM 3000 // max timeout bitween start stream until end

#define INTERVAL_2_CHECK_TIMEOUT_REQUEST 10  // interval time for check the Queue time

#define LAST_HEADER_INDEX (sizeof(St_ProtocolHeader)-1)

static const int WaitForMessageBeingProcessed_Default = 10000; // 10 sec
static const int DEFAULT_REQUEST_ID = -1;

#define SET_MUTEX(x) if(p->UserCallback.SetMutex){p->UserCallback.SetMutex(x);}
#define MUTEX_EXIT(e) {SET_MUTEX(e_OSMutexRelease);return e;}
print_buffer1(char *buffer, int len)
{
	int var = 0;
	printf("buffer %d:",len);
	for (var = 0; var < len; ++var)
	{
		printf(" %x",buffer[var]);
	}
	printf("\n");
}
//#define _I_AM_ALIVE_EVENT if (p->UserCallback.cbIMAlive != NULL){p->UserCallback.cbIMAlive();}

//typedef enum
//{
//	e_StateIdle,// idle wait for marker
//	e_StateWaitMarker2,// marker wait for all header
//	e_StateReciveHeader,// header wait for data
//	e_StateReciveData,// data wait for all data
//	e_StateMssgComplit,// message complit and ok
//	e_StateMssgErr,//message complit and corrupted
//}e_MssgRecivedState;
//
//typedef enum // error type of massage
//{
//	e_MssgOk,
//	e_MssgWrongLrc,// error checksum
//	e_MssgTimeOut,// time out occur
//	e_MssgErrorIndex,// bug!!!, the index not fit to protocol state
//}e_MssgError;

////////////////// structures////////////////////////////////////////////////////////////////

//typedef unsigned short TW_P_WORD;
//typedef unsigned char TW_P_BYTE;
//typedef unsigned long TW_P_DWORD;

typedef enum
{
	e_TaskReguler,
	e_TaskSendReceive
}e_TaskState;


/*  the header of tw_protocol
see base_protocol_usynchron.doc && base protocol.doc
*/
#define USE_ITEM 0xBB
//typedef struct
//{
//	unsigned short  RequestCount;
//	unsigned char Cmd;
//	unsigned long l_TimeStamp;
//	unsigned long TimeOut;
//	unsigned char Use;//USE_ITEM
//}St_Q_Item;

//typedef struct
//{
//	int MaxItem;
//	St_Q_Item *Array;
//}St_QObject;


//struct   TAG_St_ProtocolHeader
//{
//	TW_P_BYTE Marker1;// a5
//	TW_P_BYTE Marker2;// a5
//	TW_P_BYTE Address;// must be 0
//	TW_P_BYTE Cmd;// the application command
//	TW_P_BYTE RequestType;//e_RequestType  request / respond/event
//	TW_P_WORD StatusBit;// 16 bits flags free for aplication using
//	TW_P_BYTE ProtocolStatus;//e_ProtocoleError protocol error handling field   (relavnt in respond state)
//	TW_P_BYTE AplicationStatus; // application error handler field
//	TW_P_WORD RequestCount;// Request counter for link bitween respond & request
//	TW_P_WORD DataSize;// the data size of fields
//	TW_P_BYTE LrcCheckSum;// xoring of all Checksum= XOR(Header+Data)XOR 0xaa  the LrcCheckSum field at calculate must be ZERO
//};//St_ProtocolHeader;// protocol header
//
//typedef struct TAG_St_ProtocolHeader St_ProtocolHeader;

// this structure handling one user handler  state  (the handler that return from InitProtocolChannel function)
//typedef struct
//{
//	// const data
//	int i_OpenStatus;// if thr value is OPEN_CHANNEL  the channel open
//	St_UserCallback UserCallback;// user function for Serial device command
//	St_UserEvent UserEvent;// user event
//	St_HandShake UserHandShake;// user function for handling the serial handshake (for asynchronic mode )
//	PRTCOLOL_TASK_EXETNTION_PROC userproc;
//	int ComId;// the  serial com id
//	COM_HANDLER  Handler;// the serial com handler
//	// the protocol parameter (that send by InitProtocolChannel function)
//	int i_IsMasterSlaveNotSupported;
//	int i_IsWorkingWithByteEvent;
//	int i_ProtocolPeriodTimer;
//	TW_TIMER_HANDLER p_TimerHandler;
//	int i_IsMaster;
//	USER_SYSTEM_DELAY UserSystemDelay;
//	int DelayTimeOnTask;
//
//	int i_MaxDataRecivedBuffer;// alocate size of data buffer
//	unsigned char *cp_ReceivedDataBuffer;// the data buffer
//
//
//
//	// dinamic data//////////////////////////
//	//  data set at send
//	TW_P_WORD RequestCnt;// the request counter
//	int SendTimeStamp;// the time stamp of sending time
//
//	// data at on received
//	TW_P_BYTE i_HeaderRecevePointer;// how many bytes received in header
//	TW_P_WORD i_DataRecevePointer;// how mant bytes recevied in data
//	e_MssgRecivedState  state;// the state of current receiving  message
//	e_MssgError e_Err;// the message state when the receive completed
//	int i_TimeStampFirstByteReceved;// the time stamp of first byte of header that received
//	St_ProtocolHeader Header;// the header
//	TW_P_BYTE CalcLrc;// the current lrc value
//	int i_FlagIgnoreRegulerTimer;
//	unsigned long i_TaskCiount;
//	St_QObject Q;
//
//
//}St_UserData;

#define COM_BUSY(cm) ((cm)->i_OpenStatus==OPEN_CHANNEL)
////////////////// globals////////////////////////////////////////////////////////////////
// all handler memory for MAX_CHANNELS
static St_UserData  UserArray[MAX_CHANNELS];


//prototypes
static int ValidateHandler(St_UserData  *p);

static unsigned long ul_GetTimeStamp(St_UserData  *p);

static e_ComResult e_SendMessegePrivate(int p_handler, //[IN]
	int i_cmd, //[IN]
	e_RequestType e_IsRequesType,//[IN]
	unsigned short ApplicationResult,
	unsigned short  ApplicationStatusBits,//[IN]
	int i_size,//[IN]
	void *p_data,//[IN]
	int i_TimeOutMillSec ,//[IN]
	e_TaskState mode,//[IN]
	e_ProtocoleError e,
	int i_RequestID);//[IN]

static void v_Proc_protocol_timer_Task_Private(CHANNEL_HANDLER p_handler,e_TaskState Mode);

static void BuildMssg(St_UserData *p,//[IN]
	St_ProtocolHeader *ProtocolHeader,//[OUT]
	int i_cmd  ,//[IN]
	e_RequestType e_RequestType,
	int ApplicationResult,
	int ApplicationStatusBits,
	int i_size,
	void *p_data,
	e_ProtocoleError ProtocolErr,
	int i_RequestID /*If DEFAULT_REQUEST_ID - use internal counter, if >DEFAULT_REQUEST_ID use this value instead*/);

static void WaitTimeBitweenMessage(unsigned long TimeStamp , long TimeOut,St_UserData  *p);
static void WaitForMessageBeingProcessed (St_UserData* p, int i_TimeOutMs);

static void v_InitReciveState(St_UserData  *p);

static int i_PrAdd2ReqQ(St_UserData *p_handler,St_ProtocolHeader *p_Header,long TimeStamp, long TimeoutInMillisec);

static void Privatev_OnByteReceive(COM_HANDLER p_handler , //[IN]
	unsigned char uc_Character // [IN]
	);

static void v_ProccesQTimeOut(St_UserData *p_User);

////////////////// internal function////////////////////////////////////////////////////////////////
/*///////////////////////////////////////////////////////
FUNCTION:
Validate_Q
DESCRIPTION:
validate Queue
//////////////////////////////////////////////////////
*/
static int Validate_Q(St_QObject *Q)
{
	if(Q)
	{
		if(Q->Array  && Q->MaxItem>0)
			return 1;
	}
	return 0;
}

/*///////////////////////////////////////////////////////
FUNCTION:
GetFreeItem
DESCRIPTION:
search for free item in Queue

//////////////////////////////////////////////////////
*/
static St_Q_Item *GetFreeItem(St_QObject *Q)
{
	int i;
	if(!Validate_Q(Q))
		return NULL;

	// look for free item
	for(i=0;i<Q->MaxItem;i++)
	{

		if(Q->Array[i].Use!=USE_ITEM)
			return &Q->Array[i];// free item found
	}
	return NULL;// free item not found
}

/*///////////////////////////////////////////////////////
FUNCTION:
i_IsItemInTimeOut
DESCRIPTION:
test if item  time Expire
//////////////////////////////////////////////////////
*/
static int i_IsItemInTimeOut(St_Q_Item *p_Item,St_UserData *p_User)
{
	long CurrTime=ul_GetTimeStamp(p_User);

	if(CurrTime)//
	{
		// get the item timestamp
		long TimeStamp=p_Item->l_TimeStamp;
		// get the item timeout
		long Timeout=p_Item->TimeOut;
		// if time expire return true
		if((CurrTime-TimeStamp)>Timeout)
			return 1;
	}
	return 0;

}

/*///////////////////////////////////////////////////////
FUNCTION:
Q_DeleteItem
DESCRIPTION:
delete one item
//////////////////////////////////////////////////////
*/
static void Q_DeleteItem(St_Q_Item *p_Item)
{
	p_Item->Use=0;
}

/*///////////////////////////////////////////////////////
FUNCTION:
i_Add2Q
DESCRIPTION:
add item 2 Queue
//////////////////////////////////////////////////////
*/
static int i_Add2Q(St_QObject *Q, St_Q_Item *p_Item)
{
	St_Q_Item *item=GetFreeItem(Q);
	if(!item)
	{
		return 0;//there is not place on
	}

	*item=*p_Item;
	item->Use=USE_ITEM;
	return 1;

}

static St_Q_Item * p_ForSearchItem(St_QObject *Q,int cmd,int i_RequestCount)
{

	int i;
	for(i=0;i<Q->MaxItem;i++)
		if(Q->Array[i].Cmd==cmd && Q->Array[i].RequestCount==i_RequestCount)
			return &Q->Array[i];

	return NULL;
}

static int i_WaitHandShaikTransmitEnable(HANDSHAKE_GET_OTHER_SIDE_STATE Proc,int TimeOut,St_UserData *p)
{
	long StartTime=ul_GetTimeStamp(p),curr;
	if(StartTime)
	{
		while(Proc(p->Handler)!=e_Hand_SH_Permit2Send)
		{
			curr=ul_GetTimeStamp(p);
			if((curr-StartTime)>TimeOut)
				return 0;

		}

	}
	return 1;
}

/*///////////////////////////////////////////////////////
FUNCTION:
sh_CalLrc
DESCRIPTION:
calculate Lrc
//////////////////////////////////////////////////////
*/
TW_P_BYTE c_CalLrc(St_ProtocolHeader *SrcTWHeader,unsigned char *p_data,int i_size)
{
	St_ProtocolHeader ProtocolHeader;
	TW_P_BYTE Lrc=INITLIZE_LRC_VAL;
	int i;
	unsigned char *ptr= NULL;


	if (SrcTWHeader == NULL)
		return Lrc;

	memcpy(&ProtocolHeader, SrcTWHeader, sizeof(St_ProtocolHeader));
	ProtocolHeader.LrcCheckSum = 0;
	ptr = (unsigned char *)&ProtocolHeader;

	for(i=0;i<sizeof(St_ProtocolHeader);i++)
		Lrc^=ptr[i];

	if (p_data != NULL)
	{
		for(i=0;i<i_size;i++)
			Lrc^=p_data[i];
	}

	return Lrc;
}

/*///////////////////////////////////////////////////////
FUNCTION:
e_SendMessegePrivate
DESCRIPTION:
send message

//////////////////////////////////////////////////////
*/

static e_ComResult e_SendMessegePrivate(int p_handler, //[IN]
	int i_cmd, //[IN]
	e_RequestType e_IsRequesType,//[IN]
	unsigned short ApplicationResult,
	unsigned short  ApplicationStatusBits,//[IN]
	int i_size,//[IN]
	void *p_data,//[IN]
	int i_TimeOutMillSec ,
	e_TaskState Mode,
	e_ProtocoleError e_pr,
	int i_RequestID) //[IN]
{

	St_ProtocolHeader ProtocolHeader;
	e_ComResult e=e_ComOk;

	St_UserData  *p=(St_UserData*)p_handler;
	int phsize = sizeof(ProtocolHeader);
	// test the handler
	if(!ValidateHandler(p))
		return e_ComInvalidHandler;

	// build header body mesage
	BuildMssg(p,&ProtocolHeader,i_cmd,e_IsRequesType,ApplicationResult,ApplicationStatusBits,i_size,p_data,e_pr, i_RequestID);
    char *buff = (char*)malloc(phsize+i_size);
	// send header

    memcpy(buff,&ProtocolHeader,phsize);
    memcpy(buff+phsize,p_data,i_size);
	e = Protocol_SEND_COM_PROC(p_handler,(void*)buff,(unsigned short)(phsize+i_size));
	printf("send Header \n");
	//e=p->UserCallback.ProcSend(p->Handler,&ProtocolHeader,sizeof(ProtocolHeader));
	if(e!=e_ComOk)
		return e;// send fail

	return e;
}
/*///////////////////////////////////////////////////////
FUNCTION:
tw_SendNak
DESCRIPTION:
send nack at comminication error

//////////////////////////////////////////////////////
*/
void tw_SendNak(St_UserData *p,e_ProtocoleError e)
{
	// send nack with the protocol error error
	e_SendMessegePrivate(p,p->Header.Cmd,e_ReqTypeRespond,0,0,0,NULL,0,e_TaskReguler,e, DEFAULT_REQUEST_ID /*Use internal counter*/);
}

/*///////////////////////////////////////////////////////
FUNCTION:
ProtocolErr2ComErr
DESCRIPTION:
convert e_ProtocoleError  to   e_ComResult

//////////////////////////////////////////////////////
*/
static e_ComResult ProtocolErr2ComErr(e_ProtocoleError e)
{
	e_ComResult ee=e_ComUnKnwon;
	switch(e)
	{
	case eRT_OK:// = 0,				// correct
		ee=e_ComOk;
		break;
	case eRT_TIMEOUT:// = 1,			// timeout
		ee=e_ComTimeOut;
		break;

	case eRT_WRONG_LEN_PARAM:// = 2, // len field is out of range
	case eRT_WRONG_CHECKSUM://=3,		// checksum is incorerct

		ee=e_ComLinkNack;
		break;

	}
	return ee;
}

/*///////////////////////////////////////////////////////
FUNCTION:
ConvertErr
DESCRIPTION:
convert e_MssgError  to   e_ProtocoleError

//////////////////////////////////////////////////////
*/
static e_ProtocoleError ConvertErr(e_MssgError e)
{
	e_ProtocoleError  ee=eRT_UNKNOWN;
	switch(e)
	{
	case e_MssgOk:
		ee=eRT_OK;
		break;

	case  e_MssgWrongLrc:// error checksum
		ee=eRT_WRONG_CHECKSUM;
		break;
	case  e_MssgTimeOut:// time out occur
		ee=eRT_TIMEOUT;
		break;
	case  e_MssgErrorIndex:// bug!!!, the index not fit to protocol state  :
		ee=eRT_UNKNOWN;
		break;


	}
	return ee;
}

/*///////////////////////////////////////////////////////
FUNCTION:
v_InitReciveState
DESCRIPTION:
initilize the protocol state

//////////////////////////////////////////////////////
*/
static void v_InitReciveState(St_UserData  *p)
{
	volatile unsigned char x = 0;
    p->state = e_StateIdle;
	p->i_HeaderRecevePointer = 0;
	p->e_Err = e_MssgOk; // No error at the beginning
	p->i_DataRecevePointer = 0;
	p->i_TimeStampFirstByteReceved=0;
	memset(&p->Header,0,sizeof(p->Header));

    if(p->UserCallback.PurgeCom)
        p->UserCallback.PurgeCom(p->Handler);
}

/*///////////////////////////////////////////////////////
FUNCTION:
e_MssgRecivedState
DESCRIPTION:
return message state

//////////////////////////////////////////////////////
*/
static e_MssgRecivedState GetPotocolSate(St_UserData  *p)
{
	return p->state;
}

/*///////////////////////////////////////////////////////
FUNCTION:
i_IsStreamTimeOut
DESCRIPTION:
return 1 if timeout occur

//////////////////////////////////////////////////////
*/
static int i_IsStreamTimeOut(St_UserData  *p)
{
	unsigned long curr=ul_GetTimeStamp(p);
	unsigned long MaxTime;//=MAX_TIME_STREAM;

	// to received header the timeout is MAX_TIME_STREAM
	if(p->i_HeaderRecevePointer<LAST_HEADER_INDEX)
		MaxTime=MAX_TIME_STREAM;
	else  // to received all data the timeouit is one millisec each byte
		MaxTime=MAX_TIME_STREAM;//  Header.DataSize*10;


	if((curr- p->i_TimeStampFirstByteReceved)>MaxTime)
	{
		if (p->UserEvent.On_ProcError)
			p->UserEvent.On_ProcError(p->Header.RequestCount, p->Header.Cmd, eRT_TIMEOUT);

		return 1;
	}
	else
		return 0;
}

/*///////////////////////////////////////////////////////
FUNCTION:
MessageComplete
DESCRIPTION:
return message compilt

//////////////////////////////////////////////////////
*/
static int MessageComplete(St_UserData  *p, e_MssgError *p_e)
{
	if(p)
	{
		if(p->state==e_StateMssgComplit ||p->state==e_StateMssgErr)
		{
			*p_e=p->e_Err;
			return 1;
		}
	}

	return 0;
}

/*///////////////////////////////////////////////////////
FUNCTION:
i_IsMasterMode
DESCRIPTION:
return if this channel work on Master  mode

//////////////////////////////////////////////////////
*/
static int i_IsMasterMode(St_UserData  *p)
{
	if(p)
	{
		if((!p->i_IsMasterSlaveNotSupported) &&(p->i_IsMaster))
			return 1;
	}
	return 0;
}

/*///////////////////////////////////////////////////////
FUNCTION:
p_FindFreeChannel
DESCRIPTION:
look for free channel

//////////////////////////////////////////////////////
*/
static St_UserData * p_FindFreeChannel()
{
	int i;
	for(i=0;i<MAX_CHANNELS;i++)// for all channel check if channel free
		if(UserArray[i].i_OpenStatus!=OPEN_CHANNEL)
			return &UserArray[i];

	return NULL;// all channels use
}

static St_UserData * p_FindChannelByCOMHndl(COM_HANDLER p)
{
	int i;

	for(i=0;i<MAX_CHANNELS;i++) // for all channel check if channel free
		if ((UserArray[i].i_OpenStatus == OPEN_CHANNEL) && (UserArray[i].Handler == p))
			return &UserArray[i];

	return NULL;
}

COM_HANDLER GetCOMHndlrByChannel(CHANNEL_HANDLER h)
{
	if (!h)
		return NULL;

	return ((St_UserData*)h)->Handler;
}

/*///////////////////////////////////////////////////////
FUNCTION:
p_IsUseChannel
DESCRIPTION:
check if specific channel is use

//////////////////////////////////////////////////////
*/
static St_UserData * p_IsUseChannel(int ComId)
{
	int i;
	for(i=0;i<MAX_CHANNELS;i++)// look for channel that has the same ComId
	{
		if((UserArray[i].ComId==ComId) && (UserArray[i].i_OpenStatus==OPEN_CHANNEL))
			return &UserArray[i];

	}

	return NULL;// the ComId is not use

}

/*///////////////////////////////////////////////////////
FUNCTION:
v_ProccesQTimeOut
DESCRIPTION:
check all Queue if timeout occur
//////////////////////////////////////////////////////
*/
static void WaitTimeBitweenMessage(unsigned long TimeStamp ,long TimeOut,St_UserData *p)
{
	long Delayime=TimeOut;
	long timepass;

	long curr=ul_GetTimeStamp(p);


	if(curr && p->UserHandShake.ProcDelay)
	{
		// the time that pass from the last sending is
		timepass=curr-TimeStamp;
		if(timepass>0)
		{
			if(timepass<TimeOut)
			{
				Delayime=TimeOut-timepass;
				p->UserHandShake.ProcDelay(Delayime);

			}
		}
	}
}

/*///////////////////////////////////////////////////////
FUNCTION:
i_Add2ReqQ
DESCRIPTION:
Add item to request Queue

//////////////////////////////////////////////////////
*/
static int i_PrAdd2ReqQ(St_UserData *p, St_ProtocolHeader *p_Header,long TimeStamp, long TimeoutInMillisec)
{

	int r;
	St_QObject *Q=&p->Q;
	St_Q_Item p_Item;
	p_Item.Cmd=p_Header->Cmd;
	p_Item.l_TimeStamp=TimeStamp;
	p_Item.RequestCount=p_Header->RequestCount;
	p_Item.TimeOut = TimeoutInMillisec;

	r=i_Add2Q(Q,&p_Item);
	if(!r)//
	{
		v_ProccesQTimeOut(p);
		r=i_Add2Q(Q,&p_Item);
	}
	return r;
}

/*///////////////////////////////////////////////////////
FUNCTION:
v_ProccesQTimeOut
DESCRIPTION:
check all Queue if timeout occur
//////////////////////////////////////////////////////
*/
static void v_ProccesQTimeOut(St_UserData *p_User)
{
	St_QObject *Q=&p_User->Q;
	St_Q_Item *p_Item;
	int i,Cnt=Q->MaxItem;

	// for all Queue check if any item in timeout
	for(i=0;i<Cnt;i++)
	{
		p_Item=&Q->Array[i];
		// just the use item
		if(p_Item->Use==USE_ITEM)
		{
			// if item in timeout state
			if(i_IsItemInTimeOut(p_Item,p_User))
			{
				// if user function valid than throw event to user
				if(p_User->UserEvent.On_ProcError)
					p_User->UserEvent.On_ProcError(p_Item->RequestCount,p_Item->Cmd,eRT_TIMEOUT);
				// delete item from Queue
				Q_DeleteItem(p_Item);
			}
		}

	}
}

static void WaitForMessageBeingProcessed (St_UserData* p, int i_TimeOutMs)
{
	int start, curr;
	e_MssgError e;

	if (!p)
		return;

	/*
	e_StateIdle,// idle wait for marker
	e_StateWaitMarker2,// marker wait for all header
	e_StateReciveHeader,// header wait for data
	e_StateReciveData,// data wait for all data
	e_StateMssgComplit,// message complit and ok
	e_StateMssgErr,//message complit and corrupted
	*/

	start=ul_GetTimeStamp(p);
	curr=ul_GetTimeStamp(p);

	while ((MessageComplete(p, &e) == 1) && ((curr-start) < i_TimeOutMs))
	{
		if (p->UserHandShake.ProcDelay)
			// Wait for compleation
			p->UserHandShake.ProcDelay(5);

		// Check timeout
		curr=ul_GetTimeStamp(p);
	}
	//if((curr-start) >= i_TimeOutMs)		// TEST!!!
	//	 curr=ul_GetTimeStamp(p);

}

/*///////////////////////////////////////////////////////
FUNCTION:
v_RecieveBytesFromSerialDevice
DESCRIPTION:
read from comm any bytes that received  and send it to Privatev_OnByteReceive
//////////////////////////////////////////////////////
*/
static void v_RecieveBytesFromSerialDevice(St_UserData *p_User)
{
	int size,i;
	int nRead;
	unsigned  char c;
	int t=0;
	if(!p_User->UserCallback.ProcReceive)
		return;

	// if user support GetLengthProc proc
	if(p_User->UserCallback.GetLengthProc)
	{

		t=0;
		size=0;
		// get Rx Queue
		p_User->UserCallback.GetLengthProc(p_User->Handler,&size);
		// read all data from Q
		for(i=0;i<size;i++)
		{
			nRead=1;
			// read byte from Device Queue
			p_User->UserCallback.ProcReceive (p_User->Handler,&c,1,&nRead,0);
			if(nRead)// if byte received send it to protocol handler
			{
				Privatev_OnByteReceive(p_User->Handler,c);
				WaitForMessageBeingProcessed(p_User, WaitForMessageBeingProcessed_Default);
			}
		}
	}
	else// user not support GetLengthProc
	{
		t=0;
		do// read all bytes from device
		{
			nRead=0;
			p_User->UserCallback.ProcReceive(p_User->Handler,&c,1,&nRead,t);
			if(nRead)
			{
				t=1;
				Privatev_OnByteReceive(p_User->Handler,c);
				WaitForMessageBeingProcessed(p_User, WaitForMessageBeingProcessed_Default);
			}
		}while(nRead);
	}
}

/*///////////////////////////////////////////////////////////////////////////////
FUNCTION:
v_Proc_protocol_timer_Task

DESCRIPTION:

1 if i_IsWorkingWithByteEvent == 0 (the case that user device not send event when bytes received
)the task read the bytes that arrived on serial port  by calling to v_RecieveBytesFromSerialDevice

2 manege the timeout of recive stream in case of slave / asynchrnic mode

3 manege the Queue  in asynchrnic mode (all the request that send and the other side still not respond)

4 when message complit  call to user event ( PROC_ON_MSG_RECEIVED or to PROC_ON_MSG_ERR (relevant in all modes except master mode ))


///////////////////////////////////////////////////////////////////////////////
*/
void v_Proc_protocol_timer_Task(CHANNEL_HANDLER p_handler)
{
	St_UserData  *p=(St_UserData*)p_handler;
	e_MssgError  e2 = e_MssgTimeOut;

//	_I_AM_ALIVE_EVENT

	if(!MessageComplete(p,&e2))
		e2 = e_MssgTimeOut;
	else
		e2 = e_MssgTimeOut;
	v_Proc_protocol_timer_Task_Private(p_handler, e_TaskReguler);
}

static void v_Proc_protocol_timer_Task_Private(CHANNEL_HANDLER p_handler,e_TaskState Mode)
{
	St_UserData  *p=(St_UserData*)p_handler;
	e_MssgError  e2 = e_MssgTimeOut;
    int IsDoReset=1;

	// validate  the handler
	if(!ValidateHandler(p))
		return ;

	// timer now run in polling mode so the timer mode ignore to avoid collision
	if(p->i_FlagIgnoreRegulerTimer&& Mode==e_TaskReguler)
		return ;
	////////////////////////////////////////////////////////////////////////////////
	// receive bytes from serial device and call to Privatev_OnByteReceive
	////////////////////////////////////////////////////////////////////////////////
	if(!p->i_IsWorkingWithByteEvent)
		v_RecieveBytesFromSerialDevice(p);

	if(Mode==e_TaskSendReceive)
		return ;

	// hime 20/01/2013 delay to give orher thread cpu
	if(p->UserSystemDelay)
		p->UserSystemDelay(p->DelayTimeOnTask);

	////////////////////////////////  //////////////////////////////////////////////
	// this block handle the  asynchronic protocol timeout and
	// and call to  the  On_ProcError  and the On_ProcRecived  user event , this block
	// is not relevant to master mode
	//	if(p->i_IsMasterSlaveNotSupported || (p->i_IsMaster==0))
	// it is not muster  mode
	////////////////////////////////////////////////////////////////////////////////
	if(/*i_IsMasterMode(p)==0*/ Mode==e_TaskReguler)
	{
		p->i_TaskCiount++;

		// the massage may be start and not complit  so the timer check if timeout occur

		if(!MessageComplete(p,&e2))
		{
			// check if there is stream timeout(to much time bitween first bytes)
			if(GetPotocolSate(p)!=e_StateIdle	&& i_IsStreamTimeOut(p))
				v_InitReciveState(p); // init protocol state
		}
		else// message complit so task execute user callback
		{

			if ((e2!=e_MssgOk) || (p->Header.ProtocolStatus != eRT_OK)) // message error
			{
				e_ProtocoleError e=ConvertErr(e2);

				// send nack to other size if it is Request
				if(p->i_HeaderRecevePointer>=LAST_HEADER_INDEX ) // all header arrived
				{
					if(p->Header.RequestType==e_ReqTypeRequest)
						tw_SendNak(p,e);// send nack
				}// end if p->i_HeaderRecevePointer>=LAST_HEADER_INDEX

				// if user callbac valid throw event to user
				if(p->UserEvent.On_ProcError)
				{
					if (e != eRT_OK)
						p->UserEvent.On_ProcError(p->Header.RequestCount, p->Header.Cmd, e);
					else
						p->UserEvent.On_ProcError(p->Header.RequestCount, p->Header.Cmd, (e_ProtocoleError)p->Header.ProtocolStatus);
				}
			}
			else // on_received user callbak
			{
             if(p->Header.RequestType==e_ReqTypeRequest)
                IsDoReset=0;
				if(p->UserEvent.On_ProcRecived)
				{
					p->UserEvent.On_ProcRecived(
						p->cp_ReceivedDataBuffer,
						p->Header.DataSize,
						p->Header.RequestCount,
						p->Header.Cmd,
						p->Header.AplicationStatus,
						(e_RequestType)p->Header.RequestType);
				}
			}

			// Remove message from Queue
			if(p->Header.RequestType==e_ReqTypeRespond)//  if it espond then we have to delete the item from Queue
			{
				if(p->Q.Array)
				{
					St_Q_Item p_Item,*ptr;
					p_Item.Cmd=p->Header.Cmd;
					p_Item.RequestCount=p->Header.RequestCount;
					// test if item exist in Queue
					ptr=p_ForSearchItem(&p->Q,p_Item.Cmd,p_Item.RequestCount);
					if(ptr)// if exist delete item
						Q_DeleteItem(ptr);
				}// end if p->Q.Array
			}//end if p->Header.RequestType==e_ReqTypeRespond

            //if(p->Header.RequestType!=e_ReqTypeRequest)
            if(IsDoReset)
               v_InitReciveState(p);// init protocol state



			// relese the handshake
			if(p->UserHandShake.HandShakeUserFunction)// send to other side Stop sending
				p->UserHandShake.HandShakeUserFunction(p->Handler,e_Hand_SH_Permit2Send);
		}

		//  handling all Q timeout
		if((p->i_TaskCiount % INTERVAL_2_CHECK_TIMEOUT_REQUEST)==0)
			v_ProccesQTimeOut(p);

		if(p->userproc)
			p->userproc();

//		_I_AM_ALIVE_EVENT
	}
}

/*///////////////////////////////////////////////////////////////////////////////
FUNCTION:
Validate_Param

DESCRIPTION:
validation of initialize channel parameter


///////////////////////////////////////////////////////////////////////////////
*/
static int Validate_Param(St_UserCallback *p_User,St_UserEvent *p_Event,St_HandShake *p_HandShake ,St_UserData *p_UserData)
{
	// St_UserCallback validation

	if(!(p_User&&p_Event&&p_HandShake))
		return 0;

	if(!(p_User->ProcOpen&&p_User->ComClose&&p_User->ProcSend&&p_User->ProcTick&&p_User->UserCreateTimerProc))
		return 0;
	if(p_UserData->i_IsWorkingWithByteEvent==0)
	{
		//if(!(p_User->ProcReceive&&p_User->GetLengthProc))
			//return 0;
	}

	//St_UserEvent validation
	// at all modes except master mode the On_ProcRecived user function must be valid
	if(i_IsMasterMode(p_UserData)==0)
	{
		if(!p_Event->On_ProcRecived)
			return 0;
	}

	// at aSynchronic mode Handsake must be active
	if(p_UserData->i_IsMasterSlaveNotSupported)
	{

		if(p_HandShake->Method==e_HandshakeNon)
			return 0;
	}

	// in case of e_HandshakeCommand handshake
	if(p_HandShake->Method==e_HandshakeCommand)
		if((p_HandShake->HandShakeUserFunction==NULL)||(p_HandShake->ProcGetHandShakeState==NULL))
			return 0;

	// in case of
	if(p_HandShake->Method==e_HandshakeGuardTime)
		if(p_HandShake->ProcDelay==NULL)
			return 0;

	return 1;
}

/*///////////////////////////////////////////////////////////////////////////////
FUNCTION:
ValidateHandler

DESCRIPTION:
validation of handler


///////////////////////////////////////////////////////////////////////////////
*/
static int ValidateHandler(St_UserData *p)
{
//	_I_AM_ALIVE_EVENT

//	if(!p)
//		return 0;
//	if(p->i_OpenStatus!=OPEN_CHANNEL)
//		return 0;
//	if(p->Handler==NULL)
//		return 0;
	return 1;
}

/*///////////////////////////////////////////////////////////////////////////////
FUNCTION:
BuildMssg

DESCRIPTION:
build mesasage for transmition


///////////////////////////////////////////////////////////////////////////////
*/
static void 	BuildMssg(St_UserData *p,//[IN]
	St_ProtocolHeader *ProtocolHeader, //[OUT]
	int i_cmd  ,//[IN]
	e_RequestType e_RequestType,
	int ApplicationResult,
	int ApplicationStatusBits,
	int i_size,
	void *p_data,
	e_ProtocoleError ProtocolErr,
	int i_RequestID /*If DEFAULT_REQUEST_ID - use internal counter, if >DEFAULT_REQUEST_ID use this value instead*/
	)
{
//	_I_AM_ALIVE_EVENT

	memset(ProtocolHeader,0,sizeof(*ProtocolHeader));

	ProtocolHeader->Cmd=i_cmd;
	//		ProtocolHeader->Address=0;
	ProtocolHeader->AplicationStatus=ApplicationResult;
	ProtocolHeader->DataSize=i_size;
	//		ProtocolHeader->LrcCheckSum=0;
	ProtocolHeader->Marker1=MARKER_VAL;
	ProtocolHeader->Marker2=MARKER_VAL;
	ProtocolHeader->ProtocolStatus=ProtocolErr;
	// Place
//	if (i_RequestID == DEFAULT_REQUEST_ID)
//	{
//		ProtocolHeader->RequestCount=p->RequestCnt;
//
//		// incement protocol request counter
//		if(e_RequestType != e_ReqTypeRespond)
//			p->RequestCnt++;
//	}
//	else
		ProtocolHeader->RequestCount=i_RequestID;

	ProtocolHeader->RequestType=e_RequestType;
	ProtocolHeader->StatusBit=ApplicationStatusBits;
	//print_buffer1(p_data,i_size);
	ProtocolHeader->LrcCheckSum=c_CalLrc(ProtocolHeader, (unsigned char*)p_data, i_size);

}

/*///////////////////////////////////////////////////////////////////////////////
FUNCTION:
ul_GetTimeStamp

DESCRIPTION:
get time stamp
///////////////////////////////////////////////////////////////////////////////
*/
static unsigned long ul_GetTimeStamp(St_UserData  *p)
{
	if(p->UserCallback.ProcTick)
		return p->UserCallback.ProcTick();
	else
		return 0;

}
/*///////////////////////////////////////////////////////////////////////////////
FUNCTION:
Privatev_OnByteReceive

DESCRIPTION:
recieive one byte and put it in message and update protocol state
///////////////////////////////////////////////////////////////////////////////
*/
static void Privatev_OnByteReceive(COM_HANDLER p_handler , //[IN]
	unsigned char uc_Character // [IN]
	)
{
	unsigned char *p_Header = NULL;
	St_UserData  *p = p_handler;//p_FindChannelByCOMHndl(p_handler);
	//p->Handler = p_handler;

	if (!p)
		return;

//	_I_AM_ALIVE_EVENT

	p_Header=(unsigned char *)&p->Header;

	switch(p->state)
	{
	case e_StateIdle:// start message
		// it is not marker character
		if(MARKER_VAL!=uc_Character)
			return;


		p->i_HeaderRecevePointer=0;// init pointer
		p_Header[p->i_HeaderRecevePointer]=uc_Character;// get data
		p->i_HeaderRecevePointer++;
		p->state=e_StateWaitMarker2;// set next state
		p->CalcLrc=uc_Character^INITLIZE_LRC_VAL;//init lrc
		p->i_TimeStampFirstByteReceved = ul_GetTimeStamp(p);

		break;

	case e_StateWaitMarker2:// the second marker
		// it is not marker character -> the message is not realy message
		if(MARKER_VAL!=uc_Character)
		{
			v_InitReciveState(p);
			return;
		}
		// validate index
		if(p->i_HeaderRecevePointer>MARKER2_INDEX)
		{
			v_InitReciveState(p);
			p->e_Err=e_MssgErrorIndex;
			return;

		}

		p_Header[p->i_HeaderRecevePointer]=uc_Character;
		p->i_HeaderRecevePointer++;
		p->CalcLrc^=uc_Character;// calculate lrc
		p->state=e_StateReciveHeader;//// set next state
		break;

	case e_StateReciveHeader:// all header form data untill


		// validate index
		if(p->i_HeaderRecevePointer>LAST_HEADER_INDEX)
		{
			v_InitReciveState(p);
			p->e_Err=e_MssgErrorIndex;
			return;

		}
		p_Header[p->i_HeaderRecevePointer]=uc_Character;
		p->i_HeaderRecevePointer++;
		p->CalcLrc^=uc_Character;// calculate lrc


		// last header character
		if((p->i_HeaderRecevePointer-1)==LAST_HEADER_INDEX)
		{

			// message without data
			if(p->Header.DataSize==0)
			{
				if(p->UserHandShake.HandShakeUserFunction)// send to other side Stop sending  signal
					p->UserHandShake.HandShakeUserFunction(p->Handler,e_Hand_SH_Transmit_ForBidden);

				// validate lrc
				if(p->CalcLrc==0/*p->Header.LrcCheckSum*/)
					p->state=e_StateMssgComplit;
				else// wrong lrc
				{
					p->state=e_StateMssgErr;
					p->e_Err=e_MssgWrongLrc;
				}


			}
			else// message with data
			{
				p->state=e_StateReciveData;
				p->i_DataRecevePointer=0;
			}

		}// end if p->i_HeaderRecevePointer==LAST_HEADER_INDEX

		break;
	case e_StateReciveData:


		// validate data size if big than allocation size
		if((p->i_DataRecevePointer> p->i_MaxDataRecivedBuffer-1)
			||(p->i_DataRecevePointer+1 > p->Header.DataSize))// validate data size if big than header data size
		{
			v_InitReciveState(p);
			p->e_Err=e_MssgErrorIndex;
			return;

		}

		// update data
		p->cp_ReceivedDataBuffer[p->i_DataRecevePointer]=uc_Character;
		p->CalcLrc^=uc_Character;// calculate lrc
		// if all data arrived the message complit
		if(p->i_DataRecevePointer == p->Header.DataSize-1)
		{
			if(p->UserHandShake.HandShakeUserFunction)// send to other side Stop sending
				p->UserHandShake.HandShakeUserFunction(p->Handler,e_Hand_SH_Transmit_ForBidden);

			// if checksum ok message complit
			if(p->CalcLrc==0/*p->Header.LrcCheckSum*/)
				p->state=e_StateMssgComplit;
			else// lrc fail
			{
				p->state=e_StateMssgErr;
				p->e_Err=e_MssgWrongLrc;
			}

		}
		else// not all data arrived
			p->i_DataRecevePointer++;

		break;

	case e_StateMssgComplit:
		break;

	default:
		break;
	}

}

////////////////// API///////////////////////////////////////////////////////////////////////////////////////////////////




/*/////////////////////////////////////////////////////////////////////////////////////////

description:

parametres:

logic:
1 check if comid validation
2 check if comm busy
3 register user data in global object
4 try to open comm
5 if open succseded then sign the comm as Use
6 return pointer to user global object


return :


///////////////////////////////////////////////////////////////////////////////////////////
*/
// Initialize the channel
CHANNEL_HANDLER InitProtocolChannel(int i_ComId,//[IN]
				St_UserCallback *p_UserFunction//[IN]
				,St_UserEvent *UserEvent,//[IN]
				St_HandShake  *UserHandShake,//[IN] in mater /slaver  case this parmeter shuld be NULL  `
				int i_IsMasterSlaveNotSupported, //[IN]
				int i_IsWorkingWithByteEvent,/*[IN] if true the client 232 driver have to call to
					v_OnByteReceive every byte that recived*/
					int i_ProtocolPeriodTimer,//[IN] using for time create parameter
					int i_IsMaster,
					int i_MaxDataRecivedBuffer,
					unsigned char *cp_ReceivedDataBuffer,
					int i_AlocatedSizeFoeQueue,//[IN]
					void * QueueBuffer//[IN]
					)
{

	e_ComResult state;
	TW_TIMER_HANDLER p_TimerHandler;
	// find if i_ComId is open
	St_UserData  *p=p_IsUseChannel(i_ComId);
	if(p)
		return NULL;

	if (!p_UserFunction)
		return NULL;

	// check if there any free channel
	p= p_FindFreeChannel();
	if(!p)
		return NULL;
	// check if all calbak that necessary for specific mode is not NULL

	// set all user data members to 0
	memset(p,0,sizeof(*p));

	// set user data
	p->i_IsMasterSlaveNotSupported=i_IsMasterSlaveNotSupported;
	p->i_IsWorkingWithByteEvent=i_IsWorkingWithByteEvent;

	p->i_ProtocolPeriodTimer=i_ProtocolPeriodTimer;
	p->UserCallback=*p_UserFunction;
	p->UserEvent=*UserEvent;
	p->UserHandShake=*UserHandShake;
	p->i_IsMaster=i_IsMaster;
	p->i_FlagIgnoreRegulerTimer=0;

	// init Queue
	p->Q.Array=(St_Q_Item*)QueueBuffer;
	p->Q.MaxItem=i_AlocatedSizeFoeQueue/sizeof(St_Q_Item);

	if(!Validate_Param(p_UserFunction,UserEvent,UserHandShake,p))
		return NULL;

	if(i_MaxDataRecivedBuffer &&cp_ReceivedDataBuffer)
	{
		p->i_MaxDataRecivedBuffer=i_MaxDataRecivedBuffer;
		p->cp_ReceivedDataBuffer=cp_ReceivedDataBuffer;
	}
	else
		return NULL;


	// try open comm
	p->Handler=p->UserCallback.ProcOpen(i_ComId,&state);
	if(!p->Handler)
	{
		return NULL;
	}

	if (p->i_IsWorkingWithByteEvent)
		if (! (p->UserCallback.StartAsyncListening))
			return NULL;
		else
			p->UserCallback.StartAsyncListening(p->Handler);

	// create timer just in slave mode or in Asynchronous protocol
	if(p->i_IsMasterSlaveNotSupported||p->i_IsMaster==0)
	{
		p_TimerHandler = p->UserCallback.UserCreateTimerProc(i_ProtocolPeriodTimer, p, v_Proc_protocol_timer_Task);

		// try to create timer for protocol  for specific channel
		if(!p_TimerHandler)
		{
			p->UserCallback.ComClose(p->Handler);
			return NULL;
		}

		// save time handler
		p->p_TimerHandler=p_TimerHandler;
	}
  p->UserSystemDelay= p->UserCallback.UserSystemDelay;
  p->DelayTimeOnTask= p->UserCallback.DelayTimeOnTask;


    // init mutex
    SET_MUTEX(e_OSMutexInit);

	// set satus to open
	p->ComId=i_ComId;
	p->i_OpenStatus=OPEN_CHANNEL;

	// Success
	return (CHANNEL_HANDLER)(p);
}

// register user function to be execute in main task
int i_RegisterExetnationUserFunction(CHANNEL_HANDLER p_handler ,PRTCOLOL_TASK_EXETNTION_PROC userproc)
{
	St_UserData  *p=(St_UserData*)p_handler;

	// test the handler
	if(!ValidateHandler(p))
		return 0;;

	p->userproc=userproc;
	return 1;
}


/*/////////////////////////////////////////////////////////////////////////////////////////
function:
e_ClosePrtocolChannel

description:
close comm resurce
parametres:
p_handler

return :
e_ComResult
*/
///////////////////////////////////////////////////////////////////////////////////////////


e_ComResult e_ClosePrtocolChannel(CHANNEL_HANDLER p_handler)
{
	St_UserData  *p=(St_UserData*)p_handler;

	// test the handler
	if(!ValidateHandler(p))
		return e_ComInvalidHandler;

	SET_MUTEX(e_OSMutexUnInit);

	// close timer
	if(p->UserCallback.CloseTimerProc)
		p->UserCallback.CloseTimerProc(p->p_TimerHandler);

	// close comm
	if(p->UserCallback.ComClose)
		p->UserCallback.ComClose(p->Handler);
	// sign com as close
	p->i_OpenStatus=0;
	p->ComId=0;

	return e_ComOk;

}
/* in master slave protocol Only for the slave
*/
/*/////////////////////////////////////////////////////////////////////////////////////////

function:
e_ClosePrtocolChannel

description:
close comm resurce
parametres:
p_handler
command
e_IsRequesType
ApplicationResult  for respond case
ApplicationStatusBits for application flags



return :
e_ComResult

///////////////////////////////////////////////////////////////////////////////////////////
*/

e_ComResult e_SendMessage(int p_handler, //[IN]
	int i_cmd, //[IN]
	e_RequestType e_IsRequesType,//[IN]
	unsigned short ApplicationResult,
	unsigned short  ApplicationStatusBits,//[IN]
	int i_size,//[IN]
	void *p_data,//[IN]
	int i_TimeOutMillSec)//[IN]
{
	e_ComResult e;
    static int msgid=0;
	St_UserData  *p=(St_UserData*)p_handler;

//	_I_AM_ALIVE_EVENT

	// test the handler
	//if(!ValidateHandler(p))
	//	return e_ComInvalidHandler;

	//it iligall to call this function  in master mode
	//if(p->i_IsMasterSlaveNotSupported==0 &&p->i_IsMaster==1)
	//	return e_ComTheFunctionNotFitForThisMode;

	//if (e_IsRequesType == e_ReqTypeRespond)
	//	return e_ComInvalidArgument_Use_e_SendResponse_Instead;

    // Exclusive access
    //SET_MUTEX(e_OSMutexLock);

	// send message

	e=e_SendMessegePrivate(p_handler,
		                   i_cmd,
						   e_IsRequesType,
						   ApplicationResult,
						   ApplicationStatusBits,
						   i_size,
						   p_data,
						   i_TimeOutMillSec,
						   e_TaskReguler,
						   eRT_OK,
						   msgid/*DEFAULT_REQUEST_ID*/          /*Use internal counter*/);

	// Release exclusive access
    //SET_MUTEX(e_OSMutexRelease);
	msgid++;
    return e;
}

e_ComResult e_SendResponse(CHANNEL_HANDLER p_handler, //[IN]
						int i_cmd, //[IN]
						int i_RequestID,//[IN]
						unsigned short ApplicationResult,// in case of i_IsRequest=1 the function  e_SendMessage ignore this value
                        unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
						int i_size,//[IN]
						void *p_data,//[IN]
						int i_TimeOutMillSec //[IN]
						)
{
	e_ComResult e;

	St_UserData  *p=(St_UserData*)p_handler;

//	_I_AM_ALIVE_EVENT

	// test the handler
	if(!ValidateHandler(p))
		return e_ComInvalidHandler;

	//it iligall to call this function  in master mode
	if(p->i_IsMasterSlaveNotSupported==0 &&p->i_IsMaster==1)
		return e_ComTheFunctionNotFitForThisMode;

    // Exclusive access
    SET_MUTEX(e_OSMutexLock);

   v_InitReciveState(p);// init protocol state

    // send message
	e=e_SendMessegePrivate
		(p_handler,
		i_cmd,
		e_ReqTypeRespond,
		ApplicationResult,
		ApplicationStatusBits,
		i_size,
		p_data,
		i_TimeOutMillSec,
		e_TaskReguler,
		eRT_OK,
		i_RequestID);

	// Release exclusive access
    SET_MUTEX(e_OSMutexRelease);

    return e;
}


/*/////////////////////////////////////////////////////////////////////////////////////////

function:
e_Tr1020ExchangeData

description:
send and received answer at master slave protocol


return :
e_ComResult

///////////////////////////////////////////////////////////////////////////////////////////
*/

// in master slave protocol Only for the master
// in master slave protocol Only for the master
e_ComResult e_Tr1020ExchangeData(CHANNEL_HANDLER p_handler,//[IN]
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
//	int i_ThisRequestNumber = 0;
	e_ComResult e=e_ComOk;
	e_MssgError  e2;

	int start,curr;//GetTimeStamp();
	St_UserData  *p=(St_UserData*)p_handler;

//	_I_AM_ALIVE_EVENT

	// handler validation
	if(!ValidateHandler(p))
		return e_ComInvalid;

	// not for slave mode
	if((p->i_IsMasterSlaveNotSupported==0) && (p->i_IsMaster==0))
		return e_ComTheFunctionNotFitForThisMode;

	// close timer task if it is Asynchronic mode

	SET_MUTEX(e_OSMutexLock);

	// send data
	//i_ThisRequestNumber = p->RequestCnt;
	p->i_FlagIgnoreRegulerTimer=1;

	e=e_SendMessegePrivate(p_handler,
		i_cmd,
		e_ReqTypeRequest,
		0,
		ApplicationStatusBits,
		i_ObjectInSize,
		p_ObjectIn,
		i_TimeOutMs,
		e_TaskSendReceive,
		eRT_OK,
		DEFAULT_REQUEST_ID /*Use internal counter*/);

	if(e!=e_ComOk)
		MUTEX_EXIT(e);

	// take timestamp for timeout calculation
	start=ul_GetTimeStamp(p);

	do
	{
		// take timestamp for timeout calculation
		curr=ul_GetTimeStamp(p);

		// execute timer handler (this function manage the receive  protocol process )
		v_Proc_protocol_timer_Task_Private(p_handler, e_TaskSendReceive);
		//ProtoUARTPolling();

		// if massege complit
		if(MessageComplete((St_UserData*)p_handler, &e2))
		{
			// It may be not the response that we're waiting for
            if (/*i_ThisRequestNumber != p->Header.RequestCount ||*/ p->Header.RequestType==e_ReqTypeEvent)
			{
				if ((e2 == e_MssgOk) && (p->Header.ProtocolStatus == eRT_OK))
				{
					if (p->UserEvent.On_ProcRecived)
					{
						p->UserEvent.On_ProcRecived(
													p->cp_ReceivedDataBuffer,
													p->Header.DataSize,
													p->Header.RequestCount,
													p->Header.Cmd,
													p->Header.AplicationStatus,
													(e_RequestType)p->Header.RequestType
													);
					}
				}
				else
				{
					if (p->UserEvent.On_ProcError)
					{
						if (e2 != e_MssgOk)
							p->UserEvent.On_ProcError(p->Header.RequestCount, p->Header.Cmd, ConvertErr(e2));
						else
							p->UserEvent.On_ProcError(p->Header.RequestCount, p->Header.Cmd, (e_ProtocoleError)p->Header.ProtocolStatus);
					}
				}

				v_InitReciveState(p);// init protocol state

				// relese the handshake
				if(p->UserHandShake.HandShakeUserFunction)// send to other side Stop sending
					p->UserHandShake.HandShakeUserFunction(p->Handler,e_Hand_SH_Permit2Send);
			}
			else
			{
				// message fail
				if(e2!=eRT_OK)
				{
					v_InitReciveState(p);// init protocol state

					// relese the handshake
					if(p->UserHandShake.HandShakeUserFunction)// send to other side Stop sending
						p->UserHandShake.HandShakeUserFunction(p->Handler,e_Hand_SH_Permit2Send);

					// Job Complete
					p->i_FlagIgnoreRegulerTimer=0;

					MUTEX_EXIT(ProtocolErr2ComErr(ConvertErr(e2)));
				}
				else// preaper data out for caller
				{
					// the size to copy is the minimum of data arived and data requset size
					int Diff = 0;
					int i_Size2Copy= MIN(p->Header.DataSize, i_ObjectOutSizeReq);
					*p_OutSizeArive = i_Size2Copy;

					if (i_Size2Copy > 0)
						// copy the data to user buffer
						memcpy(p_ObjectOut, p->cp_ReceivedDataBuffer, i_Size2Copy);

					// modify application status
					*p_ApplicationError=p->Header.AplicationStatus;

					Diff = (p->Header.DataSize) - i_ObjectOutSizeReq;

					if (Diff > 0)
						e = e_ComDataLengthMoreThenAllocatedSize;
					else
					{
						// Validate protocol result
						e = ProtocolErr2ComErr((e_ProtocoleError)(p->Header.ProtocolStatus));
					}

					v_InitReciveState(p);// init protocol state

					// relese the handshake
					if(p->UserHandShake.HandShakeUserFunction)// send to other side Stop sending
						p->UserHandShake.HandShakeUserFunction(p->Handler,e_Hand_SH_Permit2Send);

					// Job Complete
					p->i_FlagIgnoreRegulerTimer=0;

					// Exit
					MUTEX_EXIT(e);
				}
			}
		}

	}while((curr-start)<i_TimeOutMs);// test timeout

	// Job Complete
	p->i_FlagIgnoreRegulerTimer=0;

	if ((curr-start) >= i_TimeOutMs)
		// Exit on timeout
		MUTEX_EXIT(e_ComTimeOut)
	else
		MUTEX_EXIT(e);
}

/*/////////////////////////////////////////////////////////////////////////////////////////

function:
v_OnByteReceive

description:
received

return :
e_ComResult

///////////////////////////////////////////////////////////////////////////////////////////
*/

// Client 232 driver call to this function at receve byte (i_IsWorkingWithByteEvent=1)

void v_OnByteReceive(COM_HANDLER p_handler , //[IN]
	unsigned char *p_Characters, // [IN]
	int size//[IN]
	)
{
	int i;
	St_UserData  *p = p_handler;//p_FindChannelByCOMHndl(p_handler);

	if (!p)
		return;

	// send all bytes to protocol handler
	for(i=0;i< size;i++)
	{
		Privatev_OnByteReceive(uart_fd,p_Characters[i]);

	}
}

// return the number of bytes that the driver need to mannage Queue with  i_QitemCount items
// the caller must call to this function befor InitProtocolChannel for now how many bytes he need to  alocate for Queue
int  i_ProtocolCalcAlocateSizeForQ(int i_QitemCount)
{
	return sizeof(St_Q_Item)*i_QitemCount;
}

/*/////////////////////////////////////////////////////////////////////////////////////////

function:
v_OnByteReceive

description:
received

return :
e_ComResult

///////////////////////////////////////////////////////////////////////////////////////////
*/




