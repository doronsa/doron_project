
#ifndef CLYCARDOS_H
#define CLYCARDOS_H


///////////////////////////////////////////////////////////////////////////////////////
//
//     Abstract: ClyCrd.c
//         
//         
//
///////////////////////////////////////////////////////////////////////////////////////


#define CLY_CARD_STDCALL  

#include <7816Contactless.h>
#include <Iso7816.h>
#include <ClySamOs.h>


#define CLY_CARD_MAX_ATR_LEN 20
#define MAX_RATIF_LEN 4
#define REC_SIZE 29
#define SW1_SW2_Len 2

//////////////////////////////////////////////////////////////////
//                                  ENUMS
//////////////////////////////////////////////////////////////////

typedef unsigned char clyCard_BYTE;
typedef unsigned char CERTIF4[4],RANDOM4[4];
#ifndef  INCLUDE_KEIL
typedef unsigned char SN8[8];
#endif
 

#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif
#ifdef win_or_linux
#pragma pack(push, CLY_CRD_OS, 1) // correct way to align
#endif


TR_PACK_PREFIX struct TAG_st_ReaderComInfo
{
	unsigned char mPairedReaderId;  //TBD:yoram: e_7816_DEVICE 
	long mComPort;	//int mComPort;
    long mUART;		//int mUART;
    unsigned char mIsExist;
	unsigned char mIsSAMReader;
};
typedef TR_PACK_PREFIX struct TAG_st_ReaderComInfo st_ReaderComInfo;
typedef enum
{
	clyCard_FALSE=0,
	clyCard_TRUE=1 
}clyCard_BOOL;

typedef enum
{
	clyCardKeyChangeLen_PIN=0x10,
	clyCardKeyChangeLen_KEY_LOW=0x18,
	clyCardKeyChangeLen_KEY=0x20,
}clyCardKeyChangeLen;


typedef enum
{
	e_clyCard_KeyIssuer=1,              // key1 -  Use to change other keys
	e_clyCard_KeyCredit,                // key2 -  Credit Key
	e_clyCard_KeyDebit	                // key3 -  Debit Key

}e_clyCard_KeyType;

typedef enum
{
	e_clyCard_NoFile2Select,		    // No file to select - current file remain unchanged
	e_clyCard_TicketingDF,			    // DF ID = "1TIC.ICA" = 0x315449432e494341
	e_clyCard_EnvironmentFile=0x07,	    // Linear,>= 1 records
	e_clyCard_ContractsFile=0x09,	    // Linear,>= 4 records 
	e_clyCard_CountersFile=0x19,	    // Linear/Counter >=9 
	e_clyCard_EventLogFile=0x08,	    // Cyclic>=3 records
	e_clyCard_SpecialEventFile=0x1d,    // Linear,>= 1 records
	e_clyCard_ContractListFile=0x1e     // Linear,>= 1 records
}e_clyCard_FileId;


typedef enum
{
	eClyCardUNKNOWN=0,
	eClyCardCD_LIGHT=0x08,
	eClyCardCD97BX=0x0a,
	eClyCardCD21=0x28
}eClyCardTypes;
/*
/////////////////////////////////////////////////////////////////
STRUCTURES
/////////////////////////////////////////////////////////////////
*/
#if linux
	struct __attribute__((__packed__))  TAG_St_clyCard_OpenSessionInput
	{
		clyCard_BYTE KeyType;			//TBD:yoram e_clyCard_KeyType        // Key Type to use for the session
		clyCard_BYTE b_Is2ReturnKeyKvc; //TBD:yoram clyCard_BOOL     //1= return the kvc of the key in the command output data
		clyCard_BYTE RecNum2Return;         // 0 = read not requested ,n= return the record in the command output data
		clyCard_BYTE FileToSelect;      //TBD:yoram  // e_clyCard_NoFile2Select = not requested - current file remain unchanged
		RANDOM4 Random4;                    // To send to the card from the sam
	};
	typedef struct __attribute__((__packed__))  TAG_St_clyCard_OpenSessionInput  St_clyCard_OpenSessionInput;
#else
typedef TR_PACK_PREFIX struct 
{
	clyCard_BYTE KeyType;			//TBD:yoram e_clyCard_KeyType        // Key Type to use for the session 
	clyCard_BYTE b_Is2ReturnKeyKvc; //TBD:yoram clyCard_BOOL     //1= return the kvc of the key in the command output data  
	clyCard_BYTE RecNum2Return;         // 0 = read not requested ,n= return the record in the command output data  
	clyCard_BYTE FileToSelect;      //TBD:yoram  // e_clyCard_NoFile2Select = not requested - current file remain unchanged
	RANDOM4 Random4;                    // To send to the card from the sam	
}St_clyCard_OpenSessionInput;
#endif


#if linux
	struct __attribute__((__packed__))  TAG_St_clyCard_Ratification
	{
		clyCard_BOOL b_IsRatifExist;            // 1 = last session was not ratified
		clyCard_BYTE RatifLen;                   // 0,2 or 4
		clyCard_BYTE RatifVal[MAX_RATIF_LEN];   // Ratification value
	};
	typedef struct __attribute__((__packed__))  TAG_St_clyCard_Ratification  St_clyCard_Ratification;
#else
typedef TR_PACK_PREFIX struct 
{
	clyCard_BOOL b_IsRatifExist;            // 1 = last session was not ratified
	clyCard_BYTE RatifLen;                   // 0,2 or 4
	clyCard_BYTE RatifVal[MAX_RATIF_LEN];   // Ratification value
}St_clyCard_Ratification;
#endif


#if linux
	struct __attribute__((__packed__))  TAG_St_clyCard_OpenSessionOutput
	{
		clyCard_BYTE KeyKVC;                // if Is2ReturnKeyKvc != 0
		CERTIF4 Certif2Sam;                 // to send to the sam from the card
		St_clyCard_Ratification St_Ratif;
		clyCard_BYTE RecDataRead[REC_SIZE]; // (if RecNum2Return != 0 ) this data need casting in the application layer accurding to the application data strucuts
	};
	typedef struct __attribute__((__packed__))  TAG_St_clyCard_OpenSessionOutput  St_clyCard_OpenSessionOutput;
#else
typedef TR_PACK_PREFIX  struct 
{
	clyCard_BYTE KeyKVC;                // if Is2ReturnKeyKvc != 0
	CERTIF4 Certif2Sam;                 // to send to the sam from the card 
	St_clyCard_Ratification St_Ratif;
	clyCard_BYTE RecDataRead[REC_SIZE]; // (if RecNum2Return != 0 ) this data need casting in the application layer accurding to the application data strucuts 
}St_clyCard_OpenSessionOutput;
#endif


#if linux
	struct __attribute__((__packed__))  TAG_St_clyCard_SN
	{
		unsigned long p_SerNum4; // [OUT] the serial number use for clypso API - 4 lower bytes of the serial number
		SN8 FullSerNum8;
	};
	typedef struct __attribute__((__packed__))  TAG_St_clyCard_SN  St_clyCard_SN;
#else
typedef TR_PACK_PREFIX struct 
{

	unsigned long p_SerNum4; // [OUT] the serial number use for clypso API - 4 lower bytes of the serial number
	SN8 FullSerNum8;
}St_clyCard_SN;
#endif

typedef RESPONSE_OBJ* (*SESSION_CALLBACK)(PACKET_7816 *p7816, // [IN] packet 7816 input
	unsigned char sw1_sw2[SW1_SW2_Len], // [IN] SW1 SW2 respond
	e_7816_DEVICE SamReaderId); // [IN]  sam reader id with which the session was open
/////////////////////////////////////////////////////////////////
//                           API   FUNCTIONS
/////////////////////////////////////////////////////////////////


//initilize the interface 
clyCard_BOOL CLY_CARD_STDCALL b_ClyCard_InitInterface(st_ReaderComInfo *op_CardReaderIdArr,  // [IN]Card reader ID array
	unsigned char uc_ArrLen); // [IN]Card reader ID array len

// Close the interface 
clyCard_BOOL CLY_CARD_STDCALL b_ClyCard_CloseInterface(void);

// Rreturn clyCard_TRUE if card in
clyCard_BOOL CLY_CARD_STDCALL b_ClyCard_DetectCard(e_7816_DEVICE ReaderId); // [IN] SAM reader id

// Reset sam
RESPONSE_OBJ* CLY_CARD_STDCALL pSt_ClyCard_Reset( e_7816_DEVICE ReaderId, // [IN] SAM reader id
	clyCard_BYTE p_Atr[CLY_CARD_MAX_ATR_LEN],   // [OUT] the card atr
	int *ip_AtrLen,                             // [OUT] // the atr Length
	eClyCardTypes* type);                       // type of card

// In case the card reset was made outside this layer - update the layer state machine with it's require information
void CLY_CARD_STDCALL b_ClyCard_StartWorkWithCard(e_7816_DEVICE CardReaderId, // [IN]   card reader id
	e_7816_DEVICE SamReaderId,                        // [IN]    sam reader id
	St_clyCard_SN *stp_CardSN);             // [IN]  serial number in the 

// Eject card card
clyCard_BOOL CLY_CARD_STDCALL b_ClyCard_EjectCard(e_7816_DEVICE ReaderId);


// Get sam serial number - 4 bytes long 
clyCard_BOOL CLY_CARD_STDCALL b_ClyCard_GetSerNum(e_7816_DEVICE ReaderId, // [IN] SAM reader id
	St_clyCard_SN *St_SN,                   // [OUT] card serial numer
	eClyCardTypes* type);                   //[OUT] type of card

//==========================CARD COMMANDS=====================

// Open Secure Session
RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClyCard_OpenSecureSession(e_7816_DEVICE CardReaderId, // [IN]  card reader id
	e_7816_DEVICE SamReaderId,                                        // [IN]  sam reader id
	const St_clyCard_OpenSessionInput *St_OpenSessionInput, // [IN] Open Session Input parameters
	St_clyCard_OpenSessionOutput *St_OpenSessionOutput);    // [OUT]Open Session output parameters

// Close Secure Session

RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClyCard_CloseSecureSession(e_7816_DEVICE CardReaderId, // [IN]  card reader id
	e_7816_DEVICE SamReaderId,                     // [IN]  sam reader id
	clyCard_BOOL b_IsRatifyImmediatly,   // [IN] //1= the session will be immediately Ratified
	const CERTIF4 CertifHiIn,            // [IN] To send to the card from the sam	
	CERTIF4 CertifLoOut);                // [OUT] To send to the sam from the card

//Read Record
RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClyCard_ReadRecord(e_7816_DEVICE ReaderId, // [IN]  reader id
	clyCard_BYTE RecNum,                 // [IN] //record number to read - 1 is always the first record 
	e_clyCard_FileId FileToSelect,       // [IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
	clyCard_BYTE Len2Read,               // [IN] len to read - 1 to record size 
	clyCard_BYTE RecDataOut[REC_SIZE],  // [OUT] data read - this data need casting in the application layer accurding to the application data strucuts 
	clyCard_BYTE ForceRead);

//Write Record
RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClyCard_WriteRecord(e_7816_DEVICE ReaderId, // [IN]  reader id
	clyCard_BYTE RecNum,             // [IN] //record number to Write - 1 is always the first record 
	e_clyCard_FileId FileToSelect,   // [IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
	clyCard_BYTE Len2Write,          // [IN] len to Write - 1 to record size 
	clyCard_BYTE *RecData2Write);    // [IN] data to write will be binary OR with the existing data - in case len<Full recode size, the record will be padded with zeroes


//Update Record
RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClyCard_UpdateRecord(e_7816_DEVICE ReaderId,     // [IN]  reader id
	clyCard_BYTE RecNum,                 // [IN] //record number to Write - set to 1 for cyclic or counter EF
	e_clyCard_FileId FileToSelect,       // [IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
	clyCard_BYTE Len2Update,             // [IN] len to Write - 1 to record size 
	clyCard_BYTE *RecData2Update);       // [IN] data to Update - in case len<Full recode size, the record will be padded with zeroes

// Increase
RESPONSE_OBJ* CLY_CARD_STDCALL pSt_ClyCard_IncreaseDecrease(
	e_7816_DEVICE ReaderId,                   // [IN]reader id
	clyCard_BYTE CountNumber,       // [IN] counter number
	e_clyCard_FileId FileToSelect,  // [IN] file name enum value
	clyCard_BYTE UpdateData[3],     //   data to the card
	clyCard_BYTE NewCountData[3],   //   new couner value
	clyCard_BYTE IsEncreaseFlag);   // 1 for increase 0 for decrease

// Get chalenge
RESPONSE_OBJ* CLY_CARD_STDCALL pSt_ClyCard_GetChalenge(
	e_7816_DEVICE ReaderId,                   // [IN]reader id
	clyCard_BYTE Chlng[8]);         // [OUT] chalenge data

// Change key
RESPONSE_OBJ* CLY_CARD_STDCALL pSt_ClyCard_ChangeKey(
	e_7816_DEVICE ReaderId,                   // [IN] reader id
	e_clyCard_KeyType type,         // [IN] enum value of key for change
	clyCardKeyChangeLen ChngLen,    // [IN] enum value of message len
	clyCard_BYTE *msg);             // [IN]   encrypting message from sam


// Get chalenge
RESPONSE_OBJ* CLY_CARD_STDCALL pSt_ClyCard_Invalidate(e_7816_DEVICE ReaderId);

RESPONSE_OBJ* CLY_CARD_STDCALL pSt_ClyCard_Rehabilitate(e_7816_DEVICE ReaderId);

void  CLY_CARD_STDCALL v_ClyCard_SetSessionCallBack(SESSION_CALLBACK  Proc);

// Get Last Callback Response Object
RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClyCard_GetLastCallbackResponseObj(void);

// Return the last card respone
RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClyCard_GetLastCardResponseObj(void);

///////////////////////////////////////////////////////////////////////
// FUNCTION:    Read and write test low level test API
//              Eitan 13/3/2011
// Discription:  Test the card (tech mode) for read and write operations
//
///////////////////////////////////////////////////////////////////////


RESPONSE_OBJ*   CLY_CARD_STDCALL    pSt_ClyCard_TestReadWrite(e_7816_DEVICE CardReaderId, e_7816_DEVICE SamReaderId);
unsigned  char                      pSt_ClyCard_TestRead(void);

#ifdef win_or_linux
#pragma pack(pop, CLY_CRD_OS) // correct way to align
#endif


#endif
