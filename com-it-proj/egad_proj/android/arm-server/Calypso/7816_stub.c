#include <Iso7816.h>
#include <7816Contactless.h>
#include <AppProtocol.h>


#ifdef ENABLE_COMM
BOOL_7816 _7816_CheckCardComm(int i_ReaderId,RESPONSE_OBJ **p)
{
	int ApplicationError;
	int RecvSize;
	St_7816_CheckCardCommReq  req;

	req.ReaderId = i_ReaderId;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_7816_CheckCardComm, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 0,	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 0,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		
		if(ApplicationError)//OK
		{
#ifdef WIN32
			_DEBUG_PRINT_EX(e_CmdK10_7816_CheckCardComm,ApplicationError, " ID %d", i_ReaderId)
#endif
			return TRUE_7816;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_7816_CheckCardComm,ApplicationError)
#endif
	return FALSE_7816;
}

unsigned char  ContactlessDetect(void)
{
	int ApplicationError;
	int RecvSize;

	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_7816_ContactlessDetect, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 0,	//int i_ObjectInSize,//[IN]// the data size to send
							 0, //void *p_ObjectIn,//[IN] the data 
							 0,	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 0,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
#ifdef WIN32
		_DEBUG_PRINT(e_CmdK10_7816_ContactlessDetect,ApplicationError)
#endif
		return ApplicationError; ///OK
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_7816_ContactlessDetect,ApplicationError)
#endif
	return 0; //error
}


void _7816_GetCardResetInfo(e_7816_DEVICE DevID,        // [IN] the reader DevID
							st_7816_CardResetInfo  *stp_CardResetInfo)
{
	int ApplicationError;
	int RecvSize;
	St_7816_GetCardResetInfoReq  req;
	St_7816_GetCardResetInfoResp resp;

	req.ReaderId = DevID;
	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_7816_GetCardResetInfo, 	//int i_cmd,//[IN] the command
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
			*stp_CardResetInfo = resp.st_CardResetInfo;
		}
	}
	//_DEBUG_PRINT(e_CmdK10_7816_GetCardResetInfo,ApplicationError, DevID)
#ifdef WIN32
	_DEBUG_PRINT_EX(e_CmdK10_7816_GetCardResetInfo,ApplicationError, " ID %d",DevID)
#endif
}

e_7816_STATUS _7816_CloseReader(e_7816_DEVICE DevID)        // [IN] the reader id  TR1020
{
	return e_7816_STATUS_OK;
#if 0
	int ApplicationError;
	int RecvSize;
	St_7816_CloseReaderReq  req;

	req.ReaderId = DevID;
	
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,			//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_7816_CloseReader, 		//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,				//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),						//int i_ObjectInSize,//[IN]// the data size to send
							 &req,								//void *p_ObjectIn,//[IN] the data 
							 0,									//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 0,									//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,							//int *p_OutSizeArive,//[IN]// the size of data respond
							 1,									//unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk )	//int *p_ApplicationError [OUT]// the application respond			  
	{
		if(ApplicationError)//OK
		{
			return e_7816_STATUS_OK;
		}
	}
	return e_7816_STATUS_ERROR;
#endif  
}



e_ContacLessErr ContactlessForgetCard(void)
{
	int ApplicationError;
	int RecvSize;

	ApplicationError = 0; //TBD:yoram debug
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_7816_ContactlessForgetCard, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 0,			//int i_ObjectInSize,//[IN]// the data size to send
							 0,			//void *p_ObjectIn,//[IN] the data 
							 0,			//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 0,			//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		if(ApplicationError)//OK
		{
#ifdef WIN32
			_DEBUG_PRINT(e_CmdK10_7816_ContactlessForgetCard,ApplicationError)
#endif
			return e_Contacless_OK;
		}
	}
#ifdef WIN32
	_DEBUG_PRINT(e_CmdK10_7816_ContactlessForgetCard,ApplicationError)
#endif
	return e_Contactless_ForgetCardErr;
}

//_7816_CardInOut


TR_BOOL DetectCardAndReadData(St_ClyApp_SmartCardData *pData)
{
	int ApplicationError;
	int RecvSize;

	if(pData == 0)
		return TR_FALSE;


	RecvSize = 0; 
	ApplicationError = 0; 
	if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							e_CmdK10_ClyApp_IsCardIn, 	//int i_cmd,//[IN] the command
							PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							0,	//int i_ObjectInSize,//[IN]// the data size to send
							0, //void *p_ObjectIn,//[IN] the data 
							sizeof(St_ClyApp_SmartCardData),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							(void*)pData,	//void *p_ObjectOut,//[OUT]// the data return 
							&RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							&ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		if(ApplicationError && pData->CARD_IN == 1)//OK
		{
#ifdef WIN32
			_DEBUG_PRINT_EX(e_CmdK10_ClyApp_IsCardIn,ApplicationError, "Card In %d Num %d",pData->CARD_IN,pData->SerialNumber)
#endif
			return TR_TRUE;
		}
	}

	//_DEBUG_PRINT_EX(e_CmdK10_ClyApp_IsCardIn,ApplicationError, "Card In %d",pData->CARD_IN)
	return TR_FALSE;
}

#endif
