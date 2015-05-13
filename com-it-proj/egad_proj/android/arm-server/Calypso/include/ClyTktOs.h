#ifndef _CALYPSO_TKT_OS_H_
#define _CALYPSO_TKT_OS_H_


#ifdef  __cplusplus 
extern "C" {
#endif

///////////////////////////////////////
//         DEFINE
///////////////////////////////////////

#ifdef WIN32
    #define CLYTKT_STDCALL _stdcall 
#else 
    #define CLYTKT_STDCALL  
#endif
      
#define MAX_TIKET_KEYS 4
#define TICKET_IN_THE_CODE 0
      
///////////////////////////////////////
//         TICKET ENUMS
///////////////////////////////////////

typedef unsigned char clyTkt_BYTE;
typedef unsigned short clyTkt_WORD;
typedef unsigned char CalypsoBinTktType[32];
typedef unsigned char TKT_SN_TYPE[8];
typedef unsigned char clyApp_TktKeyType[16];

typedef enum
{
    ClyTkt_SignType_8=1,
    ClyTkt_SignType_16=2,
    ClyTkt_SignType_32=4,
}ClyTkt_SignType;
typedef enum
{
 ClyTkt_FALSE=0,
 ClyTkt_TRUE=1 
}ClyTkt_BOOL;

typedef enum
{
    e_ClyTkt_NO_ERROR=0,
    e_ClyTkt_MEDIA_INCORRECT, // unknown card type
    e_ClyTkt_RECORD_TYPE_INCORECT,//ilegal record type
    e_ClyTkt_NO_SIGN_CALBACK,//null
    e_ClyTkt_CHECK_SIGN_FAIL,//contract sign fail
    e_ClyTkt_CHECK_SEASON_THERD_SIGN_FAIL,//season pass - therd sign
    e_ClyTkt_DECRYPT_FAIL,// internal error
    e_ClyTkt_ENCRYPT_FAIL,// internal error
/// e_ClyTkt_SECOND_MULTI_SIGN_FAIL_BUT_BKPFLAG_TRUE,
//  e_ClyTkt_BKPFLAG_TRUE_BUT_SECOND_MULTI_SIGN_FAIL,
    e_ClyTkt_BKFLGTRU_SECMULSIGNFAIL,
    e_ClyTkt_SECOND_MULTI_SIGN_FAIL_BUT_BKPFLAG_FALSE,
    e_ClyTkt_ILLEGAL_DATE,//not used
    e_ClyTkt_ILLEGAL_TIME,//not used
    e_ClyTkt_SIGN_CALLBACK_NULL,
    e_ClyTkt_DATE_TIME_CALLBACK_NULL,
    e_ClyTkt_KEY_NOT_EXIST,//internal
    e_ClyTkt_KEY_LRC_FAIL,//key coraption 
    e_ClyTkt_DATE_PLUS_H_D_CALLBACK_NULL//null
}e_ClyTkt_ERR;

typedef enum
{   
    e_ClyTkt_TariffMultiRideFareCodeTicket,
    e_ClyTkt_TariffPointToPointMultiRideTicket,
    e_ClyTkt_TariffPredefinedMultiRideTicket,
    e_ClyTkt_TariffRFU1,
    e_ClyTkt_TariffAreAeasonPass,
    e_ClyTkt_TariffPointToPointSeasonPass,
    e_ClyTkt_TariffPredefinedSeasonPass,
    e_ClyTkt_TariffRFU2, 
}e_ClyTkt_TicketTariffAppType;


typedef enum
{
    e_ClyTkt_Forward,//origin to destination
    e_ClyTkt_Backward// destination to origin
}e_ClyTkt_Direction;


typedef enum
{   
    e_ClyTkt_ValidityStartsAtIssueDate, 
    e_ClyTkt_ValidityStartsAtFirstUse 
}e_ClyTkt_ValidityStartsType;


typedef enum
{   
    e_ClyTkt_DurationInHours,
    e_ClyTkt_DurationInMonths,
    e_ClyTkt_DurationInDays,
    e_ClyTkt_DurationRFU,
}e_ClyTkt_TicketDurationType;


typedef struct
{   
    e_ClyTkt_TicketDurationType e_TicketDurationType;
    clyTkt_BYTE uc_DurationValue; // Hours from 0 to 31 ,Months from 0 to 31 , Days from 0 to 31 ,
}st_ClyTkt_TicketDuration;


typedef enum
{   
    e_ClyTkt_IssuingData=0,

    e_ClyTkt_MultiRideContractRec=1, 
    e_ClyTkt_MultiRideFirstValidationRec=2,
    e_ClyTkt_MultiRideLocationRec=3,

    e_ClyTkt_SeasonPassContractRec=4, 
    e_ClyTkt_SeasonPassInitialRec=5,
    e_ClyTkt_SeasonPassValidationRe=6,

    e_ClyTkt_LastRecordType

}e_ClyTkt_TicketRecordType;

typedef enum
{
    e_ClyTkt_TktDTConv_Bit2ShortDate,
    e_ClyTkt_TktDTConv_ShortDate2Bit,
    e_ClyTkt_TktDTConv_Bit2CompactDate,
    e_ClyTkt_TktDTConv_CompactDate2Bit,
    e_ClyTkt_TktDTConv_Bit2CompactTime,
    e_ClyTkt_TktDTConv_CompactTime2Bit,
    e_ClyTkt_TktDTConv_DayInWORD2stDate,
    e_ClyTkt_TktDTConv_stDate2DayInWORD,
    e_ClyTkt_TktDTConv_HourInWORD2stDate,
    e_ClyTkt_TktDTConv_stDate2HourInWORD
}e_ClyTkt_TktDateTimeConv;

typedef struct
{
    unsigned short Year; // 20XX
    unsigned char Month; // 1-12
    unsigned char Day;   // 1-31
}st_Cly_Date;


typedef struct
{
    unsigned char sec;     /* seconds after the minute - [0,59] */
    unsigned char min;     /* minutes after the hour - [0,59] */
    unsigned char hour;    /* hours since midnight - [0,23] */
}st_Cly_Time;

typedef struct
{
    unsigned short Year;
    unsigned char Month;
}st_Cly_DateShort;

typedef struct
{
    st_Cly_DateShort stShortDate;
    st_Cly_Date st_Date;
    st_Cly_Time st_Time;
}st_Cly_DateAndTime;


///   get sign of data   
typedef e_ClyTkt_ERR (*SIGN_CALLBACK)(clyTkt_BYTE * dataIN,/// data IN
                                      clyTkt_WORD datalen,/// data len IN
                                      ClyTkt_SignType type,/// type of sign (8,16,32 bits)IN
                                      clyTkt_BYTE* outsign);/// result sign OUT

/// converting from WORD to struct (short date,compact date,compact time)
/// or from struct to WORD,convertation by last parametr
/// not used structs parametrs will be 0
/// short date:
///     number of month since January first ,1997(being date 0)
///     Last complete year is 2039
/// compact date:
///     number of days since January first , 1997 (being date 0)
///     Last complete year is 2040
/// compact time:
///     duration,or time of the day:
///     0-287 time since 0:00, in 5mn units(1/12 of an hour)
typedef e_ClyTkt_ERR (*TIME_DATE_CALLBACK)(
                                clyTkt_WORD *date_timeINOUT,///short of time/date INOUT
                                st_Cly_DateShort *shdateINOUT,/// short date struct INOUT
                                st_Cly_Time *comptimeINOUT,/// compact time struct INOUT
                                st_Cly_Date *compdateINOUT,///compact date struct INOUT
                                e_ClyTkt_TktDateTimeConv Convtype);/// type of convertation

///   fill the struct date&time(par 1) by struct com date(par 3) plus number of days/hours
///   or fill the number of days/hours by struct date&time(par 1) minus struct com date(par 3)
typedef e_ClyTkt_ERR (*DATE_PLUS_H_D_CALLBACK)(
                            st_Cly_DateAndTime *st_date_timeINOUT,/// date & time struct INOUT
                            clyTkt_WORD *DayOrHourINOUT,/// number of days or hours (by next parametr) in short INOUT
                            st_Cly_Date *TSCdateIN,/// struct of type compact date IN
                            e_ClyTkt_TktDateTimeConv Convtype);///   type of convertation
///////////////////////////////////////
//         TICKET STRUCTS
///////////////////////////////////////



typedef struct
{
    clyTkt_BYTE uc_TC_KeyIndex; // can take 4 values: 0 to 3
    e_ClyTkt_TicketTariffAppType st_Tariff; //Multi-ride ticket (0, 1 or 2 ) Season pass (4, 5 or 6 )
    clyTkt_BYTE uc_TC_Provider; //Operator identifier (service provider) (o to 127)
    clyTkt_BYTE uc_TC_Profile; //Customer profile
    clyTkt_BYTE uc_ReloadCount; //Reloading counter (0 to 15 )
}st_ClyTkt_TktCntrctCmmnData;


/*typedef struct
{
    unsigned short Year;
    unsigned char Month;
}st_Cly_DateShort;
*/


typedef struct
{
    clyTkt_BYTE uc_TMC_RoutesSystem; //Identifier (1 to 255) of the routes system where the ticket is valid.
    clyTkt_BYTE uc_TCM_FareCode; //Fare Code for each trip, within TMC_RoutesSystem
}st_TMC_TicketValiditySpatialFareCode;


typedef struct
{
    clyTkt_BYTE uc_TMC_Origin; //Identifier of the origin station, within TC_Provider
    clyTkt_BYTE uc_TCM_Destination; //Identifier of the destination station, within TC_Provider
}st_TMC_TicketValiditySpatialPoint2Point;


typedef struct
{
    clyTkt_WORD ush_ContractTypelD; //Identifier of the type of the predefined contract. 
}st_TMC_TicketValiditySpatialPredefined;


typedef union
{
    st_TMC_TicketValiditySpatialFareCode s_ValiditySpatialFareCode;
    st_TMC_TicketValiditySpatialPoint2Point s_ValiditySpatialPoint2Point;
    st_TMC_TicketValiditySpatialPredefined s_ValiditySpatialPredefined;
}union_TMC_TicketValiditySpatial;


typedef struct
{
    st_Cly_DateShort st_TMC_SaleDate; //Issuing month (also month of start of validity) Number of months since January 1\1997 (being date 0). 
    clyTkt_BYTE uc_TMC_ValidityJourneys; //Intitial number of trips (1 to 31)
    union_TMC_TicketValiditySpatial union_TMC_ValiditySpatial; //Spatial validity
}st_ClyTkt_TicketMultiRideContractRec;


typedef struct
{
    clyTkt_BYTE uc_TMF_ServiceProvider; //First validation operator or 0
    st_Cly_Date st_TMF_DateStamp; //First validation date. Number of days since January 1 \ 1997 (being date 0)
    st_Cly_Time st_TMF_TimeStamp; //First validation time. Time since 0:00, in 5mn units (1/12 of an hour).
    clyTkt_WORD ush_TMF_LocationStamp; //First validation location (within TMF _ServiceProvider). 
    clyTkt_BYTE uc_TMF_TotalJourneys; //Remaining number of allowed trips after the current one.When it reaches 0, the ticket is no more valid. 
    e_ClyTkt_Direction e_TMF_Direction; //Direction of the trip: 0: Forward (origin to destination).1: Backward (destination to origin). 
    unsigned short us_TMF_Sig;//signature on 2 bytes to validate TMF_TotalJourneys 
}stClyTktTicktMltiRideFrstVldtnRc;


typedef struct
{
    clyTkt_BYTE uc_FirstFlag;
    clyTkt_WORD ush_TML_LocationId; //Location of last validation (within TMLL_Service Provider) . 
    clyTkt_BYTE uc_TMLL_ServiceProvider; //Operator of last validation
    st_Cly_Time st_TimeStamp; //Time of last validation
}st_ClyTkt_TicketMultiRideLocationRec;

typedef struct
{
    clyTkt_BYTE uc_FirstFlag;
    clyTkt_WORD ush_TML_LocationId; //Location of last validation (within TMLL_Service Provider) . 
    clyTkt_BYTE uc_JourneysBck;
    clyTkt_WORD ush_SignatureBkp; //Time of last validation
}st_ClyTkt_TktMltLocationRecBackUp;
typedef union
{
    st_ClyTkt_TktMltLocationRecBackUp st_TicketMultiRideLocationRecBackUp;
    st_ClyTkt_TicketMultiRideLocationRec st_TicketMultiRideLocationRec;
}union_ClyTkt_TicketMultiRideLocationRec;
 

typedef struct
{
    st_ClyTkt_TicketMultiRideContractRec st_TicketMultiRideContractRec;
//  st_ClyTkt_TicketMultiRideFirstValidationRec st_TicketMultiRideFirstValidationRec;
    stClyTktTicktMltiRideFrstVldtnRc st_TicketMultiRideFirstValidationRec;
    union_ClyTkt_TicketMultiRideLocationRec union_TicketMultiRideLocationRec;
}st_ClyTkt_MultiRideTicket;


typedef struct
{
    clyTkt_BYTE uc_TSC_RoutesSystem; //Identifier (1 to 255) of the routes system where the ticket is valid. 
    clyTkt_WORD ush_TSC_ValidityZones; //One bit set per accessible zone within TMC_RoutesSystem: bO to b11 Zones 1 to 12. 
}st_TSC_ValiditySpatialAreaSP;


typedef struct
{
    clyTkt_BYTE uc_TSC_RoutesSystem; //Identifier (1 to 255) of the routes system where the ticket is valid. 
    clyTkt_BYTE uc_TSC_Origin; //Identifier of the origin station, within TSC_RoutesSystem
    clyTkt_BYTE uc_TSC_Destination; //Identifier of the destination station, within TSC_RoutesSystem
}st_TSC_ValiditySpatialPoint2PointSP;


typedef struct
{
    long l_TSC_ContractTypelD; //Identifier of the type of the season pass predefined contract.
}st_TSC_ValiditySpatialPredefinedSP;


typedef union
{
    st_TSC_ValiditySpatialAreaSP s_ValiditySpatialAreaSP;
    st_TSC_ValiditySpatialPoint2PointSP s_ValiditySpatialPoint2PointSP;
    st_TSC_ValiditySpatialPredefinedSP s_ValiditySpatialPredefinedSP;
}union_ValiditySpatial;


typedef struct
{
    e_ClyTkt_ValidityStartsType e_TSC_Sliding; //0: Validity starts at TSC_Date 1: Validity starts at date of first use 
    st_Cly_Date st_TSC_Date; //If TSC_Sliding=O: First day of validity of the contract. If TSC_Sliding=1: Issuing Date.(Number of days since January 1 \ 1997 (being date 0))
    st_ClyTkt_TicketDuration st_TicketDuration; //Duration of validity
    union_ValiditySpatial union_TSC_ValiditySpatial; //Spatial validity
}st_ClyTkt_TicketSeasonPassContractRec;


//If TSC_ ValidityDuration is in hours: Time of start of validity at TSC_Date.Else:If TSC_Sliding is 0:Ignored. Else: Date of first use, as a number of days (0 to 2047) since TSC_Date. 
typedef union
{
    clyTkt_WORD ush_TSI_Start; 
    ///st_Cly_Date st_TSC_Date;
    st_Cly_Time stTime;
}union_ClyTkt_TSI_Start;


typedef struct
{
    st_Cly_DateAndTime stTSI_start;
    ///union_ClyTkt_TSI_Start union_Start; //If TSC_ ValidityDuration is in hours: Time of start of validity at TSC_Date.Else:If TSC_Sliding is 0:Ignored. Else: Date of first use, as a number of days (0 to 2047) since TSC_Date. 
}st_ClyTkt_TicketSeasonPassInitialRec;


typedef struct
{
    ClyTkt_BOOL TSL_IsVirginFlag; //0: ticket has been used at least once
    clyTkt_WORD ush_TSLL_Locationld; //Location of validation (within TSLL_ServiceProvider). 
    clyTkt_BYTE uc_TSLL_ServiceProvider; //Operator identifier within network
    st_Cly_Time st_TSLL_TimeStamp; //Validation time
    clyTkt_WORD ush_TSSL_DateOffset; //offset of the validation day, from the start day of validity, in number of days (0 to 511)
}st_ClyTkt_TicketSeasonPassValidationRec;


typedef struct
{
    st_ClyTkt_TicketSeasonPassContractRec st_TicketSeasonPassContractRec;
    st_ClyTkt_TicketSeasonPassInitialRec st_TicketSeasonPassInitialRec;
    st_ClyTkt_TicketSeasonPassValidationRec st_TicketSeasonPassValidationRec;
}st_ClyTkt_TicketSeasonPassTicket;

typedef union
{
    st_ClyTkt_MultiRideTicket st_MultiRideTicket;
    st_ClyTkt_TicketSeasonPassTicket st_TicketSeasonPassTicket;
}union_ClyTkt_TicketAppType;


typedef struct
{
    TKT_SN_TYPE ucp_Sn;
    st_ClyTkt_TktCntrctCmmnData st_TicketContractCommonData;
    union_ClyTkt_TicketAppType union_TicketAppType;
}struct_ClyTkt_Ticket;


typedef enum
{
    e_ClyTkt_CTS256B=0,

    e_ClyTkt_LastMediaType

}e_ClyTkt_TicketMediaTypes;

/*
//////////////////////////////////////////////////////////////////////////////
                           API   FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
*/
typedef struct
{
    ClyTkt_BOOL b_IsKeyExist;
    clyApp_TktKeyType ucp_Key;
}st_ClyTkt_KeyInfo;

 
                /***************************************/
                // Init Interface
                /***************************************/
// Init Interface - Get interface keys
e_ClyTkt_ERR  CLYTKT_STDCALL v_ClyTkt_InitInterface(st_ClyTkt_KeyInfo st_KeyInfoArr[MAX_TIKET_KEYS],
                                            SIGN_CALLBACK sprocIN,
                                            TIME_DATE_CALLBACK dtprocIN,
                                            DATE_PLUS_H_D_CALLBACK hprocIN);



                /***************************************/
                // Get Ticket record Address 
                /***************************************/
// Translate Ticket Record type to physical Address
e_ClyTkt_ERR  CLYTKT_STDCALL e_ClyTkt_GetTktRecAddress( e_ClyTkt_TicketMediaTypes e_TicketMediaTypes,//[IN] Ticket Media Types
                                                e_ClyTkt_TicketRecordType e_TicketRecordType,//[IN] Record Name
                                                clyTkt_BYTE *ucp_WordStartAddInCard,//[OUT] Record start physicl address
                                                clyTkt_BYTE *ucp_WordEndAddInCard,//[OUT] Record end physicl address
                                                clyTkt_BYTE *ucp_WordStartOffsetInBuff,//[OUT] the record offset in the translated buffer
                                                clyTkt_BYTE *ucp_WordRecLenInBuff);//[OUT] the len of the record in the translated buffer



                /***************************************/
                // Convert ticket struct to binary Buff 
                /***************************************/
// Convert Ticket struct to Binary buffer 
e_ClyTkt_ERR  CLYTKT_STDCALL e_ClyTkt_ConvertTktSt2BinBuff( struct_ClyTkt_Ticket *struct_Ticket,//[IN] Ticket struct input for translation
                                                    CalypsoBinTktType ucp_BinBuffOut);//[OUT] Binary buff result


                /***************************************/
                // Convert binary Buff to ticket Struct 
                /***************************************/
// Translate Ticket struct Binary buffer
e_ClyTkt_ERR  CLYTKT_STDCALL e_ClyTkt_ConvertBinBuff2TktSt( struct_ClyTkt_Ticket *struct_Ticket,//[OUT] Ticket struct output 
                                                    CalypsoBinTktType ucp_BinBuffIn,//[IN] Binary buff to translate
                                                    CalypsoBinTktType ucp_BinBuffOut);//[OUT] Binary buff plantex result




#if SHOW_BIT2BYTE_CONVERT

                /***************************************/
                // Convert binary Buff to ticket Struct 
                /***************************************/
// Translate Ticket struct Binary buffer
e_ClyTkt_ERR  CLYTKT_STDCALL e_Debug_ConvertBinBuff2TktSt( struct_ClyTkt_Ticket *struct_Ticket,//[OUT] Ticket struct output 
                                                           CalypsoBinTktType ucp_BinBuffIn);//[IN] Binary buff to translate
#endif

#ifdef  __cplusplus 
 }
#endif


#endif
