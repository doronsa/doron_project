
#define ENABLE_COMM
#include <Core.h>

#if defined(CORE_SUPPORT_SMARTCARD) && defined(CORE_SUPPORT_SAM) 

#include <ClyCrdOs.h>
#include <ClySamOs.h>
#include <ClySessn.h>
#include <Core.h>
#ifdef ENABLE_COMM

#include <AppProtocol.h>

static RESPONSE_OBJ RespObj;
#endif 

#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif


#ifdef ENABLE_COMM

static RESPONSE_OBJ RespObj;
#endif
///////////////////////////////////////////////////////////////////////////////////////
//
//     Abstract: ClySessn.c
//         
//         
//
///////////////////////////////////////////////////////////////////////////////////////

#define CLS_INS_P1_P2_P3_Len    5
#define EVENT_COUNT_REC_NUM     2
#define EVENT_CEILING_REC_NUM   2

#define ANSI_END                "\033[0m"
#define ANSI_BLUE               "\033[36m"
#define ANSI_CRLF               "\r\n"
#define ANSI_CRLFX2             "\r\n\r\n"
#define ANSI_UL                 "\033[4m"
#define ANSI_YELLOW             "\033[33m"

typedef enum
{
	e_Session_cmdNoData,
	e_Session_cmdIn,
	e_Session_cmdOut,
	e_Session_cmdInOut,
}e_Session_CmdType;

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description:
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

static  e_Session_CmdType e_GetCmdType (PACKET_7816 *p7816 )
{
	if( p7816->LC == 0 && p7816->LE == 0 )
		return e_Session_cmdNoData;

	if( p7816->LC != 0 && p7816->LE != 0 )
		return e_Session_cmdInOut;

	if( p7816->LC != 0 && p7816->LE == 0 )
		return e_Session_cmdIn;

	return e_Session_cmdOut;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ClySamTerminalDebug 
// Description: Select Diversifier
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

#ifdef CALYPSO_DUMP_API_DEBUG

#ifdef WIN32
#pragma warning(disable : 4996) // CRT Secure - off
#endif

static void ClyTerminalDebugFile (char* func, PACKET_7816* ov_APDU, RESPONSE_OBJ *p_Answer)
{
    int i;
    
    FILE* Log_File = NULL;
    Log_File = fopen ("7816Log.txt", "at");
    
    fprintf (Log_File, "\n******************************************************************\n");
    fprintf (Log_File, "        FUNCTION NAME: %s\n", func);
    fprintf (Log_File, "          PUCKET_7816\n");
    fprintf (Log_File, "CLA = %x\n", ov_APDU->CLA); 
    fprintf (Log_File, "INS = %x\n", ov_APDU->INS); 
    fprintf (Log_File, "P1 = %x\n", ov_APDU->P1); 
    fprintf (Log_File, "P2 = %x\n", ov_APDU->P2); 
    fprintf (Log_File, "LC = %x\n", ov_APDU->LC); 
    fprintf (Log_File, "LE = %x\n", ov_APDU->LE);
    fprintf (Log_File, "Data ="); 
    
    for (i = 0; i < ov_APDU->LC; i++)
        fprintf (Log_File, " %x", ov_APDU->Data[i]); 
    
    fprintf (Log_File, "\n\n"); 
    fprintf (Log_File, "          RESPON_OBJ\n");
	if(p_Answer)
	{
		fprintf (Log_File, "sw1_sw2 = %x %x\n", p_Answer->sw1_sw2[0], p_Answer->sw1_sw2[1]); 
		fprintf (Log_File, "Len = %d\n", p_Answer->Len); 
		fprintf (Log_File, "Data ="); 
		for (i = 0; i < p_Answer->Len; i++)
			fprintf (Log_File, " %x", p_Answer->data[i]); 
	}
    fprintf (Log_File, "\n\n"); 
    fclose (Log_File);

}
#endif


////////////////////////////////////////////////////////////////////////////////////
//
// Function: ClySamTerminalDebug 
// Description: Select Diversifier
// Parameters:  
// Return:
//
//////////////////////////////////////////////////////////////////////////////////// 

void ClyTerminalDebug(char *pFunctionName, PACKET_7816  *pReq, RESPONSE_OBJ *p_Answer)
{

#ifdef CALYPSO_DUMP_API_DEBUG   
#ifndef win_or_linux
    
    int DataSize,Bytes;
    s_printf(CORE_UART_DEBUG_PORT, ANSI_BLUE  ANSI_CRLF "Functoin : %s" ANSI_CRLFX2 ANSI_END ,pFunctionName);
    s_printf(CORE_UART_DEBUG_PORT,ANSI_UL "7816 Packet: " ANSI_END ANSI_CRLFX2); 
    s_printf(CORE_UART_DEBUG_PORT,"CLA: 0x%.2x" ANSI_CRLF "INS: 0x%.2x" ANSI_CRLF "P1:  0x%.2x" ANSI_CRLF,pReq->CLA,pReq->INS,pReq->P1);
    s_printf(CORE_UART_DEBUG_PORT,"P2:  0x%.2x" ANSI_CRLF "LC:  0x%.2x" ANSI_CRLF "LE:  0x%.2x" ANSI_CRLF,pReq->P2,pReq->LC,pReq->LE);
    s_printf(CORE_UART_DEBUG_PORT,"Length: %d bytes",pReq->LC);
    Bytes = 0;
    if(pReq->LC)`
    {
        s_printf(CORE_UART_DEBUG_PORT,ANSI_CRLF "Data: ");
        for(DataSize = 0;DataSize < pReq->LC; DataSize++)
        {
            s_printf(CORE_UART_DEBUG_PORT,"0x%.2x,",pReq->Data[DataSize]);
            if(Bytes++ > 10)
            {
                s_printf(CORE_UART_DEBUG_PORT,ANSI_CRLF "      ");
                Bytes = 0;
            }
        } 
    }
    
    s_printf(CORE_UART_DEBUG_PORT,ANSI_CRLFX2 ANSI_UL "Response:" ANSI_END ANSI_CRLFX2);
    s_printf(CORE_UART_DEBUG_PORT,"SW1 & SW2: " ANSI_YELLOW "0x%.2x,0x%.2x" ANSI_END ANSI_CRLF,p_Answer->sw1_sw2[0],p_Answer->sw1_sw2[1]);
    s_printf(CORE_UART_DEBUG_PORT,"Length: %d bytes",p_Answer->Len);
    
    if(p_Answer->Len)
    {
        s_printf(CORE_UART_DEBUG_PORT,ANSI_CRLF "Data: ");
        Bytes = 0;
        for(DataSize = 0;DataSize < p_Answer->Len; DataSize++)
        {
            s_printf(CORE_UART_DEBUG_PORT,"0x%.2x,",p_Answer->data[DataSize]);
            if(Bytes++ > 10)
            {
                s_printf(CORE_UART_DEBUG_PORT,ANSI_CRLF "\t");
                Bytes = 0;
            }     
        }
    }
    s_printf(CORE_UART_DEBUG_PORT,ANSI_CRLF ANSI_CRLF  "----------------------------------------------------------------------" ANSI_END ANSI_CRLF);
#else
    ClyTerminalDebugFile(pFunctionName,pReq,p_Answer);
#endif
#endif
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description:
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef ENABLE_COMM
RESPONSE_OBJ*  pSt_DigestUpdateCallback(PACKET_7816 *p7816,       //[IN] packet 7816 input
	unsigned char sw1_sw2[SW1_SW2_Len],                         //[IN] SW1 SW2 respond
	e_7816_DEVICE SamReaderId)                                            //[IN]  sam reader id with which the session was open
{
	RESPONSE_OBJ*  obj;
	RESPONSE_OBJ*  obj1;
    RESPONSE_OBJ   objPrev;

	int iv_Len;
	int i_SaveLc=p7816->LC;

	// Get command type
	e_Session_CmdType e_CmdType = e_GetCmdType( p7816 );
	obj1 = pSt_ClyCard_GetLastCardResponseObj ();
	memcpy(&objPrev,obj1,sizeof(RESPONSE_OBJ));
    obj1 = &objPrev;

    if (obj1->Len == 0)
		iv_Len = 2;
	else
		iv_Len = obj1->Len+2;   // data + switch
	switch ( e_CmdType)
	{
	case e_Session_cmdNoData:
	case e_Session_cmdInOut:
	case e_Session_cmdIn:
		break;
	case e_Session_cmdOut:
		p7816->LC=p7816->LE;

		break;
	}

	// Digest Update 1
	obj = pSt_ClySam_DigestUpdate(SamReaderId,      //  [IN] SAM reader id 
		(clySam_BYTE*)p7816,                        //  [IN]the data send to the card
		CLS_INS_P1_P2_P3_Len+i_SaveLc );            //  [IN]Init Data len
	if ( IS_RES_OK( obj ) )
		
        // Digest Update 1
		return  pSt_ClySam_DigestUpdate(SamReaderId,//  [IN] SAM reader id 
		(clySam_BYTE*)obj1->data,                   //  [IN]the data send to the card
		(unsigned char)iv_Len);                     //  [IN]Init Data len


	return obj;
}


////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description:
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_ChangeKeys(
	e_7816_DEVICE CardReaderId,                   // [IN] card reader id
	e_7816_DEVICE SamReaderId,                    // [IN] sam reader id
	e_clyCard_KeyType keytype,          // [IN] type of key to change
	st_CipherDataCard s_CipherDataCard
	)
{

	eClyCardTypes type;
	RESPONSE_OBJ* obj;
	St_clyCard_SN St_SN;
	clyCard_BYTE Chlng[8]={0};
	clyCard_BYTE ChiperOut[32]={0};

	char TmpCh[5];
	char TmpCh1[10];

	memset (TmpCh, 0, 5);
	memset (TmpCh1, 0, 10);

	// Get Card serial number - 4 bytes long
	if(  b_ClyCard_GetSerNum(CardReaderId,  // [IN] SAM reader id
		&St_SN,&type) == clyCard_FALSE )    // [OUT] card serial numer
		return 0;

	memcpy (TmpCh, (void*)&St_SN.p_SerNum4, 4);
	memrev ((unsigned char*)TmpCh,4);
	memcpy (&TmpCh1[4], TmpCh, 4);
	// Select Diversifier in SAM using the card SN
	obj = pSt_ClySam_SelectDiversifier(SamReaderId,     // [IN] SAM reader id
		e_clySam_Diver8Byte,                            // [IN] 4 byte of diversifier type
		(const unsigned char *)TmpCh1);                 // [IN] pointer to 4 byte SN of diversifier
	
    // check response
	if(! IS_RES_OK( obj ))
		return obj;


	///   get chellenge (card)
	obj=pSt_ClyCard_GetChalenge(CardReaderId,Chlng);
	//check response
	if(! IS_RES_OK( obj ))
		return obj;


	// Give random (sam)
	obj=pSt_ClySam_GiveRandom(SamReaderId,Chlng);
	
    // Check response
	if(! IS_RES_OK( obj ))
		return obj;

	// Chiper card data (sam)
	obj=pSt_ClySam_CipherCardData(SamReaderId,s_CipherDataCard,ChiperOut);
	
    // Check response
	if(! IS_RES_OK( obj ))
		return obj;

	// Change key   (card)
	return pSt_ClyCard_ChangeKey(CardReaderId,keytype,
		clyCardKeyChangeLen_KEY,
		ChiperOut);
}
#endif
////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: Read Remain value :  ceiling value - counter value = Remain value
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_SamClCeilingRemain(
	e_7816_DEVICE CardReaderId,               // [IN] card reader id
	e_7816_DEVICE SamReaderId,                // [IN] sam reader id
	unsigned long *ulp_ValRemain)
{
	RESPONSE_OBJ* resp;
	e_clySam_DataType FileOrRec;            //[IN] type of data - file or record
	Union_clySam_ReadDataType ReadDataIn;   //[IN] type of file / record to read
	St_clySam_ReadDataResult ReadResult;    //[OUT] read result
	unsigned long Count=0;

	FileOrRec=e_clySam_Rec;
	ReadDataIn.DataRecType.RecNum=EVENT_COUNT_REC_NUM;
	ReadDataIn.DataRecType.RecType=e_clySam_EventCounterRec;

	resp=pSt_ClySam_ReadData(CardReaderId,FileOrRec,&ReadDataIn,&ReadResult);
	if(resp && resp->sw1_sw2[0]==90 && resp->sw1_sw2[1]==0 && resp->Len)
	{
		Count=ReadResult.DataOut.ul_EventCounter;
		memset(&ReadResult,0,sizeof(ReadResult));
		ReadDataIn.DataRecType.RecNum=EVENT_CEILING_REC_NUM;
		ReadDataIn.DataRecType.RecType=e_clySam_EventCeillingRec;
		resp=pSt_ClySam_ReadData(CardReaderId,FileOrRec,&ReadDataIn,&ReadResult);
		if(resp && resp->sw1_sw2[0]==90 && resp->sw1_sw2[1]==0 && resp->Len)
			*ulp_ValRemain=ReadResult.DataOut.EventCeillingArr[EVENT_CEILING_REC_NUM].ul_EventCounter-Count;
	}

	return resp;
}
#ifndef ENABLE_COMM
////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: Add value to ceiling service
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_SamClCeilingUpdate(
	e_7816_DEVICE CardReaderId,                   // [IN] card reader id
	e_7816_DEVICE SamReaderId,                    // [IN] sam reader id
	unsigned long ul_Val2Add)           // [IN] Value to add to ceiling
{
	e_clySam_WriteDataMode WriteMode=e_clySam_DynamicCiphMode;
	e_clySam_WriteDataSamP2 e_WriteDataSamP2=e_clySam_EventCeillFileRec1Type;
	st_clySam_CipherDataSam s_CipherDataSam;    // [IN] struct to fill command parameters (PACKET_7816)
	clySam_BYTE *p=0;


	s_CipherDataSam.e_DataGenOptions=e_clySam_CiphedKeyGen;
	s_CipherDataSam.e_DiversKeyGen=e_clySam_NotDiversKeyGen;
	s_CipherDataSam.s_TargetSamDataRef.GenType=e_clySam_GenEventCeillTargRecType;
	s_CipherDataSam.s_TargetSamDataRef.RecNum=3;
	s_CipherDataSam.CipherKeyKVC=0x60;
	s_CipherDataSam.NumOfCeill2Update=2;    // Number of ceilling to update (0 to 26)

	p=(unsigned char *)&ul_Val2Add;
	memcpy(s_CipherDataSam.NewValue,p,3);


	return pSt_ClySession_WriteSamData(CardReaderId,SamReaderId,
		WriteMode,          // Dynamic or static mode
		e_WriteDataSamP2,   // [IN] enum to fill P2
		s_CipherDataSam);   // [IN] struct to fill command parameters (PACKET_7816)

}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: write Sam Data
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_WriteSamData (
	e_7816_DEVICE SamTargReaderId,                        // [IN]  target sam reader id
	e_7816_DEVICE SamControlReaderId,                     // [IN]  control sam reader id
	e_clySam_WriteDataMode WriteMode,           // Dynamic or static mode
	e_clySam_WriteDataSamP2 e_WriteDataSamP2,   // [IN] enum to fill P2
	st_clySam_CipherDataSam s_CipherDataSam)    // [IN] struct to fill command parameters (PACKET_7816)

{
	RESPONSE_OBJ * obj;
	St_clySam_SN St_SN;
	clySam_BYTE p_Atr[SAM_MAX_ATR_LEN];
	int ip_AtrLen;
	clySam_BYTE uc_Challenge[8] = {0};
	clySam_BYTE Data2Write[48] = {0};

	char TmpCh[5];
	char TmpCh1[10];
	char Pin[16]={(char)0xA4,(char)0x0B, (char)0x01, (char)0xC3, (char)0x9C, (char)0x99, (char)0xCB, (char)0x91, (char)0x0F, (char)0xE6, 
		(char)0x2A, (char)0x23, (char)0x19, (char)0x2A, (char)0x0C, (char)0x5C};


	memset (TmpCh, 0, 5);
	memset (TmpCh1, 0, 10);

	//reset for target sam
	pSt_ClySam_Reset(SamTargReaderId, p_Atr, &ip_AtrLen);
	pSt_ClySam_Unlock(SamTargReaderId, (unsigned char *)Pin);
	//get Card serial number - 4 bytes long 
	if (b_ClySam_GetSerNum (SamTargReaderId, &St_SN) == clySam_FALSE)
		return 0;

	memcpy (TmpCh, (void*)&St_SN.p_SerNum4, 4);
	memrev ((unsigned char*)TmpCh,4);
	memcpy (&TmpCh1[4], TmpCh, 4);

	// Select Diversifier in SAM using the card SN 
	obj = pSt_ClySam_SelectDiversifier(SamControlReaderId,      // [IN] SAM reader id 
		e_clySam_Diver8Byte,                                    // [IN] 4 byte of diversifier type
		(const unsigned char *)TmpCh1);                         // [IN] pointer to 4 byte SN of diversifier
	
    // Check response
	if(! IS_RES_OK( obj ))
		return obj;

	// Get chellenge (card)
	obj=pSt_ClySam_GetChallenge (SamTargReaderId, e_clySam_Challenge8Byte, uc_Challenge);
	
    // Check response
	if(! IS_RES_OK( obj ))
		return obj;

	// Give random (sam)
	obj=pSt_ClySam_GiveRandom(SamControlReaderId, uc_Challenge);
	
    // Check response
	if(! IS_RES_OK( obj ))
		return obj;

	// Chiper sam data (sam)
	obj=pSt_ClySam_CipherSamData (SamControlReaderId, s_CipherDataSam, Data2Write);
	
    // Check response
	if(! IS_RES_OK( obj ))
		return obj;

	// Write data
	return pSt_ClySam_WriteData (SamTargReaderId, WriteMode, e_WriteDataSamP2, Data2Write);
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

RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_OpenSecureSession(e_7816_DEVICE CardReaderId,    //[IN]  card reader id
	e_7816_DEVICE SamReaderId,                                                                //[IN]  sam reader id
	St_clyCard_OpenSessionInput *St_OpenSessionInput,                               //[IN]  Open Session Input parameters
	e_clySam_KeyAccessType KeyAccessType,                                           //[IN]  choose SAM access type -  index \ KIF+KVC  are only available  
	Union_clySam_WorKeyParamsAcess SessionWorkKey,                                  //[IN]  the SAM session work key ( index \ KIF+KVC ) found in the sam work keys                                               
	St_clyCard_OpenSessionOutput *St_OpenSessionOutput)                             //[OUT] Open Session output parameters
{
#ifdef ENABLE_COMM
	RESPONSE_OBJ* p;
	int ApplicationError;
	int RecvSize;

	St_ClyCard_OpenSecureSessionReq  req;
	St_ClyCard_OpenSecureSessionResp resp;
	int s1 = sizeof(req);
	int s2 = sizeof(resp);
	//set the input data
	req.CardReaderId = CardReaderId;
	req.SamReaderId  = SamReaderId;
	req.St_OpenSessionInput = *St_OpenSessionInput;
	req.KifAndKvc = SessionWorkKey.KifAndKvc;
	req.KeyAccessType = KeyAccessType;
	p = NULL;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClySession_OpenSecureSession, 	//int i_cmd,//[IN] the command
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
			*St_OpenSessionOutput = resp.St_OpenSessionOutput;
			RespObj.sw1_sw2[0] = resp.sw1_sw2[0];
			RespObj.sw1_sw2[1] = resp.sw1_sw2[1];
			p = &RespObj;
			
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_ClySession_OpenSecureSession,ApplicationError)
#endif
	return p;
#else
	eClyCardTypes type;
	RESPONSE_OBJ* obj;
	St_clyCard_SN St_SN;
	RANDOM4 Random4;
	clySam_BYTE InitData[70];
	char TmpCh[5];
	char TmpCh1[10];

	memset (TmpCh, 0, 5);
	memset (TmpCh1, 0, 10);
	memset (InitData, 0, 70);

	// Get Card serial number - 4 bytes long 
	if(  b_ClyCard_GetSerNum(CardReaderId,  // [IN] SAM reader id
		&St_SN,&type) == clyCard_FALSE )    // [OUT] card serial numer
		return 0;

	memcpy (TmpCh, (void*)&St_SN.p_SerNum4, 4);
	memrev ((unsigned char*)TmpCh,4);
	memcpy (&TmpCh1[4], TmpCh, 4);

	// Select Diversifier in SAM using the card SN 
	obj = pSt_ClySam_SelectDiversifier(SamReaderId,     // [IN] SAM reader id 
		e_clySam_Diver8Byte,                            // [IN] 4 byte of diversifier type
		(const unsigned char *)TmpCh1);                 // [IN] pointer to 4 byte SN of diversifier
	
    // Check response
	if(! IS_RES_OK( obj ))
		return obj;

	// Get Challenge from SAM
	obj = pSt_ClySam_GetChallenge(SamReaderId,          // [IN] SAM reader id 
		e_clySam_Challenge4Byte,                        // [IN] request for 4 or 8 byte Challenge output
		Random4);                                       // [OUT] the challeng output
	//check response
	if ( !IS_RES_OK( obj ) )
		return obj;

	memcpy(St_OpenSessionInput->Random4,Random4,4);

	// Open Secure Session
	obj = pSt_ClyCard_OpenSecureSession(CardReaderId,   // [IN]  card reader id
		SamReaderId,                                    // [IN]  sam reader id
		St_OpenSessionInput,                            // [IN]  Open Session Input parameters
		St_OpenSessionOutput);                          // [OUT] Open Session output parameters

	// Check response
	if ( !IS_RES_OK( obj ) )
		return obj;

	// Use the KeyKVC return 
	if(St_OpenSessionInput->b_Is2ReturnKeyKvc && KeyAccessType == e_clySam_KeyKIFandKVC )
		SessionWorkKey.KifAndKvc.KVC = St_OpenSessionOutput->KeyKVC;

	// Digest Init
	obj = pSt_ClySam_DigestInit(SamReaderId,            //[IN] SAM reader id 
		KeyAccessType,                                  //[IN] choose access type -  index \ KIF+KVC  are only available  
		SessionWorkKey,                                 //[IN] the session work key ( index \ KIF+KVC ) found in the sam work keys                                               
		obj->data,
		obj->Len);//strlen(InitData) );//[IN]Init Data len
	//check response
	if ( !IS_RES_OK( obj ) )
		return obj;

	//set user callback
	v_ClyCard_SetSessionCallBack(pSt_DigestUpdateCallback);

	return obj;
#endif
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: 
// Description: Close Secure Session
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_CloseSecureSession(e_7816_DEVICE CardReaderId,   //[IN]  card reader id
	e_7816_DEVICE SamReaderId,                                                                //[IN]  sam reader id
	clyCard_BOOL b_IsRatifyImmediatly)                                              //[IN]  1= the session will be immediately Ratified
{

#ifdef ENABLE_COMM
	RESPONSE_OBJ* p;
	int ApplicationError;
	int RecvSize;

	St_ClyCard_CloseSecureSessionReq  req;
	St_ClyCard_CloseSecureSessionResp resp;
	//set the input data
	req.CardReaderId = CardReaderId;
	req.SamReaderId  = SamReaderId;
	req.b_IsRatifyImmediatly = b_IsRatifyImmediatly;
	
	p = NULL;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClySession_CloseSecureSession, 	//int i_cmd,//[IN] the command
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
			RespObj.sw1_sw2[0] = resp.sw1_sw2[0];
			RespObj.sw1_sw2[1] = resp.sw1_sw2[1];
			p = &RespObj;
			
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_ClySession_CloseSecureSession,ApplicationError)
#endif
	return p;
#else
	RESPONSE_OBJ* obj;
	CERTIFIACTE4 certif4Sam;
	CERTIF4 CertifCardLo;

	// Clear user callback
	v_ClyCard_SetSessionCallBack((SESSION_CALLBACK)0);

	// SAM Digest Close
	obj = pSt_ClySam_DigestClose(SamReaderId,//[IN] SAM reader id 
		certif4Sam);//[OUT] sam output certification 

	// Check response
	if ( !IS_RES_OK( obj ) )
		return obj;

	// Close Secure Session - get card certificate
	obj = pSt_ClyCard_CloseSecureSession(CardReaderId,      //[IN]  card reader id
		SamReaderId,                                        //[IN]  sam reader id
		b_IsRatifyImmediatly,                               //[IN]  the session will be immediately Ratified
		certif4Sam,                                         //[IN]  To send to the card from the sam 
		CertifCardLo);                                      //[OUT] To send to the sam from the card
	//check response    
	if ( !IS_RES_OK( obj ) )
		return obj;

	// External Aut - check card certificate
	obj = pSt_ClySam_ExternalAut(SamReaderId,               // [IN] SAM reader id 
		CertifCardLo);                                      // [IN] Card certification to check

	return obj;
#endif
}

#endif // #if defined(CORE_SUPPORT_SMARTCARD) && defined(CORE_SUPPORT_SAM) 
