#define ENABLE_COMM
#include <os_def.h>
#include <Core.h>

#if defined(CORE_SUPPORT_SMARTCARD) && defined(CORE_SUPPORT_SAM) 
   
#include <ClySamOs.h>
#include <ClySessn.h>
#include <Iso7816.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#ifdef TIM7020

#include <AppProtocol.h>


///////////////////////////////////////////////////////////////////////////////////////
//
//     Abstract: ClySamOs.c
//         
//         
//
///////////////////////////////////////////////////////////////////////////////////////


#ifndef FALSE 
#define FALSE  0
#endif
#define GET_CALYPSO_BIT(a,bit) ((a>>bit)&1)

static PACKET_7816  ov_APDU;
static St_clySam_SN st_SerNumber;

////////////////////////////////////////////////////////////////////////////////////
//
// Function: b_IsResponObjOK
// Description: Checks if the 7816 response is valid
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

static clySam_BOOL b_IsResponObjOK (RESPONSE_OBJ* Obj)
{
	if (Obj && Obj->sw1_sw2[0] == 0x90 && Obj->sw1_sw2[1] == 0)
		return clySam_TRUE;
	return clySam_FALSE;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: b_ClySam_InitInterface 
// Description: Initialize the interface 
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

clySam_BOOL  b_ClySam_InitInterface (int ComPort, e_7816_DEVICE i_ReaderId )//[IN] SAM reader id
{
	clySam_BOOL retval;
#ifdef ENABLE_COMM
#ifdef INCLUDE_KEIL
#erorr ENABLE_COMM 
#endif    

	int ApplicationError;
	int RecvSize;
	St_ClySam_InitInterface req;

	req.ComPort	   = ComPort;   
	req.ReaderId = i_ReaderId;
	retval = clySam_FALSE;
	
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClySam_InitInterface, 	//int i_cmd,//[IN] the command
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
			retval = clySam_TRUE;
		}
		
	}
#ifdef WIN32
	_DEBUG_PRINT_EX(e_CmdK10_ClySam_InitInterface, ApplicationError, " ID %d", i_ReaderId)
#endif

#else
	retval = (clySam_BOOL)(_7816_Init ((eCoreUARTId)ComPort,(e_7816_DEVICE) i_ReaderId,USE_7816_SAM_HIGH_SPEED) == e_7816_STATUS_OK);
#endif
	return retval;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: b_ClySam_CloseInterface 
// Description: close the interface 
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

clySam_BOOL  b_ClySam_CloseInterface (e_7816_DEVICE ReaderId)  // [IN] SAM reader id
{
	st_SerNumber.p_SerNum4 = 0xFFFFFFFF;

	return (clySam_BOOL)(_7816_CloseReader ((e_7816_DEVICE) ReaderId) == e_7816_STATUS_OK);

}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: b_ClySam_DetectCard
// Description: return clySam_TRUE if card in
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

clySam_BOOL  b_ClySam_DetectCard (e_7816_DEVICE ReaderId)  // [IN] SAM reader id
{
	clySam_BOOL b;
#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;
	St_ClySam_DetectCard req;

	req.ReaderId = ReaderId;
	b = clySam_FALSE;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClySam_DetectCard, 	//int i_cmd,//[IN] the command
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
			b = clySam_TRUE;
		}
		
	}
#ifdef WIN32
	_DEBUG_PRINT_EX(e_CmdK10_ClySam_DetectCard,ApplicationError, " ID %d",ReaderId)
#endif
#else
	b = (clySam_BOOL)_7816_IsSmartCardIn((e_7816_DEVICE) ReaderId);
#endif
	return b;
}


////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_Reset 
// Description: reset sam
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 
#if ANDROID
#include <android/log.h>
#endif
#ifdef ENABLE_COMM
static RESPONSE_OBJ obj;
#endif
RESPONSE_OBJ* pSt_ClySam_Reset (e_7816_DEVICE ReaderId,     // [IN] SAM reader id
	clySam_BYTE p_Atr[SAM_MAX_ATR_LEN],                         // [OUT] the card atr
	int *ip_AtrLen)                                             // [OUT] the atr Length
{
	RESPONSE_OBJ *p;
#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize = 0;//13/3/14
	St_ClySam_ResetReq  req;
	St_ClySam_ResetResp	resp;

	req.ReaderId = ReaderId;
	p = NULL;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClySam_Reset, 	//int i_cmd,//[IN] the command
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
		//TBD: check ApplicationError
		if(ApplicationError && RecvSize == sizeof(resp))
		{
			//__android_log_print(ANDROID_LOG_INFO, "ClySamOs", "resp.iAtrLen=%d ApplicationError=%d p_Atr=%p resp.p_Atr=%p RecvSize=%d",resp.iAtrLen, ApplicationError, p_Atr,  resp.p_Atr, RecvSize);
			LOGI( "ClySamOs", "resp.iAtrLen=%d ApplicationError=%d p_Atr=%p resp.p_Atr=%p RecvSize=%d",resp.iAtrLen, ApplicationError, p_Atr,  resp.p_Atr, RecvSize)
			obj = resp.obj;
			p = &obj;
			//__android_log_print(ANDROID_LOG_INFO, "ClySamOs", "resp.iAtrLen=%d ApplicationError=%d p_Atr=%p resp.p_Atr=%p RecvSize=%d",resp.iAtrLen, ApplicationError, p_Atr,  resp.p_Atr, RecvSize);
			memcpy ((void*) p_Atr, (void*) resp.p_Atr, (unsigned int) resp.iAtrLen);
			*ip_AtrLen = resp.iAtrLen;

		}
		else if(RecvSize < sizeof(resp))
		{
			//__android_log_print(ANDROID_LOG_INFO, "ClySamOs", "RecvSize=%d",RecvSize);
		}
	}
#ifdef WIN32
	_DEBUG_PRINT_EX(e_CmdK10_ClySam_Reset,ApplicationError, " ID %d",ReaderId)
#endif
#else

	
	clySam_BYTE* p_TmpAtr=NULL;
	unsigned char c_SerNum[5];
	c_SerNum[4]='\0';

	if (ip_AtrLen)
		*ip_AtrLen = 0;


	p = _7816_ResetCard ((e_7816_DEVICE) ReaderId, 100);
	if (p && p->sw1_sw2[0] != 0)
	{
		p_TmpAtr = (clySam_BYTE*)_7816_GetCardAtr((e_7816_DEVICE)ReaderId, (long *)ip_AtrLen); // Check typecast
		if (p_TmpAtr)
		{
			memcpy ((void*) p_Atr, (void*) p_TmpAtr, (size_t) *ip_AtrLen);
			
            // fill serial number
			memcpy (c_SerNum, &p_Atr[12], 4);
			memrev (c_SerNum, 4);
			st_SerNumber.p_SerNum4 = (long) (*(long*)c_SerNum);
		}
	}
	else
		st_SerNumber.p_SerNum4 = 0xFFFFFFFF;
#endif
	return p;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: b_ClySam_EjectCard
// Description: eject card
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 
#ifndef ENABLE_COMM
clySam_BOOL  b_ClySam_EjectCard(e_7816_DEVICE ReaderId)        // [IN] SAM reader id
{
	st_SerNumber.p_SerNum4 = 0xFFFFFFFF;
	return (clySam_BOOL) _7816_EjectCard ((e_7816_DEVICE) ReaderId);

}
#endif
////////////////////////////////////////////////////////////////////////////////////
//
// Function: b_ClySam_GetSerNum
// Description: get sam serial number - 4 bytes long
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

clySam_BOOL  b_ClySam_GetSerNum(e_7816_DEVICE ReaderId,        // [IN] SAM reader id
	St_clySam_SN *St_SN)                                            // [OUT] card serial numer
{
#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;
	St_ClySam_SerNumReq  req;
	St_ClySam_SerNumResp resp;

	req.ReaderId = ReaderId;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClySam_GetSerNum, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		if (resp.SerNum.p_SerNum4 != 0xFFFFFFFF)
		{
#ifdef WIN32
			_DEBUG_PRINT_EX(e_CmdK10_ClySam_GetSerNum,ApplicationError," ID %d", ReaderId)
#endif
			St_SN->p_SerNum4 = resp.SerNum.p_SerNum4;
			return clySam_TRUE;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT_EX(e_CmdK10_ClySam_GetSerNum,ApplicationError, " ID %d", ReaderId)
#endif
#else
	if (st_SerNumber.p_SerNum4 != 0xFFFFFFFF)
	{
		St_SN->p_SerNum4 = st_SerNumber.p_SerNum4;
		return clySam_TRUE;
	}
#endif
	return clySam_FALSE;
}
#ifndef ENABLE_COMM
////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_SelectDiversifier 
// Description: Select Diversifier
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_SelectDiversifier(e_7816_DEVICE ReaderId, // [IN] SAM reader id
	e_clySam_DiversifierType e_DiverType,                               // [IN] 4 or 8 byte of diversifier type
	const clySam_BYTE *uc_Diversifier)                                  // [IN]pointer to 4 or 8 byte of diversifier
{

	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0x94;
	ov_APDU.INS=0x14;
	ov_APDU.P1=0;
	ov_APDU.P2=0;
	memcpy (ov_APDU.Data, uc_Diversifier, e_DiverType); //Buffer;
	ov_APDU.LC = e_DiverType;

	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE) ReaderId, 1000);

    ClyTerminalDebug("pSt_ClySam_SelectDiversifier",&ov_APDU,p_Answer);
	return p_Answer;

}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_GetChallenge 
// Description: Get Challenge
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_GetChallenge(e_7816_DEVICE ReaderId,      // [IN] SAM reader id
	e_clySam_ChallengeType e_ChallType,                                 // [IN] request for 4 or 8 byte Challenge output
	clySam_BYTE *uc_Challenge)                                          // [OUT] the challeng output
{

	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0x94;
	ov_APDU.INS=0x84;
	ov_APDU.P1=0;
	ov_APDU.P2=0;
	ov_APDU.LE=e_ChallType;

	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE) ReaderId, 1000);

	if (b_IsResponObjOK (p_Answer))
		memcpy (uc_Challenge, p_Answer->data, e_ChallType);
	
    ClyTerminalDebug("pSt_ClySam_GetChallenge",&ov_APDU,p_Answer);

    return p_Answer;

}



////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_DigestInit
// Description: Digest Init
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_DigestInit(e_7816_DEVICE ReaderId,    //[IN] SAM reader id
	e_clySam_KeyAccessType KeyAccessType,                           //[IN] choose access type -  index \ KIF+KVC  are only available
	Union_clySam_WorKeyParamsAcess SessionWorkKey,                  //[IN] the session work key ( index \ KIF+KVC ) found in the sam work keys
	const clySam_BYTE *uc_InitData,                                 //[IN] the data retured by the card to the OPEN SECURED SESSION command
	unsigned int i_InitDataLen )                                    //[IN] Init Data len
{

	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0x94;
	ov_APDU.INS=0x8a;
	ov_APDU.P1=0;
	if (KeyAccessType == e_clySam_KeyIndex)
		ov_APDU.P2=SessionWorkKey.KeyIndex;
	else
	{
		ov_APDU.P2=0xFF;
		ov_APDU.Data[0] = SessionWorkKey.KifAndKvc.KIF;
		ov_APDU.Data[1] = SessionWorkKey.KifAndKvc.KVC;
	}
	if (ov_APDU.P2==0xFF)
	{
		ov_APDU.LC=i_InitDataLen + 2;
		memcpy (&ov_APDU.Data[2], uc_InitData, i_InitDataLen);
	}
	else
	{
		ov_APDU.LC=i_InitDataLen;
		memcpy (ov_APDU.Data, uc_InitData, i_InitDataLen);
	}

	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE) ReaderId, 1000);

    ClyTerminalDebug("pSt_ClySam_DigestInit",&ov_APDU,p_Answer);

	return p_Answer;

}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_DigestUpdate
// Description: Digest Update 
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_DigestUpdate(e_7816_DEVICE ReaderId,  // [IN] SAM reader id
	clySam_BYTE *uc_Data2Add,                                       // [IN]the data send to the card
	unsigned int i_DataLen )                                        // [IN]Init Data len
{
	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA =   0x94;
	ov_APDU.INS =   0x8c;
	ov_APDU.P1  =   0;
	ov_APDU.P2  =   0;
	ov_APDU.LC  =   i_DataLen;
	if (ov_APDU.LC > 0)
		memcpy (ov_APDU.Data, uc_Data2Add, i_DataLen);//Buffer;

	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE) ReaderId, 1000);
	
    ClyTerminalDebug("pSt_ClySam_DigestUpdate",&ov_APDU,p_Answer);

    return p_Answer;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_DigestClose
// Description: Digest Close
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_DigestClose(e_7816_DEVICE ReaderId,   // [IN] SAM reader id
	CERTIFIACTE4 certif4Out)                                        // [OUT] sam output certification 
{

	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA =   0x94;
	ov_APDU.INS =   0x8e;
	ov_APDU.P1  =   0;
	ov_APDU.P2  =   0;
	ov_APDU.LE  =   0x4;

	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE) ReaderId, 1000);

	if (b_IsResponObjOK (p_Answer))
		memcpy (certif4Out, p_Answer->data, 4);

	ClyTerminalDebug("pSt_ClySam_DigestClose",&ov_APDU,p_Answer);

    return p_Answer;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_ExternalAut 
// Description: External Aut
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_ExternalAut(e_7816_DEVICE ReaderId,    // [IN] SAM reader id
	CERTIFIACTE4 certif4In)                             // [IN] Card certification to check
{

	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0x94;
	ov_APDU.INS=0x82;
	ov_APDU.P1=0;
	ov_APDU.P2=0;
	ov_APDU.LC=0x4;
	memcpy (ov_APDU.Data, certif4In, 4);

	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE) ReaderId, 1000);

    ClyTerminalDebug("pSt_ClySam_ExternalAut",&ov_APDU,p_Answer);

	return p_Answer;

}

#endif

//! Byte swap unsigned int
unsigned long  swap_uint32( unsigned long  val )
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | (val >> 16);
}


//Yoni 03/2013
//convert buffer of 3 bytes which represents a value in big endian, to unsigned long in little endian
unsigned long ul_Convert3ByteBigEndianToUnsignedLongLittle(const char in[3])
{
	char tmp[4];
	memset(tmp, 0, sizeof(tmp));
	memcpy(&tmp[1], in, 3);
	return swap_uint32(*(unsigned long*)tmp);
}


////////////////////////////////////////////////////////////////////////////////////
// updated by Yoni on 03/2013 
// Function: v_FillReadResultStruct
// Description: 
// Parameters:  
// Return:
//////////////////////////////////////////////////////////////////////////////////// 
static void v_FillReadResultStruct (RESPONSE_OBJ *p_Answer,
	e_clySam_DataType FileOrRec,
	Union_clySam_ReadDataType * ReadDataIn,
	St_clySam_ReadDataResult* ReadResult)
{
	char c_TmpStr[10];
	int i;
	clySam_BYTE byte;
	int index = -1;


	memset (c_TmpStr, 0, 10);
	memcpy (ReadResult->GeneralReadInfo.Certif8, p_Answer->data, 8);

    switch (FileOrRec)
    {
        case e_clySam_File:
            switch (ReadDataIn->DataFileType)
            {
                case e_clySam_EPTransactionNum:
										//why here? nevermind
										ReadResult->DataOut.EventCeillingArr[0].ul_EventCounter = 
											ul_Convert3ByteBigEndianToUnsignedLongLittle((const char*)&p_Answer->data[8]);                    
                    index = 11;
                    break;
                case e_clySam_ParamFile:
                    memcpy (&byte, &p_Answer->data[8], 1);
                    ReadResult->DataOut.Params.b_IsSystemLockEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte, 7);
                    ReadResult->DataOut.Params.b_IsWorkKeysLockEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte, 6);
                    ReadResult->DataOut.Params.b_IsPlainSamKeyInputEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte, 5);
                    ReadResult->DataOut.Params.b_IsRandomKeyGenerationEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte, 4);

                    ReadResult->DataOut.Params.ul_ParamsVer=
											ul_Convert3ByteBigEndianToUnsignedLongLittle((const char*)&p_Answer->data[19]);
										
                    ReadResult->DataOut.Params.e_DataStructType=(e_clySam_DataStructType) p_Answer->data[36]; 
                    break;
                default:
                    break;
            }
            break;
        case e_clySam_Rec:
            switch (ReadDataIn->DataRecType.RecType)
            {
                case e_clySam_EventCounterRec:
                    ReadResult->DataOut.ul_EventCounter = 
											ul_Convert3ByteBigEndianToUnsignedLongLittle((const char*)&p_Answer->data[8]);
                    index = 11;
                    break;
                case e_clySam_EventCeillingRec:
                    index = 37;
                    memcpy (&byte, &p_Answer->data[35], 1);
                    ReadResult->DataOut.EventCeillingArr[8].b_IsFreeEventCounter = (clySam_BOOL) GET_CALYPSO_BIT(byte, 0);
                    memcpy (&byte, &p_Answer->data[36], 1);
                    for (i=0; i<9; i++)
                    {
                        ReadResult->DataOut.EventCeillingArr[i].ul_EventCounter = 
														ul_Convert3ByteBigEndianToUnsignedLongLittle((const char*)&p_Answer->data[8+i*3]);
                    }
                    for (i=7; i>=0; i--)
                        ReadResult->DataOut.EventCeillingArr[i].b_IsFreeEventCounter = (clySam_BOOL) GET_CALYPSO_BIT(byte, i);
                    break;
                case e_clySam_SumRec:
                    index = 37;
                    ReadResult->DataOut.ValueCounter.ul_Sum = 
											swap_uint32(*(unsigned long*)&p_Answer->data[8]);
											

                    ReadResult->DataOut.ValueCounter.ul_IncreaseNum = 
											ul_Convert3ByteBigEndianToUnsignedLongLittle((const char*)&p_Answer->data[12]);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

	if (index != -1)
	{
		memcpy (&ReadResult->GeneralReadInfo.P2, &p_Answer->data[index], 1);
		index++;
		memcpy (&ReadResult->GeneralReadInfo.sn, &p_Answer->data[index], 4);
		index+=4;
		memcpy (&ReadResult->GeneralReadInfo.KIF, &p_Answer->data[index], 1);
		index++;
		memcpy (&ReadResult->GeneralReadInfo.KVC, &p_Answer->data[index], 1);
		index++;
		memcpy (&ReadResult->GeneralReadInfo.ALG, &p_Answer->data[index], 1);
	}
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_ReadData 
// Description: this command read all sam file / record except for Key Params files
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_ReadData(e_7816_DEVICE ReaderId,  //[IN] SAM reader id
	e_clySam_DataType FileOrRec,                                //[IN] type of data - file or record
	const Union_clySam_ReadDataType * ReadDataIn,               //[IN] type of file / record to read
	St_clySam_ReadDataResult *ReadResult)                       //[OUT] read result
{
	RESPONSE_OBJ *p_Answer;
#ifdef ENABLE_COMM
	int ApplicationError;
	int RecvSize;
	St_ClySam_ReadDataReq  req;
	St_ClySam_ReadDataResp resp;

	p_Answer = NULL;
	//Init data in
	req.ReaderId	= ReaderId;
	req.FileOrRec	= FileOrRec;
	if(FileOrRec == e_clySam_File)
		req.DataFileType = ReadDataIn->DataFileType;
	else if(FileOrRec == e_clySam_Rec)
		req.DataRecType = ReadDataIn->DataRecType;
	else
		return NULL;//error

	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClySam_ReadData, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		if(ApplicationError)//OK
		{
			//OK 
			obj = resp.obj;
			p_Answer = &obj;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_ClySam_ReadData,ApplicationError)
#endif
#else

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0x94;
	ov_APDU.INS=0xbe;
	ov_APDU.P1=0;
	switch (FileOrRec)
	{
	case e_clySam_File:
		if (ReadDataIn->DataFileType == e_clySam_EPTransactionNum)
			ov_APDU.P2 = 0x81;
		else
		{
			if (ReadDataIn->DataFileType == e_clySam_ParamFile)
				ov_APDU.P2 = 0xa0;
		}
		break;
	case e_clySam_Rec:
		{
			switch (ReadDataIn->DataRecType.RecType)
			{
			case e_clySam_EventCounterRec:
				ov_APDU.P2 = 0x82 + ReadDataIn->DataRecType.RecNum - 1;
				break;

			case e_clySam_EventCeillingRec:
				ov_APDU.P2 = 0xb1 + ReadDataIn->DataRecType.RecNum - 1;
				break;

			case e_clySam_SumRec:
				ov_APDU.P2 = 0xd1 + ReadDataIn->DataRecType.RecNum - 1;
				break;

			default:
				break;
			}
		}
		break;

	default:
		break;
	}
	if (ov_APDU.P2 >= 0x81 && ov_APDU.P2 <= 0x9b)
		ov_APDU.LE = 0x18;
	else
		ov_APDU.LE = 0x30;

	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE) ReaderId, 1000);
#endif
	if (b_IsResponObjOK (p_Answer))

		// fill ReadResult
		v_FillReadResultStruct (p_Answer, FileOrRec, (Union_clySam_ReadDataType*) ReadDataIn, ReadResult);

    ClyTerminalDebug("pSt_ClySam_ReadData",&ov_APDU,p_Answer);

	return p_Answer;
}

#define KIF_KeySystem(a) (a&0x80)>>7
#define KIF_KeyApplication(a) (a&0x60)>>5
#define KIF_KeyFunction(a) a&0x1f

////////////////////////////////////////////////////////////////////////////////////
//
// Function: v_FillReadKeyResultStruct
// Description: 
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

static void v_FillReadKeyResultStruct (RESPONSE_OBJ *p_Answer, 
	e_clySam_KeyParamsType ParamsType, 
	e_clySam_KeyAccessType KeyAccessType, 
	Union_clySam_KeyInfo * KeyInfoIn, 
	St_clySam_ReadKeyResult* ReadResult)
{
	clySam_BYTE byte;

	memcpy (&ReadResult->GeneralReadInfo.Certif8, p_Answer->data, 8);
	memcpy (&ReadResult->GeneralReadInfo.KIF, &p_Answer->data[26], 1);
	memcpy (&ReadResult->GeneralReadInfo.KVC, &p_Answer->data[27], 1);
	memcpy (&ReadResult->GeneralReadInfo.ALG, &p_Answer->data[10], 1);
	memcpy (&ReadResult->GeneralReadInfo.P2, &p_Answer->data[21], 1);
	memcpy (&ReadResult->GeneralReadInfo.sn, &p_Answer->data[22], 4);

	switch (ParamsType)
	{
	case e_clySam_WorkKeyParams:
	case e_clySam_SystemKeyParams:

		memcpy (&ReadResult->KeyRec.WorkAndSystemKeyRec.KVC, &p_Answer->data[9], 1);
		memcpy (&ReadResult->KeyRec.WorkAndSystemKeyRec.alg, &p_Answer->data[10], 1);
		memcpy (&byte, &p_Answer->data[8], 1);
		ReadResult->KeyRec.WorkAndSystemKeyRec.KIF.KeySystem = (e_clySam_KeySystem) (KIF_KeySystem(byte));
		ReadResult->KeyRec.WorkAndSystemKeyRec.KIF.KeyApplication = (e_clySam_KeyApplication) (KIF_KeyApplication(byte));
		ReadResult->KeyRec.WorkAndSystemKeyRec.KIF.KeyFunction = (e_clySam_KeyFunction) (KIF_KeyFunction(byte));
		memcpy (&byte, &p_Answer->data[11], 1);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR1.b_IsDeciperEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,7);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR1.b_IsStaticCipherEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,6);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR1.b_IsCiperEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,5);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR1.b_IsDiversifiedCipherEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,4);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR1.b_IsCertifComputeEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,3);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR1.b_IsCertifFourBytesEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,2);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR1.b_IsSessionVerifyEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,0);
		memcpy (&byte, &p_Answer->data[12], 1);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR2.b_IsAnyReloadAmountEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,7);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR2.b_IsCancelPurchaseEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,6);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR2.b_IsCardUpdateKeyEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,5);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR2.b_IsCardChangePinEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,4);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR2.b_IsCardVerifyPinEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,3);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR2.b_IsCardStampeModeEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,2);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR2.b_IsProtectedModeEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,1);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR2.b_IsCardSessionModeEnable = (clySam_BOOL) GET_CALYPSO_BIT(byte,0);
		memcpy (&byte, &p_Answer->data[13], 1);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR3.KeyGenerateLimitations.b_IsTransferPlainWithoutDiversify = (clySam_BOOL) GET_CALYPSO_BIT(byte,7);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR3.KeyGenerateLimitations.b_IsTransferCipherWithoutDiversify = (clySam_BOOL) GET_CALYPSO_BIT(byte,6);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR3.KeyGenerateLimitations.b_IsTransferPlainWithDiversify = (clySam_BOOL) GET_CALYPSO_BIT(byte,5);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR3.KeyGenerateLimitations.b_IsTransferCipherWithDiversify = (clySam_BOOL) GET_CALYPSO_BIT(byte,4);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR3.KeyTransferAuthorization.b_IsTransferPlainWithoutDiversify = (clySam_BOOL) GET_CALYPSO_BIT(byte,3);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR3.KeyTransferAuthorization.b_IsTransferCipherWithoutDiversify = (clySam_BOOL) GET_CALYPSO_BIT(byte,2);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR3.KeyTransferAuthorization.b_IsTransferPlainWithDiversify = (clySam_BOOL) GET_CALYPSO_BIT(byte,1);
		ReadResult->KeyRec.WorkAndSystemKeyRec.PAR3.KeyTransferAuthorization.b_IsTransferCipherWithDiversify = (clySam_BOOL) GET_CALYPSO_BIT(byte,0);
		memcpy (&ReadResult->KeyRec.WorkAndSystemKeyRec.PAR4_KeyUsageCounter, &p_Answer->data[14], 1);
		break;
	case e_clySam_LockFileKeyParams:
		memcpy (&ReadResult->KeyRec.LockKeyRec.KIF, &p_Answer->data[23], 1);
		memcpy (&ReadResult->KeyRec.LockKeyRec.KVC, &p_Answer->data[24], 1);
		memcpy (&byte, &p_Answer->data[15], 1);
		ReadResult->KeyRec.LockKeyRec.b_IsLockAtReset = (clySam_BOOL) GET_CALYPSO_BIT(byte,7);
		ReadResult->KeyRec.LockKeyRec.b_IsLockedLock = (clySam_BOOL) GET_CALYPSO_BIT(byte,6);
		break;
	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_ReadKeyParams
// Description: Read Key Params
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 
#ifndef ENABLE_COMM
RESPONSE_OBJ*   pSt_ClySam_ReadKeyParams(e_7816_DEVICE ReaderId, //[IN] SAM reader id 
	e_clySam_KeyParamsType ParamsType,                              //[IN] the Key Params Type
	e_clySam_KeyAccessType KeyAccessType,                           //[IN] choose access type ( index \ KIF ) for lock file this parameter is not relevat 
	const Union_clySam_KeyInfo * KeyInfoIn,                         //[IN] Key  index \ KIF - this parameter is not relevat for the lock file  
	St_clySam_ReadKeyResult *ReadResult)                            //[OUT] read result
{
	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0x94;
	ov_APDU.INS=0xbc;
	ov_APDU.P1=0;
	ov_APDU.LC=0x2;
	ov_APDU.LE=0x20;
	switch (ParamsType)
	{
	case e_clySam_WorkKeyParams:
		switch (KeyAccessType)
		{
		case e_clySam_KeyIndex:
			ov_APDU.P2 = 0x01 + KeyInfoIn->WorKeyParamsAcess.KeyIndex - 1;
			break;
		case e_clySam_KeyKIF:
			ov_APDU.P2 = 0xf8;
			break;
		case e_clySam_KeyKIFandKVC:
			ov_APDU.P2 = 0xf0;
			ov_APDU.Data[0]=KeyInfoIn->WorKeyParamsAcess.KifAndKvc.KIF;
			ov_APDU.Data[1]=KeyInfoIn->WorKeyParamsAcess.KifAndKvc.KVC;
			break;
		default:
			break;
		}
		break;
	case e_clySam_SystemKeyParams:
		switch (KeyAccessType)
		{
		case e_clySam_KeyIndex:
			ov_APDU.P2 = 0xc1 + KeyInfoIn->SystemKeyParamsAcess.KeyIndex - 1;
			break;
		case e_clySam_KeyKIF:
			ov_APDU.P2 = 0xc0;
			break;
		default:
			break;
		}
		break;
	case e_clySam_LockFileKeyParams:
		ov_APDU.P2 = 0xe0;
		break;
	default:
		break;
	}

	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE) ReaderId, 1000);

	if (b_IsResponObjOK (p_Answer))
	{
		p_Answer = pSt_ClySam_GetResponse (ReaderId, ov_APDU.LE);
		if (b_IsResponObjOK (p_Answer))
			// fill keyReadResult struct
			v_FillReadKeyResultStruct (p_Answer, ParamsType, KeyAccessType, (Union_clySam_KeyInfo*) KeyInfoIn, ReadResult);
	}

	return p_Answer;
}
#endif
////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_Unlock 
// Description: 
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_Unlock (e_7816_DEVICE ReaderId, UULOCK_PIN16 Pin)
{
	RESPONSE_OBJ *p_Answer;
#ifdef ENABLE_COMM

	int ApplicationError;
	int RecvSize;
	St_ClySam_UnlockReq  req;
	St_ClySam_UnlockResp resp;

	req.ReaderId = ReaderId;
	memcpy(req.PIN,Pin,16);//TBD:yoram 
	p_Answer = NULL;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClySam_Unlock, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		if(ApplicationError)//OK
		{
			//OK 
			obj.sw1_sw2[0] = resp.sw1_sw2[0];
			obj.sw1_sw2[1] = resp.sw1_sw2[1];
			//obj = resp.obj;
			p_Answer = &obj;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT_EX(e_CmdK10_ClySam_Unlock,ApplicationError, " ID %d", ReaderId)
#endif
#else
	

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0x94;
	ov_APDU.INS=0x20;
	ov_APDU.P1=0;
	ov_APDU.P2=0;
	ov_APDU.LC=0x10;
	memcpy (ov_APDU.Data, Pin, 16);

	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE) ReaderId, 1000);
#endif    
   // ClyTerminalDebug("pSt_ClySam_Unlock",&ov_APDU,p_Answer);

	return p_Answer;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_GiveRandom 
// Description: give random
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 
#ifndef ENABLE_COMM
RESPONSE_OBJ*   pSt_ClySam_GiveRandom (e_7816_DEVICE ReaderId,   // [IN] SAM reader id
	RANDOM8 rand8)                                                  // [IN] random
{
	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0x94;
	ov_APDU.INS=0x86;
	ov_APDU.P1=0;
	ov_APDU.P2=0;
	ov_APDU.LC=0x8;
	memcpy (ov_APDU.Data, rand8, 8);

	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE) ReaderId, 1000);

	return p_Answer;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_GetResponse 
// Description: get response
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_GetResponse (e_7816_DEVICE ReaderId,  // [IN] SAM reader id
	int Len)                                                        // [IN] response length request
{
	static PACKET_7816 ov_APDU;
	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0;
	ov_APDU.INS=0xc0;
	ov_APDU.P1=0;
	ov_APDU.P2=0;
	ov_APDU.LC=0;
	ov_APDU.LE=Len;

	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE) ReaderId, 1000);

    ClyTerminalDebug("pSt_ClySam_GetResponse",&ov_APDU,p_Answer);

	return p_Answer;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_WriteData
// Description: this command updates the Parameters file record, an Event Ceilings
//              file record or on Event Ceiling 
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_WriteData (e_7816_DEVICE ReaderId,    //[IN] SAM reader id
	e_clySam_WriteDataMode WriteMode,                               //[IN] dynamic or static mode
	e_clySam_WriteDataSamP2 e_WriteDataSamP2,                       //[IN] enum to fill P2
	clySam_BYTE* WriteData)                                         //[IN] write data 48 bytes
{
	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0x94;
	ov_APDU.INS=0xd8;

	switch (WriteMode)
	{
	case e_clySam_DynamicCiphMode:
		ov_APDU.P1 = 0;
		break;
	case e_clySam_StaticCiphMode:
		ov_APDU.P1 = 8;
		break;
	default:
		break;
	}

	ov_APDU.P2 = e_WriteDataSamP2;
	ov_APDU.LC = 0x30;
	memcpy (ov_APDU.Data, WriteData, strlen ((char*)WriteData)); // Check typecast?

	p_Answer =_7816_CardInOut (&ov_APDU,(e_7816_DEVICE)ReaderId, 1000);

    ClyTerminalDebug("pSt_ClySam_WriteData",&ov_APDU,p_Answer);

	return p_Answer;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_CipherCardData 
// Description: this command generates the signed/ciphered data to send to a card
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_CipherCardData (e_7816_DEVICE ReaderId,   // [IN] SAM reader id
	st_CipherDataCard s_CipherDataCard,                                 // [IN] all information for command (index(KIF || KVC), type of command, key or pin
	unsigned char* DataOut)                                             // [OUT] Data out (8, 16, 24 or 32 bytes)
{
	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));

	p_Answer =_7816_CardInOut (&ov_APDU,(e_7816_DEVICE)ReaderId, 1000);
	if (b_IsResponObjOK (p_Answer))
	{
		p_Answer = pSt_ClySam_GetResponse (ReaderId, ov_APDU.LE);
		if (b_IsResponObjOK (p_Answer) && p_Answer->Len > 0)
			memcpy (DataOut, p_Answer->data, p_Answer->Len);
	}

    ClyTerminalDebug("pSt_ClySam_CipherCardData",&ov_APDU,p_Answer);

	return p_Answer;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_CipherSamData
// Description: this command generates the data to send to another SAM with the commands WRITE DATA and WRITE KEY
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_CipherSamData (e_7816_DEVICE ReaderId,    // [IN] SAM reader id
	st_clySam_CipherDataSam s_CipherDataSam,                            // [IN] struct to fill command parameters (PACKET_7816)
	clySam_BYTE* Data2Write)                                            // [OUT] 6 blocks of 8 bytes
{
	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0x94;
	ov_APDU.INS=0x16;
	switch (s_CipherDataSam.e_DataGenOptions)
	{
	case e_clySam_CiphedKeyGen:
		switch (s_CipherDataSam.e_DiversKeyGen)
		{
		case e_clySam_NotDiversKeyGen:
			ov_APDU.P1 = 0;
			break;
		case e_clySam_DiversKeyGen:
			ov_APDU.P1 = 1;
			break;
		default:
			break;
		}
		break;
	case e_clySam_PlainKeyGen:
		switch (s_CipherDataSam.e_DiversKeyGen)
		{
		case e_clySam_NotDiversKeyGen:
			ov_APDU.P1 = 128;
			break;
		case e_clySam_DiversKeyGen:
			ov_APDU.P1 = 129;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	switch (s_CipherDataSam.s_TargetSamDataRef.GenType)
	{
	case e_clySam_GenParamAndEventType:
	case e_clySam_GenParamType:
	case e_clySam_GenEventCeillDefRecType:
	case e_clySam_GenOneEventCeillType:
	case e_clySam_GenSysKeyByKifKvcType:
	case e_clySam_GenWorkKeyByKifKvcType:
		ov_APDU.P2 = s_CipherDataSam.s_TargetSamDataRef.GenType;
		break;
	case e_clySam_GenIndexWorkKeyType:
		ov_APDU.P2 = e_clySam_GenIndexWorkKeyType + s_CipherDataSam.s_TargetSamDataRef.RecNum - 1;
		break;
	case e_clySam_GenEventCeillTargRecType:
		ov_APDU.P2 = e_clySam_GenEventCeillTargRecType + s_CipherDataSam.s_TargetSamDataRef.RecNum - 1;
		break;
	default:
		break;
	}
	ov_APDU.LC=0x1e;
	ov_APDU.LE=0x30;

	memcpy (ov_APDU.Data, &s_CipherDataSam.CipherKeyKVC, 1);
	memcpy (&ov_APDU.Data[1], &s_CipherDataSam.NumOfCeill2Update, 1);
	memcpy (&ov_APDU.Data[2], s_CipherDataSam.NewValue, 3);

	p_Answer =_7816_CardInOut (&ov_APDU,(e_7816_DEVICE)ReaderId, 1000);
	if (b_IsResponObjOK (p_Answer))
	{
		p_Answer = pSt_ClySam_GetResponse (ReaderId, ov_APDU.LE);
		if (b_IsResponObjOK (p_Answer) && p_Answer->Len > 0)
			memcpy (Data2Write, p_Answer->data, p_Answer->Len);
	}

	ClyTerminalDebug("pSt_ClySam_CipherSamData",&ov_APDU,p_Answer);

    return p_Answer;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_Increase
// Description: This command increases the value of a given Value Counter by any amount
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_Increase (e_7816_DEVICE ReaderId, // [IN] SAM reader id
	unsigned int i_ValueCountNumber,                            // [IN] Value Counter Number from 1 to 7
	AMOUNT4 Amount,                                             // [IN] amount
	SUM_VALUE4 SumValue,                                        // [OUT] new value of the VALUE COUNTER
	INCREASE_NUMBER3 IncreaseNumber)                            // [OUT] new number of times the VALUE COUNTER has been increased
{
	RESPONSE_OBJ *p_Answer;

	// Build APDU packet
	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0x94;
	ov_APDU.INS=0x32;
	ov_APDU.P1=0;
	ov_APDU.P2=1;
	ov_APDU.LC=4;

	memcpy (ov_APDU.Data, Amount, 4);

	p_Answer =_7816_CardInOut (&ov_APDU, (e_7816_DEVICE)ReaderId, 1000);
	if (b_IsResponObjOK (p_Answer))
	{
		p_Answer = pSt_ClySam_GetResponse (ReaderId, 7);
		if (b_IsResponObjOK (p_Answer) && p_Answer->Len > 0)
		{
			memcpy (SumValue, p_Answer->data, 4);
			memcpy (IncreaseNumber, &p_Answer->data[4], 3);
		}
	}

	return p_Answer;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_CipherData 
// Description: This command ciphers arbitrary message made of 8 bytes data block
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_CipherData (e_7816_DEVICE ReaderId,   // [IN] SAM reader id
	St_clySam_KIF_And_KVC KifAndKvc,                                // [IN] KIF and KVC
	clySam_BYTE* PlainData,                                         // [IN] Plain data
	unsigned int i_DataLenIn,                                       // [IN] Plain data length
	clySam_BYTE* DataOut,                                           // [OUT] data out
	unsigned int* i_DataLenOut)                                     // [OUT] out data length
{
	RESPONSE_OBJ *p_Answer;
	short i=0;

	if(KifAndKvc.KIF)
	{
		i+=2;
		ov_APDU.Data[0]=KifAndKvc.KIF;
		ov_APDU.Data[1]=KifAndKvc.KVC;

	}
	else
		if(KifAndKvc.KVC)
		{
			i++;
			ov_APDU.Data[0]=KifAndKvc.KVC;
		}

		ov_APDU.CLA=0x94;
		ov_APDU.INS=0x1c;
		// empty
		ov_APDU.P1=0;
		// empty
		ov_APDU.P2=0;
		// len in
		ov_APDU.LC=i_DataLenIn+i;
		// len out
		ov_APDU.LE=i_DataLenIn;
		// data in
		memcpy(ov_APDU.Data+i,PlainData,i_DataLenIn);
		p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE)ReaderId, 1000);
#
		if (b_IsResponObjOK (p_Answer))
		{
			p_Answer = pSt_ClySam_GetResponse (ReaderId, ov_APDU.LE);
			if (b_IsResponObjOK (p_Answer) && p_Answer->Len>0)
			{
				*i_DataLenOut=p_Answer->Len;
				memcpy(DataOut,p_Answer->data,p_Answer->Len);
			}
		}

		return p_Answer;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_DecipherData 
// Description: this command deciphers arbitrary message made of 8 bytes data block 
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_DecipherData (e_7816_DEVICE ReaderId,     // [IN] SAM reader id
	St_clySam_KIF_And_KVC KifAndKvc,                                    // [IN] KIF and KVC
	clySam_BYTE* DataIn,                                                // [IN] data in
	unsigned int i_DataLenIn,                                           // [IN] data in length
	clySam_BYTE* DataOut,                                               // [OUT] data out
	unsigned int* i_DataLenOut)                                         // [OUT] out data length
{
	RESPONSE_OBJ *p_Answer;

	ov_APDU.Data[0]=KifAndKvc.KVC;

	ov_APDU.CLA=0x94;
	ov_APDU.INS=0x1c;
	//   empty
	ov_APDU.P1=0x80;
	//   empty
	ov_APDU.P2=0;
	//   len in 
	ov_APDU.LC=i_DataLenIn+1;
	//   len out 
	ov_APDU.LE=i_DataLenIn;
	//   data in
	memcpy(ov_APDU.Data+1,DataIn,i_DataLenIn);
	p_Answer = _7816_CardInOut (&ov_APDU, (e_7816_DEVICE)ReaderId, 1000);
	if (b_IsResponObjOK (p_Answer))
	{
		p_Answer = pSt_ClySam_GetResponse (ReaderId, ov_APDU.LE);
		if (b_IsResponObjOK (p_Answer) && p_Answer->Len>0)
		{
			*i_DataLenOut=p_Answer->Len;
			memcpy(DataOut,p_Answer->data,p_Answer->Len);
		}
	}

	return p_Answer;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: pSt_ClySam_Compute
// Description: 
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

RESPONSE_OBJ*   pSt_ClySam_Compute (e_7816_DEVICE ReaderId,      // [IN] SAM reader id
	e_clySam_KeyAccessType KeyAccessType,                           // [IN] choose access type -  index \ KIF+KVC  are only available
	Union_clySam_WorKeyParamsAcess KeyAccess,                       // [IN]
	SN8 SerialNum,                                                  // [IN] serial number
	unsigned char* Data4Cert,                                       // [IN] data to certify
	int CertSizeIn,                                                 // [IN] size of data
	int CertDataOutLen,                                             // [IN] certificate size out
	unsigned char* CertOut)                                         // [OUT] certificate out
{
	RESPONSE_OBJ *p_Answer;

	memset(&ov_APDU,0,sizeof(ov_APDU));
	ov_APDU.CLA=0xf0;
	ov_APDU.INS=0x2a;
	ov_APDU.P1=0x9e;
	ov_APDU.P2=0x9a;
	ov_APDU.LC=0;
	memcpy(ov_APDU.Data,SerialNum,8);
	ov_APDU.Data[8]=0;
	ov_APDU.Data[9]=0xff;

	ov_APDU.Data[10]=KeyAccess.KifAndKvc.KIF ;	// KIF
	ov_APDU.Data[11]=KeyAccess.KifAndKvc.KVC;   // KVC
	ov_APDU.LE=CertDataOutLen;					// certification out size
	ov_APDU.Data[12]=CertDataOutLen;			// certification size
	memcpy(&ov_APDU.Data[13],Data4Cert,CertSizeIn);
	ov_APDU.LC=0xd+CertSizeIn;


	p_Answer =_7816_CardInOut (&ov_APDU,(e_7816_DEVICE)ReaderId, 1000);
	if (b_IsResponObjOK (p_Answer)|| ( p_Answer->data[0]==0x90 && p_Answer->data[1]==0))
	{
		p_Answer = pSt_ClySam_GetResponse (ReaderId, ov_APDU.LE);
		if (b_IsResponObjOK (p_Answer) && p_Answer->Len>0)
			memcpy(CertOut,p_Answer->data,p_Answer->Len);
	}


	return p_Answer;
}
#endif
#ifdef ENABLE_COMM
eCalypsoErr //CLYAPP_STDCALL 
e_ClyApp_GetSamType(e_7816_DEVICE i_SamReaderId, //[IN]SAM reader ID
					e_ClyApp_SamType *e_SamType)//[OUT] the SAM type
{

	int ApplicationError;
	int RecvSize;
	St_ClySam_GetTypeReq  req;
	St_ClySam_GetTypeResp resp;

	req.ReaderId = i_SamReaderId;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClySam_GetType, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		if(ApplicationError)//OK
		{
			*e_SamType = (e_ClyApp_SamType)resp.e_SamType;
#ifdef WIN32
			_DEBUG_PRINT_EX(e_CmdK10_ClySam_GetType,ApplicationError," ID %d",i_SamReaderId)
#endif
			return e_ClyApp_Ok;
		}
	}
#ifdef WIN32
  _DEBUG_PRINT_EX(e_CmdK10_ClySam_GetType,ApplicationError, " ID %d",i_SamReaderId)
#endif
  return e_ClyApp_CardReadErr;
}

#endif

#endif // #if defined(CORE_SUPPORT_SMARTCARD) && defined(CORE_SUPPORT_SAM) 
