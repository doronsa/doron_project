
#ifndef __ISO_7816_H
#define __ISO_7816_H
#include<os_def.h>
#include <Core.h>
///////////////////////////////////////////////////////////////////////////////
//  This modul work with Sam , SmartCard ,ContacLess in level
//  7816-3
//  14443 - 4
//  supprort t=cl in contactless and t=0 in contact
//
// the language that we talk with this modul is ISO 7816-4
// t=cl   (CLAS,INS,P1,P2,LE,LC);  with out GetRespon command
// if the Dirver talk with t=0 card he will translate the t=cl command to
// t=0 command
//
///////////////////////////////////////////////////////////////////////////////


#ifdef  __cplusplus 
extern "C" {
#endif
#define linux 1
#define InitObject(n,ch)            memset(&n,ch,sizeof(n))
#define RESPONOBJ(n)                (RESPONSE_arr+(n))
#define MAX_CARD_INFO_LEN           255
#define MAX_CARD_UID_LEN            10
#define IS_RES_OBJ_OK(Obj)           (Obj && Obj->sw1_sw2[0] == 0x90 && Obj->sw1_sw2[1] == 0)



#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif

#ifndef win_or_linux
    #define MAX_RESPONSE_LEN            110
    #define MAX_DATA_TRANSMIT_LEN       110
    #define USE_7816_SAM_HIGH_SPEED     1
    #ifndef TR_PACK_PREFIX
        #define TR_PACK_PREFIX __packed
    #endif

#else

#ifdef ENABLE_COMM
	#define MAX_RESPONSE_LEN            110
    #define MAX_DATA_TRANSMIT_LEN       110
#else

    #define MAX_RESPONSE_LEN            256
    #define MAX_DATA_TRANSMIT_LEN       256
#endif
//    #define CALYPSO_DUMP_API_DEBUG
    #define USE_7816_SAM_HIGH_SPEED     0
    #define TR_PACK_PREFIX
    #pragma pack(push, Iso7816, 1) // correct way to align

#endif


////////////////////////////////////////////////////////////////////////////////////

typedef unsigned char BYTE_7816;

typedef enum 
{
	TRUE_7816=1,
	FALSE_7816=0
}BOOL_7816;


////////////////////////////////////////////////////////////////////////////////////
#define e_7816_DEVICE_WINDOWS_OFFSET 20  //TBD:yoram

typedef enum 
{
    e_7816_CONTACTLESS,
    e_7816_SAM_TW,
    e_7816_SAM_CAL, 
    e_7816_NONE,

#if  !defined(ENABLE_COMM)  && !defined(INCLUDE_KEIL)

	/////////////////////////////////////////////////////////////////////////////////////////////
    // Note: Win32 specific backwards support 
	//
    WIN32_READER_PC_SC_A       =  e_7816_DEVICE_WINDOWS_OFFSET + 0,				// PC-SC Supported Reader
    WIN32_READER_PC_SC_B       =  e_7816_DEVICE_WINDOWS_OFFSET + 1,				// PC-SC Supported Reader
    WIN32_READER_PC_SC_C       =  e_7816_DEVICE_WINDOWS_OFFSET + 2,				// PC-SC Supported Reader
    WIN32_READER_PC_SC_D       =  e_7816_DEVICE_WINDOWS_OFFSET + 3,				// PC-SC Supported Reader
    WIN32_READER_PC_SC_E       =  e_7816_DEVICE_WINDOWS_OFFSET + 4,				// PC-SC Supported Reader
    WIN32_READER_OPM_A         =  e_7816_DEVICE_WINDOWS_OFFSET + 5,				// SC_A inside OPMA 
    WIN32_READER_OPM_B         =  e_7816_DEVICE_WINDOWS_OFFSET + 6,				// SC_B inside OPMB 
    WIN32_READER_OPM_C         =  e_7816_DEVICE_WINDOWS_OFFSET + 7,				// SC_C inside OPMC 
    WIN32_READER_OPM_D         =  e_7816_DEVICE_WINDOWS_OFFSET + 8,				// SC_D inside OPMD 
    WIN32_READER_CONTACTLESS_A =  e_7816_DEVICE_WINDOWS_OFFSET + 9,				// Contactless Smart Card Reader
    WIN32_READER_CONTACTLESS_B =  e_7816_DEVICE_WINDOWS_OFFSET + 10,			// Contactless Smart Card Reader
    WIN32_READER_SOFTWARE_TERM =  e_7816_DEVICE_WINDOWS_OFFSET + 11,			// Virtual     Reader - security reasons
    WIN32_READER_NONE          =  e_7816_DEVICE_WINDOWS_OFFSET + 12,			// No Reader
	/////////////////////////////////////////////////////////////////////////////////////////////
#endif

	// Must be last
	e_7816_LAST
}e_7816_DEVICE;


typedef enum 
{
	e_7816_STATUS_OK,
	e_7816_STATUS_ERROR
}e_7816_STATUS;

////////////////////////////////////////////////////////////////////////////////////
//
// struct  RESPONSE_OBJ
// 7816 respons command
//
////////////////////////////////////////////////////////////////////////////////////
#if linux
__attribute__((__packed__)) struct TAG_RESPONSE_OBJ
{
	unsigned char data[MAX_RESPONSE_LEN];
	//unsigned char temp[2];//doron
	unsigned char sw1_sw2[2];
	short int Len;
	long ReaderErr;
};
typedef struct __attribute__((__packed__))  TAG_RESPONSE_OBJ  RESPONSE_OBJ;

#else
TR_PACK_PREFIX struct TAG_RESPONSE_OBJ
{
	unsigned char data[MAX_RESPONSE_LEN];
	unsigned char sw1_sw2[2];
	short int Len;
	long ReaderErr;
};
typedef TR_PACK_PREFIX struct TAG_RESPONSE_OBJ  RESPONSE_OBJ;
#endif


////////////////////////////////////////////////////////////////////////////////////
//
//  struct  PACKET_7816
//  it is the iso 7816 command (t=cl)
//
////////////////////////////////////////////////////////////////////////////////////

#define HEADER_786_SIZE(n) (sizeof((n).CLA)+sizeof((n).INS)+sizeof((n).P1)+sizeof((n).P2)+\
	sizeof((n).LE) +sizeof((n).LC))

////////////////////////////////////////////////////////////////////////////////////
#if linux
__attribute__((__packed__)) struct TAG_WPACKET_7816
	{
		BYTE_7816 CLA;
		BYTE_7816 INS;
		BYTE_7816 P1;
		BYTE_7816 P2;
		BYTE_7816 LC;
		BYTE_7816 Data[MAX_DATA_TRANSMIT_LEN];
		BYTE_7816 LE;
	};
	typedef struct __attribute__((__packed__))  TAG_WPACKET_7816  PACKET_7816;
#else
	TR_PACK_PREFIX struct WPACKET_7816
	{
		BYTE_7816 CLA;
		BYTE_7816 INS;
		BYTE_7816 P1;
		BYTE_7816 P2;
		BYTE_7816 LC;
		BYTE_7816 Data[MAX_DATA_TRANSMIT_LEN];
		BYTE_7816 LE;
	};
	typedef TR_PACK_PREFIX struct WPACKET_7816 PACKET_7816;
#endif

////////////////////////////////////////////////////////////////////////////////////

// Card type
#if defined (ENABLE_COMM)
 #define FLAG1
#endif
#if defined(INCLUDE_KEIL)
 #define FLAG1
#endif

typedef  enum tage_7816_CardType
{
#ifdef FLAG1 //defined(ENABLE_COMM)  || defined(INCLUDE_KEIL)//ndef WIN32	
    e_7816_Cly_CDLIGHT,
	e_7816_Cly_CD97BX,
	e_7816_Cly_CD21,
	e_7816_Cly_CTS256B,
	e_7816_Cly_SAM,
	e_7816_UNKNOWN_CARD,
#else
    e_7816_MIF_STD ,
	e_7816_MIF_PRO  ,///efc
	e_7816_MIF_DES_PRO ,
	e_7816_MIF_PLUS ,
	e_7816_MIF_STD_PLUS ,
	e_7816_MIF_ULTRALIGHT,
	e_7816_MIF_LIGHT,
	e_7816_Cly_CDLIGHT,
	e_7816_Cly_CD97BX,
	e_7816_Cly_CD21,
	e_7816_Cly_CTS256B,
	e_7816_Cly_SAM,
	e_7816_GUMPLUSE,
	e_7816_UNKNOWN_CARD,
#endif

}e_7816_CardType;



////////////////////////////////////////////////////////////////////////////////////
#if linux
__attribute__((__packed__)) struct TAG_st_7816_CardResetInfo
	{
		BYTE_7816       b_IsReaderInit;                 // Is reader Init
	#if !defined(ENABLE_COMM)  && !defined(INCLUDE_KEIL)
		e_7816_CardType e_CardType;                     // Card type
	#else
		BYTE_7816       e_CardType;
	#endif
		BYTE_7816       c_UidLen;                       // Cl Card size of UID
		BYTE_7816       cp_ClUid[MAX_CARD_UID_LEN];     // Cl Card UID
		BYTE_7816       uc_CardAtrSetectDataLen;        // len of the ATS / ATR
		BYTE_7816       uc_AtrSetectData [MAX_CARD_INFO_LEN]; // Contactless ATS or Contact ATR
	};
	typedef struct __attribute__((__packed__))  TAG_st_7816_CardResetInfo  st_7816_CardResetInfo;
#else
	typedef  TR_PACK_PREFIX struct
	{
		BYTE_7816       b_IsReaderInit;                 // Is reader Init
	#if !defined(ENABLE_COMM)  && !defined(INCLUDE_KEIL)
		e_7816_CardType e_CardType;                     // Card type
	#else
		BYTE_7816       e_CardType;
	#endif
		BYTE_7816       c_UidLen;                       // Cl Card size of UID
		BYTE_7816       cp_ClUid[MAX_CARD_UID_LEN];     // Cl Card UID
		BYTE_7816       uc_CardAtrSetectDataLen;        // len of the ATS / ATR
		BYTE_7816       uc_AtrSetectData [MAX_CARD_INFO_LEN]; // Contactless ATS or Contact ATR
	}st_7816_CardResetInfo;
#endif


////////////////////////////////////////////////////////////////////////////////////
//
// Function: _7816_GetCardResetInfo
// Description: 
// Parameters:  
// Return:
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

void _7816_GetCardResetInfo(
	e_7816_DEVICE DevID,//[IN] the reader id
	st_7816_CardResetInfo  *stp_CardResetInfo);//[OUT] the card info TR1020 

////////////////////////////////////////////////////////////////////////////////////
//
// Function: _7816_GetCardAtr
// Description: 
// Parameters:  
// Return:
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

char* _7816_GetCardAtr(
	e_7816_DEVICE DevID,        // [IN] the reader id
	long *AtrLenOut);           // [OUT] the ATR Len  TR1020 

////////////////////////////////////////////////////////////////////////////////////
//
// Function: _7816_IsSmartCardIn
// Description: 
// Parameters:  
// Return:
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

BOOL_7816 _7816_IsSmartCardIn(
	e_7816_DEVICE DevID);        // [IN] the reader id must be sam0 or sam1


////////////////////////////////////////////////////////////////////////////////////
//
// Function: _7816_ResetCard
// Description: 
// Parameters:  
// Return:
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

RESPONSE_OBJ * _7816_ResetCard(
	e_7816_DEVICE DevID,        // [IN] the reader id
	int TimeOut);               // [IN] the time out for ATR TR1020

////////////////////////////////////////////////////////////////////////////////////
//
// Function: _7816_CardInOut
// Description: 
// Parameters:  
// Return:
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

RESPONSE_OBJ * _7816_CardInOut(
	PACKET_7816 *Ptr,           // [IN] the 7816 APDU
	e_7816_DEVICE DevID,        // [IN] the reader id
	int _TimeOut);              // [IN] the time out  TR1020

////////////////////////////////////////////////////////////////////////////////////
//
// Function: _7816_Init 
// Description: 
// Parameters:  
// Return:
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  


e_7816_STATUS _7816_Init(eCoreUARTId UARTId,    // [IN] The Uart for this sam
	e_7816_DEVICE DevID,                        // [IN] The reader id
	unsigned char HighSpeed);                   // [IN] 0 = false, else true

////////////////////////////////////////////////////////////////////////////////////
//
// Function: _7816_CloseReader
// Description: 
// Parameters:  
// Return:
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

e_7816_STATUS _7816_CloseReader(
	e_7816_DEVICE DevID);       // [IN] the reader id  TR1020

////////////////////////////////////////////////////////////////////////////////////
//
// Function: _7816_EjectCard
// Description: 
// Parameters:  
// Return:
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

BOOL_7816 _7816_EjectCard(
	e_7816_DEVICE DevID);       //[IN] the reader id   TR1020

////////////////////////////////////////////////////////////////////////////////////
//
// Function: _7818_CheckCardComm
// Description: 
// Parameters:  
// Return:
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

BOOL_7816 _7816_CheckCardComm(int i_ReaderId,RESPONSE_OBJ **p);


////////////////////////////////////////////////////////////////////////////////////
//
// Function: Init7816Win32
// Description: 
// Parameters:
// Return:    0 - fail
//            1- success
//
////////////////////////////////////////////////////////////////////////////////////

BOOL_7816 _7816_Win32Init(void);


#ifdef win_or_linux
#pragma pack(pop, Iso7816)
#endif

#ifdef  __cplusplus 
}

#endif

#endif // __ISO_7816_H
