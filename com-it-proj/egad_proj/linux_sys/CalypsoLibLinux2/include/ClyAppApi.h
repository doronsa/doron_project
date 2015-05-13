
#ifndef _CLYAPP_H_
#define _CLYAPP_H_
#include<os_def.h>
#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif
#ifdef win_or_linux
  #define TR_PACK pck
//  #ifndef TR_PACK_PREFIX
  #define TR_PACK_PREFIX
 // #endif
  #pragma pack(push, ClyAppApi, 1) // correct way to align
  //define new pack
//  #pragma pack (1)//Specifies packing alignment = 1-byte boundaries.
#endif

#ifdef INCLUDE_KEIL
  // #define TR_PACK __attribute__ ((aligned (1),packed))
  #define TR_PACK_PREFIX __packed
#endif

 
#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif
#ifdef win_or_linux
  #pragma pack(pop, ClyAppApi)
#endif


#define TIM7020

///////////////////////////////////////////////////////////
// Constants and Types
//////////////////////////////////////////////////////////
#define TR_MAX_CONTRACTS_COUNT              8
#define TR_MAX_EVENT_COUNT                  6
#define RESTRICT_DURATION_END_OF_DAY        9998 //this value of remainig minutes in maavar/continue means "until end of day"
#define RESTRICT_DURATION_END_OF_SERVICE    9999 //this value of remainig minutes in maavar/continue means "until end of service"
#define RESTRICT_CODE_5_MINUTE_RESOLUTION   0x10 //if restrictcode==0x10 and interchange then resolution is in 5 minutes instead of 30 minutes
#define TR_VIRTUAL_CARD_VALID_FLAG          0xa5

#define MAX_ACTION_COUNT       6


#define TR_FALSE 0
#define TR_TRUE  1
typedef char TR_BOOL;
typedef unsigned short  TR_USHORT;
typedef unsigned long   TR_ULONG;
typedef unsigned char   TR_BYTE;
typedef unsigned char   TR_CHLNG[8];
///////////////////////////////////////////////////////////
// Calypso Codes
///////////////////////////////////////////////////////////
typedef enum
{
	// these errors are sent by stRTCmd.app_res of protocol.h
	// card errors
	e_ClyApp_Ok = 1,
	e_ClyApp_WrongParamErr=2,             //Parameter Error   2
	e_ClyApp_InterfaceNotInitErr=3,       //Interface NotInit Error   3
	e_ClyApp_WrongPasswordErr=4,          //SAM Wrong Password Error   4
	e_ClyApp_WrongSamFamilyErr=5,         //Wrong Sam Error - real or test SAM   5
	e_ClyApp_WrongSamTypeErr=6,           //Wrong Sam Type Err`or - CL/SL/CPP/CP  6
	e_ClyApp_SamRemovedErr=7,             //Sam Removed Error - for example if SAM has been reset   7
	e_ClyApp_SamNotLoginErr=8,            //Sam Not Login Error - SAM is locked                   8
	e_ClyApp_ResetErr=9,                  //Card / SAM Reset Error            9
	e_ClyApp_UnknownErr=10,                //Unknown Error                      10
	e_ClyApp_NoCardErr=11,                 //No Card foud in the reader or card not reset Error    11
	e_ClyApp_WrongCardTypeErr=12,          //Wrong Card Type Err                                     12
	e_ClyApp_ReaderErr=13,                 //Reader Error - contact/contactless                        13
	e_ClyApp_ReaderSamErr=14,              //Reader SAM Error                                            14
	e_ClyApp_ReaderCardErr=15,             //Reader Issued Error contact / contactless                 15
	e_ClyApp_CardLockedErr=16,             //Card Locked Error - black list for example                 16
	e_ClyApp_CardSecurityErr=17,           //Card Security Error - key/session problem                 17
	e_ClyApp_DataSecurityErr=18,           //Data Security Error - Data signature / CRC / LRC error          18
	e_ClyApp_CardErr=19,                   //Card Error - Unknown Err                                          19
	e_ClyApp_CardNotIssuedErr=20,          //Card Not Issued Error - when expecting an issued card              20
	e_ClyApp_CardNotEmptyErr=21,           //Card Not Empty Error - when expecting a manufacturer mode card   21
	e_ClyApp_CardWriteErr=22,              //Card Write Error                                                   22
	e_ClyApp_CardReadErr=23,               //Card Read Error                                                    23
	e_ClyApp_CardEnvEndDateErr=24,         //Expiration date of the ticketing application                       24
	e_ClyApp_CardEnvErr=25,                //Environment file data is not supporterd                            25
	e_ClyApp_CardContractErr=26,           //Contract file data is not supporterd                               26
	e_ClyApp_CardEventErr=27,              //Event file data is not supporterd                                  27
	e_ClyApp_CardContractLRCErr=28,        //Contract Security Err - 0 bit count error ( checksum err)          28
	e_ClyApp_NoContractSelectedErr=29,     //No Contract was Selected using e_ClyApp_IsValidContractExist Err - must be called before use  29
	e_ClyApp_NotEnoughRightsForUseErr=30,  //Not Enough Rights For Use Err - for example when trying to use 2 tokens when there is only 1 left 30
	e_ClyApp_SessionNotOpenErr=31,         //before some operations e_ClyApp_IsValidCardEnvironment must be called to open session   31
	e_ClyApp_CardProfileErr=32,            //The contract profile must exist in ENV && contract end date can not exceed the end date of the profile  32
	e_ClyApp_CanNotCancleErr=33,           //wrong card inserted for cancel
	e_ClyApp_NoValidContractErr=34,        //Environment file data is not supporterd
	e_ClyApp_SCReplacedInReader=35,        //SC was replaced with another card in reader
	e_ClyApp_TicketMaxReloadCounterErr=36, //Expiration date of the ticketing application
	e_ClyApp_TicketBitLockErr=37,          //Ticket Bit Lock Err - when trying to update a locked area
	e_ClyApp_MemErr=38,
	e_ClyApp_TicketKeyVerErr=39,           //Ticket Key Ver Err
	e_ClyApp_DepositRefundLockErr=40,      //Ticket Key Ver Err 40
	e_ClyApp_CounterCeilingErr=41,         //Ticket Key Ver Err
	e_ClyApp_CancelFailed=42,              //failed in the cancel operation itself (writing)
	e_ClyApp_CancelNotPossible=43,         //cancel not possible
	e_ClyApp_CanNotLoadOverWriteSameContract=44,//there is a valid  Identical monthly contract with the same code
	e_ClyApp_CanNotLoadThisContractOnUnanimous=45,//CanNot Load This Contract On Unanimous
	e_ClyApp_SamTwSetDateFail=46,  ///   update date on sam tw fail
	e_ClyApp_DateOverLimitErr=47,  ///   update date on sam tw fail
	e_ClyApp_CreateSignFailDateReason=48,// cannot create a Signature because date is an ol date
	e_ClyApp_CreateSignFailOtherReason=49,// cannot create a Signature other reason
	e_ClyApp_CardSamKeysDifferent=50, // card and sam keys don't match    50
	e_ClyApp_RecordNotFoundErr=51, // Record not found
	e_ClyApp_MoreThanOneStoredValueAlready=52,  //2 or more stored value contracts already exist on card
	e_ClyApp_StoredValueOverflow=53,      //can't load, will pass ceiling
	e_ClyApp_StoredValueIllegalLoadProfile=54,  //can't load, contract profile is invalid
	e_ClyApp_StoredValueNotEnoughCreditForUSe=55, //can't use, insufficient credit
	e_ClyApp_CardContractListLRCErr=56, //contract list Authenticator error
	e_ClyApp_CardIsFull=57, //no free rec
	e_ClyApp_IllegalReuseInStation=58, //can't use again in same station
    e_ClyApp_FailedOnCloseSession=59,
	e_ClyApp_ComError=99,//this is a special error that means that there was a com problem with reader/host
	e_ClyApp_InvalidHostType=100, //Not a Calypso. There is not other valid way to inform a caller about specific error
	e_ClyApp_InvalidArg=101,
	e_ClyApp_InvalidCommand=102,    // The host could not handle the command
	e_ClyApp_NotOk,
	e_ClyApp_TR1020m_v_SetParameters_Failed,
	e_ClyApp_TR1020m_InitReaderComInterface_Required,
	e_ClyApp_TR1020m_NotImplemented,
	e_ClyApp_TR1020m_NotConnectedOrUnknownOrFailed,
	e_ClyApp_TR1020m_ProtocolStart_Failed,
	e_ClyApp_TR1020m_IsCardInIgnoredBecauseOfSensorDoesNotDetectCard,
	e_ClyApp_TR1020m_ForgetCardIgnoredBecauseOfSensor_CardIsStillInside,
	e_ClyApp_LoadResultUnKnownCardOut
} eCalypsoErr;



///////////////////////////////////////////////////////////
// Validity Status
///////////////////////////////////////////////////////////
typedef enum
{
	e_Undefined,
	e_ValidForUse=10,//start from 10 not to confuse with e_clyApp_ValidityType
	e_ValidButCantBeUsed,//for example daily before morning hour
	e_Future,//for example future periodic		
	e_Invalid,//for example no tokens, passed month

}eValidityStatus;

///////////////////////////////////////////////////////////
// ETT
///////////////////////////////////////////////////////////
typedef enum
{
	e_Ett_Unknown=0,
	e_EttSingle=10,
	e_EttHalosh=11,
	e_EttMlt2=12,
	e_EttMlt5=13,
	e_EttMlt10=14,
	e_EttMlt11=15,
	e_EttMlt15=16,
	e_EttMlt20=17,
	e_EttMlt4=70,
	e_EttMlt6=71,
	 e_EttMltAbove20=73,
	e_EttMonthly=20,
	e_EttWeekly=21,
	e_Ettdaily=22,
	e_EttSemesterB=23, 
	e_EttSemesterA=24, 
	e_EttYearly=25,
	e_EttHour=26,

	e_EttPass=30,

	e_EttSoldjerVoutcher=40,
	e_EttPectialTrip=41,

	e_EttStoreValue1_30=60,
	e_EttStoreValue2_50=61,
	e_EttStoreValue3_100=62,
	e_EttStoreValue4_150=63,
	e_EttStoreValue5_200=64,

	e_EttStoreValue6_Special=65,
	e_EttStoreValue7_Temp=66,
	e_EttStoreValue8=67,	

	e_EttHemshechDiscount=75,
	e_EttHemshechSpectial=76,
}e_EttType;

///////////////////////////////////////////////////////////
// Spatial Type
///////////////////////////////////////////////////////////
typedef enum
{
    e_CardSpatialTypeZones,
    e_CardSpatialTypeFareCode,
    e_CardSpatialTypeLinesList,
    e_CardSpatialTypeRide,
    e_CardSpatialTypeRideAndRunType,
    e_CardSpatialTypeRideAndRunID,
    e_CardSpatialTypeRideRunAndSeat,
    e_CardSpatialTypeRideZones,
    e_CardSpatialTypeParking,
    e_CardSpatialTypePredefinedContract,
	e_CardSpatialTypeRouteSystemsList,
	e_CardSpatialTypeFareCodeExtension,
    e_3CardSpatialTypeRFUOfUnknownFormat,
    e_4CardSpatialTypeRFUOfUnknownFormat,
    e_CardSpatialTypeRFUOfKnownSize,
    e_CardSpatialTypeEndLocationList    //  e_CardSpatialTypeEndOfLocationList
}e_ClyApp_CardSpatialType;

///////////////////////////////////////////////////////////
// Device Type
///////////////////////////////////////////////////////////
/*
typedef enum 
{
    e_7816_CONTACTLESS,
    e_7816_SAM_TW,
    e_7816_SAM_CAL, 
    e_7816_NONE,

	// Must be last
	e_7816_LAST
}e_7816_DEVICE;
*/

///////////////////////////////////////////////////////////
// Date and Time data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_DateTime
{
	short Year;
	short Month;
	short Day;
	short Hour;
	short Minute;
	short Second;
};
typedef TR_PACK_PREFIX struct TAG_TR_St_DateTime TR_St_DateTime;

///////////////////////////////////////////////////////////
// Parameters
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_st_Parameters
{
	unsigned long   lv_DeviceNumber;
	unsigned char   uc_ProviderId;
	unsigned long   lv_StoredValueCeiling;//in agorot
	unsigned short	us_StoredValuePredefineCode;//predefine code for stored value for use
	unsigned short  us_MorningEndHour;//in minutes from 00:00 (for example 10:30 => 630)
	unsigned short  us_EveningStartHour;//in minutes from 00:00 (for example 18:00 => 1080)
	unsigned short	us_EndOfServiceHour;////in minutes from 00:00 (for example 18:00 => 1080)
	unsigned short	us_ReuseLimitInMinutes;//in minutes. interval for allowing card reuse in same station
	//unsigned short  us_BusLineNumber;      // MAKAT of MOT
//	unsigned short  us_Cluster;			   // Current Cluster
	unsigned short  us_StationNumber;      // MAKAT of MOT
	long	lv_time_zone_bias_minutes; //GMT offset by timezone in Minuts
};
typedef TR_PACK_PREFIX struct TAG_TR_st_Parameters TR_st_Parameters;
///////////////////////////////////////////////////////////
// Profile Information
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_stHolderFrofile
{
   unsigned char   uc_HolderProfCode; // profile code
   TR_St_DateTime  st_HolderProfDate; // profile validity end date
   TR_BOOL         IsValid; // the profile is valid
};
typedef TR_PACK_PREFIX struct TAG_TR_stHolderFrofile TR_stHolderFrofile;
///////////////////////////////////////////////////////////
// Card Information
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_st_CardInfo
{
	unsigned short m_cardType;//enm_TR_CardMediaType
	unsigned char  IsEnvOk;
	unsigned char  IsCardLock;
	unsigned char  m_serialNumber[20];//without null termination
	unsigned short PermissionType;//enm_TR_PermissionType
	unsigned char  IsLastEventOk; //NEW
};
typedef TR_PACK_PREFIX struct TAG_TR_st_CardInfo TR_st_CardInfo;

///////////////////////////////////////////////////////////
// Environment Data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_st_EnvironmentData
{
	unsigned char  AppVer;
	unsigned short sh_EnvCountryld;
	unsigned char  uc_Envlssuerld; //Ticketing application issuer identifier
	unsigned long  ul_EnvApplicationNumber; //Card Application Number
	TR_St_DateTime st_EnvlssuingDate; //Transport application issuing date (Number of days since January 1 st, 1997 (being date 0))
	TR_St_DateTime st_EnvEndDate; //Expiration date of the ticketing application (Number of days since January 1st, 1997 (being date 0))
	unsigned char  uc_EnvPayMethod; //Allowed contracts payment methods -can take the values PAY_METHOD_PRE_PAYMENT, PAY_METHOD_PRE_PAYMENT_ELECTRONIC,PAY_METHOD_POST_PAYMENT and OR operation between them if several pay method allowed
	TR_St_DateTime st_HolderBirthDate; //Holder birth date
	unsigned short sh_HolderCompany; //Short holder company identifier
	unsigned long  ul_HolderCompanylD; // Holder identifier, within company
	unsigned long  ul_HolderldNumber; // national (????? ????) holder  identifier
	TR_stHolderFrofile st_HoiderProf1; //First profile code and validity ending date
	TR_stHolderFrofile st_HoiderProf2; //second profile code and validity ending date
	unsigned char  CustomerLangUage;// 0 heberew 1 arabic 2 English 3 Russian
	unsigned char bIsAnonymous;
};
typedef TR_PACK_PREFIX struct TAG_TR_st_EnvironmentData TR_st_EnvironmentData;

///////////////////////////////////////////////////////////
// Contract Data for Report
///////////////////////////////////////////////////////////
typedef enum
{
	e_NoInterchange=0,
	e_OneHemshech=1,
	e_TwoHemshech=2,
	e_Maavar=9,
}e_InterchangeType;

typedef enum
{
	e_Calendar,
	e_Sliding,

	e_PeriodUndefined,
}e_PeriodStartType;

typedef enum
{
	e_DurationInMonths,
	e_DurationInWeeks,
	e_DurationInDays,
	e_DurationInHalfHours,

	e_DurationUndefined,
}e_ClyApp_DurationType;


TR_PACK_PREFIX struct TAG_TR_St_ContractReportData
{
	unsigned short  usValidityStatus;//eValidityStatus
	unsigned char	ucIndexOnCard;//0-7
	unsigned short  ucEtt;
	//unsigned char ucIsInterchangeContract;
	e_InterchangeType ucInterchangeType; 
	unsigned char	uc_ContractProvider; //Provider identifier
	TR_St_DateTime  st_ContractValidityStartDate; //Start date of the contract.
	TR_St_DateTime  st_ContractValidityEndDate; //All 0's if doesn't exist
	TR_St_DateTime  st_ContractSaleDate;    
	unsigned short  us_SaleNumberDaily;     
	unsigned char   uc_Restrict_Code;        
	unsigned char   uc_Rrestrict_Duration;   
	unsigned char   uc_RestrictTime;         
	e_PeriodStartType  usPeriodStartType; 
	e_ClyApp_DurationType  usDurationUnitsType; 
	unsigned short  usDurationCount;
	unsigned char	uc_ContractCustomerProfile; //Social profile giving predefined transportation rights
	unsigned long   ulCounter;
	unsigned char	ucSpatialType;
	unsigned short  usCluster;
	unsigned short  sFareCode;
	unsigned short  sZoneBitmap;
	unsigned short  sPredefinedCode;
	unsigned short  usWhiteListId;
	unsigned short  usDeviceNumber;
};
typedef TR_PACK_PREFIX struct TAG_TR_St_ContractReportData TR_St_ContractReportData;

///////////////////////////////////////////////////////////
// All Contracts Data for Report
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_st_AllContracts
{
	TR_BOOL m_FlagIsLock;
	TR_BOOL m_FlagIsEnvaromentOk;
	unsigned char ContractsCount;
	TR_St_ContractReportData Contracts[8] ;// the reader may send a Several contracts 0 -> (8 - MAX_CONTRACTS_COUNT

};
typedef TR_PACK_PREFIX struct TAG_TR_st_AllContracts TR_st_AllContracts;

///////////////////////////////////////////////////////////
// Contract data for use
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_ContractForUse
{
	unsigned char	ucIndexOnCard;//0-7
	unsigned char	ucBCPL_Val;
	unsigned short  ucEtt;
	e_InterchangeType	ucInterchangeType; 
	unsigned char	uc_ContractProvider; //Provider identifier
	TR_St_DateTime  st_ContractValidityStartDate; //Start date of the contract.
	TR_St_DateTime  st_ContractValidityEndDate; //All 0's if doesn't exist
	e_PeriodStartType  usPeriodStartType; 
	e_ClyApp_DurationType  usDurationUnitsType;
	unsigned short  usDurationCount;// if less than 9998 then remaning minutes for maavar else RESTRICT_DURATION_END_OF_DAY or RESTRICT_DURATION_END_OF_SERVICE
	unsigned char	uc_ContractCustomerProfile; //Social profile giving predefined transportation rights
	unsigned long   ulCounter;
	unsigned char	ucSpatialType;
	unsigned short  usCluster;
	unsigned char	FareCode;	//In spatialtype "fare" the code of the contract, in SV in maavar, the fare code of first trip
	unsigned short  sZoneBitmap;
	unsigned short  sPredefinedCode;
	TR_BOOL			bIsInterchangeValid;
	unsigned char	ucInterchangeRights;	//In case of kartisia maavar/SV, defines the azmash where 
	//first boarding was done. in case of kartisia hemshech defines the bitmap of trips that were done (i.e: 0x3 means 11b means trips 1 and 2)

	unsigned short  usInterchangeValidityMinutes;
	unsigned char		ucPsngrCount;//how many psngrs were on first trip (for kartisia/stored value)

	TR_St_DateTime  st_ContractSaleDate;    // 
	unsigned short  usDeviceNumber;         // 
	unsigned short  usSaleNumberDaily;      // 

//  new fields for egged  12/04/2015 
    TR_BYTE m_MaavarPrevProvider;//provider id in which prev use was done, 0 if first use of nikuv (hime 12/04/15)
    TR_USHORT m_MaavarPrevLine;//if prev use (same nikuv) is Egged, then this is the makat line of prev trip (hime 12/04/15)

};
typedef TR_PACK_PREFIX struct TAG_TR_St_ContractForUse TR_St_ContractForUse;

///////////////////////////////////////////////////////////
// Response of Request Contract data for use
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_st_ContractsForUseResponse
{
	TR_BOOL m_FlagIsLock;
	TR_BOOL m_FlagIsEnvironmentOk;
	unsigned char ContractsCount;
	TR_St_ContractForUse Contracts[8] ;// the reader may send several contracts 0 -> 8 - MAX_CONTRACTS_COUNT
};
typedef TR_PACK_PREFIX struct TAG_TR_st_ContractsForUseResponse TR_st_ContractsForUseResponse;


///////////////////////////////////////////////////////////
// Multi-token interchange Contract Specific Data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_MaavarOrHemshechKartisiaLoadData //including single, maava/hemshech
{    		
	unsigned char bIsFiveMinutesResolution;//if false than half hour resolution
	unsigned char uc_RestrictDuration;
	//(hime) new fields for egged 12/04/2015
	//TR_BYTE m_NumSegmentsHemshech;//0 or 2 or 3 : for kartisia hemshech only
};
typedef TR_PACK_PREFIX struct TAG_MaavarOrHemshechKartisiaLoadData MaavarOrHemshechKartisiaLoadData;

///////////////////////////////////////////////////////////
// Periodical Contract Specific Data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_PeriodLoadData
{
	TR_St_DateTime  st_ContractValidityEndDate; //All 0's if doesn't exist
	unsigned short  usPeriodStartType; //e_PeriodStartType
	unsigned short  usDurationUnitsType;//e_DurationUnits
	unsigned short  usDurationCount;

};
typedef TR_PACK_PREFIX struct TAG_PeriodLoadData PeriodLoadData;

///////////////////////////////////////////////////////////
// Stored value Contract Specific Data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_StoredValueLoadData
{
	unsigned char bIsFiveMinutesResolution;//if false than half hour resolution
	unsigned char uc_RestrictDuration;
	unsigned long ul_StoredValueSumToLoadAgorot;
};
typedef TR_PACK_PREFIX struct TAG_StoredValueLoadData StoredValueLoadData;

///////////////////////////////////////////////////////////
// Contract Specific Data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX union TAG_unionSpecificLoadData
{
	MaavarOrHemshechKartisiaLoadData  ov_MaavarKartisiaLoadData;
	PeriodLoadData					ov_PeriodLoadData;
	StoredValueLoadData			ov_StoredValueLoadData;
};
typedef TR_PACK_PREFIX union TAG_unionSpecificLoadData unionSpecificLoadData;

///////////////////////////////////////////////////////////
// Contract data for load
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_LoadContract
{
	unsigned short  usSaleNumberDaily;

	unsigned char	ucEtt;
	unsigned char	ucInterchangeType;//e_InterchangeType
	unsigned char	ucRestrictTimeCode;
	unsigned short  usWhiteListId;//0 unless loading from white list. this field indicates whether to update contractlist on card
	unsigned char	uc_ContractCustomerProfile; //Social profile giving predefined transportation rights
	unsigned char	ucSpatialType;
	unsigned short  usCluster;
	unsigned short  sFareCode;
	unsigned short  sZoneBitmap;
	unsigned short  sPredefinedCode;
	TR_St_DateTime  st_ContractValidityStartDate; //Start date of the contract. set 0 if sliding.
	unionSpecificLoadData ov_SpecificLoadData;//different for each app type (kartisia/period...)
	TR_St_DateTime  st_ActionTime;//the time came from host and have to be writen in the contract in the card
	//unsigned char	IsRestore;                   // Is this a restore oprtaion ( 1= true, 0 = false)  - Personalization - 
	// 12/4/2015 rename  IsRestore to LoadOperationType feild
	unsigned char	ucLoadOperationType;     // 0 - reguler load 1 - restore (for personalization) 2 - load & automat use (tim some in some case )
	unsigned long	RetoreKartisiyaUnits;       

};
typedef TR_PACK_PREFIX struct TAG_TR_St_LoadContract TR_St_LoadContract;



///////////////////////////////////////////////////////////
// Card cancel data
///////////////////////////////////////////////////////////
#define BIN_DATA_OF_LOAD 1
#define BIN_DATA_OF_USE  2

TR_PACK_PREFIX struct TAG_st_ClyApp_CardCancelData
{
	unsigned char uc_ContractRecNumBefore;        	// Contract Record number used/loaded
	unsigned char ucp_ContractDataBeforeAction[29]; // Contract Data before Action (the contract we possibly changed)
	unsigned char ucp_CounterDataBeforeAction[29];  // Counter Data before Action  (the counter we possibly changed)
	unsigned char ucp_Event1BeforeAction[29];       // event1 before Action
	unsigned char uc_ContractRecNumAfter;        	// same as uc_ContractRecNumBefore except in case of SV reload
	unsigned char Action;							// 1=LOAD 2=USE
};
typedef TR_PACK_PREFIX struct TAG_st_ClyApp_CardCancelData st_ClyApp_CardCancelData;

///////////////////////////////////////////////////////////
// Cancel Data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX union TAG_TR_St_CancelData
{
	unsigned char ucp_BinTktDataBeforUse[32]; 	// ticket binary data
	st_ClyApp_CardCancelData st_CardCancelData;
};
typedef TR_PACK_PREFIX union TAG_TR_St_CancelData TR_St_CancelData;

///////////////////////////////////////////////////////////
// Is Possible Cancel Data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_IsPossibleCancel
{
	unsigned char m_serialNumber[20];//without null termination
	unsigned char ContractNum;// 1 - 8 
};
typedef struct TAG_IsPossibleCancel IsPossibleCancel;

///////////////////////////////////////////////////////////
// Cancel Contract Data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_CancelContract
{
    unsigned char m_serialNumber[20];//without null termination
    unsigned char uc_RecNum;//1-8
};
typedef struct TAG_TR_St_CancelContract TR_St_CancelContract;


///////////////////////////////////////////////////////////
// Transaction Data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_TransactionData
{
	//todo union for card ticket
	char Contract[29];
	char Env[29];
	char Event1[29];
	char Event2[29];
	char Event3[29];
	char cCounterStr[8];	//the numeric value of the counter of the contract, in string format without null termination, padded with 0's on left
	char cAuthorizationCodeStr[5];//the authorixation code of the contract, in string format without null termination, padded with 0's on left		
};
typedef TR_PACK_PREFIX struct TAG_TR_St_TransactionData TR_St_TransactionData;

///////////////////////////////////////////////////////////
// Response of contract loading
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_LoadContractResponse
{
	unsigned char		uc_IsStoredValueCacelDataExist;  // 0=not exist, else exist
	TR_St_CancelData    CancelData;
	TR_St_TransactionData TransactionData;	
	TR_St_TransactionData StoredValueCancelTransactionData;

};
typedef TR_PACK_PREFIX struct TAG_TR_St_LoadContractResponse TR_St_LoadContractResponse;

///////////////////////////////////////////////////////////
// Contract Data for Use
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_UseContractData
{
	unsigned char	m_Contract_index_on_the_card;
	unsigned short  m_PassengerCount;
	//unsigned short  m_ClusterNumber;//current cluster
	unsigned short  m_Code;//fare code if validation
	unsigned long   m_StoredValueSum;//sum for validation in stored value, oherwise 0
	unsigned long   m_TokensCounter;//set to 1 if using kartisia, else 0 
	unsigned long   m_StationNumber;//MOT station number
//	unsigned long   m_LineNumber;
	TR_BOOL         m_IsFirstTrip;//set to false if interchange trip
	unsigned char	m_SegmentForHemshech;//segment for use in benironi only. 1,2 or 3		
	unsigned char	m_CurrAzmash; // Interchange code for first boarding

};
typedef TR_PACK_PREFIX struct TAG_TR_St_UseContractData TR_St_UseContractData, TR_St_IsPossibleUseContract;

///////////////////////////////////////////////////////////
// Response Data for Contract Use
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_UseContractResponse
{
	TR_St_CancelData    CancelData;
	TR_St_TransactionData TransactionData;
};
typedef TR_PACK_PREFIX struct TAG_TR_St_UseContractResponse TR_St_UseContractResponse;

///////////////////////////////////////////////////////////
// Response Data of Card lock
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_LockResponse
{
	TR_St_TransactionData TransactionData;
};
typedef TR_PACK_PREFIX struct TAG_TR_St_LockResponse TR_St_LockResponse;

///////////////////////////////////////////////////////////
// White contracts list
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_WhiteContractList
{
	unsigned short Count;
	unsigned short Codes[8];
};
typedef TR_PACK_PREFIX struct TAG_TR_St_WhiteContractList TR_St_WhiteContractList;

///////////////////////////////////////////////////////////
// Inspector data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_st_UserData
{
	unsigned long  lv_UserId;
	//unsigned short us_MainCompany;
	unsigned short iv_UserType;
};
typedef TR_PACK_PREFIX struct TAG_TR_st_UserData TR_st_UserData;



///////////////////////////////////////////////////////////
// Event data for Report
///////////////////////////////////////////////////////////
#define MAX_EVENT_FOR_REPORT (6)

typedef enum 
{
	e_ClyApp_EventTypeUse,
	e_ClyApp_EventTypeLoading,
	e_ClyApp_EventTypeCancel,

	e_ClyApp_EventTypeUnknown
}e_ClyApp_EventType;

//TR_PACK_PREFIX struct TAG_TR_St_EventForReport
TR_PACK_PREFIX  struct TAG_TR_St_EventForReport
{
	unsigned char		isValidEvent;             // 0 = invlid, 1 = valid
	unsigned char	    EventType;		  	      // see e_ClyApp_EventType
	unsigned char		EventServiceProvider;     // the  provider id
	unsigned char		EventNumOfPassanges;      // the passnger count 
	TR_St_DateTime	DateTime;
	// hime -> new fields for egged  12/04/2015 
    //TR_BYTE   		TicketType;                  
	TR_BYTE   		IsInterchange;               // 1 intechange 0 first use
	TR_USHORT       Shilut;                      // the line number 
    TR_USHORT   	PredefinedCode;              // the predefine code
	TR_BYTE 		ETT;                             
	TR_BYTE 		RestrictCode;                // 
	TR_BYTE 		StartType;
	TR_BYTE 		ContinueFlag;
    TR_BYTE         EventTicketFareCode;        //  
	TR_BYTE         InterchangeType; // e_InterchangeType
};

typedef TR_PACK_PREFIX struct TAG_TR_St_EventForReport TR_St_EventForReport;

TR_PACK_PREFIX struct TAG_St_GetActionReportRespond
{
    TR_BYTE m_FlagIsLock;
    TR_BYTE m_FlagIsEnvaromentOk;
    TR_BYTE m_ActionCount;// 0 -> 6
    TR_St_EventForReport m_Actions[MAX_ACTION_COUNT];// the reader may sent a count of actions 0 -> 6 
};

typedef TR_PACK_PREFIX struct TAG_St_GetActionReportRespond St_GetActionReportRespond;

///////////////////////////////////////////////////////////
// SAM data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_SamData
{
	unsigned long SamCounter;//the actual value doesn't exceed 2^24 
	unsigned long SamNumber;//serial number
	unsigned long SamType;//e_ClyApp_SamCL/e_ClyApp_SamCV...

	// new fields for egged  12/04/2015 
	unsigned long SamEggedSerialNumber;// the serial number of sam  egged , if  zero -> sam egged not exist (hime  12/04/15)

};
typedef TR_PACK_PREFIX struct TAG_TR_St_SamData TR_St_SamData;


///////////////////////////////////////////////////////////
// Card binary data
///////////////////////////////////////////////////////////
#define BIN_EVENTS_COUNT             (6)
#define BIN_SPECIAL_EVENTS_COUNT     (4)
#define BIN_CONTRACTS_COUNT          (8)


TR_PACK_PREFIX struct TAG_TR_CardBinData
{
	unsigned char Env[29];				// Environment 
	unsigned char Contracts[BIN_CONTRACTS_COUNT][29];		// Contract 
	unsigned char Counters[29];		    // Counter of the contract
	unsigned char Events[BIN_EVENTS_COUNT][29];			// Events 
	unsigned char SpecialEvents[BIN_SPECIAL_EVENTS_COUNT][29];	// Special Events 
	unsigned char ContractList[29];		// Contract List
};
typedef TR_PACK_PREFIX struct TAG_TR_CardBinData TR_CardBinData;


//(hime) new struct for egged 12/04/2015

TR_PACK_PREFIX struct TAG_TR_TrnsactionQuery
{
	unsigned char ucOperation; // 0 load 1 use 2 cancel 
	unsigned long SmartCardSerialNumber;
	unsigned short DayliNumber;// when ucOperation=2 
};

typedef TR_PACK_PREFIX struct TAG_TR_TrnsactionQuery TR_TrnsactionQuery;


///////////////////////////////////////////<ChngTripcmd>////////////////////////////////////////
//(hime) new struct for egged 12/04/2015
TR_PACK_PREFIX struct TAG_St_ChngTripcmd
{
    short shilut ;//14 bit: 10 bit for number, 14th bit for 'alef'
    TR_USHORT	 m_Clustrer;// m_EventTicketRouteSystem;
	TR_USHORT	 m_MotLine;//0/2013   	
};
typedef TR_PACK_PREFIX struct TAG_St_ChngTripcmd St_ChngTripcmd;

//(hime) new struct for egged 12/04/2015
// test reader request 
TR_PACK_PREFIX struct TAG_St_TestReaderRequest
{
	unsigned char uc_ReaderSelect;// 0 - coustomer reader(the main reader) 1 - driver reader 
	unsigned long TimeOutWaitCardMS;  // test wait until timeout or card in 
	unsigned char IsTestLed;// if true the reader  turn the leds on and of (blinking)
	unsigned long Time2SetOneLedMs;// time to set each led
	unsigned long Time2ClearOneLedMS;// time to clear each led
	unsigned char IsTestSam; // if 1 test write and read if 0 test read

};
typedef TR_PACK_PREFIX struct TAG_St_TestReaderRequest St_TestReaderRequest;

//(hime) new struct for egged 12/04/2015
// test reader result 
TR_PACK_PREFIX struct TAG_St_TestReaderResults
{
	unsigned char SamResult;//1- ok 0 -fail  FF - not tested
	unsigned char AntenaResult;//1 ok 0 fail    
};
typedef TR_PACK_PREFIX struct TAG_St_TestReaderResults St_TestReaderResults;




///////////////////////////////////////////////////////////////
// Name: GetTimeAndDateCallBack
// DESCRIPTION: Callback function for getting the current time and date
// PARAMETERS:  [IN/OUT] TR_St_DateTime - pointer to Time and date structure
///////////////////////////////////////////////////////////////
typedef TR_BOOL (*GetTimeAndDateCallBack)(TR_St_DateTime* trDt);

///////////////////////////////////////////////////////////////
// Name: ReaderExchangeDataCallBack
// DESCRIPTION: Protocol Callback function for sending and receiving data from reader
// RETURNS: 0- Seccsues 1-Error
///////////////////////////////////////////////////////////////
typedef int (*ReaderExchangeDataCallBack)(void* pHandler,						//[IN] pointer to protocol handler
									int  i_cmd, 									//[IN] the command
                                    int  i_TimeOutMs,								//[IN] time out in [ms]
                                    int  i_ObjectInSize,							//[IN] the data size to send
                                    void *p_ObjectIn, 							//[IN] pointer to the data 
                                    int  i_ObjectOutSizeReq, 						//[IN] the data respond size except to 
                                    void *p_ObjectOut,							//[OUT] pointer ot the data return 
									int  *p_OutSizeArive,							//[IN] the size of data respond
									unsigned short  ApplicationStatusBits,		//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
                                    int  *p_ApplicationError);						//[OUT] the application return code: 1=ok, 0=error
 ///////////////////////////////////////////////////////////////
// Initialization data structure
///////////////////////////////////////////////////////////////
typedef struct 
{
	//Comm Protocol
	void						*pHandler;		// pointer to protocol handler
	ReaderExchangeDataCallBack	ProtocolCB;				// pointer ReaderExchangeDataCallBack function
	//Time and Date
	GetTimeAndDateCallBack		TimeAndDateCB;			//	pointer GetTimeAndDateCallBack CB function
}
st_InitResource;

///////////////////////////////////////////////////////////
// API Functions 
///////////////////////////////////////////////////////////

 	///////////////////////////////////////////////////////////
    // NAME: TR_InitReader
	// DESCRIPTION: Inizialization of reader
	// PARAMETERS:  [IN] pointer to the initialization data structure 
	// RETURNS: eCalypsoErr result
 	///////////////////////////////////////////////////////////
	eCalypsoErr TR_InitReader(const st_InitResource* );
	
	///////////////////////////////////////////////////////////
	// NAME: TR_SetParam,
	// DESCRIPTION: sets parameters for reader (all or partial)
	// PARAMETERS:  [IN] pointer to the parameters data structure
	// PRE REQUIREMENT:  call to TR_InitReader
	// RETURNS: eCalypsoErr result
	///////////////////////////////////////////////////////////
	eCalypsoErr TR_SetParam(const TR_st_Parameters*);

	///////////////////////////////////////////////////////////
	// NAME: TR_SetNewBusLineParam,
	// DESCRIPTION: sets parameters on a new trip
	// PARAMETERS:  Line , Cluster, shilut 
	// PRE REQUIREMENT:  call to TR_InitReader
	// RETURNS: eCalypsoErr result
	///////////////////////////////////////////////////////////
	eCalypsoErr TR_SetNewBusLineParam(const St_ChngTripcmd *p_ChangeTripData);      // MAKAT of MOT

	///////////////////////////////////////////////////////////
	// NAME: TR_IsCardIn
	// DESCRIPTION: Checks if card is in 
	// PARAMETERS:  1- result, IN, OUT
	//              2- st_CardInfo, IN,OUT, contains information about read card
	// PRE REQUIREMENT:  call to TR_InitReader
	// RETURNS: eCalypsoErr result
	///////////////////////////////////////////////////////////
	eCalypsoErr TR_IsCardIn(TR_BOOL* cardIn, TR_st_CardInfo* pInfo);

    ///////////////////////////////////////////////////////////
	// NAME: TR_ForgetCard
	// DESCRIPTION: It finishes work with current card 
	// PARAMETERS: no
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event
	// RETURNS: eCalypsoErr result
	///////////////////////////////////////////////////////////
	eCalypsoErr TR_ForgetCard(void);

	///////////////////////////////////////////////////////////
	// NAME: TR_GetEnvironmentData
	// DESCRIPTION: returns environment data
	// PARAMETERS: st_EnvironmentData - IN, OUT
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_GetEnvironmentData(TR_st_EnvironmentData*);

    ///////////////////////////////////////////////////////////
	// NAME: TR_GetListForReportAndReload
	// DESCRIPTION: returns all contracts data for reload
	// PARAMETERS: st_Contracts - IN, OUT
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_GetListForReportAndReload(TR_st_AllContracts*);


    ///////////////////////////////////////////////////////////
	// NAME: TR_GetListForUse
	// DESCRIPTION: returns all contracts data for use
	// PARAMETERS: st_Contracts - IN, OUT
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_GetListForUse(TR_st_ContractsForUseResponse *ContractData);


	///////////////////////////////////////////////////////////
	// NAME: TR_IsFreeRecExistForLoad
	// DESCRIPTION: returns if free slot for loading exists 
	// PARAMETERS: none
	// PRE REQUIREMENT: cardin
	// RETURNS: e_ClyApp_Ok if exists, else e_ClyApp_CardIsFull
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_IsFreeRecExistForLoad(void);


	///////////////////////////////////////////////////////////
	// NAME: TR_IsPossibleLoad
	// DESCRIPTION: is load possible (call this before load)
	// PARAMETERS: 1 - St_LoadContractData - IN	//            
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_IsPossibleLoad(const TR_St_LoadContract* pLoadData);


	///////////////////////////////////////////////////////////
	// NAME: TR_Load
	// DESCRIPTION: loads contract
	// PARAMETERS: 1 - St_LoadContractData - IN
	//             2 - St_LoadContractResponse - IN,OUT  
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_Load(const TR_St_LoadContract*, TR_St_LoadContractResponse*);

	///////////////////////////////////////////////////////////
	// NAME: TR_Use
	// DESCRIPTION: performs using of contract
	// PARAMETERS: 1 - St_UseContractData - IN
	//             2 - St_UseContractResponse - IN,OUT  
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_Use(const TR_St_UseContractData*, TR_St_UseContractResponse*);


	///////////////////////////////////////////////////////////
	// NAME: TR_IsPossibleCancel
	// DESCRIPTION: checks that card is in and same 
	// PARAMETERS: 1 - IsPossibleCancel - IN
	// PRE REQUIREMENT: cardin
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_IsPossibleCancel(const IsPossibleCancel* pIsPossible);

	
	///////////////////////////////////////////////////////////
	// NAME: TR_CancelOp
	// DESCRIPTION: cancels load/use contract
	// PARAMETERS: 1 - TR_St_CancelData - IN
	//             2 -  - IN,OUT  
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event, Load
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_CancelOp(const TR_St_CancelData*, TR_St_TransactionData*);

	///////////////////////////////////////////////////////////
	// NAME: TR_LockCard
	// DESCRIPTION: locks card
	// PARAMETERS: out TR_St_LockResponse
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_LockCard(TR_St_LockResponse* pLockResponse);

	///////////////////////////////////////////////////////////
	// NAME: TR_GetWhiteContractList
	// DESCRIPTION: get numbers of ContractList on card (white list)
	// PARAMETERS: TR_St_WhiteContractList - OUT
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_GetWhiteContractList(TR_St_WhiteContractList* pStContracts);


    //NOT FOR REGULAR USER, ONLY FOR SPECIAL CARDS 
	///////////////////////////////////////////////////////////
	// NAME: TR_GetInspectorCardData
	// DESCRIPTION: Read the Inspector data from card
	// PARAMETERS: 1 - st_CardInfo - IN, OUT, data aboput user from card
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_GetInspectorCardData(TR_st_UserData* pUserData);

	///////////////////////////////////////////////////////////
	// NAME: TR_GetAllEventsForReport
	// DESCRIPTION: gets all events on card. 
	// PARAMETERS:  [IN/OUT] EventsArr - Array buffer for writing the output events
	//				[IN/OUT] EventNum  - Number of the event that ware wirtten to the EventsArr array
	// PRE REQUIREMENT:  card in
	// RETURNS: eCalypsoErr result
	///////////////////////////////////////////////////////////
	//eCalypsoErr TR_GetAllEventsForReport(TR_St_EventForReport EventsArr[MAX_EVENT_FOR_REPORT]);
	eCalypsoErr TR_GetAllEventsForReport(St_GetActionReportRespond *p_ActionOut);

	




    ///////////////////////////////////////////////////////////
	// NAME: `
	// DESCRIPTION: Get SAM data
	// PARAMETERS: [IN/OUT] pSamCounter - pointer to sam data
	// PRE REQUIREMENT:  to TR_InitReader,
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_GetSamData(TR_St_SamData* pSamCounter);


	///////////////////////////////////////////////////////////
	// NAME: TR_CheckCardReader
	// DESCRIPTION: Check the card reader by writing data and read it back 
	// PARAMETERS: 
	// PRE REQUIREMENT:  Call to TR_InitReader,
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_CheckCardReader(void);
	
	///////////////////////////////////////////////////////////
	// NAME: TR_ReadAllCardBinData
	// DESCRIPTION: Read all the card binary data 
	// PARAMETERS: [IN] pBuf - pointer to user buffer
	// PRE REQUIREMENT:  Call to TR_InitReader,
	// RETURNS: eCalypsoErr result
	/////////////////////////////////////////////////////////
	eCalypsoErr TR_ReadAllCardBinData(TR_CardBinData* pBuf);

	///////////////////////////////////////////////////////////
	// NAME: TR_VerifyTransaction
	// DESCRIPTION: verify if trnsaction ended and if it ended return the trnsaction data 
	// PARAMETERS: [IN] TR_TrnsactionQuey contain the operaration and the card serial number
	// PRE REQUIREMENT:  Call to TR_LOAD /TR_USE/TR_CancelOp
	// RETURNS: eCalypsoErr result  , error=e_ClyApp_LoadResultUnKnownCardOut  means that it is unknown if transaction succeeded or not 
	/////////////////////////////////////////////////////////
	
	eCalypsoErr TR_VerifyTransaction(TR_TrnsactionQuery *p_In,TR_St_LoadContractResponse *p_Respond);

	

	///////////////////////////////////////////////////////////
	// NAME: TestReaderRequest
	// DESCRIPTION: test reader antena & sam 
	// PARAMETERS: [IN] test parameter 
	// PRE REQUIREMENT:  Call to Call to TR_InitReader
	// RETURNS: eCalypsoErr
	/////////////////////////////////////////////////////////

	eCalypsoErr  TestReaderRequest(St_TestReaderRequest * p_TestReaderRequest, St_TestReaderResults *p_Result);
	

#endif 

