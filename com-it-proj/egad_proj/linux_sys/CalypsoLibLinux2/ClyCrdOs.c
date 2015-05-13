
#include <Core.h>
#if defined(CORE_SUPPORT_SMARTCARD) && defined(CORE_SUPPORT_CALYPSO)

#include <ClyCrdOs.h>
#include <ClySessn.h>
#include <Iso7816.h>
#include <string.h>
//#ifdef ENABLE_COMM
#include <AppProtocol.h>
//#endif
///////////////////////////////////////////////////////////////////////////////////////
//
//     Abstract: ClyCrd.c
//         
//         
//
///////////////////////////////////////////////////////////////////////////////////////

#ifndef IS_RES_OBJ_OK
#define IS_RES_OBJ_OK(Obj)  (Obj->sw1_sw2[0] == 0x90 && Obj->sw1_sw2[1] == 0)
#endif


////////////////////////////////////////////////////////////////////////////////////
//                                     ENUMS && DEFINES
////////////////////////////////////////////////////////////////////////////////////

#define RATIFICATION 2
#define NUMBER_START_INDEX 12     // Here start ser number in the atr
#define GET_RESPONSE_TIMEOUT  100 //(31/8/10 no change)
#define OPEN_SESSION_TIMEOUT  100 //(31/8/10 no change)
#define CLOSE_SESSION_TIMEOUT 400 //(31/8/10 no change)
#define READ_TIMEOUT          100 //(31/8/10 no change)
#define WRITE_TIMEOUT         100 //(31/8/10 no change)
#define GET_CHELLANGE_TIMEOUT 100 //(31/8/10 no change)
#define CHANGE_KEY_TIMEOUT    100 //(31/8/10 no change)
#define REHAB_TIMEOUT         100 //(31/8/10 no change)
#define MAX_READER_COUNT 2 // One contact and one contactless

#define READ_RETRIES  1



////////////////////////////////////////////////////////////////////////////////////
//                                     STRUCTURES
////////////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	e_7816_DEVICE SamReaderId;				//  sam reader id
	clyCard_BOOL InitFlag;					//  init interface flag
	St_clyCard_SN CardSN;					//  serial number in the 4 & 8 byte format
	eClyCardTypes CardType;					//  type of card cd_light or cd97 or cd97bx or another   
}clyCardData;


////////////////////////////////////////////////////////////////////////////////////
//                                     GLOBALS
////////////////////////////////////////////////////////////////////////////////////

#define SESSION_STATE       0xAA

#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif

#ifdef win_or_linux
#define IF_7816_PROTOCOL_T0(ReaderID) if(1)
#else
#define IF_7816_PROTOCOL_T0(ReaderID) if(0)
#endif



static RESPONSE_OBJ         *p_CallbackAns=0;
static RESPONSE_OBJ         *p_CardAns=0;
static clyCardData          clyCardDataArr[e_7816_LAST];
static SESSION_CALLBACK     SessionCallBack;
unsigned char               c_SessionState=0;
static PACKET_7816          packet;

#ifdef ENABLE_COMM
static RESPONSE_OBJ         obj;
#endif

////////////////////////////////////////////////////////////////////////////////////
//                                    INTERNAL FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////

static void             InitAllCardsDataStructs (clyCard_BOOL b);
static void             InitCardsDataStructs    (int index);
static RESPONSE_OBJ*    GetRespone              (int readerid,int lenOut_LE);

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: 
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef ENABLE_COMM
RESPONSE_OBJ* GetRespone(int readerid,int lenOut_LE)
{
	RESPONSE_OBJ* ans=0;
	static PACKET_7816 p7816;

	p7816.CLA       = 0;
	p7816.INS       = 0xc0;
	p7816.LC        = 0;
	p7816.LE        = lenOut_LE;
	p7816.P1        = 0;
	p7816.P2        = 0;

	ans = _7816_CardInOut(&p7816,(e_7816_DEVICE)readerid,GET_RESPONSE_TIMEOUT);
	if(ans && ans->sw1_sw2[0]==0x6c)
	{
		p7816.LE=ans->sw1_sw2[1];
		ans=_7816_CardInOut(&p7816,(e_7816_DEVICE)readerid,GET_RESPONSE_TIMEOUT);
	}
	
    p_CardAns=ans;
	ClyTerminalDebug("GetRespone",&p7816,ans);
	
    return ans;
}
#endif //#ifdef ENABLE_COMM
////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: 
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

void InitCardsDataStructs(int index)
{
	clyCardDataArr[index].InitFlag      =   clyCard_FALSE;
	clyCardDataArr[index].SamReaderId   =   e_7816_LAST;   // Invalid sam device id
	memset(&clyCardDataArr[index].CardSN,0,sizeof(clyCardDataArr[index].CardSN));
	clyCardDataArr[index].CardType      =   eClyCardUNKNOWN;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: 
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

void InitAllCardsDataStructs(clyCard_BOOL b)
{
	int i;
	static clyCard_BOOL ready=clyCard_FALSE;

	if(b==clyCard_FALSE)
		ready=b;

	if(ready==clyCard_TRUE)
		return;     

	for(i=0;i<e_7816_LAST;i++)
		InitCardsDataStructs(i);


	ready = clyCard_TRUE;
}

////////////////////////////////////////////////////////////////////////////////////
//                                    API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: initilize the interface 
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////


clyCard_BOOL b_ClyCard_InitInterface(st_ReaderComInfo *op_CardReaderIdArr, // [IN] Card reader ID array
	unsigned char uc_ArrLen)                                                                // [IN] Card reader ID array len

{

#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;
	St_ClyCard_initReq  req;
	int i;

	for(i=0; i < uc_ArrLen; i++)
	{
		req.CardReaderIdArr[i] = op_CardReaderIdArr[i];
#ifdef WIN32
		_DEBUG_PRINT_EX( e_CmdK10_ClyCard_Init, 1," ID %d", op_CardReaderIdArr[i].mPairedReaderId)
#endif
	}

	req.uc_ArrLen = uc_ArrLen;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_Init, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 0,	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 0,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK 
		if(ApplicationError)
		{
#ifdef WIN32
			_DEBUG_PRINT(e_CmdK10_ClyCard_Init,ApplicationError)
#endif
			return clyCard_TRUE;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_ClyCard_Init,ApplicationError)
#endif
    return clyCard_FALSE;
#else
	unsigned char i,readers = 0;

	// It's run one time only
	InitAllCardsDataStructs(clyCard_TRUE);

	for(i=0;i<uc_ArrLen;i++)
	{
		//Dima & Yoni  02/2012 handle case when already init - return true
		if(clyCardDataArr[i].InitFlag==clyCard_TRUE)
		{
			readers++;
		}
		else if(op_CardReaderIdArr[i].mIsExist)
		{
			if((_7816_Init((eCoreUARTId)op_CardReaderIdArr[i].mUART, (e_7816_DEVICE)i, USE_7816_SAM_HIGH_SPEED)==e_7816_STATUS_OK))
			{
				clyCardDataArr[i].InitFlag=clyCard_TRUE;
				readers++;
			}
		}
	}

	if(readers > 0) 
        return clyCard_TRUE;
    else
        return clyCard_FALSE;
#endif    
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: close the interface 
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

clyCard_BOOL b_ClyCard_CloseInterface(void)
{
	unsigned char i,uc_ReaderArrLen = (sizeof(clyCardDataArr)/sizeof(clyCardDataArr[0]));
	clyCard_BOOL b = clyCard_TRUE;
	for(i=0;i<uc_ReaderArrLen;i++)
	{
		// If reader open
		if(clyCardDataArr[i].InitFlag==clyCard_TRUE)
		{
			if (_7816_CloseReader((e_7816_DEVICE)i) != e_7816_STATUS_OK)
				b = clyCard_FALSE;
			// Return data struct to init state
			InitCardsDataStructs(i);
		}
	}
	return b;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: 
// Parameters:
// Return: return clyCard_TRUE if card in
//
////////////////////////////////////////////////////////////////////////////////////

clyCard_BOOL b_ClyCard_DetectCard(e_7816_DEVICE ReaderId)    // [IN] SAM reader id
{
	clyCard_BOOL bb;
#ifdef ENABLE_COMM
	int ApplicationError;
	St_ClyCard_DetectCard		req;
	int		RecvSize;

	bb = clyCard_FALSE;
	req.ReaderId = ReaderId;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,			//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_DetectCard, 		//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,				//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),						//int i_ObjectInSize,//[IN]// the data size to send
							 &req,								//void *p_ObjectIn,//[IN] the data 
							 0,									//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 0,									//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,							//int *p_OutSizeArive,//[IN]// the size of data respond
							 1,									//unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk )	//int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK
		if(ApplicationError)
		{
			bb = clyCard_TRUE;
		}
		
	}
#ifdef WIN32
	_DEBUG_PRINT_EX(e_CmdK10_ClyCard_DetectCard,ApplicationError, "ID %d",ReaderId)
#endif
#else
	bb=(clyCard_BOOL)(_7816_IsSmartCardIn((e_7816_DEVICE)ReaderId));
#endif
	// Empty the serial number data if card not exist
	if(bb==clyCard_FALSE)
		memset(&clyCardDataArr[ReaderId].CardSN,0,sizeof(clyCardDataArr[ReaderId].CardSN));
	return bb;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: reset sam
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* pSt_ClyCard_Reset( e_7816_DEVICE ReaderId,   //[IN]  SAM reader id
	clyCard_BYTE p_Atr[CLY_CARD_MAX_ATR_LEN],                   //[OUT] the card atr
	int *ip_AtrLen,eClyCardTypes* type)                         //[OUT] the atr Length
{

#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;
	RESPONSE_OBJ *p_Ans;
	St_ClyCard_ResetReq  req;
	St_ClyCard_ResetResp resp;

	req.ReaderId = ReaderId;
	p_Ans = NULL;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_Reset, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK 
		if(ApplicationError)
		{
			obj = resp.obj;
			p_Ans = &obj;
			memcpy ((void*) p_Atr, (void*) resp.p_Atr, (size_t) resp.iAtrLen);
			*ip_AtrLen = resp.iAtrLen;
			*type = (eClyCardTypes)resp.eClyCardType;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_ClyCard_Reset,ApplicationError)
#endif
#else
	RESPONSE_OBJ *p_Ans=0;
	char str[5]={0};
	st_7816_CardResetInfo stp_CardResetInfo;


	// reader not open 
	if(ip_AtrLen)
		*ip_AtrLen=0;

	p_Ans = _7816_ResetCard((e_7816_DEVICE)ReaderId,100);
	if(p_Ans && p_Ans->sw1_sw2[0]!=0)
	{

		_7816_GetCardResetInfo((e_7816_DEVICE)ReaderId,&stp_CardResetInfo);
		if(stp_CardResetInfo.b_IsReaderInit)
		{
			clyCardDataArr[ReaderId].InitFlag=clyCard_TRUE;

			*ip_AtrLen=stp_CardResetInfo.uc_CardAtrSetectDataLen;
			memcpy(p_Atr,stp_CardResetInfo.uc_AtrSetectData,stp_CardResetInfo.uc_CardAtrSetectDataLen);
			memcpy(clyCardDataArr[ReaderId].CardSN.FullSerNum8+4,stp_CardResetInfo.cp_ClUid,4);

			memcpy(str,stp_CardResetInfo.cp_ClUid,4);
			clyCardDataArr[ReaderId].CardSN.p_SerNum4=(long)(*(long*)(str));
			clyCardDataArr[ReaderId].CardType=(*type)=(eClyCardTypes)stp_CardResetInfo.e_CardType;
		}
	}
#endif
	return p_Ans;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: in case the card reset was made outside this layer - 
//              update the layer state machine with it's require information
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

void b_ClyCard_StartWorkWithCard(e_7816_DEVICE CardReaderId, //[IN]  card reader id
	e_7816_DEVICE SamReaderId,                                                //[IN]  sam reader id
	St_clyCard_SN *stp_CardSN)                                      //[IN]  serial number in the 
{
#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;

	St_ClyCard_StartWorkWithCardReq  req;
	St_ClyCard_StartWorkWithCardResp resp;

	req.CardReaderId = CardReaderId;
	req.SamReaderId  = SamReaderId;
	ApplicationError = 0;
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_StartWorkWithCard, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk )//int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK 
		if(ApplicationError)
		{
			*stp_CardSN = resp.SerNum;
		}
	}
#ifdef WIN32
		_DEBUG_PRINT(e_CmdK10_ClyCard_StartWorkWithCard,ApplicationError)
#endif
#else
    char str[5]={0};
	st_7816_CardResetInfo  stp_CardResetInfo;

	memset(&stp_CardResetInfo,0,sizeof(stp_CardResetInfo));
	_7816_GetCardResetInfo((e_7816_DEVICE)CardReaderId,&stp_CardResetInfo);

	clyCardDataArr[CardReaderId].InitFlag = (clyCard_BOOL)stp_CardResetInfo.b_IsReaderInit;
	if(stp_CardResetInfo.e_CardType==e_7816_Cly_CTS256B || (stp_CardResetInfo.uc_CardAtrSetectDataLen && stp_CardResetInfo.uc_CardAtrSetectDataLen<=CLY_CARD_MAX_ATR_LEN))
	{
		if(stp_CardResetInfo.e_CardType==e_7816_Cly_CTS256B)
		{
			memcpy(clyCardDataArr[CardReaderId].CardSN.FullSerNum8,stp_CardResetInfo.cp_ClUid,8);
			memcpy(str,stp_CardResetInfo.cp_ClUid+4,4);
		}
		else
		{
			memcpy(clyCardDataArr[CardReaderId].CardSN.FullSerNum8+4,stp_CardResetInfo.cp_ClUid,4);
			memcpy(str,stp_CardResetInfo.cp_ClUid,4);
		}

		clyCardDataArr[CardReaderId].CardSN.p_SerNum4=(long)(*(long*)(str));
		clyCardDataArr[CardReaderId].CardType=(eClyCardTypes)stp_CardResetInfo.uc_AtrSetectData[6];
		memcpy(stp_CardSN,&clyCardDataArr[CardReaderId].CardSN,sizeof(clyCardDataArr[CardReaderId].CardSN));
	}
	else
		memset(stp_CardSN,0,sizeof(St_clyCard_SN));

	clyCardDataArr[CardReaderId].SamReaderId=SamReaderId;
#endif
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: eject card card
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

clyCard_BOOL b_ClyCard_EjectCard(e_7816_DEVICE ReaderId)
{
#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;
	St_ClyCard_EjectCard  req;
    St_ClyApp_Debug       resp;

	req.ReaderId = ReaderId;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,			//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_EjectCard, 		//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,				//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),						//int i_ObjectInSize,//[IN]// the data size to send
							 &req,								//void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),//0,									//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,//0,									//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,							//int *p_OutSizeArive,//[IN]// the size of data respond
							 1,									//unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk )	//int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK 
		if(ApplicationError)
		{
#ifdef WIN32
			_DEBUG_PRINT_EX(e_CmdK10_ClyCard_EjectCard,ApplicationError,"Tdiff %d",resp.id)
#endif
			return clyCard_TRUE;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_ClyCard_EjectCard,ApplicationError)
#endif
    return clyCard_FALSE;
#else


	clyCard_BOOL bb;


	bb = (clyCard_BOOL)_7816_EjectCard((e_7816_DEVICE)ReaderId);      // [IN] the reader id
    //clear user callback
    v_ClyCard_SetSessionCallBack((SESSION_CALLBACK)0);

	// Empty the serial number data
	memset(&clyCardDataArr[ReaderId].CardSN,0,sizeof(clyCardDataArr[ReaderId].CardSN));
	clyCardDataArr[ReaderId].CardType=eClyCardUNKNOWN;
	return bb; 
#endif
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: get sam serial number - 4 bytes long 
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

clyCard_BOOL b_ClyCard_GetSerNum(e_7816_DEVICE ReaderId, // [IN] SAM reader id
	St_clyCard_SN *St_SN,eClyCardTypes* type)                   // [OUT] card serial numer
{

#ifdef ENABLE_COMM
	clyCard_BOOL ret;
	int ApplicationError;
	int RecvSize;
	St_ClyCard_SerNumReq  req;
	St_ClyCard_SerNumResp	resp;

	req.ReaderId = ReaderId;
	ret = clyCard_FALSE;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_GetSerNum, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK 
		if(ApplicationError)
		{
			*St_SN = resp.SerNum;
			*type = (eClyCardTypes)resp.type; 
			ret = clyCard_TRUE;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_ClyCard_GetSerNum,ApplicationError)
#endif
	return ret;
#else

	// Reader not open 
	if(clyCardDataArr[ReaderId].InitFlag==clyCard_FALSE)
		return clyCard_FALSE;

	memcpy(St_SN,&clyCardDataArr[ReaderId].CardSN,sizeof(clyCardDataArr[ReaderId].CardSN));
	(*type)=clyCardDataArr[ReaderId].CardType;
	return clyCard_TRUE;
#endif
}
#ifndef ENABLE_COMM
////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: Open Secure Session
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

static RESPONSE_OBJ*  PRV_pSt_ClyCard_OpenSecureSession(e_7816_DEVICE CardReaderId,   //[IN]  card reader id
	e_7816_DEVICE SamReaderId,                                                            //[IN]  sam reader id
	const St_clyCard_OpenSessionInput *St_OpenSessionInput,                     //[IN]  Open Session Input parameters
	St_clyCard_OpenSessionOutput *St_OpenSessionOutput)                         //[OUT] Open Session output parameters
{
	clyCard_BYTE bytik;     

	RESPONSE_OBJ *p_Ans;

	// Reader not open 
	if(clyCardDataArr[CardReaderId].InitFlag==clyCard_FALSE)
		return 0;

	clyCardDataArr[CardReaderId].SamReaderId=SamReaderId;

	packet.CLA  = 0x94;
	packet.INS  = 0x8a;
	packet.LC   = 4;

	// Random number
	memcpy(packet.Data,St_OpenSessionInput->Random4,4); 

	//   kvc,record number for return,key number
	packet.P1=0;
	packet.P1=(1&St_OpenSessionInput->b_Is2ReturnKeyKvc)<<7;
	if(St_OpenSessionInput->RecNum2Return)
	{
		bytik=(/*1<<*/(St_OpenSessionInput->RecNum2Return-1)*8);
		packet.P1|=bytik;
	}
	bytik = (St_OpenSessionInput->KeyType/*-1*/);
	packet.P1|=bytik;

	// File to select
	packet.P2=St_OpenSessionInput->FileToSelect*8;

	// Len to respon
	packet.LE=4+St_OpenSessionInput->b_Is2ReturnKeyKvc+RATIFICATION;
	if(St_OpenSessionInput->RecNum2Return)
		packet.LE+=REC_SIZE;

	// Send
	p_Ans=_7816_CardInOut(&packet,(e_7816_DEVICE)CardReaderId,OPEN_SESSION_TIMEOUT);
	if(p_Ans && p_Ans->sw1_sw2[0]==0x90 && p_Ans->sw1_sw2[1]==0)// && p_Ans->Len)
	{
		
        IF_7816_PROTOCOL_T0(CardReaderId)
			p_Ans=GetRespone(CardReaderId,packet.LE);
        
        if(p_Ans && p_Ans->sw1_sw2[0]==0x90 && p_Ans->sw1_sw2[1]==0 && p_Ans->Len)
		{
			//  Save session state
			c_SessionState=(char)SESSION_STATE;
			bytik = 0;

			//  kvc
			if(St_OpenSessionInput->b_Is2ReturnKeyKvc)
			{
				memcpy(&St_OpenSessionOutput->KeyKVC,p_Ans->data,1);
				bytik=1;
			}

			// Random data
			memcpy(St_OpenSessionOutput->Certif2Sam,p_Ans->data+bytik,4);
			bytik+=4;

			// Ratification
			// No ratification
			if(p_Ans->Len==packet.LE-RATIFICATION)
			{
				St_OpenSessionOutput->St_Ratif.b_IsRatifExist=clyCard_FALSE;
				St_OpenSessionOutput->St_Ratif.RatifLen=0;
			}
			// Ratification exist
			else
			{
				St_OpenSessionOutput->St_Ratif.b_IsRatifExist=clyCard_TRUE;
				if(p_Ans->Len==packet.LE)
				{
					St_OpenSessionOutput->St_Ratif.RatifLen=RATIFICATION;
					memcpy(St_OpenSessionOutput->St_Ratif.RatifVal,p_Ans->data+bytik,RATIFICATION);
					bytik+=RATIFICATION;
				}
				else
				{
					St_OpenSessionOutput->St_Ratif.RatifLen=RATIFICATION+2;
					memcpy(St_OpenSessionOutput->St_Ratif.RatifVal,p_Ans->data+bytik,RATIFICATION+2);
					bytik+=(RATIFICATION+2);
				}
			} // else ratification exist

			// record data
			if(St_OpenSessionInput->RecNum2Return)
				memcpy(St_OpenSessionOutput->RecDataRead,p_Ans->data+bytik,REC_SIZE);
		} // if(switc of respone)
	} // if(switch)

	p_CardAns=p_Ans;

	ClyTerminalDebug("pSt_ClyCard_OpenSecureSession",&packet,p_CardAns);
	return p_Ans;   
}

RESPONSE_OBJ*  pSt_ClyCard_OpenSecureSession(e_7816_DEVICE CardReaderId,   //[IN]  card reader id
	e_7816_DEVICE SamReaderId,                                                            //[IN]  sam reader id
	const St_clyCard_OpenSessionInput *St_OpenSessionInput,                     //[IN]  Open Session Input parameters
	St_clyCard_OpenSessionOutput *St_OpenSessionOutput)                         //[OUT] Open Session output parameters
{
	RESPONSE_OBJ* p;
	p=PRV_pSt_ClyCard_OpenSecureSession(CardReaderId,SamReaderId,St_OpenSessionInput,St_OpenSessionOutput);
		if(!IS_RES_OBJ_OK(p))
			{
				p++;
				p--;
			}
			
	return p;		
	
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: Close Secure Session
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

static RESPONSE_OBJ*  PRV_pSt_ClyCard_CloseSecureSession(e_7816_DEVICE CardReaderId,  //[IN]  card reader id
	e_7816_DEVICE SamReaderId,                                                            //[IN]  sam reader id
	clyCard_BOOL b_IsRatifyImmediatly,                                          //[IN]  1= the session will be immediately Ratified
	const CERTIF4 CertifHiIn,                                                   //[IN]  To send to the card from the sam  
	CERTIF4 CertifLoOut)                                                        //[OUT] To send to the sam from the card
{

	RESPONSE_OBJ *p_Ans;

	// Reader not open 
	if(clyCardDataArr[CardReaderId].InitFlag==clyCard_FALSE)
		return 0;

	packet.CLA      = 0x94;
	packet.INS      = 0x8e;
	if(b_IsRatifyImmediatly)
		packet.P1   = 0x80;
	else
		packet.P1   = 0;
	packet.P2       = 0;
	packet.LC       = 4;

	memcpy(packet.Data,CertifHiIn,4);
	packet.LE=4;
	p_Ans=_7816_CardInOut(&packet,(e_7816_DEVICE)CardReaderId,CLOSE_SESSION_TIMEOUT);
	if(p_Ans && p_Ans->sw1_sw2[0]==0x90 && p_Ans->sw1_sw2[1]==0)// && p_Ans->Len==4)
	{   
		c_SessionState=0;
		
        IF_7816_PROTOCOL_T0(CardReaderId)
			p_Ans=GetRespone(CardReaderId,packet.LE);
		
        if(p_Ans && p_Ans->sw1_sw2[0]==0x90 && p_Ans->sw1_sw2[1]==0 && p_Ans->Len)
			memcpy(CertifLoOut,p_Ans->data,p_Ans->Len);
	}
	p_CardAns=p_Ans;

	ClyTerminalDebug("pSt_ClyCard_CloseSecureSession",&packet,p_CardAns);
	
 

	return p_Ans;
}

RESPONSE_OBJ*  pSt_ClyCard_CloseSecureSession(e_7816_DEVICE CardReaderId,		//[IN]  card reader id
	e_7816_DEVICE SamReaderId,                                                  //[IN]  sam reader id
	clyCard_BOOL b_IsRatifyImmediatly,                                          //[IN]  1= the session will be immediately Ratified
	const CERTIF4 CertifHiIn,                                                   //[IN]  To send to the card from the sam  
	CERTIF4 CertifLoOut)                                                        //[OUT] To send to the sam from the card
{
	RESPONSE_OBJ  *p;
	
	p=PRV_pSt_ClyCard_CloseSecureSession(CardReaderId,SamReaderId,b_IsRatifyImmediatly,
	CertifHiIn,CertifLoOut);
	
		if(!IS_RES_OBJ_OK(p))
			{
				p++;
				p--;
			}
			
	return p;		
	
}


////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: Read Record
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

static RESPONSE_OBJ*  PRV_pSt_ClyCard_ReadRecord(e_7816_DEVICE ReaderId,    //[IN] reader id
	clyCard_BYTE RecNum2Read,                                           //[IN] record number to read - 1 is always the first record
	e_clyCard_FileId FileToSelect,                                      //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
	clyCard_BYTE Len2Read,                                              //[IN] len to read - 1 to record size
	clyCard_BYTE RecDataOut[REC_SIZE])                                  //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
{

	RESPONSE_OBJ *p_Ans;


	packet.CLA=0x94;
	packet.INS=0xb2;

	// Number of record
	packet.P1=RecNum2Read;

	// File select
	packet.P2=(FileToSelect*8)+4;

	// Len in empty
	packet.LC=0;

	// Len to read
	packet.LE=Len2Read;
	p_Ans=_7816_CardInOut(&packet,(e_7816_DEVICE)ReaderId,READ_TIMEOUT);
	packet.LC=0;
	p_CardAns=p_Ans;
	ClyTerminalDebug("pSt_ClyCard_ReadRecord",&packet,p_Ans);

	if(p_Ans && p_Ans->sw1_sw2[0]==0x90 && p_Ans->sw1_sw2[1]==0 && p_Ans->Len)
	{
		memcpy(RecDataOut,p_Ans->data,p_Ans->Len);

		// call the SAM
		if(SessionCallBack &&c_SessionState==(unsigned char)SESSION_STATE)
		{
			p_CallbackAns=SessionCallBack(&packet,p_Ans->sw1_sw2,clyCardDataArr[ReaderId].SamReaderId);

			// SAM receive error
			if(!(p_CallbackAns && p_CallbackAns->sw1_sw2[0]==0x90 && p_CallbackAns->sw1_sw2[1]==0))
				return p_CallbackAns;

		}
	} // if switch

	return p_Ans;
}
#endif
RESPONSE_OBJ*  pSt_ClyCard_ReadRecord(e_7816_DEVICE ReaderId,    //[IN] reader id
	clyCard_BYTE RecNum2Read,                                           //[IN] record number to read - 1 is always the first record
	e_clyCard_FileId FileToSelect,                                      //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
	clyCard_BYTE Len2Read,                                              //[IN] len to read - 1 to record size
	clyCard_BYTE RecDataOut[REC_SIZE],                                  //[OUT] data read - th
	clyCard_BYTE ForceRead)
{

#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;
	RESPONSE_OBJ *p;
	St_ClyCard_ReadRecordReq  req;
	St_ClyCard_ReadRecordResp resp;

	//Set in data
	req.ReaderId	 = ReaderId;
	req.RecNum2Read	 = RecNum2Read;
	req.FileToSelect = FileToSelect;
	req.Len2Read	 = Len2Read;
	req.ForceRead    = ForceRead;

	p = NULL;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_ReadRecord, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK 
		if(ApplicationError)
		{
			//obj = resp.obj;
			obj.sw1_sw2[0] = resp.sw1_sw2[0];
			obj.sw1_sw2[1] = resp.sw1_sw2[1];
			p = &obj;
			memcpy (RecDataOut, resp.RecDataOut, REC_SIZE);
		}
	}
#ifdef WIN32
	_DEBUG_PRINT_EX(e_CmdK10_ClyCard_ReadRecord,ApplicationError," Type %d Num %d ",FileToSelect,RecNum2Read)
#endif
#else

	int i;
	RESPONSE_OBJ*p;
	
	for(i=0;i<READ_RETRIES;i++)
	{
		p=PRV_pSt_ClyCard_ReadRecord(ReaderId,RecNum2Read,FileToSelect,Len2Read,RecDataOut);
		if(IS_RES_OBJ_OK(p))
			return p;
		//else
		//{
		//	p++;
		//	p--;
		//}
			
	}
#endif

	return p;
}


#ifndef ENABLE_COMM
////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: Write Record
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ*  PRV_pSt_ClyCard_WriteRecord(e_7816_DEVICE ReaderId, //[IN] reader id
	clyCard_BYTE RecNum,                                            //[IN] record number to Write - 1 is always the first record
	e_clyCard_FileId FileToSelect,                                  //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
	clyCard_BYTE Len2Write,                                         //[IN] len to Write - 1 to record size
	clyCard_BYTE *RecData2Write)                                    //[IN] data to write will be binary OR with the existing data - in case len<Full recode size, the record will be padded with zeroes
{
	RESPONSE_OBJ *p_Ans;

	// reader not open 
	if(clyCardDataArr[ReaderId].InitFlag==clyCard_FALSE)
		return 0;

	packet.CLA=0x94;
	packet.INS=0xd2;

	// number of record
	packet.P1=RecNum;

	// file select
	packet.P2=(FileToSelect*8)+4;

	// len in 
	packet.LC=Len2Write;

	// data write
	memcpy(packet.Data,RecData2Write,packet.LC);

	// len to read empty
	packet.LE=0;

	p_Ans=_7816_CardInOut(&packet,(e_7816_DEVICE)ReaderId,WRITE_TIMEOUT);
	p_CardAns=p_Ans;
	if(p_Ans && p_Ans->sw1_sw2[0]==0x90 && p_Ans->sw1_sw2[1]==0)
	{
		// call the SAM
		if(SessionCallBack)
		{
			p_CallbackAns=SessionCallBack(&packet,p_Ans->sw1_sw2,clyCardDataArr[ReaderId].SamReaderId);
			// SAM receive error
			if(!(p_CallbackAns && p_CallbackAns->sw1_sw2[0]==0x90 && p_CallbackAns->sw1_sw2[1]==0))
			{
				ClyTerminalDebug("pSt_ClyCard_WriteRecord",&packet,p_Ans);
				return p_CallbackAns;
			}
		}
	} // if switch

	ClyTerminalDebug("pSt_ClyCard_WriteRecord",&packet,p_Ans);

	return p_Ans;
}
#endif

RESPONSE_OBJ*  pSt_ClyCard_WriteRecord(e_7816_DEVICE ReaderId, //[IN] reader id
	clyCard_BYTE RecNum,                                            //[IN] record number to Write - 1 is always the first record
	e_clyCard_FileId FileToSelect,                                  //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
	clyCard_BYTE Len2Write,                                         //[IN] len to Write - 1 to record size
	clyCard_BYTE *RecData2Write)                                    //[IN] data to wr
{
#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;
	RESPONSE_OBJ *p;
	St_ClyCard_WriteRecordReq  req;
	St_ClyCard_WriteRecordResp resp;

	//Set in data
	req.ReaderId	 = ReaderId;
	req.RecNum		 = RecNum;
	req.FileToSelect = FileToSelect;
	req.Len2Write	 = Len2Write;
	// TBD:yoram make sure: (Len2Write <= REC_SIZE)
	memcpy(req.RecDataOut, RecData2Write, Len2Write); 

	p = NULL;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_WriteRecord, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK 
		if(ApplicationError)
		{
			obj.sw1_sw2[0] = resp.sw1_sw2[0];
			obj.sw1_sw2[1] = resp.sw1_sw2[1];
			p = &obj;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT_EX(e_CmdK10_ClyCard_WriteRecord,ApplicationError," Type %d Num %d ",FileToSelect,RecNum)
#endif
#else
	
	
	RESPONSE_OBJ *p =
	 PRV_pSt_ClyCard_WriteRecord(ReaderId,RecNum,FileToSelect,
	
	Len2Write,RecData2Write);
	
	if(!IS_RES_OBJ_OK(p))
	{
		p++;
		p--;
		
	}
#endif
	return p;
}
////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: Update Record
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ*  pSt_ClyCard_UpdateRecord(e_7816_DEVICE ReaderId,        //[IN] reader id
	clyCard_BYTE RecNum,                                                    //[IN] record number to Write - set to 1 for cyclic or counter EF
	e_clyCard_FileId FileToSelect,                                          //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
	clyCard_BYTE Len2Update,                                                //[IN] len to Write - 1 to record size 
	clyCard_BYTE *RecData2Update)                                           //[IN] data to Update - in case len<Full recode size, the record will be padded with zeroes

{
	RESPONSE_OBJ *p_Ans;
#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;
	St_ClyCard_UpdateRecordReq  req;
	St_ClyCard_UpdateRecordResp resp;

	//Set in data
	req.ReaderId	 = ReaderId;
	req.RecNum		 = RecNum;
	req.FileToSelect = FileToSelect;
	req.Len2Write	 = Len2Update;
	// TBD:yoram make sure: (Len2Write <= REC_SIZE)
	memcpy(req.RecDataOut, RecData2Update, Len2Update); 

	p_Ans = NULL;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_UpdateRecord, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK 
		if(ApplicationError)
		{
			obj.sw1_sw2[0] = resp.sw1_sw2[0];
			obj.sw1_sw2[1] = resp.sw1_sw2[1];
			p_Ans = &obj;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_ClyCard_UpdateRecord,ApplicationError)
#endif
#else
	//   reader not open 
	if(clyCardDataArr[ReaderId].InitFlag==clyCard_FALSE)
		return 0;

	// Patch to resolve cyclic append problem
	if( FileToSelect == e_clyCard_EventLogFile )
	{
		packet.CLA=0x94;
		packet.INS=0xe2;

		// number of record
		packet.P1=0;

		// file select
		packet.P2=(FileToSelect*8);
	}
	else
	{
		packet.CLA=0x94;
		packet.INS=0xdc;

		// number of record
		packet.P1=RecNum;
		// file select
		packet.P2=(FileToSelect*8)+4;
	}
	// len in 
	packet.LC=Len2Update;

	// data write
	memcpy(packet.Data,RecData2Update,packet.LC);

	// len to read empty
	packet.LE=0;


	p_Ans=_7816_CardInOut(&packet,(e_7816_DEVICE)ReaderId,WRITE_TIMEOUT);
	p_CardAns=p_Ans;
	ClyTerminalDebug("pSt_ClyCard_UpdateRecord",&packet,p_Ans);

	if(p_Ans && p_Ans->sw1_sw2[0]==0x90 && p_Ans->sw1_sw2[1]==0)
	{
		// call the SAM
		if(SessionCallBack)
		{
			p_CallbackAns=SessionCallBack(&packet,p_Ans->sw1_sw2,clyCardDataArr[ReaderId].SamReaderId);
			// SAM receive error
			if(!(p_CallbackAns && p_CallbackAns->sw1_sw2[0]==0x90 && p_CallbackAns->sw1_sw2[1]==0))
				return p_CallbackAns;
		}
	} // if switch
#endif
	return p_Ans;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: 
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

void  v_ClyCard_SetSessionCallBack(SESSION_CALLBACK  Proc)
{
	SessionCallBack = Proc;     // if Proc = NULL - user callback is disabled 
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: Get Last Callback Response Object
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ*  pSt_ClyCard_GetLastCallbackResponseObj(void)
{
	return p_CallbackAns;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: 
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ*  pSt_ClyCard_GetLastCardResponseObj(void)
{
#ifdef ENABLE_COMM

	return &obj; // TBD:yoram not always contains the correct value
#else
	return p_CardAns;
#endif
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description:  Increase
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* pSt_ClyCard_IncreaseDecrease(
	e_7816_DEVICE ReaderId,                   //(IN)reader id
	clyCard_BYTE CountNumber,       //(IN) counter number
	e_clyCard_FileId FileToSelect,  //(IN) file name enum value
	clyCard_BYTE UpdateData[3],     // data to the card
	clyCard_BYTE NewCountData[3],   // new couner value
	clyCard_BYTE IsEncreaseFlag)    // 1 for increase 0 for decrease
{
	RESPONSE_OBJ *p_Ans;
#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;
	St_ClyCard_IncDecCntRecordReq  req;
	St_ClyCard_IncDecCntRecordResp resp;

	//Set in data
	req.ReaderId		= ReaderId;
	req.CountNumber		= CountNumber;    
	req.FileToSelect	= FileToSelect;	
	memcpy(req.UpdateData,	 UpdateData,   3);  
	memcpy(req.NewCountData, NewCountData, 3);  
	req.IsEncreaseFlag = IsEncreaseFlag; 

	p_Ans = NULL;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_IncreaseDecrease, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK 
		if(ApplicationError)
		{
			obj.sw1_sw2[0] = resp.sw1_sw2[0];
			obj.sw1_sw2[1] = resp.sw1_sw2[1];
			p_Ans = &obj;
			memcpy(NewCountData, resp.counter_val, 3);
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_ClyCard_IncreaseDecrease,ApplicationError)
#endif
#else
	// reader not open 
	if(clyCardDataArr[ReaderId].InitFlag==clyCard_FALSE)
		return 0;

	packet.CLA  =   0x94;
	if(IsEncreaseFlag)
		packet.INS=0x32;
	else
		packet.INS=0x30;
	// number of counter

	packet.P1=CountNumber;

	// file select
	packet.P2=(FileToSelect*8);

	// len in 
	packet.LC=3;

	// data write

	memcpy(packet.Data,UpdateData,packet.LC);

	// len to read
	packet.LE=3;
	p_Ans=_7816_CardInOut(&packet,(e_7816_DEVICE)ReaderId,WRITE_TIMEOUT);
	p_CardAns=p_Ans;
	if(p_Ans && p_Ans->sw1_sw2[0]==0x90 && p_Ans->sw1_sw2[1]==0)
	{
		IF_7816_PROTOCOL_T0(CardReaderId)
			p_Ans=GetRespone(ReaderId,packet.LE);

		// copy respon data to out object
		if(p_Ans && p_Ans->sw1_sw2[0]==0x90 && p_Ans->sw1_sw2[1]==0 && p_Ans->Len==packet.LE)
			memcpy(NewCountData,p_Ans->data,p_Ans->Len);
		else
			memset(NewCountData,0,3);

		// call the SAM
		if(SessionCallBack)
		{
			p_CallbackAns=SessionCallBack(&packet,p_Ans->sw1_sw2,clyCardDataArr[ReaderId].SamReaderId);
			// SAM receive error
			if(!(p_CallbackAns && p_CallbackAns->sw1_sw2[0]==0x90 && p_CallbackAns->sw1_sw2[1]==0))
				return p_CallbackAns;
		}
	} // if switch
#endif
	return p_Ans;
}
#ifndef ENABLE_COMM
////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description:  get chalenge
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* pSt_ClyCard_GetChalenge(
	e_7816_DEVICE ReaderId,           //  (IN)reader id
	clyCard_BYTE Chlng[8])  //  (OUT) chalenge data
{
	RESPONSE_OBJ *p_Ans;

	// reader not open 
	if(clyCardDataArr[ReaderId].InitFlag==clyCard_FALSE)
		return 0;

	packet.CLA=0x94;
	packet.INS=0x84;
	// empty
	packet.P1=0;
	// empty
	packet.P2=0;
	// len in empty
	packet.LC=0;
	// len to read 
	packet.LE=8;
	p_Ans=_7816_CardInOut(&packet,(e_7816_DEVICE)ReaderId,GET_CHELLANGE_TIMEOUT);

	if(p_Ans && p_Ans->sw1_sw2[0]==0x90 && p_Ans->sw1_sw2[1]==0)
	{
		IF_7816_PROTOCOL_T0(CardReaderId)
			p_Ans=GetRespone(ReaderId,packet.LE);

		// copy respon data to out object
		if(p_Ans && p_Ans->sw1_sw2[0]==0x90 && p_Ans->sw1_sw2[1]==0 && p_Ans->Len==packet.LE)
			memcpy(Chlng,p_Ans->data,p_Ans->Len);
		else
			memset(Chlng,0,0);

	} // if switch
	p_CardAns=p_Ans;

	ClyTerminalDebug("pSt_ClyCard_GetChalenge",&packet,p_Ans);

	return p_Ans;
}
#endif
////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: 
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* pSt_ClyCard_Invalidate(
	e_7816_DEVICE ReaderId)       // (IN)reader id
{
	RESPONSE_OBJ *p_Ans;
#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;
	St_ClyCard_InvalidateReq  req;
	St_ClyCard_InvalidateResp resp;

	//Set in data
	req.ReaderId		= ReaderId;

	p_Ans = NULL;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_Invalidate, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK 
		if(ApplicationError)
		{
			obj.sw1_sw2[0] = resp.sw1_sw2[0];
			obj.sw1_sw2[1] = resp.sw1_sw2[1];
			p_Ans = &obj;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_ClyCard_Invalidate,ApplicationError)
#endif
#else
	//   reader not open 
	if(clyCardDataArr[ReaderId].InitFlag==clyCard_FALSE)
		return 0;

	packet.CLA=0x94;
	packet.INS=0x4;
	//   empty
	packet.P1=0;
	//   empty
	packet.P2=0;
	//   len in empty
	packet.LC=0;
	//   len out empty
	packet.LE=0;


	p_Ans=_7816_CardInOut(&packet,(e_7816_DEVICE)ReaderId,WRITE_TIMEOUT);

	if(SessionCallBack &&c_SessionState==(unsigned char)SESSION_STATE)
	{
		p_CallbackAns=SessionCallBack(&packet,p_Ans->sw1_sw2,clyCardDataArr[ReaderId].SamReaderId);
		// SAM receive error
		if(!(p_CallbackAns && p_CallbackAns->sw1_sw2[0]==0x90 &&  p_CallbackAns->sw1_sw2[1]==0))
			return p_CallbackAns;
	}

	p_CardAns=p_Ans;
#endif
	return p_Ans;
}
#ifndef ENABLE_COMM
////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description:  change key
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* pSt_ClyCard_ChangeKey(
	e_7816_DEVICE ReaderId,                   //   (IN)   reader id
	e_clyCard_KeyType type,         //   (IN)   enum value of key for change
	clyCardKeyChangeLen ChngLen,    //   (IN)   enum value of message len
	clyCard_BYTE *msg)              //   (IN)   encrypting message from sam
{
	RESPONSE_OBJ *p_Ans;

	// reader not open
	if(clyCardDataArr[ReaderId].InitFlag==clyCard_FALSE)
		return 0;

	packet.CLA=0x94;
	packet.INS=0xd8;

	// empty
	packet.P1=0;

	// key type from 1 to 3
	packet.P2=type;

	// len in
	packet.LC=(unsigned char)ChngLen;

	// len to read empty
	packet.LE=0;
	// data for send to the card
	memcpy(packet.Data,msg,packet.LC);

	p_Ans=_7816_CardInOut(&packet,(e_7816_DEVICE)ReaderId,CHANGE_KEY_TIMEOUT*100);
	p_CardAns=p_Ans;
	return p_Ans;
}


////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: Rehabilitate (undo invalidate)
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* pSt_ClyCard_Rehabilitate(
	e_7816_DEVICE ReaderId)// (IN)reader id
{
	RESPONSE_OBJ *p_Ans;
	//   reader not open 
	if(clyCardDataArr[ReaderId].InitFlag==clyCard_FALSE)
		return 0;

	packet.CLA=0x94;
	packet.INS=0x44;
	//   empty
	packet.P1=0;
	//   empty
	packet.P2=0;
	//   len in empty
	packet.LC=0;
	//   len out empty
	packet.LE=0;

	p_Ans=_7816_CardInOut(&packet,(e_7816_DEVICE)ReaderId,REHAB_TIMEOUT);

	if(SessionCallBack &&c_SessionState==(unsigned char)SESSION_STATE)
	{
		p_CallbackAns=SessionCallBack(&packet,p_Ans->sw1_sw2,clyCardDataArr[ReaderId].SamReaderId);
		//   SAM receive error
		if(!(p_CallbackAns && p_CallbackAns->sw1_sw2[0]==0x90 &&  p_CallbackAns->sw1_sw2[1]==0))
			return p_CallbackAns;
	}

	p_CardAns=p_Ans;
	return p_Ans;

}
#endif
/////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    pSt_ClyCard_TestReadAndWrite
//              Eitan 13/3/2011
// Discription:  Test the card (tech mode) for read and write operations
// return: clyApp_TRUE - success, clyApp_FALSE - error
// LOGIC:
/////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ*  pSt_ClyCard_TestReadWrite(e_7816_DEVICE CardReaderId, e_7816_DEVICE SamReaderId)
{

#ifdef ENABLE_COMM
	RESPONSE_OBJ                     *p_Ans;
	int ApplicationError;
	int RecvSize;
	St_ClyCard_TestReadWriteReq  req;
	St_ClyCard_TestReadWriteResp resp;

	//Set in data
	req.CardReaderId		= CardReaderId;
	req.SamReaderId			= SamReaderId;

	p_Ans = NULL;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClyCard_TestReadWrite, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK 
		if(ApplicationError)
		{
			obj.sw1_sw2[0] = resp.sw1_sw2[0];
			obj.sw1_sw2[1] = resp.sw1_sw2[1];
			p_Ans = &obj;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_ClyCard_TestReadWrite,ApplicationError)
#endif
#else


    St_clyCard_OpenSessionInput     St_CardOpenSessionInput;
    Union_clySam_WorKeyParamsAcess  SessionWorkKey;
    St_clyCard_OpenSessionOutput    St_OpenSessionOutput;
    RESPONSE_OBJ                     *p_Ans;
    clyCard_BYTE                    RecData[REC_SIZE];

    char Pin[16]={(char)0xA4,(char)0x0B, (char)0x01, (char)0xC3, (char)0x9C, (char)0x99, (char)0xCB, (char)0x91, (char)0x0F, (char)0xE6, 
        (char)0x2A, (char)0x23, (char)0x19, (char)0x2A, (char)0x0C, (char)0x5C};


    // Open Secure Session Input 
    // Key Type - Distinguishe between the two type so that the operation can be done also with CL SAM

    // Overide session check, set params manualy
    clyCardDataArr[e_7816_CONTACTLESS].InitFlag  = clyCard_TRUE;

    St_CardOpenSessionInput.KeyType = e_clyCard_KeyCredit; // We are going to read the special event so it's not  a KeyIssuer type

    // Return Kvc Key?
    St_CardOpenSessionInput.b_Is2ReturnKeyKvc = clyCard_TRUE;

    // Rec Num to Return:  0 = read not requested
    St_CardOpenSessionInput.RecNum2Return = 0;

    // File ID To Select
    St_CardOpenSessionInput.FileToSelect = e_clyCard_SpecialEventFile;

    memset( &St_CardOpenSessionInput.Random4,0,4);

    // SAM Session Work Key
    SessionWorkKey.KifAndKvc.KVC = 0x60;
    SessionWorkKey.KifAndKvc.KIF = 0x27; // EventLogFile

    p_Ans = pSt_ClySam_Unlock(e_7816_SAM_CAL, (unsigned char *)Pin);


    do
    {

        /////////////////////////////////////////////////////////////////////////////////////////////////////

        p_Ans=pSt_ClyCard_ReadRecord(
            CardReaderId,
            1,
            e_clyCard_SpecialEventFile, 
            REC_SIZE ,  //[IN] len to read - 1 to record size 
            RecData,     //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts 
            0);

        if(!IS_RES_OBJ_OK(p_Ans)) 
            break; // error

        /////////////////////////////////////////////////////////////////////////////////////////////////////

        // Open Secure Session

        p_Ans= pSt_ClySession_OpenSecureSession( CardReaderId,    //[IN] card reader id
            SamReaderId,                                             //[IN] sam reader id
            &St_CardOpenSessionInput,                                   //[IN] Open Session Input parameters
            e_clySam_KeyKIFandKVC,                                      //[IN] choose SAM access type -  index \ KIF+KVC  are only available  
            SessionWorkKey,                                             //[IN] the SAM session work key ( index \ KIF+KVC ) found in the sam work keys												
            &St_OpenSessionOutput);                                     //[OUT]Open Session output parameters


        if(!IS_RES_OBJ_OK(p_Ans))
            break;

        /////////////////////////////////////////////////////////////////////////////////////////////////////
        //Update Record - full record



        p_Ans=pSt_ClyCard_UpdateRecord(CardReaderId, //[IN]  reader id
            1,                                              //[IN] //record number to Write - set to 1 for cyclic or counter EF
            e_clyCard_SpecialEventFile, 
            REC_SIZE,                                       //[IN] len to Write - 1 to record size 
            RecData);                                       //[IN] data to Update - in case len<Full recode size, the record will be padded with zeroes

        if(!IS_RES_OBJ_OK(p_Ans))
            break;

        /////////////////////////////////////////////////////////////////////////////////////////////////////
        //Close Secure Session

        p_Ans=pSt_ClySession_CloseSecureSession(CardReaderId,    //[IN]  card reader id
            SamReaderId,clyCard_FALSE);


        if(!IS_RES_OBJ_OK(p_Ans))
            break; // error

        /////////////////////////////////////////////////////////////////////////////////////////////////////

    }while(0);


    p_CardAns=p_Ans;
#endif
    return p_Ans; 

}


/////////////////////////////////////////////////////////////////////////////////////
// FUNCTION:    pSt_ClyCard_TestRead
//              
// Discription:  Test the card (tech mode) for read all contracts
// return: 1 =  success, 0  = error
// LOGIC:
/////////////////////////////////////////////////////////////////////////////////////

unsigned char pSt_ClyCard_TestRead(void)
{

    RESPONSE_OBJ                     *p_Ans;
    clyCard_BYTE                    RecData[REC_SIZE];
    unsigned char                   ReadRec  = 1;

    while(ReadRec < 8)
    {

        /////////////////////////////////////////////////////////////////////////////////////////////////////

        p_Ans=pSt_ClyCard_ReadRecord(
            e_7816_CONTACTLESS,
            ReadRec,
            e_clyCard_ContractsFile, 
            REC_SIZE ,  //[IN] len to read - 1 to record size 
            RecData,     //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts 
            0);

        if(!IS_RES_OBJ_OK(p_Ans)) 
            return 0; // error
        
        ReadRec++;

    };

    return ReadRec==8;

}

#endif // #ifdef CORE_SUPPORT_CALYPSO
