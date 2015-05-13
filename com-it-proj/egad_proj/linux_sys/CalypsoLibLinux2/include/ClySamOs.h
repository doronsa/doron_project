
///////////////////////////////////////////////////////////////////////////////////////
//
//     Abstract: ClySamOs.h
//         
//         
//
///////////////////////////////////////////////////////////////////////////////////////

#ifndef CLYSAMOS_H
#define CLYSAMOS_H

//////////////////////////////////////////////////////////////////////////////
//
//                                      DEFINES
//
//////////////////////////////////////////////////////////////////////////////
#include<os_def.h>

#include <Iso7816.h>

#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif
#ifdef win_or_linux
#pragma pack(push, CLY_SAM_OS, 1) // correct way to align
#endif

#define SAM_MAX_ATR_LEN     20
#define COUNTERS_PER_REC    9
#define MAX_SUM_REC         7

//////////////////////////////////////////////////////////////////////////////
//
//                                       ENUMS
//
//////////////////////////////////////////////////////////////////////////////

typedef unsigned char clySam_BYTE;
typedef unsigned char CERTIFIACTE4[4], CERTIFIACTE3[3], AMOUNT4[4], SUM_VALUE4[4], INCREASE_NUMBER3[3], COUNT_VALUE3[3];
typedef unsigned char CERTIFIACTE8[8],RANDOM8[8],UULOCK_PIN16[16], PIN4[4];
typedef unsigned long SN4;
typedef unsigned char SN8[8];

typedef enum
{
	clySam_FALSE=0,
	clySam_TRUE=1 
}clySam_BOOL;

typedef enum
{
    e_ClyApp_SamSL,
    e_ClyApp_SamDV,
    e_ClyApp_SamCPP,
    e_ClyApp_SamCP,
    e_ClyApp_SamCL,
    e_ClyApp_SamCV
}e_ClyApp_SamType;


typedef enum 
{
	e_clySam_Diver4Byte=0x4,
	e_clySam_Diver8Byte=0x8
}e_clySam_DiversifierType;

typedef enum 
{
	e_clySam_Challenge4Byte=0x4,
	e_clySam_Challenge8Byte=0x8
}e_clySam_ChallengeType;


typedef enum 
{
	e_clySam_WorkKeyFile,
	e_clySam_SystemKeyFile
}e_clySam_ReadKeyFileType;


typedef enum 
{
	e_clySam_File,
	e_clySam_Rec
}e_clySam_DataType;


typedef enum 
{
	e_clySam_EventCounterRec,
	e_clySam_EventCeillingRec,
	e_clySam_SumRec
}e_clySam_DataRecType;


typedef enum 
{
	e_clySam_EPTransactionNum,
	e_clySam_ParamFile
}e_clySam_DataFileType;


typedef enum
{
	e_clySam_SAM_S1=2
}e_clySam_DataStructType;


typedef enum
{
	e_clySam_WorkKeyParams,
	e_clySam_SystemKeyParams,
	e_clySam_LockFileKeyParams
}e_clySam_KeyParamsType;


typedef enum
{
	e_clySam_KeyIndex,
	e_clySam_KeyKIF,        // key Identifier only
	e_clySam_KeyKIFandKVC   // key Identifier only + version and category
}e_clySam_KeyAccessType;


typedef enum
{
	e_clySam_DES,
	e_clySam_DESX
}e_clySam_KeyAlg;

typedef enum
{
	e_clySam_CardKey,
	e_clySam_SamKey
}e_clySam_KeySystem;

typedef enum
{
	e_clySam_ElectronicPurse,
	e_clySam_Ticketing,
	e_clySam_Service,
	e_clySam_Global
}e_clySam_KeyApplication;

typedef enum
{
	e_clySam_Personalization=0x01,
	e_clySam_Fabrication=0x03,
	e_clySam_Reloading=0x07,
	e_clySam_ReloadingRequest=0x07,
	e_clySam_DigitalSignature=0x0B,
	e_clySam_Authentication=0x0C,
	e_clySam_Lock=0x0F,
	e_clySam_Debit=0x10,
	e_clySam_Collect=0x18,
	e_clySam_Certification=0x1A,
	e_clySam_ParametersUpdate=0x1C,
	e_clySam_KeyUpdate=0x1D
}e_clySam_KeyFunction;

typedef enum 
{
	e_clySam_NotSwitch2HSP=0,
	e_clySam_Switch2HSP=0x80
}e_clySam_ChangeSpeedType;						

typedef enum 
{
	e_clySam_DynamicCiphMode=0,
	e_clySam_StaticCiphMode=1
}e_clySam_WriteDataMode;

typedef enum 
{
	e_clySam_ChallangeFromGetChal=0,
	e_clySam_ChallangeFromGiveRand=1
}e_clySam_ChallangeType;

typedef enum 
{
	e_clySam_Cipher8BytesKey,
	e_clySam_CipherPinModification,
	e_clySam_CipherPinVerification,
	e_clySam_Cipher16BytesKey,
}e_clySam_CipherCommandType;

typedef enum
{
	e_clySam_CiphedKeyGen,
	e_clySam_PlainKeyGen,
}e_clySam_DataGenOptions;

typedef enum
{
	e_clySam_NotDiversKeyGen,
	e_clySam_DiversKeyGen
}e_clySam_DiversKeyGenerator;

typedef enum
{
	e_clySam_GenParamAndEventType       = 0,
	e_clySam_GenIndexWorkKeyType        = 1,
	e_clySam_GenParamType               = 0xa0,
	e_clySam_GenEventCeillDefRecType    = 0xb0,
	e_clySam_GenEventCeillTargRecType   = 0xb1,
	e_clySam_GenOneEventCeillType       = 0xb8,
	e_clySam_GenSysKeyByKifKvcType      = 0xc0,
	e_clySam_GenWorkKeyByKifKvcType     = 0xf0
}e_clySam_GenType;

typedef enum
{
	e_clySam_MiniSamType                = 0,
	e_clySam_ParamRecType               = 0xa0,
	e_clySam_EventCeillRec1Type         = 0xb0,
	e_clySam_EventCeillFileRec1Type     = 0xb1,
	e_clySam_EventCeillFileRec2Type     = 0xb2,
	e_clySam_EventCeillFileRec3Type     = 0xb3,
	e_clySam_EventCeillOneRecType       = 0xb8,
}e_clySam_WriteDataSamP2;

//////////////////////////////////////////////////////////////////////////////
//
// STRUCTURES
//
//////////////////////////////////////////////////////////////////////////////
#if linux
	struct __attribute__((__packed__))  TAG_St_clySam_DataRecType
	{
		clySam_BYTE RecType;  //e_clySam_DataRecType
		clySam_BYTE RecNum;
	};
	typedef struct __attribute__((__packed__))  TAG_St_clySam_DataRecType  St_clySam_DataRecType;
#else
	typedef TR_PACK_PREFIX struct
	{
		clySam_BYTE RecType;  //e_clySam_DataRecType
		clySam_BYTE RecNum;
	}St_clySam_DataRecType;
#endif

typedef union 
{
	e_clySam_DataFileType DataFileType;         // params,event ceilling, value counter
	St_clySam_DataRecType DataRecType;          // event counter
}Union_clySam_ReadDataType;


typedef struct 
{
	clySam_BOOL b_IsSystemLockEnable;           // 1=system key modification forbidden
	clySam_BOOL b_IsWorkKeysLockEnable;         // 1= work key modification forbidden
	clySam_BOOL b_IsPlainSamKeyInputEnable;     // 1= clear key loading autorized
	clySam_BOOL b_IsRandomKeyGenerationEnable;  // 1= Random Key Generation Enable
	unsigned long ul_ParamsVer;	                // example 020100h = v02.01.00
	e_clySam_DataStructType e_DataStructType;
}St_clySam_Params;


typedef struct 
{
	unsigned long ul_EventCounter;
	clySam_BOOL b_IsFreeEventCounter;           //1= is free event counter config - the corresponding event counter can be incremented freely with INCREMENT command
}St_clySam_EventCounter;


typedef St_clySam_EventCounter EventCeillingArrType[COUNTERS_PER_REC];


typedef struct 
{
	unsigned long ul_Sum;
	unsigned long ul_IncreaseNum;
}St_clySam_ValueCounter;


typedef union 
{
	unsigned long ul_EventCounter;
	St_clySam_Params Params;
	EventCeillingArrType EventCeillingArr;
	St_clySam_ValueCounter ValueCounter;
}Union_clySam_DataOut;



typedef struct 
{
	CERTIFIACTE8 Certif8;
	Union_clySam_DataOut DataOut;
	clySam_BYTE P2;
	SN4 sn;
	clySam_BYTE KIF;
	clySam_BYTE KVC;
	clySam_BYTE ALG;
}St_clySam_GeneralReadInfo;

typedef struct 
{
	Union_clySam_DataOut DataOut;
	St_clySam_GeneralReadInfo GeneralReadInfo;
}St_clySam_ReadDataResult;


#if linux
	struct __attribute__((__packed__))  TAG_St_clySam_KIF_And_KVC
	{
		clySam_BYTE KIF;
		clySam_BYTE KVC;
	};
	typedef struct __attribute__((__packed__))  TAG_St_clySam_KIF_And_KVC  St_clySam_KIF_And_KVC;
#else
	typedef TR_PACK_PREFIX struct
	{
		clySam_BYTE KIF;
		clySam_BYTE KVC;
	}St_clySam_KIF_And_KVC;
#endif



typedef union
{
	clySam_BYTE KeyIndex;
	clySam_BYTE KeyKIF;
	St_clySam_KIF_And_KVC KifAndKvc;
}Union_clySam_WorKeyParamsAcess;

typedef union
{
	clySam_BYTE KeyIndex;
	clySam_BYTE KeyKIF;
}Union_clySam_SystemKeyParamsAcess;


typedef union
{
	Union_clySam_WorKeyParamsAcess WorKeyParamsAcess;
	Union_clySam_SystemKeyParamsAcess SystemKeyParamsAcess;
}Union_clySam_KeyInfo;


typedef struct 
{
	e_clySam_KeySystem KeySystem;
	e_clySam_KeyApplication KeyApplication;
	e_clySam_KeyFunction KeyFunction;
}St_clySam_KIF;

typedef struct 
{
	e_clySam_KeySystem KeySystem;
	e_clySam_KeyApplication KeyApplication;
	e_clySam_KeyFunction KeyFunction;
}St_clySam_KVC;


typedef struct 
{
	clySam_BOOL b_IsDeciperEnable;
	clySam_BOOL b_IsStaticCipherEnable;
	clySam_BOOL b_IsCiperEnable;
	clySam_BOOL b_IsDiversifiedCipherEnable;
	clySam_BOOL b_IsCertifComputeEnable;
	clySam_BOOL b_IsCertifFourBytesEnable;
	clySam_BOOL b_IsSessionVerifyEnable;

}St_clySam_PAR1;

typedef struct 
{
	clySam_BOOL b_IsAnyReloadAmountEnable;
	clySam_BOOL b_IsCancelPurchaseEnable;
	clySam_BOOL b_IsCardUpdateKeyEnable;
	clySam_BOOL b_IsCardChangePinEnable;
	clySam_BOOL b_IsCardVerifyPinEnable;
	clySam_BOOL b_IsCardStampeModeEnable;
	clySam_BOOL b_IsProtectedModeEnable;
	clySam_BOOL b_IsCardSessionModeEnable;

}St_clySam_PAR2;

typedef struct 
{
	clySam_BOOL b_IsTransferPlainWithoutDiversify;
	clySam_BOOL b_IsTransferPlainWithDiversify;
	clySam_BOOL b_IsTransferCipherWithoutDiversify;
	clySam_BOOL b_IsTransferCipherWithDiversify;

}St_clySam_KeyTransferAndGenerateLimitations;



typedef struct 
{
	St_clySam_KeyTransferAndGenerateLimitations KeyTransferAuthorization;
	St_clySam_KeyTransferAndGenerateLimitations  KeyGenerateLimitations;

}St_clySam_PAR3;



typedef struct 
{
	St_clySam_KIF KIF;
	clySam_BYTE KVC;
	e_clySam_KeyAlg alg;
	St_clySam_PAR1 PAR1;
	St_clySam_PAR2 PAR2;
	St_clySam_PAR3 PAR3;
	clySam_BYTE PAR4_KeyUsageCounter; // 0 = no key usage counter associate
}St_clySam_WorkAndSystemKeyRec;


typedef struct 
{
	St_clySam_KIF KIF;
	clySam_BYTE KVC;
	clySam_BOOL b_IsLockAtReset;
	clySam_BOOL b_IsLockedLock;
}St_clySam_LockKeyRec;


typedef union
{
	St_clySam_WorkAndSystemKeyRec WorkAndSystemKeyRec;
	St_clySam_LockKeyRec LockKeyRec;
}Union_clySam_KeyRec ;

typedef struct 
{
	Union_clySam_KeyRec KeyRec;
	St_clySam_GeneralReadInfo GeneralReadInfo;
}St_clySam_ReadKeyResult;


#if linux
	struct __attribute__((__packed__))  TAG_St_clySam_SN
	{

		unsigned long p_SerNum4;// [OUT] the serial number use for clypso API - 4 lower bytes of the serial number
		SN8 FullSerNum8;
	};
	typedef struct __attribute__((__packed__))  TAG_St_clySam_SN  St_clySam_SN;
#else
	typedef TR_PACK_PREFIX struct
	{

		unsigned long p_SerNum4;// [OUT] the serial number use for clypso API - 4 lower bytes of the serial number
		SN8 FullSerNum8;
	}St_clySam_SN;
#endif

typedef struct
{
	e_clySam_GenType GenType;
	short RecNum; //record number (if not -1)
}St_clySam_TargetSamDataRef;

typedef struct
{
	e_clySam_CipherCommandType e_CipherCommandType;
	unsigned char* KeyOrPin;
	unsigned char* NewPin;
	e_clySam_KeyAccessType e_AccessType4Cipher;
	Union_clySam_WorKeyParamsAcess u_ParamsAccess4Cipher;
	e_clySam_KeyAccessType e_AccessTypeCipher;
	Union_clySam_WorKeyParamsAcess u_ParamsAccessCipher;
}st_CipherDataCard;

typedef struct
{
	e_clySam_DataGenOptions e_DataGenOptions;
	e_clySam_DiversKeyGenerator e_DiversKeyGen;
	St_clySam_TargetSamDataRef s_TargetSamDataRef;
	clySam_BYTE CipherKeyKVC;
	clySam_BYTE NumOfCeill2Update; //number of ceilling to update (0 to 26)
	clySam_BYTE NewValue[3];
}st_clySam_CipherDataSam;

//////////////////////////////////////////////////////////////////////////////
//
//API   FUNCTIONS
//
//////////////////////////////////////////////////////////////////////////////


// initilize the interface 
clySam_BOOL  b_ClySam_InitInterface(int ComPort,e_7816_DEVICE ReaderId);  // [IN] SAM reader id


//close the interface 
clySam_BOOL  b_ClySam_CloseInterface(e_7816_DEVICE ReaderId);             // [IN] SAM reader id);


//return clySam_TRUE if card in
clySam_BOOL  b_ClySam_DetectCard(e_7816_DEVICE ReaderId);                 // [IN] SAM reader id


//reset sam
RESPONSE_OBJ*  pSt_ClySam_Reset(e_7816_DEVICE ReaderId,                   // [IN] SAM reader id 
	clySam_BYTE p_Atr[SAM_MAX_ATR_LEN],                         // [OUT] the card atr
	int *ip_AtrLen);                                            // [OUT] // the atr Length

//eject card card
clySam_BOOL  b_ClySam_EjectCard(e_7816_DEVICE ReaderId);                  // [IN] SAM reader id

//get sam serial number - 4 bytes long
clySam_BOOL  b_ClySam_GetSerNum(e_7816_DEVICE ReaderId,                   // [IN] SAM reader id 
	St_clySam_SN *St_SN);                                       // [OUT] card serial numer

//==========================SAM COMMANDS=====================
//Select Diversifier
RESPONSE_OBJ*   pSt_ClySam_SelectDiversifier(e_7816_DEVICE ReaderId,      // [IN] SAM reader id 
	e_clySam_DiversifierType e_DiverType,                       // [IN] 4 or 8 byte of diversifier type
	const clySam_BYTE *uc_Diversifier);                         // [IN]pointer to 4 or 8 byte of diversifier

//Get Challenge
RESPONSE_OBJ*   pSt_ClySam_GetChallenge(e_7816_DEVICE ReaderId,           // [IN] SAM reader id 
	e_clySam_ChallengeType e_ChallType,                         // [IN] request for 4 or 8 byte Challenge output
	clySam_BYTE *uc_Challenge);                                 // [out] the challeng output

//Digest Init
RESPONSE_OBJ*   pSt_ClySam_DigestInit(e_7816_DEVICE ReaderId,             // [IN] SAM reader id 
	e_clySam_KeyAccessType KeyAccessType,                       // [IN] choose access type -  index \ KIF+KVC  are only available  
	Union_clySam_WorKeyParamsAcess SessionWorkKey,              // [IN] the session work key ( index \ KIF+KVC ) found in the sam work keys												
	const clySam_BYTE *uc_InitData,                             // [IN]the data retured by the card to the OPEN SECURED SESSION command
	unsigned int i_InitDataLen );                               // [IN]Init Data len

//Digest Update
RESPONSE_OBJ*   pSt_ClySam_DigestUpdate(e_7816_DEVICE ReaderId,           // [IN] SAM reader id 
	clySam_BYTE *uc_Data2Add,                                   // [IN]the data send to the card
	unsigned int i_DataLen );                                   // [IN]Init Data len

//Digest Close
RESPONSE_OBJ*   pSt_ClySam_DigestClose(e_7816_DEVICE ReaderId,            // [IN] SAM reader id 
	CERTIFIACTE4 certif4Out);                                   // [OUT] sam output certification 

//External Aut
RESPONSE_OBJ*   pSt_ClySam_ExternalAut(e_7816_DEVICE ReaderId,            // [IN] SAM reader id 
	CERTIFIACTE4 certif4In);                                    // [IN] Card certification to check

//Read Data - this command read all sam file / record except for Key Params files
RESPONSE_OBJ*   pSt_ClySam_ReadData(e_7816_DEVICE ReaderId,               // [IN] SAM reader id 
	e_clySam_DataType FileOrRec,                                // [IN] type of data - file or record
	const Union_clySam_ReadDataType * ReadDataIn,               // [IN] type of file / record to read
	St_clySam_ReadDataResult *ReadResult);                      // [out] read result

//Read Key Params
RESPONSE_OBJ*   pSt_ClySam_ReadKeyParams(e_7816_DEVICE ReaderId,          // [IN] SAM reader id 
	e_clySam_KeyParamsType ParamsType,                          // [IN] the Key Params Type
	e_clySam_KeyAccessType KeyAccessType,                       // [IN] choose access type ( index \ KIF ) for lock file this parameter is not relevat 
	const Union_clySam_KeyInfo * KeyInfoIn,                     // [IN] Key  index \ KIF - this parameter is not relevat for the lock file  
	St_clySam_ReadKeyResult *ReadResult);                       // [out] read result
RESPONSE_OBJ*   pSt_ClySam_Unlock (e_7816_DEVICE ReaderId, UULOCK_PIN16 Pin);

//give random
RESPONSE_OBJ*   pSt_ClySam_GiveRandom (e_7816_DEVICE ReaderId,            // [IN] SAM reader id
	RANDOM8 rand8); // [IN] random

//get response
RESPONSE_OBJ*   pSt_ClySam_GetResponse (e_7816_DEVICE ReaderId,           // [IN] SAM reader id
	int Len);                                                   // [IN] response length request

//this command updates the Parameters file record, an Event Ceilings 
//file record or on Event Ceiling
RESPONSE_OBJ*   pSt_ClySam_WriteData (e_7816_DEVICE ReaderId,             // [IN] SAM reader id
	e_clySam_WriteDataMode WriteMode,                           //dynamic or static mode
	e_clySam_WriteDataSamP2 e_WriteDataSamP2,                   // [IN] enum to fill P2
	clySam_BYTE* WriteData);                                    // [IN] write data 48 bytes

//this command generates the signed/ciphered data to send to a card
RESPONSE_OBJ*   pSt_ClySam_CipherCardData (e_7816_DEVICE ReaderId,        // [IN] SAM reader id
	st_CipherDataCard s_CipherDataCard,                         // [IN] all information for command (index(KIF || KVC), type of command, key or pin
	unsigned char* DataOut);                                    // [OUT] Data out (8, 16, 24 or 32 bytes)

//this command generates the data to send to another SAM with the commands WRITE DATA and WRITE KEY
RESPONSE_OBJ*   pSt_ClySam_CipherSamData (e_7816_DEVICE ReaderId,         // [IN] SAM reader id
	st_clySam_CipherDataSam s_CipherDataSam,                    // [IN] struct to fill command parameters (PACKET_7816)
	clySam_BYTE* Data2Write);                                   // [OUT] 6 blocks of 8 bytes

//this command increases the value of a given Value Counter by any amount
RESPONSE_OBJ*   pSt_ClySam_Increase (e_7816_DEVICE ReaderId,              // [IN] SAM reader id
	unsigned int i_ValueCountNumber,                            // [IN] Value Counter Number from 1 to 7
	AMOUNT4 Amount,                                             // [IN] amount
	SUM_VALUE4 SumValue,                                        // [OUT] new value of the VALUE COUNTER
	INCREASE_NUMBER3 IncreaseNumber);                           // [OUT] new number of times the VALUE COUNTER has been increased

RESPONSE_OBJ*   pSt_ClySam_Compute (e_7816_DEVICE ReaderId,               // [IN] SAM reader id
	e_clySam_KeyAccessType KeyAccessType,                       // [IN] choose access type -  index \ KIF+KVC  are only available  
	Union_clySam_WorKeyParamsAcess KeyAccess,                   // [IN]
	SN8 SerialNum,                                              // [IN] serial number
	unsigned char* Data4Cert,                                   // [IN] data to certify
	int CertSizeIn,                                             // [IN] size of data 
	int CertDataOutLen,                                         // [IN] certificate size out
	unsigned char* CertOut);                                    // [OUT] certificate out

RESPONSE_OBJ*   pSt_ClySam_InternalAut (e_7816_DEVICE ReaderId,           // [IN] SAM reader id
	e_clySam_KeyAccessType KeyAccessType,                       // [IN] choose access type -  index \ KIF+KVC  are only available  
	Union_clySam_KeyInfo* KeyInfoIn,                            // [IN] Key  index \ KIF - this parameter is not relevat for the lock file  
	clySam_BYTE* uc_DataIn,                                     // [IN]the data send to the sam
	unsigned int i_DataLenIn,                                   // [IN]Len of incoming data 
	clySam_BYTE* uc_DataOut,                                    // [OUT] cryptogram
	unsigned int* i_DataLenOut);                                // [OUT] cryptogram len

//this command ciphers arbitrary message made of 8 bytes data block
RESPONSE_OBJ*   pSt_ClySam_CipherData (e_7816_DEVICE ReaderId,            // [IN] SAM reader id
	St_clySam_KIF_And_KVC KifAndKvc,                            // [IN] KIF and KVC
	clySam_BYTE* PlainData,                                     // [IN] Plain data
	unsigned int i_DataLenIn,                                   // [IN] Plain data length
	clySam_BYTE* DataOut,                                       // [OUT] data out
	unsigned int* i_DataLenOut);                                // [OUT] out data length

//this command deciphers arbitrary message made of 8 bytes data block
RESPONSE_OBJ*   pSt_ClySam_DecipherData (e_7816_DEVICE ReaderId,          // [IN] SAM reader id
	St_clySam_KIF_And_KVC KifAndKvc,                            // [IN] KIF and KVC
	clySam_BYTE* DataIn,                                        // [IN] data in
	unsigned int i_DataLenIn,                                   // [IN] data in length
	clySam_BYTE* DataOut,                                       // [OUT] data out
	unsigned int* i_DataLenOut);                                // [OUT] out data length


#ifdef win_or_linux
#pragma pack(pop, CLY_SAM_OS) // correct way to align
#endif


#endif
