
#ifndef _CALYPSO_APP_H_
#define _CALYPSO_APP_H_


#include <ClyTktOs.h>
//#include <stdio.h>
#include <Iso7816.h>
#include <ClySamOs.h>
#include <ClyAppApi.h>
//#include <ClyAppTypes.h> //TBD:yoram
#include <ClyCrdOs.h>
#include <Bit2Byte.h>
#include<os_def.h>
#ifdef TR4010
     #define eCalypsoErr unsigned short
     #define e_EttType unsigned short
     typedef unsigned char UULOCK_PIN16[16];
#endif


// define debug for load contract fail
#ifdef CORE_DEBUG
 #ifdef ENABLE_DEBUG_LOAD
   #undef ENABLE_DEBUG_LOAD
 #endif
 #define ENABLE_DEBUG_LOAD 1
#endif	


#ifdef BUILD_ZABAD_TOOL
 #ifdef ENABLE_DEBUG_LOAD
   #undef ENABLE_DEBUG_LOAD
 #endif
 #define ENABLE_DEBUG_LOAD 1
#endif	

#define CONTRACT_VERSION_NUM 0 ///   moveving from c file
#define WORK_WITH_RATIFICATION 1

#define CLY_DEBUG_SHOW 0


#define CALYPSO_ADULT 0

#ifdef  __cplusplus 
extern "C" {
#endif



#ifdef WIN32
    #define CLYAPP_STDCALL ///???   _stdcall 
#else 
    #define CLYAPP_STDCALL  
#endif

typedef unsigned char clyApp_BYTE;
typedef unsigned short clyApp_WORD;
typedef unsigned char CalypsoFileType[29];

// Stored Value Macro
#define IsStoredValue(ett) (((ett) >= e_EttStoreValue1_30) && ((ett)<= e_EttStoreValue8))



typedef unsigned char TR_BYTE;
typedef unsigned short TR_USHORT;

extern const st_Cly_Date stSlidingZeroDate;

/*
//////////////////////////////////////////////////////////////////////////////
                                    METRONIT USERS STRUCTS AND DEFINES
//////////////////////////////////////////////////////////////////////////////
*/
#ifdef win32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif
#ifdef win_or_linux
  #define TR_PACK pck
  #define TR_PACK_PREFIX
  #pragma pack(push, ClyApp, 1) // correct way to align
#else //keil
  #define TR_PACK_PREFIX __packed
#endif


#if linux
struct __attribute__((__packed__)) TAG_stUserData
{
   	long lv_UserId;
	short iv_UserType;
	char Password[8];//encrpyted
	char cp_Signature[8];
	char lrc;
};
typedef struct __attribute__((__packed__)) TAG_stUserData stUserData;

#else
//Yoni 4/8/14 inspector struct like other employees
TR_PACK_PREFIX struct TAG_stUserData
{
   	long lv_UserId;
	short iv_UserType;
	char Password[8];//encrpyted	
	char cp_Signature[8];
	char lrc;
};
typedef TR_PACK_PREFIX struct TAG_stUserData stUserData;
#endif






/*
//////////////////////////////////////////////////////////////////////////////
                                    END IF METRONIT USERS
//////////////////////////////////////////////////////////////////////////////
*/

#define DEVICE_NUMBER_MASK	(0xfff)  //set 12LSBs of device number 1-4095

/*
//////////////////////////////////////////////////////////////////////////////
                                    CARD   DEFINES
//////////////////////////////////////////////////////////////////////////////
*/


#define MAX_VALIDITY_LOCATION 7 //10
#define MAX_SPATIAL_RFU_DATA_SIZE 10
#define MAX_PARKING_DATA_SIZE 10
#define NUM_OF_SPATIAL_LINES 8
#define NUM_OF_SPATIAL_ROUTESYSTEMS 7
#define MAX_CONTRACT_COUT 8
#define MAX_EVENT_COUT 6
#define SPECIAL_EVENTS_COUNT 4
#define VIRTUAL_CARD_VALID_FLAG 0xa5
#define PAY_METHOD_PRE_PAYMENT 1 
#define PAY_METHOD_PRE_PAYMENT_ELECTRONIC 2 
#define PAY_METHOD_POST_PAYMENT 4

#define DATE_COMPACT_START_YEAR 1997
#define ISRAEL_COUNTRY_ISO_IDENTIFICATION  0x376
/*
//////////////////////////////////////////////////////////////////////////////
                                  CARD  ENUMS 
//////////////////////////////////////////////////////////////////////////////
*/



/*
typedef enum
{
    e_ClyApp_NoErr,                     //OK        
    e_ClyApp_WrongParamErr,             //Parameter Error
    e_ClyApp_InterfaceNotInitErr,       //Interface NotInit Error
    e_ClyApp_WrongPasswordErr,          //SAM Wrong Password Error
    e_ClyApp_WrongSamFamilyErr,         //Wrong Sam Error - real or test SAM 
    e_ClyApp_WrongSamTypeErr,           //Wrong Sam Type Err`or - CL/SL/CPP/CP  
    e_ClyApp_SamRemovedErr,             //Sam Removed Error - for example if SAM has been reset
    e_ClyApp_SamNotLoginErr,            //Sam Not Login Error - SAM is locked
    e_ClyApp_ResetErr,                  //Card / SAM Reset Error
    e_ClyApp_UnknownErr,                //Unknown Error
    e_ClyApp_NoCardErr,                 //No Card foud in the reader or card not reset Error
    e_ClyApp_WrongCardTypeErr,          //Wrong Card Type Err 
    e_ClyApp_ReaderErr,                 //Reader Error - contact/contactless
    e_ClyApp_ReaderSamErr,              //Reader SAM Error
    e_ClyApp_ReaderCardErr,             //Reader Issued Error contact / contactless
    e_ClyApp_CardLockedErr,             //Card Locked Error - black list for example
    e_ClyApp_CardSecurityErr,           //Card Security Error - key/session problem
    e_ClyApp_DataSecurityErr,           //Data Security Error - Data signature / CRC / LRC error
    e_ClyApp_CardErr,                   //Card Error - Unknown Err
    e_ClyApp_CardNotIssuedErr,          //Card Not Issued Error - when expecting an issued card
    e_ClyApp_CardNotEmptyErr,           //Card Not Empty Error - when expecting a manufacturer mode card
    e_ClyApp_CardWriteErr,              //Card Write Error
    e_ClyApp_CardReadErr,               //Card Read Error
    e_ClyApp_CardEnvEndDateErr,         //Expiration date of the ticketing application 
    e_ClyApp_CardEnvErr,                //Environment file data is not supporterd  
    e_ClyApp_CardContractErr,           //Contract file data is not supporterd  
    e_ClyApp_CardEventErr,              //Event file data is not supporterd  
    e_ClyApp_CardContractLRCErr,        //Contract Security Err - 0 bit count error ( checksum err)
    e_ClyApp_NoContractSelectedErr,     //No Contract was Selected using e_ClyApp_IsValidContractExist Err - must be called before use
    e_ClyApp_NotEnoughRightsForUseErr,  //Not Enough Rights For Use Err - for example when trying to use 2 tokens when there is only 1 left 
    e_ClyApp_SessionNotOpenErr,         //before some operations e_ClyApp_IsValidCardEnvironment must be called to open session
    e_ClyApp_CardProfileErr,            //The contract profile must exist in ENV && contract end date can not exceed the end date of the profile
    e_ClyApp_CanNotCancleErr,           //can 

    e_ClyApp_NoValidContractErr,        //Environment file data is not supporterd  
    e_ClyApp_SCReplacedInReader,        //SC was replaced with another card in reader

    e_ClyApp_TicketMaxReloadCounterErr, //Expiration date of the ticketing application 
    e_ClyApp_TicketBitLockErr,          //Ticket Bit Lock Err - when trying to update a locked area 
    e_ClyApp_MemErr,
    e_ClyApp_TicketKeyVerErr,           //Ticket Key Ver Err
    e_ClyApp_DepositRefundLockErr,      //Ticket Key Ver Err
    e_ClyApp_CounterCeilingErr,         //Ticket Key Ver Err

 
}eCalypsoErr;
*/




//Card State
typedef enum
{
 e_ClyApp_ManufacturerState, 
 e_ClyApp_IssuedSatate,
 e_ClyApp_UnknownMasterSatate
}e_ClyApp_CardState;


typedef enum
{
 clyApp_FALSE=0,
 clyApp_TRUE=1 
}clyApp_BOOL;




typedef enum
{
    e_CardEventTransUnspecified = 0, 
    e_CardEventTransUrbanBus = 1,
    e_CardEventTransTransInterurbanBus = 2,
    e_CardEventTransSubway = 3, 
    e_CardEventTransLightRail = 4,
    e_CardEventTransTrain = 5, 
    e_CardEventTransFerryOrBoat = 6,
    e_CardEventTransToll = 7,
    e_CardEventTransParking = 8, 
    e_CardEventTransTaxi = 9, 
    e_CardEventTransTunnelOrBridge = 10,
    e_CardEventTransRefilling = 11,
    e_CardEventTransBreakdown = 12, 
    e_CardEventTransRepair = 13, 
    e_CardEventTransHighSpeedTrain = 14, 
    e_CardEventTransOther = 15, 
}e_clyApp_CardEventTransportMean;


typedef enum
{
    e_CardEventCircumUnspecified = 0, 
    e_CardEventCircumEntry = 1,
    e_CardEventCircumExit = 2,
    e_CardEventCircumPassage = 3, 
    e_CardEventCircumOnBoardControl = 4,
    e_CardEventCircumTest = 5, 
    e_CardEventCircumInterchangeEntry = 6,
    e_CardEventCircumInterchangeExit = 7,
    e_CardEventCircumCancellation = 9, 
    e_CardEventCircumContractLodingWithImmediateFirstUse  = 12,
    e_CardEventCircumContractLoading = 13, 
    e_CardEventCircumApplicationIssuing = 14, 
    e_CardEventCircumInvalidation = 15, 
    e_CardEventCircumRFU
}e_clyApp_CardEventCircumstances;


typedef enum
{
    e_CardPriorityHighestLevel = 0, //current season pass 
    e_CardPriorityOneBelowHighestLevel = 1,//next season pass
    e_CardPriorityTwoBelowHighestLevel = 2,//current restricted season pass
    e_CardPriorityThreeBelowHighestLevel = 3,//next restricted season pass 
    e_CardPriorityFourBelowHighestLevel = 4,//current one time or multi-ride ticket
    e_CardPriorityFiveBelowHighest = 5,//next one time or multi-ride ticket
    e_CardPriorityRFU1 = 6,
    e_CardPriorityRFU2 = 7,     
    e_CardPriorityRFU3 = 8,     
    e_CardPriorityRFU4 = 9,     
    e_CardPriorityRFU5 = 10,    
    e_CardPriorityRFU6 = 11,    
    e_CardPriorityLowestLevelOfPriority = 12,//contract not for transport 
    e_CardPriorityInvalid = 13,//expired
    e_CardPriorityErasable = 14,//or empty
    e_CardPriorityAbsent = 15,//(record cannot exist in the card, e.g. contract #5 in a 4 contracts card) 
}e_ClyApp_CardPriorityType;


typedef enum
{
    e_ClyApp_TransporRFU = 0,
    e_ClyApp_TransportAccessOnly = 1,
    e_ClyApp_ParkingAccessOnly = 2,
    e_ClyApp_ParkingAndTransport = 3

}e_ClyApp_TariffTransportType;


typedef enum
{
    e_ClyApp_CounterNotUsed =0,
    e_ClyApp_CounterAsDateAndRemainingNumOfJourneys =1,
    e_ClyApp_CounterAsNumOfToken =2,
    e_ClyApp_CounterAsMonetaryAmount =3,

}e_ClyApp_TariffCounterType;


typedef enum
{
    e_ClyApp_OneTimeOrMultiRideTicket =1,
    e_ClyApp_SeasonPass =2,
    e_ClyApp_TransferTick =3,
    e_ClyApp_FreeCertificate =4,
    e_ClyApp_ParkAndRideSeasonPass =5,
    e_ClyApp_StoredValue =6,
    e_ClyApp_OneTimeOrMultiRideTicket46 =7,
}e_ClyApp_CardTariffAppType;

#if 0 
//defined in mtr1020,h
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
	e_CardSpatialTypeRouteSystemsList,//11/2008
	e_CardSpatialTypeFareCodeExtension,//11/2008
    e_3CardSpatialTypeRFUOfUnknownFormat,
    e_4CardSpatialTypeRFUOfUnknownFormat,
    e_CardSpatialTypeRFUOfKnownSize,
    e_CardSpatialTypeEndLocationList    //  e_CardSpatialTypeEndOfLocationList
}e_ClyApp_CardSpatialType;

#endif
#ifdef TIM7020  //TBD: yoram
typedef enum
{
	enmTicket,
	enmSCard
}enm_TR_CardMediaType;


typedef enum
{
	enmClient=0,
	enmCashier=1,
	enmInspector=2,
	enmPos=3,
	enmSuperCashier=5,
	enmTechnician=6,
	enmUserUnknown=7,
} enm_TR_PermissionType;


TR_PACK_PREFIX struct TAG_TR_st_SetTime
{
	TR_St_DateTime st_CurrDateTime;//local time
	long lv_time_zone_bias_minutes;//in israel 120 or 180
};
typedef TR_PACK_PREFIX struct TAG_TR_st_SetTime TR_st_SetTime;

TR_PACK_PREFIX struct TAG_TR_st_InspectionData
{
	unsigned long   StationId;
	unsigned short  Zone;
	unsigned short  Cluster;
	TR_St_DateTime  TimeStamp;
	unsigned char lrc;
};
typedef TR_PACK_PREFIX struct TAG_TR_st_InspectionData TR_st_InspectionData;
	
eCalypsoErr TR_WriteUserData(long l_password,
		long l_userid,
		short type,
		const TR_CHLNG chlng
		);	   

//the following struct represents one record from AzmashMx.txt
TR_PACK_PREFIX struct TAG_TR_St_AzmashRecord
{
	//azmash is 8 bit
	unsigned char ucSrc;//Azmash src
	unsigned char ucDst;//Azmash dst
	unsigned short usPriceCode;//a price code which is valid for this combination
};
typedef TR_PACK_PREFIX struct TAG_TR_St_AzmashRecord TR_St_AzmashRecord;




//the following struct represents AzmashMx, to be passed to reader
TR_PACK_PREFIX struct TAG_TR_St_AzmashArray
{
	unsigned short RecCount;//how many records in array
	TR_St_AzmashRecord Records[100];  
};
typedef TR_PACK_PREFIX struct TAG_TR_St_AzmashArray TR_St_AzmashArray;

	/////////////////////////////////////////////////////////
	//// the following structs are used to translate interop record to fare or zone validity location
	TR_PACK_PREFIX  struct TAG_st_ZoneLocation
	{
		unsigned char	ucCluster;
		unsigned short	usZoneBitmap;
	};
	typedef TR_PACK_PREFIX struct TAG_st_ZoneLocation st_ZoneLocation;	
	TR_PACK_PREFIX  struct TAG_st_FareLocation
	{
		unsigned char	ucCluster;
		unsigned char	ucFareCode;
	};
	typedef TR_PACK_PREFIX struct TAG_st_FareLocation st_FareLocation;	

  TR_PACK_PREFIX  struct TAG_union_ValidityLocation
	{
		st_ZoneLocation	stZones;
		st_FareLocation	stFare;
	};
  typedef TR_PACK_PREFIX struct TAG_union_ValidityLocation union_ValidityLocation;

	TR_PACK_PREFIX  struct TAG_St_TblValidityLocation
	{
		unsigned char ucSpatialType;//0=zone 1=fare
		union_ValidityLocation st_Location;
	};
	typedef TR_PACK_PREFIX struct TAG_St_TblValidityLocation St_TblValidityLocation;

    ///////////////////////////////////////////////////////////
	// NAME: TR_SetContratAgreementUserFunction
	// DESCRIPTION: set user  function for check agreement with other operators 
	// PRE REQUIREMENT:  call to TR_InitReader, TR_IsCardIn or CardInside event
	// RETURNS: eCalypsoErr result
	// if function not call the calypso can not use contracts that belong to other provider
	/////////////////////////////////////////////////////////
	typedef TR_BOOL (*USER_PROC_INTEROPPERBILITY)(
		unsigned short   ContractPreDefine,//[IN]
        unsigned short  ContraxctEtt,//[IN]
		unsigned short  ContractProvider,//[IN]
		St_TblValidityLocation* pTblValidityLocation //[OUT]
		//unsigned short*  p_TableCluster,//[OUT]
		//unsigned short*  p_TableZone//[OUT]
		);


    void TR_SetContratAgreementUserFunction(USER_PROC_INTEROPPERBILITY UserProc); 




    #define MAX_RECORDS_AZMASH_ARRAY (1000)
    typedef struct 
    {
      unsigned short RecCount;//how many records in array
      TR_St_AzmashRecord Records[MAX_RECORDS_AZMASH_ARRAY];
    }st_AllAzmashRecords;
    

    void TR_AddAzmashRecords(void* pMemoryAllocationForArray	//pointer to user allocated memory for the whole data structure sizeof(st_AllAzmashRecords)
												,const TR_St_AzmashRecord* pRecordsToAdd //records to add
												,int iStartFrom									//the next record from which to start adding (0 if first call)
												,int iCount											//how many records to add												 
												);

#endif
/*
typedef enum
{
    e_DurationInMonths,
    e_DurationInWeeks,
    e_DurationInDays,
    e_DurationInHalfHours,
}e_ClyApp_DurationType;
*/

typedef enum
{
    e_PeriodIsAMonths,
    e_PeriodIsAWeeks,
    e_PeriodIsADay,
    e_PeriodKartisiaHemshech,
}e_ClyApp_PeriodType;




/*
//////////////////////////////////////////////////////////////////////////////
                                    CARD STRUCTURES
//////////////////////////////////////////////////////////////////////////////
*/

typedef struct
{
    e_clyApp_CardEventTransportMean e_CardEventTransportMean;
    e_clyApp_CardEventCircumstances e_CardEventCircumstances;
}st_clyApp_CardEventCodeType;


typedef struct
{
    e_ClyApp_TariffTransportType e_TariffTransportType ;
    e_ClyApp_TariffCounterType e_TariffCounterType;
    e_ClyApp_CardTariffAppType e_TariffAppType; 

}st_ClyApp_Tariff;


typedef struct
{
    clyApp_WORD ush_SpatialRoutesSystem; //Identifier (1 to 1023) of the routes system where the ticket is valid 
    clyApp_WORD ush_SpatialZones; //Bitmap of authorized zones, within SpatialRoutesSystem
}st_ClyApp_CardContractValidLocZones;


typedef struct
{
    clyApp_WORD ush_SpatialRoutesSystem; //Identifier (1 to 1023) of the routes system where the ticket is valid 
    clyApp_BYTE uc_SpatialFareCode; //Fare code (1 to 255) for the ticket, within SpatialRoutesSystem 
}st_ClyApp_CardContractValidLocFareCode; 


typedef struct
{
    clyApp_BYTE uc_SpatiaiLineArrLen;//the len of the Line Arr
    clyApp_WORD ush_SpatiaiLineArr[NUM_OF_SPATIAL_LINES]; //Identifier (1 to 65,535) of the line (or group of lines) where the ticket is valid 
}st_ClyApp_CardContractValidLocLinesList;


typedef struct
{
    clyApp_WORD ush_SpatialLine; //Identifier (1 to 65,535) of the line where the ride may start or end 
    clyApp_BYTE uc_SpatialStationOrigin; //Identifier (1 to 255) of the station where the ride may start 
    clyApp_BYTE uc_SpatialStationDestination;//Identifier (1 to 255) of the station where the ride may end 
}st_ClyApp_CardCntractValidLocRide;


typedef struct
{
    clyApp_WORD ush_SpatialLine; //Identifier (1 to 65,535) of the line where the ride may start or end 
    clyApp_BYTE uc_SpatialStationOrigin; //Identifier (1 to 255) of the station where the ride may start 
    clyApp_BYTE uc_SpatialStationDestination; //Identifier (1 to 255) of the station where the ride may end 
    clyApp_BYTE uc_SpatialRunType; //Type of allowed runs
}st_ClyApp_CardContractValidLocRideAndRunType;


typedef struct
{
    clyApp_WORD ush_SpatialLine; //Identifier (1 to 65,535) of the line where the ride may start or end 
    clyApp_BYTE uc_SpatialStationOrigin; //Identifier (1 to 255) of the station where the ride may start 
    clyApp_BYTE uc_SpatialStationDestination; //Identifier (1 to 255) of the station where the ride may end 
    clyApp_WORD ush_SpatialRunlD; //Identifier of the run. Coding according to the service provider
}st_ClyApp_CrdContractValidLocRideAndRunID;


typedef struct
{
    clyApp_WORD ush_SpatialLine; //Identifier (1 to 65,535) of the line where the ride may start or 
    clyApp_BYTE uc_SpatialStationOrigin; //Identifier (1 to 255) of the station where the ride may start 
    clyApp_BYTE uc_SpatialStationDestination; //Identifier (1 to 255) of the station where the ride may end 
    clyApp_WORD ush_SpatialRunlD; //Identifier of the run. Coding according to the service provider. 
    clyApp_BYTE uc_SpatialVehicleCoach; //o if irrelevant, else coach number (1 to 15). 
    clyApp_BYTE uc_SpatialSeat; //Seat reference in the coach, 0 to 127
}st_ClyApp_CardContractVlidLocRideRunAndSeat;


typedef struct
{
    clyApp_WORD ush_SpatialRoutesSystemFrom; //Identifier (1 to 1023) of the routes system where the ride may start 
    clyApp_WORD ush_SpatialZonesFrom; //Bitmap of authorized zones where the ride may start, within SpatialRoutesSystemFrom 
    clyApp_WORD ush_SpatialRoutesSystemTo; //Identifier (1 to 1023) of the routes system where the ride may end 
    clyApp_WORD ush_SpatialZonesTo; //bitmap of authorized zones where the ride may end, within SpatialRoutesSystemTo
}st_ClyApp_CardContrctValidLocRideZones;


typedef struct
{
    clyApp_BYTE uc_SpatialParkingDataSize; //Size of the parking data minus 12, in number of bits 
    clyApp_BYTE uc_SpatialParkingData[MAX_PARKING_DATA_SIZE]; //To be defined (12 to 75 bits). 
}st_ClyApp_CardContractValidLocParking; 


typedef struct
{
//  clyApp_WORD ush_SpatialContractTypelD; //Identifier of the type of the predefined contract
    clyApp_BYTE uc_Tariff_Lsb;// ><3 bits><0-7>
    clyApp_WORD ush_SpetailCode;// ><11 bits><0-2023>

}st_ClyApp_CardContractValidLocPredefinedContract; 

//08/2011
typedef struct
{
    clyApp_BYTE uc_SpatiaiRouteSystemArrLen;//the len of the RouteSystem Arr
    clyApp_WORD ush_SpatiaiRouteSystemArr[NUM_OF_SPATIAL_ROUTESYSTEMS]; //Identifier (1 to 1023) of the first route system where the ticket is valid	
}st_ClyApp_CardContractValidLocRouteSystemsList;


//08/2011
typedef struct
{
	clyApp_WORD ush_SpatialRoutesSystem;//Identifier (1 to 1023) of the routes system where the ticket is valid. 0 – means any route system.	
	clyApp_BYTE uc_FareRestrictionCode;//Restriction code with the following values:0 – any fare code 1 – only specified fare code 2 – any fare code up to the specified value (included) 3 – any fare code above the specified value (included) 4 – preferred fare code to be used, unless another code was specified. 5–7 – RFU
	clyApp_BYTE uc_SpatialFareCode;//Fare code (1 to 255) for the ticket, within SpatialRoutesSystem
}st_ClyApp_CardContractValidLocFareCodeExtension;


typedef union
{
    st_ClyApp_CardContractValidLocZones st_Zones;
    st_ClyApp_CardContractValidLocFareCode st_FareCode;
    st_ClyApp_CardContractValidLocLinesList st_LinesList;
    st_ClyApp_CardCntractValidLocRide st_Ride;
    st_ClyApp_CardContractValidLocRideAndRunType st_RideAndRunType;
    st_ClyApp_CrdContractValidLocRideAndRunID st_RideAndRunID;
    st_ClyApp_CardContractVlidLocRideRunAndSeat st_RideRunAndSeat;
    st_ClyApp_CardContrctValidLocRideZones st_RideZones;
    st_ClyApp_CardContractValidLocParking st_Parking;
    st_ClyApp_CardContractValidLocPredefinedContract st_PredefinedContract;
	st_ClyApp_CardContractValidLocRouteSystemsList st_RouteSystemsList;//08/2011
	st_ClyApp_CardContractValidLocFareCodeExtension st_FareCodeExtension;//08/2011
}union_ClyApp_ContractValidityLocation;


typedef struct
{
    e_ClyApp_CardSpatialType e_CardSpatialType;
    union_ClyApp_ContractValidityLocation union_ContractValidityLocation;
}st_ClyApp_ContractValidityLocation;


typedef struct
{
    e_ClyApp_DurationType e_DurationType;
    clyApp_BYTE uc_DurationUnitCount;
}st_ClyApp_ContractValidityDuration;


typedef struct
{
    e_ClyApp_PeriodType e_PeriodType;
    clyApp_BYTE uc_MaxNumOfTripsInPeriod;
}st_ClyApp_ContractPeriodJourneys;

//When ContractTariff indicates a counter as date and remaining number of journeys, the counter associated with the contract has the following structure: 
typedef struct
{
    st_Cly_Date st_CounterDate; //Date of most recent validation 
    short CounterValue; //Number of trips (0 to 1023) authorized for every given period (according to ContractPeriodJourneys) 
}st_ClyApp_CardCounter_DateAndRemainingJourneys;


//When ContractTariff indicates a number of tokens or a monetary amount, the counter associated with the contract has the following structure: 
typedef struct
{
    long CounterValue; //Number of units (0 to 16,777,215), according to ContractTariff 
}st_ClyApp_CardCounter_NumberOfTokensOrAmount;

typedef enum
{
    e_ClyApp_CardCounter_DateAndRemainingJourneys, 
    e_ClyApp_CardCounter_NumberOfTokensOrAmount, 
}e_ClyApp_CardCounterRecordType;

typedef union
{
    st_ClyApp_CardCounter_DateAndRemainingJourneys st_CardCounter_DateAndRemainingJourneys;
    st_ClyApp_CardCounter_NumberOfTokensOrAmount st_CardCounter_NumberOfTokensOrAmount;
}union_ClyApp_CardCounterRecord;

typedef struct
{
    e_ClyApp_CardCounterRecordType e_CardCounterRecordType;
    union_ClyApp_CardCounterRecord union_CardCounterRecord;
}st_ClyApp_CardCounterRecord;


typedef struct
{
    clyApp_BYTE uc_HoiderProfCode; // profile code
    st_Cly_Date st_HoiderProfDate; // profile validity ending date 
}st_ClyApp_HoiderProf;

typedef struct
{
    clyApp_BYTE uc_EnvApplicationVersionNumber; //Environment structure version number
    short sh_EnvCountryld; //Country ISO identification, '376'h for Israel
    clyApp_BYTE uc_Envlssuerld; //Ticketing application issuer identifier
    unsigned long ul_EnvApplicationNumber; //Card Application Number
    st_Cly_Date st_EnvlssuingDate; //Transport application issuing date (Number of days since January 1 st, 1997 (being date 0))
    st_Cly_Date st_EnvEndDate; //Expiration date of the ticketing application (Number of days since January 1st, 1997 (being date 0))
    clyApp_BYTE uc_EnvPayMethod; //Allowed contracts payment methods -can take the values PAY_METHOD_PRE_PAYMENT, PAY_METHOD_PRE_PAYMENT_ELECTRONIC,PAY_METHOD_POST_PAYMENT and OR operation between them if several pay method allowed
    st_Cly_Date st_HolderBirthDate; //Holder birth date
    short sh_HolderCompany; //Short holder company identifier
    unsigned long ul_HolderCompanylD; //National holder company identifier
    unsigned long ul_HolderldNumber; //Holder identifier, within company
    st_ClyApp_HoiderProf st_HoiderProf1; //First profile code and validity ending date
    st_ClyApp_HoiderProf st_HoiderProf2; //second profile code and validity ending date
    clyApp_BYTE uc_HolderLanguage;
}st_ClyApp_EnvAndHoldDataStruct;


typedef struct
{
    clyApp_BOOL b_IsContractRestrictTimeCodeExist;
    clyApp_BYTE uc_ContractRestrictTimeCode; //Time period restrictions
    clyApp_BOOL b_ContractRestrictCodeExist;
    clyApp_BYTE uc_ContractRestrictCode; //General restrictions 
    clyApp_BOOL b_ContractRestrictDurationExist;
    clyApp_BYTE uc_ContractRestrictDuration; //Duration restriction of a journey
    clyApp_BOOL b_ContractValidityEndDateExist;
    st_Cly_Date st_ContractValidityEndDate; //Last day of validity of the contract (Number of days since January 1st, 1997 (being date 0))
    clyApp_BOOL b_ContractValidityDurationExist;
    st_ClyApp_ContractValidityDuration  st_ContractValidityDuration; //Validity duration for the contract
    clyApp_BOOL b_ContractPeriodJourneysExist;
    st_ClyApp_ContractPeriodJourneys st_ContractPeriodJourneys; //Maximum number of trips authorized for every given period 
    clyApp_BOOL b_ContractCustomerProfileExist;
    clyApp_BYTE uc_ContractCustomerProfile; //Social profile giving predefined transportation rights 
    clyApp_BOOL b_ContractPassengersNumberExist;
    clyApp_BYTE uc_ContractPassengersNumber; //Number of passengers allowed for a trip 
    clyApp_BOOL b_ContractRFUExist;
    unsigned long ul_ContractRFUval; //Number of passengers allowed for a trip 
    st_ClyApp_ContractValidityLocation st_ContractValidityLocationArr[MAX_VALIDITY_LOCATION]; //Spatial validity location array
    clyApp_BYTE uc_LocationArrLen; //specify the actual len of the array - if not exist set len to 0 
}st_clyApp_OptionalContractData;


typedef struct
{
    clyApp_BYTE uc_ContractVersionNumber; //Contract structure version number
    st_Cly_Date st_ContractValidityStartDate; //Start date of the contract
    clyApp_BYTE uc_ContractProvider; //Provider identifier
    st_ClyApp_Tariff st_ContractTariff; //Type of contract
    st_Cly_Date st_ContractSaleDate; //Sale date of the contract (Number of days since January 1st, 1997 (being date 0))
    short sh_ContractSaleDevice; //Terminal that sold the contract (1 to 4095) 
    short sh_ContractSaleNumberDaily; //Number of the sale in the day for the device (1 to 1023) 
    clyApp_BOOL b_ContractIsJourneylnterchangesAllowed; //Set to 1 if interchange is allowed
    st_clyApp_OptionalContractData st_OptionalContractData;
    clyApp_BYTE uc_ContractAuthenticator;//Security authenticator 
}st_clyApp_CardContractIssuingData;

typedef struct
{
    st_clyApp_CardContractIssuingData  st_CardContractIssuingData;
    st_ClyApp_CardCounterRecord st_CardCounterRecord;
}st_ClyApp_CardContractRecord;

typedef struct
{
    clyApp_WORD ush_EventTicketRoutesSystem; //Identifier (1 to 1023) of the routes system where the ticket is valid 
    clyApp_BYTE uc_EventTicketFareCode; //Fare code (1 to 255)  for the ticket, within EventTicketRoutesSystem
    clyApp_WORD ush_EventTicketDebitAmount; //Amount (0 to 65535) of the contract counter decrease 

}st_ClyApp_EventTicket;

typedef struct
{
    clyApp_BOOL b_IsEventPlaceExist;
    clyApp_WORD ush_EventPlace; //Identifier (1 to 65,535) of the place where the event takes place Coding according to the service provider. 
    clyApp_BOOL b_IsEventLineExist;
    clyApp_WORD ush_EventLine; //Identifier (1 to 65,535) of the line where the event takes place 
    
		clyApp_BOOL b_IsEventRFU1Exist; //rfu
    clyApp_BYTE uc_EventRFU1; //rfu
    
		clyApp_BOOL b_IsEventRunlDExist;
    clyApp_WORD ush_EventRunlD; //Identifier of the run Coding according to the service provider. 
    
		clyApp_BOOL b_IsEventDevice4Exist;//4th field (from 0)
    clyApp_WORD ush_EventDevice4; //Terminal identifier within the location defined by the previous fields. Coding according to the service provider. 

    clyApp_BOOL b_IsEventRFU2Exist;//rfu
    clyApp_BYTE uc_EventRFU2;//rfu
    clyApp_BOOL b_IsEventInterchangeRightsExist;
    clyApp_BYTE uc_EventInterchangeRights; 
    
		clyApp_BOOL b_IsEventTicketExist;
    st_ClyApp_EventTicket st_EventTicket;// routes system + Fare code+ Amount
    clyApp_BOOL b_IsEventPassengersNumberExist;
    clyApp_BYTE uc_EventPassengersNumber; //Number of passengers
}st_clyApp_OptionalEventData;


typedef struct
{
    clyApp_BYTE uc_EventVersionNumber; //Event structure version number
    clyApp_BYTE uc_EventServiceProvider; //Provider identifier 
    clyApp_BYTE uc_EventContractPointer; //Number of the contract used for this event (1 to 8, 0 if irrelevant) 
    st_clyApp_CardEventCodeType st_EventCode; //Type of the event 
    st_Cly_DateAndTime st_EventDateTimeStamp; //Date and time of event (Number of seconds elapsed since 1/1/1997 at 0:00 GMT, (allows coding all instants until year 2031))
    clyApp_BOOL b_EventIsJourneylnterchange; //Set to 0 for a first boarding, set to 1 if interchange 
    st_Cly_DateAndTime st_EventDataTimeFirstStamp; //Time of first boarding 
    e_ClyApp_CardPriorityType e_EventBestContractPriorityListArr[MAX_CONTRACT_COUT]; //Priority of contract in Contracts file record #1 to #8
    st_clyApp_OptionalEventData st_OptionalEventData;
}st_clyApp_CardEventDataStruct;






////	contract list structure
//		Yoni 10/2011
typedef struct 
{
	clyApp_BYTE uc_Ver;
	clyApp_WORD us_Bitmap;
	clyApp_WORD	ContractListAuthorizationCodeArr[8];	
	clyApp_BYTE	ContractListAuthenticator;
}st_clyApp_ContractListStruct;
 
/*
//////////////////////////////////////////////////////////////////////////////
                                    CARD && TICKET COMMON STRUCTURES
//////////////////////////////////////////////////////////////////////////////
*/



typedef enum
{
    e_ClyApp_Ticket, //ticket
    e_ClyApp_Card,    //card
    e_ClyApp_UnKnown, //unknown
}e_ClyApp_CardType;

typedef enum
{
    e_ClyApp_Use, // cancle use operation
    e_ClyApp_Load, // cancle Load operation
}e_ClyApp_TicketCancelOperationType;


typedef union
{
    st_ClyApp_CardContractRecord st_CardContractRecord;
    struct_ClyTkt_Ticket struct_Ticket;
}union_ClyApp_ContractRecord;


typedef union
{
    st_clyApp_CardEventDataStruct st_CardEventDataStruct;
    struct_ClyTkt_Ticket struct_Ticket;
}union_ClyApp_EventRecord;


typedef struct
{
    CalypsoFileType ucp_EnvironmentData;
    CalypsoFileType ucp_ContractData;
    CalypsoFileType ucp_Event1;
    CalypsoFileType ucp_Event2;
    CalypsoFileType ucp_Event3;
		CalypsoFileType ucp_Counter;//11/2011
		CalypsoFileType ucp_ContractList;//04/2013
}st_ClyApp_CardTransactionBinData;

////////////////////////////////////////////////////////////////////////////
//
// Virtual card write suuport types
//
////////////////////////////////////////////////////////////////////////////

typedef enum 
{
    e_NormalMode,
    e_VirtualWriteMode,
    e_VirtualReadMode,
}e_CardWriteMode;

// Data and flags for the virtual image
// Flags are set to VIRTUAL_CARD_VALID_FLAG when they are valid

TR_PACK_PREFIX  struct TAG_TR_St_TransactionVirtualData
{
    CalypsoFileType         ucp_EnvironmentData;
    CalypsoFileType         ucp_ContractData[MAX_CONTRACT_COUT];
    CalypsoFileType         ucp_Event[MAX_EVENT_COUT];
	CalypsoFileType         ucp_Counter;
    unsigned char           Flag_EnvironmentData;    
    unsigned char           Flag_ContractData[MAX_CONTRACT_COUT];       
    unsigned char           Flag_EventData[MAX_EVENT_COUT];                        
    unsigned char 	        Flag_CounterData;      
};
typedef TR_PACK_PREFIX struct TAG_TR_St_TransactionVirtualData st_ClyApp_TransactionVirtualData, TR_St_TransactionVirtualData;

////////////////////////////////////////////////////////////////////////////
//
// End of - Virtual card write suuport types
//
////////////////////////////////////////////////////////////////////////////


typedef union
{
    st_ClyApp_CardTransactionBinData st_CardTransactionBinData; // card binary data 
    CalypsoBinTktType ucp_CalypsoBinTkt; // ticket binary data 
}union_ClyApp_TransactionBinData;

typedef union
{
    clyApp_BYTE uc_MultiRide_NumOfPassengers; //For MultiRide ticket - how many passangers to incease
    clyApp_BOOL uc_SeasonPass_IsVirginAfterCancel; //For MultiRide ticket - how many passangers to incease
}union_ClyApp_TicketCancelParams;


typedef union
{
    e_ClyApp_TicketCancelOperationType e_TicketCancelOperationType; //  Operation Type to cancel - load or use
    union_ClyApp_TicketCancelParams union_TicketCancelParams; //cancel parameters
}union_ClyApp_TicketCancelInput;


typedef struct
{
    SN8 SerialNum; //last card fail SN
    CalypsoBinTktType upc_FullBinTktData;// 32 byte card data
}st_ClyApp_TktRecoveryDate;

typedef struct
{
    int iv_CountryId;
}st_ClyApp_Params;


// callback to check the Card invironment data. Ticket do not contain invironment data
typedef clyApp_BOOL (*IS_VALID_ENV_CALLBACK)(st_ClyApp_EnvAndHoldDataStruct* stp_EnvAndHoldData);//[IN]Environment data to check

////callback to check if the contract is valid
//typedef clyApp_BOOL (*IS_VALID_CONTRACT_CALLBACK)(union_ClyApp_ContractRecord *union_ContractRecord,int* pContractId,unsigned long SerialNumber,char RecNum);//[In]The contract to check

//callback to check if two contracts are identicl in application terms 
typedef clyApp_BOOL (*ARE_CONTRACTS_IDENTICAL_CALLBACK)(union_ClyApp_ContractRecord *union_ContractRecord1,//[In]The contract1 to check
                                              union_ClyApp_ContractRecord *union_ContractRecord2);//[In]The contract2 to check
//check last event record data to check if last operation was done by the card - to avoid double charging
typedef clyApp_BOOL (*IS_RETIFICATION_VALID_CALLBACK)(st_clyApp_CardEventDataStruct *stp_LastCardEventDataStruct);//[In]last event  

//get currend date and time
typedef clyApp_BOOL (*GET_DATE_AND_TIME_CALLBACK)(st_Cly_DateAndTime *stp_DateAndTime);//[OUT]current date and time  

////add Message to log
//typedef void (*ADD_MSG_2_LOG_CALLBACK)(int i_CalypsoEventNumber);//[IN]Event Number to add to log

////callback to check if  Change the Contract Status to Invalid is Allowed
//typedef clyApp_BOOL (*IS_CHANGE_CONTRACT_STATUS_2_INVALID_ALLOWED_CALLBACK)(union_ClyApp_ContractRecord *union_ContractRecord);///[In]The contract to check

////get the application provier ID
//typedef clyApp_BYTE (*GET_PROVIDER_ID_CALLBACK)(void);

//  callback to store the ticket data befor the load contract operation - for recovery purpose
typedef clyApp_BOOL (*B_STORE_TKT_RECOVERY_DATA)(st_ClyApp_TktRecoveryDate *stp_inTktDate);//[IN]data to store

//  callback to get from store the ticket if exist accurding to the card Serial Number
typedef clyApp_BOOL (*B_GET_STORAGE_TKT_RECOVERY_DATA)(SN8 SerialNum,//[IN] the card serial number need to be recover ( for future use - to support multy storage data )
                                                 st_ClyApp_TktRecoveryDate *stp_inTktDate);//[OUT]data return from storge

typedef clyApp_BOOL(*B_GET_PARAMS)(st_ClyApp_Params* stp_Params);

// callback for reconnect card
typedef  eCalypsoErr (*E_WAIT_CARD)(int retries,int *p_IsCardReseleted);
//user callbacks
typedef struct
{
    IS_VALID_ENV_CALLBACK fp_IsValidCardEnvironmentCallBack;
    //IS_VALID_CONTRACT_CALLBACK fp_IsValidContractCallBack;
    ARE_CONTRACTS_IDENTICAL_CALLBACK fp_AreContaractsIdenticalCallBack;
    IS_RETIFICATION_VALID_CALLBACK  fp_IsRetificationValidCallBack;
    GET_DATE_AND_TIME_CALLBACK fp_DateAndTimeCallBack;
    //ADD_MSG_2_LOG_CALLBACK  fp_addMsg2LogCallBack;
    //IS_CHANGE_CONTRACT_STATUS_2_INVALID_ALLOWED_CALLBACK fp_IsChangeContractStatus2InvalidAllowed;
    //GET_PROVIDER_ID_CALLBACK fp_uch_GetProviderId;
    B_STORE_TKT_RECOVERY_DATA fp_b_StoreTktRecoveryDate;
    B_GET_STORAGE_TKT_RECOVERY_DATA fp_b_GetStorageTktRecoveryDate;
    //B_GET_PARAMS fp_b_GetParams;
	E_WAIT_CARD e_WaitCard;
}st_ClyApp_Callback;

/*
//////////////////////////////////////////////////////////////////////////////
                                    API   FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
*/

char CLYAPP_STDCALL isLegalContract(int recNum);

clyApp_BOOL b_Internal_IsKeyNotIssued(e_clyCard_KeyType e_KeyType);

long l_Internal_ConvertStTime2SecFrom2000 (
        const st_Cly_DateAndTime *stp_TimeIn);// [IN] time struct needs to be converted

clyApp_BOOL b_Internal_Convert2000Sec2StTime (
            const unsigned long l_SecIn,//[IN] secondes since 2000
              st_Cly_DateAndTime *stp_TimeOut);// [OUT] time struct needs to be filled


#ifdef TR4010
eCalypsoErr e_Internal_convertBitSt2ApiSt( e_ConvertType e_Type,//[IN]convert direction - struct to binary Or binary to struct
                                                  e_clyCard_FileId e_CardRecType,//[IN]Rec file type
                                                   void* vp_BinSt,//[IN if e_BitStream2St OUT if e_St2BitStream]
                                                   void* vp_ApiSt);//[OUT if e_BitStream2St IN if e_St2BitStream]

void v_Internal_FillDescriptors(void);
#endif // TR4010

///////////////////////////////////////////////
//              MANDATORY CALLBACK 
///////////////////////////////////////////////
                /****************************/
                // Set User Callbacks
                /****************************/
// Set all user callback - if callback is NULL it will not be used
///   1020 not in use!!! 
void  CLYAPP_STDCALL v_ClyApp_SetUserCallBacks(st_ClyApp_Callback *stp_Callback);//[IN] user callbacks



                /****************************/
                // init interface
                /****************************/
//the readers array describes the list all posible reader were a card/ticket/SAM SL  can be handled. 
eCalypsoErr CLYAPP_STDCALL e_ClyApp_InitInterface(UULOCK_PIN16 ucp_UnlockSampin,//[IN]  Unlock SAM pin
                                                   st_ReaderComInfo *op_CardReaderIdArr, //[IN]Card reader ID array
                                                   unsigned char uc_ArrLen, //[IN]Card reader ID array len
                                                   st_ClyTkt_KeyInfo st_KeyInfoArr[MAX_TIKET_KEYS]);//[IN] file path to the encrypted Card<->Sam key schema and the Ticket encrypt\decrypt key


                /****************************/
                // Start Work With Card
                /****************************/
//ask the provider to start work with the following card - This command is mandatory
eCalypsoErr CLYAPP_STDCALL e_ClyApp_StartWorkWithCard(e_7816_DEVICE i_ReaderId,//[IN] the reader ID in which to detect
                                                       unsigned char  *ucp_SNum, /*[IN]*/ //The card SN 
                                                       unsigned char  uc_SNumLen);//[IN] Serial Number Len



                /****************************/
                // Release Card
                /****************************/
//Forget the Card when the application finishes working with it 
eCalypsoErr CLYAPP_STDCALL e_ClyApp_ReleaseCard(e_7816_DEVICE i_ReaderId);//[IN] the reader ID



                /****************************/
                // Is Valid Environment Exist
                /****************************/
//Is Valid Card Environment - call this function only when working with card ( always returtn clyApp_TRUE if ticket )
/* 1020 not in use!!!
eCalypsoErr CLYAPP_STDCALL e_ClyApp_IsValidCardEnvironment(clyApp_BOOL *b_Result,//[OUT]1=valid,0=invalid
                                                            st_ClyApp_EnvAndHoldDataStruct *stp_EnvAndHoldDataStruct);//[OUT] the Event Data Struct
*/


                /****************************/
                // Is Valid Contract Exist
                /****************************/
#define NO_CONTRACT 0
#define CONTRACT_FOUND 1
#define CONTRACT_RETIFICATION 2
#define CONTRACT_REHABILITATION_WAS_MADE 4

//check if a Valid Contract Exist in the  - the result can be OR between the defined opetion above
/* 1020 not in use!!!
eCalypsoErr CLYAPP_STDCALL e_ClyApp_IsValidContractExist(unsigned char *c_Result, //[OUT] the type of contract found. NO_CONTRACT/CONTRACT_FOUND/CONTRACT_RETIFICATION/CONTRACT_REHABILITATION_WAS_MADE  the result can be OR between the defined opetion
                                                          union_ClyApp_ContractRecord *union_ContractRecord,//[OUT]the contract record found
                                                         int* pUserAppCallbackContractId, //[out]id of valid contract found in the user application callback
                                                         unsigned char* uc_ContractIndex,//[OUT]the returned contract record number
                                                         unsigned long SerialNumber  );//[in]SerialNumber of current card/ticket
*/
//clyApp_BOOL b_IsContractValidBasic(const union_ClyApp_ContractRecord *union_ContractRecord, char c_RecNumber);

                /****************************/
                // Use Contract
                /****************************/
//Use Contract - use the contract found by e_ClyApp_IsValidContractExist 

//egged
eCalypsoErr CLYAPP_STDCALL e_ClyApp_UseContract(clyApp_BYTE uc_NumOfPassengers,//[IN] to be recored in the event record
                                                 unsigned long ul_DebitAmount, //[IN] decrease Amount (0 to 65535) to the contract counter - for MultiRide / stored value. Not relevalt for Season Pass
                                                 unsigned long ul_StoredValueCredit, //[IN] Add Amount (0 to 65535) to the contract counter - for stored value only!!!
                                                 st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] user event data - for card
                                                 struct_ClyTkt_Ticket *stp_TicketEvent,//[IN] Ticket event data  
                                                 TR_St_CancelData *union_BinDataBeforeUseForCancle,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
                                                 clyApp_BOOL *b_Result,unsigned char cRecNum,//[OUT]1=use ok,0=use fail
                                                 clyApp_BOOL b_IsFirstInterchange,//[IN]
												 union_ClyApp_TransactionBinData *p_union_TransactionBinData //[OUT]
                                                 );



                /****************************/
                // Get Binary Transaction Data
                /****************************/
//Get the Transaction Binary Data
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetTransactionBinaryData(union_ClyApp_TransactionBinData *union_TransactionBinData);//[OUT] get the Transaction Binary Data
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetTransactionBinaryDataForEmptySale(union_ClyApp_TransactionBinData *union_TransactionBinData);//[OUT] get the Transaction Binary Data
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetSnapshotAfterStoredValueCancel(unsigned char* pucCancelledRecNumOut, //out
																union_ClyApp_TransactionBinData *union_TransactionBinData,int *p_IsAllEmpty);//[OUT] get the Transaction Binary Data

//Read recored 
eCalypsoErr e_ClyApp_ReadRecordData(clyCard_BYTE Record[29], // [IN/OUT]
										clyCard_BYTE RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
										e_clyCard_FileId FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
										void* StOut //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
										);

void v_ClyApp_EmptyStoreValueTransactionData(void);

unsigned short us_GetAuthorizationCodeAsciiFromContractList(const CalypsoFileType uc_ContractList, unsigned char RecNum/*1-8*/);

                /****************************/
                // Find free record  ( for load operation )
                /****************************/
//Find if exist free record for a new Contract loading operation 
eCalypsoErr CLYAPP_STDCALL e_ClyApp_IsFreeRecExist(
				clyApp_BOOL *b_Result,//[OUT]1=free record exist ok,0=no free record
				unsigned char * first_free_index//,///[out] first free index or ff if not found				
							);


                /****************************/
                // Check if Contract Already Exist ( for load operation )
                /****************************/
//Check if the same contract Already Exist in the card - check before load operation to avoid duplicate contract loading
/*
eCalypsoErr CLYAPP_STDCALL e_ClyApp_IsContractAlreadyExist(union_ClyApp_ContractRecord *union_ContractRecord,//[In]The ontract to check
                                                             clyApp_BOOL *b_Result);//[OUT]1=Valid contract exist ,0=no valid contract exist
*/

                /****************************/
                // Get All Contracts 
                /****************************/
eCalypsoErr e_ClyApp_SimpleReadAllContracts(union_ClyApp_ContractRecord 
                                                     union_ContractRecordArr[MAX_CONTRACT_COUT]);

//Get All Contract fond in the card/ticket - even the contract which are invalid
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetAllContracts(union_ClyApp_ContractRecord union_ContractRecordArr[MAX_CONTRACT_COUT]);//[OUT]Array memory allocation of contracts to fill

                /****************************/
                // Get Contracts 
                /****************************/
//Get All Contract fond in the card/ticket - even the contract which are invalid
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetContract(union_ClyApp_ContractRecord *union_ContractRecordArr,//[OUT]Array memory allocation of contracts to fill
                                                      unsigned char uc_ContractNumber);//[IN] contract Number to read: can take the values 1 to 8 - NOT RELEVAT FOR TICKET


eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetSpecialEvent(unsigned char ContractRecNum // 1-8
																										,st_clyApp_CardEventDataStruct* pSpecialEvent /*out*/);

                /****************************/
                // Remove Contract
                /****************************/
//Remove Contract - use GetAllContracts function to determine the contract number  
///????????????????????????   if use in 1020
#ifndef TR4010
eCalypsoErr CLYAPP_STDCALL e_ClyApp_RemoveContract(unsigned char uc_ContractNumber,//[IN] contract Number to remove: can take the values 1 to 8
                                                    st_clyApp_CardEventDataStruct *stp_CardEventDataStruct);//[IN] parameter relevant only for card. defines the parameters of the cancle operation.

#endif                /****************************/
                //Check If Load Contract Possible
                /****************************/
//Load Contract - for a card fill the contract, for a ticket fill only the contract related information ( common data + contract data )
//eCalypsoErr CLYAPP_STDCALL e_ClyApp_IsLoadContractPossible(union_ClyApp_ContractRecord *union_ContractRecord,//[IN]the contract record to load 
 //                                                st_clyApp_CardEventDataStruct *st_CardEventDataStruct,//[IN] parameter relevant only for card
 //                                                clyApp_BOOL *b_Result);//[OUT]1=Load OK ,0=Load Fail


/*====================================================
    checking the contract sign on the egged contracts
    by tw sam
        
====================================================*/
#ifndef TR4010
eCalypsoErr CheckTwSamSign(e_ClyApp_CardType e_CardType,///   [in] card or ticket
                                union_ClyApp_ContractRecord *cntr, ///   [in] pointer to contract
                                int ContractIndex   ///   [in] card ser number
                                );
#endif

                /****************************/
                // Load Contract
                /****************************/
//Load Contract - for a card fill the contract, for a ticket fill only the contract related information ( common data + contract data )
eCalypsoErr CLYAPP_STDCALL e_ClyApp_LoadContract(union_ClyApp_ContractRecord *p_union_ContractRecord,//[IN]the contract record to load 
                                                 st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card
												 long SVRequestedAmount, //how much to add to stored value
                                                 unsigned short ulWhiteListId,
												 TR_St_CancelData *union_BinDataBeforeUseForCancle,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
												 union_ClyApp_TransactionBinData *union_TransactionBinData,
                                                 clyApp_BOOL *b_Result,
												 int * index
												 );

                                                    
                /****************************/
                // Get All event records ( for cancellation operation )
                /****************************/
//Get All event record - for the cancellation operation -  helps to determine which of the last operation need to be canceled 
#ifndef TR4010
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetAllEvent( union_ClyApp_EventRecord union_EventRecordArr[MAX_EVENT_COUT],//[IN]Array memory allocation of events to fill
                                                        clyApp_BOOL bIsEventOkArr[MAX_EVENT_COUT] //[OUT] indicate for each event if ok //Yoni 14/6/10
                                                        );
#endif
eCalypsoErr CLYAPP_STDCALL e_ClyApp_SimpleGetAllEvent( union_ClyApp_EventRecord union_EventRecordArr[MAX_EVENT_COUT]//[IN]Array memory allocation of events to fill
                                                       ,clyApp_BOOL bIsEventOkArr[MAX_EVENT_COUT] //[OUT] indicate for each event if ok //Yoni 14/6/10
                                                       );


////Yoni 7/2010 for technician
//eCalypsoErr e_WriteAndReadEventRecord(void);

                /****************************/
                // Cancel Operation
                /****************************/
//Cancel the operation described by event index
eCalypsoErr CLYAPP_STDCALL e_ClyApp_CancelOperationByContractIndex(const TR_St_CancelData *union_BinDataBeforeUseForCancel,
                                                                       st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card. defines the parameters of the cancle operation.
                                                                       clyApp_BOOL *b_Result,//[OUT]1=Cancel OK ,0=Cancel fail 
																		union_ClyApp_TransactionBinData* p_TransactionBinData); //[OUT]




//mark contract as erasable, write cancel event, delete associated special event 
eCalypsoErr CLYAPP_STDCALL e_ClyApp_CardCancelContract(unsigned char uc_ContractRecNum,
                                                   st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card. defines the parameters of the cancle operation.                                                   
												   union_ClyApp_TransactionBinData* p_TransactionBinData, //[OUT]
                                                   int* pSpecialEventIndexToDelete //[OUT] if 0-3 then this special event needs to be deleted
                                                   ); 

                /****************************/
                // Lock card/Ticket
                /****************************/
//This function gives the ability to lock the a card or a ticket for future use in case of black list for example
eCalypsoErr CLYAPP_STDCALL e_ClyApp_Lock(union_ClyApp_TransactionBinData* p_TransactionBinData); 


                /****************************/
                // Is Card/Ticket Locked
                /****************************/
//This function gives the ability to check if the card or the ticket are locked
eCalypsoErr CLYAPP_STDCALL e_ClyApp_IsLock(clyApp_BOOL *b_Result);//[OUT]1=Locked ,0=not locked











//return true of continue/maavar trip is valid (also return byref relevant data)
clyApp_BOOL b_ClyApp_GetContinueValidity(int ContractIndex //[IN]
                    ,const st_clyApp_CardContractIssuingData* pContractData
                    ,const st_clyApp_CardEventDataStruct* pRegularEventData
                    ,const st_Cly_DateAndTime* pCurrDateAndTime//[IN] current date and time
                    ,TR_USHORT* pMaavarValidity//[OUT] remaining minutes. if "end of day" 9998. if "end of service" 9999.
                    ,TR_BYTE* pPrevProvider //[OUT] prev provider (from regular event)
                    ,TR_BYTE *pPassengerCount //[OUT] passenger count cound be 1 or more
                    ,TR_USHORT* pMaavarPrevLine//[OUT] prev makat line (from regular event). 0 if not egged
					,TR_USHORT  *pEventRunID //[OUT] Run ID
					,TR_BYTE *pFareCode // [OUT] Fare code
                    );





                /****************************/
                // Set Deposit Refund Operators List
                /****************************/
//List of all operators which are not the current operator which their card's defosit can be refund.
//As default only the current operator card's deposit can be refund
/* 1020 not in use!!!
void  CLYAPP_STDCALL v_ClyApp_SetDepositRefundOperatorsList (unsigned char *ucp_OtherOperatorsList,//[IN] list of all operators which are not the current operator which their card can be refund 
                                                          unsigned char uc_ListLen);//[IN] the list len ( 0 - 255 )
*/


                /****************************/
                // Deposit Refund
                /****************************/
//Lock the card for futur use - lock reason = Deposit Refund
/* 1020 not in use!!!
eCalypsoErr CLYAPP_STDCALL e_ClyApp_DepositRefund(e_7816_DEVICE i_ReaderId); //[IN]reader ID
*/

                /****************************/
                // Set Stored Value Ceiling
                /****************************/
//Define the stored value ceiling 
//Without setting this value credit can not be done, only  - default = 0
/* 1020 not in use!!!
void  CLYAPP_STDCALL v_ClyApp_SetStoredValueCeiling (unsigned short ush_ceiling);//[IN]stored value Max ceiling value
*/



                /*****************************************/
                // Get Stored Value sum on card (internal)
                /*****************************************/             
/* 1020 not in use!!!
eCalypsoErr e_ClyApp_Internal_GetSumOfAllStoredValuesOnCard(unsigned short* ush_SumStoredValue ,//[OUT] sum of all stored values on card
                                                             clyApp_BOOL* b_IsSameProvider  //[OUT]
                                                             );
*/

                /*****************************************/
                // API- can SV be loaded 
                /*****************************************/         
/* 1020 not in use!!!
eCalypsoErr e_ClyApp_IsStoredValueLoadingPossible(clyApp_BOOL* b_Resault, //[OUT] is load possible
                                                   unsigned short ush_SumToLoad //[IN] new sum to load
                                                   );
*/

/////////////////////////////////////////////////////////////////////////////////////////////////
//              SAM COMMANDS
/////////////////////////////////////////////////////////////////////////////////////////////////

                /****************************/
                // Sam Check Comunication
                /****************************/
//check the the sam is communicating and with it PIN set ( with means the it has not been reset ) 
/* 1020 not in use!!!
eCalypsoErr CLYAPP_STDCALL e_ClyApp_SamCheckComunication(void); //[IN]SAM reader ID
*/


                /****************************/
                // Return Sam To Work Mode
                /****************************/
//In case the sam has been reset - return the sam to it's work mode by reset + PIN.
/* 1020 not in use!!!
eCalypsoErr CLYAPP_STDCALL e_ClyApp_ReturnSam2WorkMode(void); //[IN]SAM reader ID
*/


                /****************************/
                // Get Sam Type
                /****************************/
//get sam type CL/SL/CPP/CP/CV
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetSamType(e_7816_DEVICE i_SamReaderId, //[IN]SAM reader ID
                                                e_ClyApp_SamType *e_SamType);//[OUT] the SAM type

////////////////////////////////////////////////////////////////////////////////////////
//  Yoni 03/2013
//  e_ClyApp_ReadSamData
//  read params of sam such as event counter / value counter etc.
////////////////////////////////////////////////////////////////////////////////////////
eCalypsoErr CLYAPP_STDCALL e_ClyApp_ReadSamData(
												e_clySam_DataType eDataType, //e_clySam_File/e_clySam_Rec
												e_clySam_DataRecType eRecType, //e_clySam_EventCounterRec/e_clySam_EventCeillingRec/e_clySam_SumRec
												int RecNum, //1-based
												St_clySam_ReadDataResult* pReadResult //[OUT]
                                                );

                /****************************/
                // Sam CL Ceiling Update
                /****************************/
//add value to the sam CL counter ceiling 
/* 1020 not in use???
eCalypsoErr CLYAPP_STDCALL e_ClyApp_SamClCeilingUpdate(unsigned long ul_ValToAdd2Ceiling);//[IN] Value To Add to Ceiling
*/


                /****************************/
                // Get Remaining Ceiling Value
                /****************************/
//Get the number of use remains before it will be locked
/* 1020 not in use???
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetRemainingCeilingVal(unsigned long* ulp_CeilingValRemain);//[IN] Value remains before counter will be locked
*/

/////////////////////////////////////////////////////////////////////////////////////////////////
//              HELPER FUNCTIONS - NOT MANDATORY FOR THIS LAYER( CAN BE DONE OUTSIDE THIS LAYER )
///////////////////////////////////////////////////////////////////////////////////////////////////

                /****************************/
                // detect card
                /****************************/
//Check if card found in one of the readers - this command is not mandatory to this layer
eCalypsoErr CLYAPP_STDCALL e_ClyApp_DetectCard(e_7816_DEVICE i_ReaderId);//[IN] the reader ID in which to detect

void CLYAPP_STDCALL  v_Internal_ClearLastCardGlobalDataStateMachine(void);
                /****************************/
                // Reset Card
                /****************************/
//Reset Card - this command is not mandatory to this layer
eCalypsoErr CLYAPP_STDCALL e_ClyApp_ResetCard(e_7816_DEVICE i_ReaderId,//[IN] the reader ID
                                             e_7816_CardType* e_CardType);//[OUT]type of card

                /****************************/
                // Get Card Serial Number
                /****************************/
//get the card Serial Number - Function does not make an internal memory allocation.
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetCardSn(e_7816_DEVICE i_ReaderId,//[IN] the reader ID
                                             unsigned long* p_pSNum );/*[OUT]*/ //The card SN 


                /****************************/
                // Get Card Full Serial Number
                /****************************/
//get the card Full Serial Number - up to 8 bytes value - Function does not make an internal memory allocation.
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetCardFullSn(e_7816_DEVICE i_ReaderId,//[IN] the reader ID
                                                unsigned char* p_pSNum,//[OUT] //The card SN 
                                                unsigned char *SnLen);//[OUT] SN byte len

//get the card Full Serial Number - up to 20 chars value - Function does not make an internal memory allocation.
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetCardFullSnStr(e_7816_DEVICE i_ReaderId,//[IN] the reader ID
                                                      char p_pSNum[20]);//[OUT] //The card SN 


//return TRUE if card is after personalization or ticket is empty (new)
// pre condition is success calling  to e_ClyApp_StartWorkWithCard
/* 1020 not in use!!!
eCalypsoErr  b_ClyApp_IsNewCard(e_7816_DEVICE i_ReaderId,//[IN]
                                 clyApp_BOOL *p_BoolResult); //[OUT]
*/

/* 1020 not in use!!!
clyApp_BOOL CLYAPP_STDCALL b_ClyApp_IsTransfer();
*/
/* 1020 not in use!!!
clyApp_BOOL CLYAPP_STDCALL b_ClyApp_IsNewSegment(int c_NexRecNum, int iSegment);
*/
/////////////////////////////////////////////////////////////////////////////////////////////////
//              HIGH LEVEL APPLICATION HELPER FUNCTIONS 
///////////////////////////////////////////////////////////////////////////////////////////////////

                              
                /****************************/
                // calc expected end date
                /****************************/
//calc expected end date using contract start date Duration period and duration units
st_Cly_Date CLYAPP_STDCALL st_CalcEndDateByPeriod(const st_Cly_Date *stp_StartDateCompact,//[IN] contract start date
                                    e_ClyApp_DurationType e_DurationType, //[IN] Duration period type
                                    unsigned char uc_DurationUnitCount); //[IN] duration units
  
eCalypsoErr CLYAPP_STDCALL e_ClyApp_UseContractT(clyApp_BYTE uc_NumOfPassengers,//[IN] to be recored in the event record
                                                 clyApp_WORD ush_DebitAmount, //[IN] Amount (0 to 65535) of the contract counter - for MultiRide / stored value. Not relevalt for Season Pass
                                                 clyApp_WORD ush_StoredValueCredit, //[IN] Add Amount (0 to 65535) to the contract counter - for stored value only!!!
                                                 st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] user event data - for card
                                                 struct_ClyTkt_Ticket *stp_TicketEvent,//[IN] Ticket event data  
                                                 TR_St_CancelData *union_BinDataBeforeUseForCancle,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
                                                 clyApp_BOOL *b_Result,
                                                 clyApp_BOOL b_IsFirstInterchange); 

                /****************************/
                // Get Datef - st to long convert
                /****************************/
//Date given as 8 BCD digits: 'yyyymmdd'h:  yyyy : year (e.g. '1968'h) , mm: month (1..12)  dd: day of the month (1..31) 
//unsigned long l_GetDatef(st_Cly_Date *st_Date);


                /****************************/
                // Get Datef - long to st convert
                /****************************/
//Date given as 8 BCD digits: 'yyyymmdd'h:  yyyy : year (e.g. '1968'h) , mm: month (1..12)  dd: day of the month (1..31)
//st_Cly_Date st_GetDatef(unsigned long l_Date);


//////////////////////////////////////////////////////////////////////
//          ISSUE STATION FUNCTIONS
//////////////////////////////////////////////////////////////////////
//The following function are defined for the issuing machine station only and not for the TIM machine
#ifdef WIN32
                /****************************/ 
                //Read Record 
                /****************************/
//to read current Environment record and for future use
//the function do not mack internal memory allocation
eCalypsoErr CLYAPP_STDCALL e_ClyApp_ReadRecord(clyCard_BYTE RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record 
                                                e_clyCard_FileId FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                                void* StOut); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts 

                /****************************/ 
                //Perform Pre Personalization = Issue Anonymous card
                /****************************/
//This function replace the card/Ticket keys and clear all it's information in adition it writes anonymous data into the inveronment file in case of card
eCalypsoErr CLYAPP_STDCALL e_ClyApp_PerformPrePersonalization(st_ClyApp_EnvAndHoldDataStruct* stp_EnvAndHoldData,//[IN]Environment data to write - necessary data: Country ID, IssuerId (operator ID ) , ApplicationNum, End date, Pay Method
                                                               clyApp_BOOL b_IsChangeKeys);//[IN] to support TEST card which already containd keys but do not contain data. in the final mode ( NOT TEST ) this parameter allways need to be TRUE!

                /****************************/
                // Perform Personalization = write personal data 
                /****************************/
//write personal data - relevalt only for card - must be done after Pre Personalization 
eCalypsoErr CLYAPP_STDCALL e_ClyApp_UpdateCardEnvAndHoldDataRec (st_ClyApp_EnvAndHoldDataStruct* stp_EnvAndHoldData);//[IN]Environment data to write
                                              

                /****************************/
                // Format Card To Manufacturer
                /****************************/
//reuse card - format it data 
eCalypsoErr CLYAPP_STDCALL e_ClyApp_FormatCardData(e_7816_DEVICE i_ReaderId); //[IN]Reader ID


                /****************************/
                // Get Card State
                /****************************/
//Get Card State 
e_ClyApp_CardState CLYAPP_STDCALL e_ClyApp_GetCardState(void);



                /****************************/
                // Format Card To Manufacturer
                /****************************/
//Change the card/Ticket keys back to manufacturer keys
eCalypsoErr CLYAPP_STDCALL e_ClyApp_FormatCard2Manufacturer(e_7816_DEVICE i_ReaderId); //[IN]Reader ID


eCalypsoErr CLYAPP_STDCALL e_ClyApp_Rehabilitate(e_7816_DEVICE i_ReaderId/*[IN] the reader ID*/);

eCalypsoErr CLYAPP_STDCALL e_ClyApp_IsCardDepositRefund(clyApp_BOOL *b_WasCardDepositRefund);//[OUT]1=DepositRefund ,0= Deposit not Refund


#endif

#if CLY_DEBUG_SHOW

//Display record - Env / Contract / Event / Counter
void vp_Debug_DisplayStRec(e_clyCard_FileId e_CardFileType,//[IN]file type to show
                           void *vp_Ptr);//[IN] the struct to show
#endif

#ifdef  __cplusplus 
 }
#endif

//////////////////////////////   1020 new functions area
/****************************/
// Get Card type   NEW FUNCTION FOR 1020
/****************************/
//get the card Serial Number - Function does not make an internal memory allocation.
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetCardType(e_7816_DEVICE i_ReaderId,//[IN] the reader ID
                                             unsigned char* p_CardType );/*[OUT]*/ //The card type 


/****************************/
// Get Environment   NEW FUNCTION FOR 1020
/****************************/
//Get All event record - for the cancellation operation -  helps to determine which of the last operation need to be canceled 
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetEnvironment(st_ClyApp_EnvAndHoldDataStruct *st_EnvAndHoldDataStruct);//[OUT] enviroment record

// Fix bits for Egged long device number support
void v_BitsFixer( int ConvertType,unsigned short *m_ContractSaleDevice, unsigned short *m_ContractSaleNumberDaily);   
unsigned short ush_GetDateCompact(const st_Cly_Date* stp_Cly_Date);
unsigned long ul_GetTimeReal(const st_Cly_DateAndTime* stp_TimeReal,int IsGmtTime);
eCalypsoErr e_Internal_Bit2Byte_Convert( e_ConvertType ConvertType,//[IN] convert direction - bit stream to struct OR struct to bit stream
                                                 e_ClyApp_CardType e_CardType,//[IN]type - card \ ticket 
                                                 e_clyCard_FileId e_CardFileType,//[IN] if not a ticket  - which record in the card
                                                 unsigned char *ucp_BitStream,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream   
                                                 void *vp_St);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream   


//Yoni 6/6/2010
//Get end date of sliding contract (currently only duration in days/weeks/months) 
st_Cly_DateAndTime stCalcEndDateByValidityDuration(const st_ClyApp_CardContractRecord* pst_CardContractRecord //[IN]                                                                            
                                                , unsigned char RecNum);


//Yoni2011
e_EttType i_GetEttType(const union_ClyApp_ContractRecord *union_ContractRecord
												);
e_EttType i_GetEttTypeFromCardContractRecord(const st_ClyApp_CardContractRecord *p_ClyApp_CardContractRecord);


//this struct contains the contract validity info that we already calculated
//fill it once for each contract, dont calculate over and over
typedef struct
{
	unsigned char		ContractIndex;//0-7
	unsigned char		BCPL_Priority;//priority according to bcpl
	eValidityStatus		status;
	st_Cly_DateAndTime	ClyDtm_StartDate;//0's if non
	st_Cly_DateAndTime	ClyDtm_EndDate;//0's if non
	unsigned long		Counter;//agorot or tokens
	clyApp_BOOL			IsValidInterchange;
	unsigned short	usRemainingInterchangeMinutes;
	unsigned char		ucInterchangeRIghts;//from special event
	unsigned char		ucPsngrCount;//passenger count from special event
	unsigned char		ucEventTicketFareCode;//fare code from eventticket frin special event
	unsigned char       m_MaavarPrevProvider;//provider id in which prev use was done, 0 if first use of nikuv (hime 12/04/15)
    unsigned short      m_MaavarPrevLine;
}st_ProcessedContractValidityInfo;

eCalypsoErr e_clyapp_GetContractsForUse(union_ClyApp_ContractRecord ContractsArr[MAX_CONTRACT_COUT]/*out*/, st_ProcessedContractValidityInfo ProcessedInfoArr[MAX_CONTRACT_COUT], int* pNumOfValidContracts);
eCalypsoErr e_clyapp_GetContractsForLoadOrReport(union_ClyApp_ContractRecord ContractsArr[MAX_CONTRACT_COUT]/*out*/, st_ProcessedContractValidityInfo ProcessedInfoArr[MAX_CONTRACT_COUT], int* pNumOfContracts);
eCalypsoErr clyApp_GetWhiteContractListOnCard(unsigned short* Bitmap/*out*/, unsigned short Codes[8]/*out*/);
eCalypsoErr e_ClyApp_CheckConditionsForStoredValue(unsigned short ContractPredefineCode, long lAmountToLoad, unsigned long* pSVTotalAmount/*out*/, long* pSVIndex/*out*/);

unsigned char uc_clyapp_GetCurrentCardType(void);
clyApp_BOOL clyapp_bIsProfileValid(int ProfileCode);
TR_BOOL b_clyapp_IsCardIn(TR_st_CardInfo* info/*info*/);
TR_BOOL b_clyapp_IsCardEmptyEnv(void);
void clyapp_ForgetCard(void);
clyApp_BOOL b_ClyApp_TestReadWrite(void);
e_InterchangeType e_GetInterchangeType(const st_clyApp_CardContractIssuingData* pContractIssuingData);
eCalypsoErr e_VerifyPassword(long Pswrd);
eCalypsoErr e_ReadUserData(stUserData* pUserData/*out*/);
eCalypsoErr e_clyapp_GetChallenge(unsigned char cp_random[8]/*out*/);
eCalypsoErr e_clyapp_CheckChallenge(const unsigned char plain_chlng[8]);
eCalypsoErr e_WriteUserData(long lv_password, long id, short type, const TR_CHLNG Chlng);
eCalypsoErr e_ReadInspectionData(TR_st_InspectionData* pInspectionData);
eCalypsoErr e_WriteInspectionData(TR_st_InspectionData* pInspectionData);

eCalypsoErr e_clyapp_CheckCardComm(void);
eCalypsoErr e_clyapp_CheckSamComm(void);
eCalypsoErr e_clyapp_IsValidEnv(const st_ClyApp_EnvAndHoldDataStruct *stEnv);
unsigned short us_GetPredefineCode(const union_ClyApp_ContractRecord *p_union_ContractRecord);



clyApp_BOOL b_GetContractFromStateMachine(int index, st_ClyApp_CardContractRecord *P_st_ClyApp_CardContractRecord);
clyApp_BOOL b_GetEventFromStateMachine(int index,st_clyApp_CardEventDataStruct  *Event_Out);
clyApp_BOOL b_GetSpectialEventFromStateMachine(int index,st_clyApp_CardEventDataStruct  *Event_Out);
unsigned short usGetShilutFromEventAccordingToVersion(const st_clyApp_CardEventDataStruct* pRegularEventData);

////////////////////////////////////////////////////////////////////////////
//
// Virtual card write suuport
//
////////////////////////////////////////////////////////////////////////////

eCalypsoErr     e_ClyApp_Virtual_SetCalypsoMode                 (TR_st_CardInfo* pInfo,e_CardWriteMode eWriteMode);
void            e_ClyApp_Virtual_GetCardImage                   (st_ClyApp_TransactionVirtualData *TransactionBinData);
void            e_ClyApp_Virtual_SetCardImage                   (st_ClyApp_TransactionVirtualData *TransactionBinData);
eCalypsoErr     e_ClyApp_Virtual_UpdateCardEnvAndHoldDataRec    (clyApp_BOOL BCPLReset, st_ClyApp_EnvAndHoldDataStruct* stp_EnvAndHoldData);
eCalypsoErr     e_ClyApp_Virtual_GetRecord                      (clyCard_BYTE RecNum, e_clyCard_FileId FileToSelect,clyCard_BYTE *RecData2Update,clyCard_BYTE BytesToCopy);


#ifdef WIN32
TR_BOOL b_WinReadCustomerCard(st_ClyApp_EnvAndHoldDataStruct*	pEnvAndHoldDataStruct, //out
															union_ClyApp_EventRecord			s_EventRecordArr[7],//out 3 regualr + 4 special
							union_ClyApp_ContractRecord		s_ContractRecordArr[MAX_CONTRACT_COUT], //out
							st_clyApp_ContractListStruct*	pContractList //out
													 );
#endif //WIN32


//Remote Sam support
eCalypsoErr e_clyappCardInOut(PACKET_7816* p_7816_Packet/*in*/, 
															RESPONSE_OBJ* p_7816_ResponseBuff/*out*/,
															unsigned short timeout
															);



// Calypso delete card utility for the zabad
eCalypsoErr e_ClyApp_DeleteCardContracts(clyApp_BOOL bBCPL_Only);


eCalypsoErr e_clyapp_WriteStreamToCardEventAndRead(const char buff[29]);

#if ENABLE_DEBUG_LOAD

#define DEBUG_LOAD_BEFOR_REGULER_MODE 0
#define DEBUG_LOAD_BEFOR_CLOSE_SESSION 1
#define DEBUG_AFTER_CLOSE_SESSION 2
#define DEBUG_BEFOR_OPEN_SESSION 3

typedef struct 
{
  int i_DebugValue;
  int i_CountDebugOpAfterX;
  //int i_CurrCount;
  

}St_ClyDebugSettin;

void e_ClyApp_SettDebugInfo(St_ClyDebugSettin *p_Setting);
void e_ClyApp_ResetDebugState(void);

#endif

#ifdef win_or_linux
  #pragma pack(pop, ClyApp)
#endif

#endif
