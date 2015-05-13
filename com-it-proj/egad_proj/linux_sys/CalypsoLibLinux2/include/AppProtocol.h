#ifndef __APP_PROTOCOL_H_
#define __APP_PROTOCOL_H_
#ifndef ENABLE_COMM
#define ENABLE_COMM
#endif
//#include <ProtoLink.h>
#include <ClySamOs.h>
#include <ClyCrdOs.h>
#include <TW_K10232_P.h>
#include <Iso7816.h>
#include <ClyAppApi.h>

#ifdef __cplusplus
	extern "C"
	{
#endif

#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif
#ifdef win_or_linux
  #define TR_PACK pck
  #ifndef TR_PACK_PREFIX
  	  #define TR_PACK_PREFIX
  #endif
  #pragma pack(push, ClyAppApi, 1) // correct way to align
  //define new pack
//  #pragma pack (1)//Specifies packing alignment = 1-byte boundaries.
#endif

#ifdef INCLUDE_KEIL
 //keil
  #define TR_PACK_PREFIX __packed
#endif

/*
  #define TR_PACK_PREFIX
  #pragma pack(push, ClyApp, 1) // correct way to align
*/
        
#ifdef ENABLE_COMM  
#define     PROTO_DEFAULT_TIMEOUT                       1000        
#define		e_ComOk										(0)
#define		e_ComError									(1)
		
extern st_InitResource InitRes;

#endif        
        
#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif

#ifdef WIN32

typedef struct _st_cmd_name
{
	e_CmdK10MssgType	eMsg;
	char*				name;
	
}st_cmd_name;

#ifdef WIN32  //DEBUG

extern st_cmd_name  CmdNameTable[];
extern int  UIprintf(const char *pAnyData,...);
#define _DEBUG_PRINT( CMD , VAL)  \
		if((CMD) >= e_CmdK10_ClySam_InitInterface && (CMD) <  e_CmdK10_ClyLast  )		\
			if(CmdNameTable[(CMD)-CmdNameTable[0].eMsg].eMsg == (CMD))	\
				UIprintf("Protocol TS %d MSG %s recieved %d ",GetTickCount(), CmdNameTable[(CMD)-CmdNameTable[0].eMsg].name, (VAL));	 

#define _DEBUG_PRINT_EX( CMD , VAL, fmt, ...)  \
		if((CMD) >= e_CmdK10_ClySam_InitInterface && (CMD) <  e_CmdK10_ClyLast  )		\
			if(CmdNameTable[(CMD)-CmdNameTable[0].eMsg].eMsg == (CMD))	\
				UIprintf("Protocol TS %d  MSG %s recv %d ,"fmt,GetTickCount(), CmdNameTable[(CMD)-CmdNameTable[0].eMsg].name, (VAL), __VA_ARGS__);	 
#else
#define _DEBUG_PRINT( CMD , VAL)  
#define _DEBUG_PRINT_EX( CMD , VAL, fmt, ...) 
#endif
#endif

/*
TIME - Performance
*/
#define LOOP_COUNT	(1)


void print_result(void);	
void init_timer(void);
void start_timer(int i);  
void stop_timer(int i);  
void set_ret_val(int i, int ret);  







typedef struct _AppCmdData 
{
    //In 
    unsigned long InDataSize;
    void*         pInData;
    //Out
    unsigned long OutDataSize;
    void*         pOutData;
}AppCmdData;

typedef int (*AppCmdRecvCB)(AppCmdData* pCmdData);

typedef struct _AppCmdHandler
{
    AppCmdRecvCB        RecvCB;
    e_CmdK10MssgType    eMsg;
    
}   AppCmdHandler;

//-----------------------------------------------------------------------------
//  e_CmdK10_ClySam_InitInterface
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_InitInterface
	{
		K_DWORD ComPort;
		K_DWORD ReaderId;

	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_InitInterface  St_ClySam_InitInterface;
#else
TR_PACK_PREFIX struct TAG_St_ClySam_InitInterface
{
	K_DWORD ComPort;
	K_DWORD ReaderId;

};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_InitInterface St_ClySam_InitInterface;
#endif
//-----------------------------------------------------------------------------
//  e_CmdK10_ClySam_DetectCard
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_DetectCard
	{
		K_DWORD ReaderId;

	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_DetectCard  St_ClySam_DetectCard;
#else
TR_PACK_PREFIX struct TAG_St_ClySam_DetectCard
{
	K_DWORD ReaderId;

};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_DetectCard St_ClySam_DetectCard;
#endif

//-----------------------------------------------------------------------------
//    e_CmdK10_ClySam_Reset
//-----------------------------------------------------------------------------    
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_ResetReq
	{
		K_DWORD ReaderId;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_ResetReq  St_ClySam_ResetReq;
#else
TR_PACK_PREFIX struct TAG_St_ClySam_ResetReq
{
	K_DWORD ReaderId;
};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_ResetReq St_ClySam_ResetReq;
#endif

//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_ResetResp
	{
		clySam_BYTE p_Atr[SAM_MAX_ATR_LEN];
		K_DWORD iAtrLen;
		RESPONSE_OBJ obj;

	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_ResetResp  St_ClySam_ResetResp;
#else
TR_PACK_PREFIX struct TAG_St_ClySam_ResetResp
{
	clySam_BYTE p_Atr[SAM_MAX_ATR_LEN];
	K_DWORD iAtrLen;
	RESPONSE_OBJ obj;

};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_ResetResp St_ClySam_ResetResp;
#endif

//-----------------------------------------------------------------------------
//    e_CmdK10_ClySam_GetType
//-----------------------------------------------------------------------------    
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_GetTypeReq
	{
		K_DWORD ReaderId;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_GetTypeReq  St_ClySam_GetTypeReq;
#else

TR_PACK_PREFIX struct TAG_St_ClySam_GetTypeReq
{
	K_DWORD ReaderId;
};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_GetTypeReq St_ClySam_GetTypeReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_GetTypeResp
	{
		K_DWORD e_SamType; //e_ClyApp_SamType
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_GetTypeResp  St_ClySam_GetTypeResp;
#else
TR_PACK_PREFIX struct TAG_St_ClySam_GetTypeResp
{
	K_DWORD e_SamType; //e_ClyApp_SamType 
};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_GetTypeResp St_ClySam_GetTypeResp;
#endif

//-----------------------------------------------------------------------------
//  e_CmdK10_ClySam_GetSerNum
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_SerNumReq
	{
		K_DWORD ReaderId;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_SerNumReq  St_ClySam_SerNumReq;
#else
TR_PACK_PREFIX struct TAG_St_ClySam_SerNumReq  
{
	K_DWORD ReaderId;
};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_SerNumReq St_ClySam_SerNumReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_SerNumResp
	{
		St_clySam_SN SerNum;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_SerNumResp  St_ClySam_SerNumResp;
#else
TR_PACK_PREFIX struct TAG_St_ClySam_SerNumResp
{
	St_clySam_SN SerNum;
};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_SerNumResp St_ClySam_SerNumResp;
#endif

//-----------------------------------------------------------------------------
//    e_CmdK10_ClySam_Unlock
//-----------------------------------------------------------------------------    
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_UnlockReq
	{
		K_DWORD ReaderId;
		K_BYTE	PIN[16];
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_UnlockReq  St_ClySam_UnlockReq;
#else
TR_PACK_PREFIX struct TAG_St_ClySam_UnlockReq
{
	K_DWORD ReaderId;
	K_BYTE	PIN[16];
};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_UnlockReq St_ClySam_UnlockReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_UnlockResp
	{
		unsigned char sw1_sw2[2]; //RESPONSE_OBJ obj;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_UnlockResp  St_ClySam_UnlockResp;
#else
TR_PACK_PREFIX struct TAG_St_ClySam_UnlockResp
{
	unsigned char sw1_sw2[2]; //RESPONSE_OBJ obj;
};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_UnlockResp St_ClySam_UnlockResp;
#endif
//-----------------------------------------------------------------------------
//    e_CmdK10_ClySam_ReadData
//-----------------------------------------------------------------------------    
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_ReadDataReq
	{
		K_DWORD ReaderId;
		K_DWORD FileOrRec;					//e_clySam_DataType
		K_DWORD DataFileType;				//  e_clySam_DataFileType:  params,event ceilling, value counter
		St_clySam_DataRecType DataRecType;  // event counter
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_ReadDataReq  St_ClySam_ReadDataReq;
#else
TR_PACK_PREFIX struct TAG_St_ClySam_ReadDataReq
{
	K_DWORD ReaderId;
	K_DWORD FileOrRec;					//e_clySam_DataType 
	K_DWORD DataFileType;				//  e_clySam_DataFileType:  params,event ceilling, value counter
	St_clySam_DataRecType DataRecType;  // event counter
};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_ReadDataReq St_ClySam_ReadDataReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClySam_ReadDataResp
	{
		//unsigned char sw1_sw2[2];
		RESPONSE_OBJ obj;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClySam_ReadDataResp  St_ClySam_ReadDataResp;
#else
TR_PACK_PREFIX struct TAG_St_ClySam_ReadDataResp
{
	//unsigned char sw1_sw2[2]; 
	RESPONSE_OBJ obj;
};
typedef TR_PACK_PREFIX struct TAG_St_ClySam_ReadDataResp St_ClySam_ReadDataResp;
#endif


//-----------------------------------------------------------------------------
//  e_CmdK10_ClyCard_Init 
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_initReq
	{
		st_ReaderComInfo CardReaderIdArr[e_7816_LAST];
		unsigned char uc_ArrLen;

	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_initReq  St_ClyCard_initReq;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_initReq  
{
	st_ReaderComInfo CardReaderIdArr[e_7816_LAST];
	unsigned char uc_ArrLen;

};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_initReq St_ClyCard_initReq;
#endif


//-----------------------------------------------------------------------------
//  e_CmdK10_ClyCard_GetSerNum
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_SerNumReq
	{
		K_DWORD ReaderId;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_SerNumReq  St_ClyCard_SerNumReq;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_SerNumReq  
{
	K_DWORD ReaderId;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_SerNumReq St_ClyCard_SerNumReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_SerNumResp
	{
		St_clyCard_SN SerNum;
		K_DWORD type; //eClyCardTypes
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_SerNumResp  St_ClyCard_SerNumResp;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_SerNumResp
{
	St_clyCard_SN SerNum;
	K_DWORD type; //eClyCardTypes
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_SerNumResp St_ClyCard_SerNumResp;
#endif


//-----------------------------------------------------------------------------
//  e_CmdK10_ClyCard_DetectCard, e_CmdK10_ClyCard_EjectCard
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_DetectCard
	{
		K_DWORD ReaderId;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_SerNumReq  St_ClyCard_DetectCard;
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_SerNumReq  St_ClyCard_EjectCard;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_DetectCard  
{
	K_DWORD ReaderId;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_SerNumReq St_ClyCard_DetectCard;
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_SerNumReq St_ClyCard_EjectCard;
#endif


//-----------------------------------------------------------------------------
//    e_CmdK10_ClyCard_Reset
//-----------------------------------------------------------------------------    
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_ResetReq
	{
		K_DWORD ReaderId;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_ResetReq  St_ClyCard_ResetReq;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_ResetReq
{
	K_DWORD ReaderId;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_ResetReq St_ClyCard_ResetReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_ResetResp
	{
		clyCard_BYTE p_Atr[CLY_CARD_MAX_ATR_LEN];
		K_DWORD iAtrLen;
		RESPONSE_OBJ obj;
		long eClyCardType; //eClyCardTypes
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_ResetResp  St_ClyCard_ResetResp;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_ResetResp
{
	clyCard_BYTE p_Atr[CLY_CARD_MAX_ATR_LEN];
	K_DWORD iAtrLen;
	RESPONSE_OBJ obj;
	long eClyCardType; //eClyCardTypes
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_ResetResp St_ClyCard_ResetResp;
#endif

//-----------------------------------------------------------------------------
//  e_CmdK10_ClyCard_StartWorkWithCard
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_StartWorkWithCardReq
	{
		K_DWORD CardReaderId;
		K_DWORD SamReaderId;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_StartWorkWithCardReq  St_ClyCard_StartWorkWithCardReq;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_StartWorkWithCardReq
{
	K_DWORD CardReaderId;
	K_DWORD SamReaderId;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_StartWorkWithCardReq St_ClyCard_StartWorkWithCardReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_StartWorkWithCardResp
	{
		St_clyCard_SN SerNum;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_StartWorkWithCardResp  St_ClyCard_StartWorkWithCardResp;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_StartWorkWithCardResp
{
	St_clyCard_SN SerNum;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_StartWorkWithCardResp St_ClyCard_StartWorkWithCardResp;
#endif
//-----------------------------------------------------------------------------
//  e_CmdK10_ClyCard_ReadRecord
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_ReadRecordReq
	{
		K_BYTE ReaderId;
		K_BYTE RecNum2Read;
		K_BYTE FileToSelect;	//e_clyCard_FileId
		K_BYTE Len2Read;
		K_BYTE ForceRead;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_ReadRecordReq  St_ClyCard_ReadRecordReq;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_ReadRecordReq
{
	K_BYTE ReaderId;
	K_BYTE RecNum2Read;
	K_BYTE FileToSelect;	//e_clyCard_FileId 
	K_BYTE Len2Read;
	K_BYTE ForceRead;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_ReadRecordReq St_ClyCard_ReadRecordReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_ReadRecordResp
	{
		K_BYTE RecDataOut[REC_SIZE];
		unsigned char sw1_sw2[2];
		//RESPONSE_OBJ obj;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_ReadRecordResp  St_ClyCard_ReadRecordResp;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_ReadRecordResp
{
	K_BYTE RecDataOut[REC_SIZE];
	unsigned char sw1_sw2[2];
	//RESPONSE_OBJ obj;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_ReadRecordResp St_ClyCard_ReadRecordResp;
#endif
//-----------------------------------------------------------------------------
//  e_CmdK10_ClyCard_WriteRecord ,  e_CmdK10_ClyCard_UpdateRecord
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_WriteRecordReq
	{
		K_BYTE ReaderId;		//e_7816_DEVICE
		K_BYTE RecNum;
		K_BYTE FileToSelect;	//e_clyCard_FileId
		K_BYTE Len2Write;		// (Len2Write <= REC_SIZE)
		K_BYTE RecDataOut[REC_SIZE];
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_WriteRecordReq  St_ClyCard_WriteRecordReq;
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_WriteRecordReq  St_ClyCard_UpdateRecordReq;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_WriteRecordReq
{
	K_BYTE ReaderId;		//e_7816_DEVICE
	K_BYTE RecNum;
	K_BYTE FileToSelect;	//e_clyCard_FileId 
	K_BYTE Len2Write;		// (Len2Write <= REC_SIZE)
	K_BYTE RecDataOut[REC_SIZE];
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_WriteRecordReq St_ClyCard_WriteRecordReq;
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_WriteRecordReq St_ClyCard_UpdateRecordReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_WriteRecordResp
	{
		unsigned char sw1_sw2[2];
		//RESPONSE_OBJ obj;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_WriteRecordResp  St_ClyCard_WriteRecordResp;
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_WriteRecordResp  St_ClyCard_UpdateRecordResp;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_WriteRecordResp
{
	unsigned char sw1_sw2[2];
	//RESPONSE_OBJ obj;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_WriteRecordResp St_ClyCard_WriteRecordResp;
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_WriteRecordResp St_ClyCard_UpdateRecordResp;
#endif
//-----------------------------------------------------------------------------
//  e_CmdK10_ClyCard_IncreaseDecrease
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_IncDecCntRecordReq
	{
		K_BYTE ReaderId;		//e_7816_DEVICE
		K_BYTE CountNumber;       //(IN) counter number
		K_BYTE FileToSelect;	//e_clyCard_FileId
		K_BYTE UpdateData[3];     // data to the card
		K_BYTE NewCountData[3];   // new couner value
		K_BYTE IsEncreaseFlag;    // 1 for increase 0 for decrease
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_IncDecCntRecordReq  St_ClyCard_IncDecCntRecordReq;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_IncDecCntRecordReq
{
	K_BYTE ReaderId;		//e_7816_DEVICE
	K_BYTE CountNumber;       //(IN) counter number
	K_BYTE FileToSelect;	//e_clyCard_FileId 
	K_BYTE UpdateData[3];     // data to the card
	K_BYTE NewCountData[3];   // new couner value
	K_BYTE IsEncreaseFlag;    // 1 for increase 0 for decrease
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_IncDecCntRecordReq St_ClyCard_IncDecCntRecordReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_IncDecCntRecordResp
	{
		unsigned char sw1_sw2[2];
		unsigned char counter_val[3];//Yoni 24/7/14
		//RESPONSE_OBJ obj;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_IncDecCntRecordResp  St_ClyCard_IncDecCntRecordResp;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_IncDecCntRecordResp
{
	unsigned char sw1_sw2[2];
	unsigned char counter_val[3];//Yoni 24/7/14
	//RESPONSE_OBJ obj;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_IncDecCntRecordResp St_ClyCard_IncDecCntRecordResp;
#endif


//-----------------------------------------------------------------------------
//  e_CmdK10_ClyCard_Invalidate
//-----------------------------------------------------------------------------
//	Request
#if linux
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_SerNumReq  St_ClyCard_InvalidateReq;
#else
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_SerNumReq St_ClyCard_InvalidateReq;
#endif
//	Response
#if linux
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_IncDecCntRecordResp  St_ClyCard_InvalidateResp;
#else
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_IncDecCntRecordResp St_ClyCard_InvalidateResp;
#endif

//-----------------------------------------------------------------------------
//  e_CmdK10_ClyCard_TestReadWrite
//-----------------------------------------------------------------------------
//	Request
#if linux
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_StartWorkWithCardReq  St_ClyCard_TestReadWriteReq;
#else
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_StartWorkWithCardReq St_ClyCard_TestReadWriteReq;
#endif
//	Response
#if linux
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_IncDecCntRecordResp  St_ClyCard_TestReadWriteResp;
#else
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_IncDecCntRecordResp  St_ClyCard_TestReadWriteResp;
#endif


//-----------------------------------------------------------------------------
//    e_CmdK10_7816_GetCardResetInfo
//-----------------------------------------------------------------------------    
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_7816_GetCardResetInfoReq
	{
		K_DWORD ReaderId;
	};
	typedef struct __attribute__((__packed__))  TAG_St_7816_GetCardResetInfoReq  St_7816_GetCardResetInfoReq;
#else
TR_PACK_PREFIX struct TAG_St_7816_GetCardResetInfoReq
{
	K_DWORD ReaderId;
};
typedef TR_PACK_PREFIX struct TAG_St_7816_GetCardResetInfoReq St_7816_GetCardResetInfoReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_7816_GetCardResetInfoResp
	{
		st_7816_CardResetInfo  st_CardResetInfo;

	};
	typedef struct __attribute__((__packed__))  TAG_St_7816_GetCardResetInfoResp St_7816_GetCardResetInfoResp;
#else
TR_PACK_PREFIX struct TAG_St_7816_GetCardResetInfoResp
{
	st_7816_CardResetInfo  st_CardResetInfo;

};
typedef TR_PACK_PREFIX struct TAG_St_7816_GetCardResetInfoResp St_7816_GetCardResetInfoResp;
#endif


//-----------------------------------------------------------------------------
//    e_CmdK10_7816_CloseReader
//-----------------------------------------------------------------------------    
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_7816_CloseReaderReq
	{
		K_DWORD ReaderId;
	};
	typedef struct __attribute__((__packed__))  TAG_St_7816_CloseReaderReq St_7816_CloseReaderReq;
#else
TR_PACK_PREFIX struct TAG_St_7816_CloseReaderReq
{
	K_DWORD ReaderId;
};
typedef TR_PACK_PREFIX struct TAG_St_7816_CloseReaderReq St_7816_CloseReaderReq;
#endif

//-----------------------------------------------------------------------------
//    e_CmdK10_7816_CheckCardComm
//-----------------------------------------------------------------------------    
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_7816_CheckCardCommReq
	{
		K_DWORD ReaderId;

	};
	typedef struct __attribute__((__packed__))  TAG_St_7816_CheckCardCommReq  St_7816_CheckCardCommReq;
#else
TR_PACK_PREFIX struct TAG_St_7816_CheckCardCommReq
{
	K_DWORD ReaderId;
	
};
typedef TR_PACK_PREFIX struct TAG_St_7816_CheckCardCommReq St_7816_CheckCardCommReq;
#endif

//	St_clyCard_OpenSessionInput *St_OpenSessionInput,                               //[IN]  Open Session Input parameters
//	e_clySam_KeyAccessType KeyAccessType,                                           //[IN]  choose SAM access type -  index \ KIF+KVC  are only available  
//	Union_clySam_WorKeyParamsAcess SessionWorkKey,                                  //[IN]  the SAM session work key ( index \ KIF+KVC ) found in the sam work keys                                               
//	St_clyCard_OpenSessionOutput *St_OpenSessionOutput)                             //[OUT] Open Session output parameters
//{

//-----------------------------------------------------------------------------
//  e_CmdK10_ClySession_OpenSecureSession
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_OpenSecureSessionReq
	{
		K_DWORD CardReaderId;									//[IN]  card reader id - e_7816_DEVICE
		K_DWORD SamReaderId;									//[IN]  sam reader id - e_7816_DEVICE
		St_clyCard_OpenSessionInput St_OpenSessionInput;		//[IN]  Open Session Input parameters
		K_BYTE	KeyAccessType;									//[IN]  e_clySam_KeyAccessType - choose SAM access type -  index \ KIF+KVC  are only available
		//Union_clySam_WorKeyParamsAcess SessionWorkKey		//[IN]  the SAM session work key ( index \ KIF+KVC ) found in the sam work keys
		//clySam_BYTE KeyIndex;
		//clySam_BYTE KeyKIF;
		St_clySam_KIF_And_KVC KifAndKvc;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_OpenSecureSessionReq  St_ClyCard_OpenSecureSessionReq;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_OpenSecureSessionReq
{
	K_DWORD CardReaderId;									//[IN]  card reader id - e_7816_DEVICE
	K_DWORD SamReaderId;									//[IN]  sam reader id - e_7816_DEVICE
	St_clyCard_OpenSessionInput St_OpenSessionInput;		//[IN]  Open Session Input parameters
	K_BYTE	KeyAccessType;									//[IN]  e_clySam_KeyAccessType - choose SAM access type -  index \ KIF+KVC  are only available  
	//Union_clySam_WorKeyParamsAcess SessionWorkKey		//[IN]  the SAM session work key ( index \ KIF+KVC ) found in the sam work keys
	//clySam_BYTE KeyIndex;
	//clySam_BYTE KeyKIF;
	St_clySam_KIF_And_KVC KifAndKvc;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_OpenSecureSessionReq St_ClyCard_OpenSecureSessionReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_OpenSecureSessionResp
{

	unsigned char sw1_sw2[2]; //RESPONSE_OBJ obj;
	St_clyCard_OpenSessionOutput St_OpenSessionOutput;		//[OUT] Open Session output parameters
};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_OpenSecureSessionResp  St_ClyCard_OpenSecureSessionResp;
#else
	TR_PACK_PREFIX struct TAG_St_ClyCard_OpenSecureSessionResp
	{

		unsigned char sw1_sw2[2]; //RESPONSE_OBJ obj;
		St_clyCard_OpenSessionOutput St_OpenSessionOutput;		//[OUT] Open Session output parameters
	};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_OpenSecureSessionResp St_ClyCard_OpenSecureSessionResp;
#endif

//-----------------------------------------------------------------------------
//  e_CmdK10_ClySession_CloseSecureSession
//-----------------------------------------------------------------------------
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_CloseSecureSessionReq
	{
		K_DWORD CardReaderId;				//[IN]  card reader id - e_7816_DEVICE
		K_DWORD SamReaderId;				//[IN]  sam reader id - e_7816_DEVICE
		K_BYTE  b_IsRatifyImmediatly;		//[IN]  1= the session will be immediately Ratified
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_CloseSecureSessionReq  St_ClyCard_CloseSecureSessionReq;
#else
TR_PACK_PREFIX struct TAG_St_ClyCard_CloseSecureSessionReq
{
	K_DWORD CardReaderId;				//[IN]  card reader id - e_7816_DEVICE
	K_DWORD SamReaderId;				//[IN]  sam reader id - e_7816_DEVICE
	K_BYTE  b_IsRatifyImmediatly;		//[IN]  1= the session will be immediately Ratified
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_CloseSecureSessionReq St_ClyCard_CloseSecureSessionReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyCard_CloseSecureSessionResp
	{
		unsigned char sw1_sw2[2]; //RESPONSE_OBJ obj;
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyCard_CloseSecureSessionResp  St_ClyCard_CloseSecureSessionResp;

#else
TR_PACK_PREFIX struct TAG_St_ClyCard_CloseSecureSessionResp
{
	unsigned char sw1_sw2[2]; //RESPONSE_OBJ obj;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyCard_CloseSecureSessionResp St_ClyCard_CloseSecureSessionResp;
#endif
//-----------------------------------------------------------------------------
//  e_CmdK10_ClyApp_TxDataTest,             
//-----------------------------------------------------------------------------
#define MAX_DATA_SIZE (1024)
//	Request
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyApp_TxDataReq
	{
		K_DWORD DataSize;				//[IN]  size of data to send
	};
	typedef struct __attribute__((__packed__))  TAG_St_ClyApp_TxDataReq  St_ClyApp_TxDataReq;

#else
TR_PACK_PREFIX struct TAG_St_ClyApp_TxDataReq
{
	K_DWORD DataSize;				//[IN]  size of data to send
};
typedef TR_PACK_PREFIX struct TAG_St_ClyApp_TxDataReq St_ClyApp_TxDataReq;
#endif
//-----------------------------------------------------------------------------    
//	Response
#if linux
	struct __attribute__((__packed__))  TAG_St_ClyApp_TxDataResp
	{
		//K_DWORD DataSize;					//[IN]  Size of received data
		K_BYTE  DataBuf[MAX_DATA_SIZE];		//[IN]  The Data
	};
	typedef struct TAG_St_ClyApp_TxDataResp St_ClyApp_TxDataResp;

#else
TR_PACK_PREFIX struct TAG_St_ClyApp_TxDataResp
{
	//K_DWORD DataSize;					//[IN]  Size of received data 
	K_BYTE  DataBuf[MAX_DATA_SIZE];		//[IN]  The Data
};
typedef TR_PACK_PREFIX struct TAG_St_ClyApp_TxDataResp St_ClyApp_TxDataResp;
#endif



//-----------------------------------------------------------------------------
//  e_CmdK10_ClyApp_IsCardIn,             
//-----------------------------------------------------------------------------

//	Request - void

#define K10_EVENTS_COUNT             (3)
#define K10_SPECIAL_EVENTS_COUNT     (4)
#define K10_CONTRACTS_COUNT          (8)

#if linux
	struct __attribute__((__packed__))  TAG_RecordData
	{
		unsigned char 		isRead;            // 1- Data was read, 0-data invalid
		unsigned char		data[REC_SIZE];     // Binary data
	};
	typedef struct __attribute__((__packed__))  TAG_RecordData  RecordData;
#else
typedef TR_PACK_PREFIX struct  _RecordData
{
	unsigned char 		isRead;            // 1- Data was read, 0-data invalid
    unsigned char		data[REC_SIZE];     // Binary data
} RecordData; 
#endif


	
//-----------------------------------------------------------------------------    
//	Response


#if linux
		struct __attribute__((__packed__))  TAG_St_ClyApp_SmartCardData
		{
			unsigned char 	CARD_IN;                                    // 1- Card Data was read, 0-data invalid
		    unsigned long 	SerialNumber;
		    RecordData		Env;                                        // Environment
		    RecordData		Event[K10_EVENTS_COUNT];                    // Events
		    RecordData		SpecialEvent[K10_SPECIAL_EVENTS_COUNT];     // Special Events
		    RecordData		Contract[K10_CONTRACTS_COUNT];              // Contract List
		    RecordData		Counter;                                    // Counter of the contracts
			RecordData		ContractList;								// Contract List
		};

		typedef struct __attribute__((__packed__))  TAG_St_ClyApp_SmartCardData  St_ClyApp_SmartCardData;

#else
TR_PACK_PREFIX struct TAG_St_ClyApp_SmartCardData
{
	unsigned char 	CARD_IN;                                    // 1- Card Data was read, 0-data invalid
    unsigned long 	SerialNumber;
    RecordData		Env;                                        // Environment 
    RecordData		Event[K10_EVENTS_COUNT];                    // Events 
    RecordData		SpecialEvent[K10_SPECIAL_EVENTS_COUNT];     // Special Events 
    RecordData		Contract[K10_CONTRACTS_COUNT];              // Contract List
    RecordData		Counter;                                    // Counter of the contracts
	RecordData		ContractList;								// Contract List
};

typedef TR_PACK_PREFIX struct TAG_St_ClyApp_SmartCardData St_ClyApp_SmartCardData;
#endif



//-----------------------------------------------------------------------------    
//  debug
TR_PACK_PREFIX struct TAG_St_ClyApp_Debug  
{
	K_DWORD id;
};
typedef TR_PACK_PREFIX struct TAG_St_ClyApp_Debug St_ClyApp_Debug;



////////////////////////////////////////////////////////////////////////////////////
//
// AppProtocol
//
////////////////////////////////////////////////////////////////////////////////////


int AppProtocolInit(unsigned int ComPortNum);
int AppProtocolClose(void);
int CmdSendEvent(e_CmdK10MssgType eCmd, AppCmdData *pData, int i_TimeOutMs);
int CmdReadCardData(void);
TR_BOOL DetectCardAndReadData(St_ClyApp_SmartCardData *pData);

#ifdef win_or_linux
  #pragma pack(pop, ClyAppApi)
#endif



#ifdef __cplusplus
	}
#endif

#endif 
