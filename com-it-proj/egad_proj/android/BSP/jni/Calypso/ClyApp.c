#define ENABLE_COMM
#define linux 1
#include <os_def.h>
#include <Core.h>

#if defined(CORE_SUPPORT_SMARTCARD) && defined(CORE_SUPPORT_CALYPSO)

#include <ClyApp.h>
#include <ClyTktOs.h>
#include <ClyCrdOs.h>
#include <ClySamOs.h>
#include <ClySessn.h>
#include <Bit2Byte.h>
#include <ClyAppApi.h>

#include <AppProtocol.h>


#define SALE_DATE_2050_YEAR     2019
#define SALE_DATE_2050_MONTH    1
#define SALE_DATE_2050_DAY      1
#define USE_EGGED_LONG_DEVICE_NUMBER
#define CALYPSO_BYPASS_ENCRYPTION
#define MAX_LOAD_RETRIES 3


#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif

#ifdef WIN32
	#pragma warning(disable : 4996) // CRT Secure - off
	#ifdef BUILD_ZABAD_TOOL

		void UIPrintHex     (const char* lbl, const unsigned char* buff, int size/*max is 90*/);
		int  UIprintf       (const char *pAnyData,...);
		void UISetProgress  (int Progress);
		#define printf UIprintf
	#else
		#define UIPrintHex
		#define UISetProgress
		#define UIprintf
	#endif
#endif

const st_Cly_Date stSlidingZeroDate={2041, 11, 9};//this is the start date we put when we want to load sliding (equal to "no date")


///   global data structure : flags , states , members , errors

//////////////////////////////////////////////////////////////////////////////
//vars defined in clyappapi
extern long g_time_zone_bias_minutes;
extern TR_st_Parameters g_Params;
extern St_ChngTripcmd g_TripInfo;

//////////////////////////////////////////////////////////////////////////////
//  us_GetSaleDeviceNumber
//  Return the sale device number of in calypso format 1-4095
//////////////////////////////////////////////////////////////////////////////
unsigned short us_GetSaleDeviceNumber(unsigned long lv_DeviceNumber);


//static eCalypsoErr read_convert_err=e_ClyApp_Ok;
void  b_Internal_UpdateSpecialEvent(int iRec, void* StOut);

//////////////////////////////////////////////////////////////////////////////
//                           FUNCTION DECLARATION
//////////////////////////////////////////////////////////////////////////////

void v_PrintVal2(void *vp_Val,char* format,char* Str2Print);

eCalypsoErr e_Internal_OpenSecureSession( St_clySam_KIF_And_KVC  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
  e_clyCard_KeyType KeyType, //[IN] Key Type to use for the session
  st_ClyApp_EnvAndHoldDataStruct *stp_EnvAndHoldDataStruct);//[OUT] the Event Data Struct read - if read not requested send NULL



////////////////////////////////////////////////////////////////////////////
//
// Virtual card write suuport
// Overide the original ClyCrdOs low level API for virtual write support
//
////////////////////////////////////////////////////////////////////////////

static eCalypsoErr                      ClyApp_Virtual_OpenSecureSession    (St_clySam_KIF_And_KVC  St_KIF_And_KVC,e_clyCard_KeyType KeyType,st_ClyApp_EnvAndHoldDataStruct *stp_EnvAndHoldDataStruct);
static RESPONSE_OBJ* CLY_CARD_STDCALL   ClyApp_Virtual_CloseSecureSession   (e_7816_DEVICE CardReaderId,e_7816_DEVICE SamReaderId,clyCard_BOOL b_IsRatifyImmediatly);
static RESPONSE_OBJ* CLY_CARD_STDCALL   ClyApp_Virtual_UpdateRecord         (e_7816_DEVICE ReaderId,clyCard_BYTE RecNum,e_clyCard_FileId FileToSelect, clyCard_BYTE Len2Update, clyCard_BYTE *RecData2Update);
static RESPONSE_OBJ* CLY_CARD_STDCALL   ClyApp_Virtual_ReadRecord           (e_7816_DEVICE ReaderId,clyCard_BYTE RecNum,e_clyCard_FileId FileToSelect,clyCard_BYTE Len2Read,clyCard_BYTE RecDataOut[REC_SIZE], clyCard_BYTE ForceRead);
static RESPONSE_OBJ* CLY_CARD_STDCALL   ClyApp_Virtual_Invalidate           (e_7816_DEVICE ReaderId);



//////////////////////////////////////////////////////////////////////////////
// DEFINES
//////////////////////////////////////////////////////////////////////////////
#define INTERNALe_7816_LAST e_7816_LAST


#define OK_VAL 0x5555
#define IS_INTERFACE_INIT() (st_Static_StateMachine.i_IsInteraceInit == OK_VAL)
//#define IS_CARD_EXIST() (st_Static_StateMachine.e_TransactionState !=e_clyApp_NoCardExist)

#define LRC_START_VAL 0xA5

#define IS_RES_OBJ_OK(Obj)            (Obj && Obj->sw1_sw2[0] == 0x90 && Obj->sw1_sw2[1] == 0)
#define IS_SAM_NOT_LOCKED(Obj)        (Obj && Obj->sw1_sw2[0] == 0x69 && Obj->sw1_sw2[1] == 0x85)
#define IS_RES_RECORD_NOT_FOUND(Obj)  (Obj && Obj->sw1_sw2[0] == 0x6A && Obj->sw1_sw2[1] == 0x83)

#define CHECK_INERFACE_INIT() if(! IS_INTERFACE_INIT()) return e_ClyApp_InterfaceNotInitErr;
#define CHECK_CARD_EXIST() if(st_Static_StateMachine.e_TransactionState ==e_clyApp_NoCardExist ) return e_ClyApp_NoCardErr;

// define is cardin behaving 
#ifdef NOT_RE_SELECT_AT_CARDIN  // the card not re select each iscardin

  #define CHECK_SESSION_OPEN() //if(!(st_Static_StateMachine.e_TransactionState == e_clyApp_SessionOpenOk) ) return e_ClyApp_SessionNotOpenErr;
#else
  #define CHECK_SESSION_OPEN() if(!(st_Static_StateMachine.e_TransactionState == e_clyApp_SessionOpenOk) ) return e_ClyApp_SessionNotOpenErr;
#endif	


// Moved to h file #define ISRAEL_COUNTRY_ISO_IDENTIFICATION  0x376
#define ENV_EVENT_LOCATIOM_FIELD_BIN_COUNT 7
#define EVENT_START_OPTIONAL_DATA_OFFSET 123
#define ENV_VERSION_NUM 0
#define EVENT_VERSION_NUM 0
#define MAX_DATA_BIT_LEN (50*8) // 50 bytes
#define MINUTS_IN_ONE_DAY 1440 // 24*60
#define MINUTS_IN_ONE_HOUR  3600// 60*60  TODO WTF!???
#define SEC_IN_ONE_DAY 86400  // 24*60*60
#define SEC_IN_ONE_WEEK 604800L //7*24*60*60
#define SEC_IN_HALF_HOURS 1800 //30*60
#define SEC_IN_HOURS 3600 //60*60
#define DAYS_FROM_1997_TO_2000 1095 //365*3
#define SEC_FROM_1997_TO_2000  94608000L //1095*24*60*60
#define DAYS_IN_REGULAR_YEAR 365
#define DAYS_IN_LEAP_YEAR ( DAYS_IN_REGULAR_YEAR +1 )
#define IS_LEAP_YAER_(i) (i%4 == 0)
#define FEBRUARY_ 2
#define GET_NUM_OF_DAYS_IN_MONTH(year, month) ((month == FEBRUARY_) && (IS_LEAP_YAER_(year)) ? (uc_ArrDaysInMonth_[month] + 1) : (uc_ArrDaysInMonth_[month]))
#define BYTE_COUNT_TO_UPDATE_STATR_DATE 3
#define COUTER_SIZE 3 // each counter is three bytes size and all counters are reside is the counter record number 1
#define COUNTER_OFFSET(RecNum) ((RecNum-1)*COUTER_SIZE)
#define TKT_CONTRACT_LOAD_KEY 0
#define MAX_RELOAD_COUT 15
#define IS_MULTI_TKT( Tariff ) ( Tariff == e_ClyTkt_TariffMultiRideFareCodeTicket || Tariff == e_ClyTkt_TariffPointToPointMultiRideTicket || Tariff == e_ClyTkt_TariffPredefinedMultiRideTicket )
#define IS_SEASON_TKT( Tariff ) ( Tariff == e_ClyTkt_TariffAreAeasonPass || Tariff == e_ClyTkt_TariffPointToPointSeasonPass || Tariff == e_ClyTkt_TariffPredefinedSeasonPass )
#define FULL_TKT_BYTE_COUNT 32


#define IS_CL_READER( i_ReaderId ) ( i_ReaderId > e_7816_CONTACTLESS  && i_ReaderId < e_7816_LAST )


#define IS_TICKET_CARD(e_Type) (e_Type == e_7816_Cly_CTS256B)
#define DEBUG_FLIP_TKT_DATA 1
#define GETBIT(var,bit) (((var)>>(bit))&1)
#define SETBIT(var,bit) ((var)|=(1<<(bit)))
#define CLRBIT(var,bit) ((var)&=(~(1<<(bit))))

#define IS_CONTRACT_LIST_ON(bitmap, recnum)  (bitmap & (1<<(recnum-1)))

///////////////////////////////////////////////////////
//INTERNAL TYPES
///////////////////////////////////////////////////////


typedef enum
{
  e_ClyApp_DateCompactType,
  e_ClyApp_DateReverseType,
  e_ClyApp_DatefType,
  e_ClyApp_TimeCompactType,
  e_ClyApp_TimeRealType,

}e_ClyApp_StDateType;


typedef enum
{
  e_ClyApp_ContractNoLongerValid,
  e_ClyApp_ContractValidButNotInThisPeriod,
  e_ClyApp_ContractValid,

}e_clyApp_ValidityType;


///////////////////////////////////////////////////////
//INTERNAL STRUCTS
///////////////////////////////////////////////////////

typedef struct
{
  st_ClyApp_EnvAndHoldDataStruct st_EnvRec;
  st_ClyApp_CardContractRecord  st_ContractRecArr[MAX_CONTRACT_COUT+1];//the counter is included in the contract
  st_clyApp_CardEventDataStruct st_EventRecArr[MAX_EVENT_COUT+1];
  st_clyApp_CardEventDataStruct st_SpecialEventRecArr[SPECIAL_EVENTS_COUNT+1];
  st_clyApp_ContractListStruct  st_ContractList;
}st_Internal_CardTransactionStData;

typedef union
{
  st_Internal_CardTransactionStData st_Card; // card struct data
  struct_ClyTkt_Ticket st_Ticket; // ticket struct data
}union_Internal_TransactionStData;

typedef struct
{
  CalypsoFileType ucp_EnvRec;
  CalypsoFileType ucp_ContractRecArr[MAX_CONTRACT_COUT+1];
  CalypsoFileType ucp_CounterRec;
  CalypsoFileType ucp_EventRecArr[MAX_EVENT_COUT+1];
  CalypsoFileType ucp_SpecialEventRecArr[SPECIAL_EVENTS_COUNT+1];
  CalypsoFileType ucp_ContractList;//10/2011
}st_Internal_CardTransactionBinData;

typedef union
{
  st_Internal_CardTransactionBinData st_CardBinData; // card struct data
  CalypsoBinTktType ucp_TktBinData; // ticket struct data
}union_Internal_TransactionBinData;

typedef struct
{
  union_Internal_TransactionStData union_TransactionStData;//transaction bin data - to send to user
  union_Internal_TransactionBinData union_TransactionBinData; // not part of the user output data
}st_Internal_CardAndTktData;


typedef struct
{
  clyApp_BOOL b_IsTktDataExist;
  clyApp_BOOL b_IsEnvRecExist;
  clyApp_BOOL b_IsTktRecoveryLoadDataExist;
  clyApp_BOOL b_IsContractRecExistArr[MAX_CONTRACT_COUT+1];
  eCalypsoErr isContractConversionValid[MAX_CONTRACT_COUT+1];
  clyApp_BOOL isContractAuthOk[MAX_CONTRACT_COUT+1];//Yoni 22/2/10 contract authentication ok or not
  clyApp_BOOL b_IsCounterRecExistArr[MAX_CONTRACT_COUT+1];
  eCalypsoErr isCounterConversionValid[MAX_CONTRACT_COUT+1];
  clyApp_BOOL b_IsEventRecExistArr[MAX_EVENT_COUT];//next record to be written in the end of the transaction
  clyApp_BOOL b_IsSpecialEventRecExistArr[SPECIAL_EVENTS_COUNT+1];//next record to be written in the end of the transaction
  clyApp_BOOL b_IsContractListExist;
  //Data read from the card - struct format + binary format
  st_Internal_CardAndTktData st_CardAndTktData;
  //Next priority list - for the next event file
  e_ClyApp_CardPriorityType e_NextEventPriorityListArr[MAX_CONTRACT_COUT];
  clyApp_BOOL b_IsPriorityListNeedUpdate;
  st_ClyApp_TktRecoveryDate st_TktRecoveryDate;
}st_cly_TransactionDataState;


typedef struct
{
  clyApp_TktKeyType Key;
  unsigned char Lrc;
  int i_IsKeyExist;// have to be 0xaaaa
}St_KeyInfo;

typedef enum
{
  e_clyApp_NoCardExist,
  e_clyApp_CardExist,
  e_clyApp_SessionOpenOk,
  e_clyApp_SessionCloseOk,
}e_clyApp_TransactionSate;

typedef struct
{
  unsigned char listLen;
  unsigned char OperatorsList[sizeof(char)];

}st_Internal_DepositRefundOperatorsList;

//module state machine
typedef struct
{
	int i_IsInteraceInit;       // need to be 0xabcd if init
	e_7816_DEVICE             SamReaderId;            // SAM reader id - common information for all readers
	e_7816_DEVICE             CardReaderId;           // Reader id
	int             SamUartId;              // SAM uart (hardware) only
	char b_IsCardEmptyEnv;
	e_ClyApp_SamType e_SamType;//Sam Type
	e_clyApp_TransactionSate e_TransactionState; // NoCardExist -> CardExist -> SessionOpen -> SessionClose
	e_7816_CardType e_7816CardType;//type of card cd_light ticket or cd97 or cd97bx or another
	e_ClyApp_CardType e_AppCardType;
	unsigned char ucp_CardSn[10];//serial number in the 4 & 8 byte format
	unsigned char uc_CardSnLen;//Serial Number len
	char c_ContractRecNumberForUse;
	char b_WasTktRehabilitate;
	clyApp_BOOL b_FreeRecFoundForLoad;//is session open - check befor operation which need open session
	clyApp_BOOL b_isManufacturerModeTkt;//is session open - check befor operation which need open session
	st_Cly_DateAndTime st_EventDataTimeFirstStampOfContract2Use; // Stamp OfContract 2 Use
	St_clyCard_Ratification St_Ratif;//if retification data exist
	clyApp_BOOL b_retification;//is session open - check befor operation which need open session
	st_cly_TransactionDataState st_TransactionData; //which data already exist

	///1020A
	st_ClyApp_Callback st_UserCallbacks; // user callbacks

	st_Internal_DepositRefundOperatorsList st_DepositRefundOperatorsList;

	// Virtial Card rite support types
	st_ClyApp_TransactionVirtualData        CardVirtualImage;
	e_CardWriteMode                         CardWriteMode;

	//binary snapshort of card after cancelling stored value contract, before loading new one (for calypso transaction file). 24/7/13
	struct 
	{
		unsigned char uc_CancelledContractRecNum;
		st_ClyApp_CardTransactionBinData ov_CardSnapshotAfterStoredValueCancel;
	}st_CanceledStoredValueSnapshot;

}st_clyApp_StateMachine;

typedef struct
{
  clyApp_BYTE uc_EnvApplicationVersionNumber; //Environment structure version number
  short sh_EnvCountryld; //Country ISO identification, '376'h for Israel
  clyApp_BYTE uc_Envlssuerld; //Ticketing application issuer identifier
  unsigned long ul_EnvApplicationNumber; //Card Application Number
  short sh_EnvlssuingDate; //Transport application issuing date (Number of days since January 1 st, 1997 (being date 0))
  short sh_EnvEndDate; //Expiration date of the ticketing application (Number of days since January 1st, 1997 (being date 0))
  clyApp_BYTE uc_EnvPayMethod; //Allowed contracts payment methods
  unsigned long ul_HolderBirthDate; //Holder birth date
  short sh_HolderCompany; //Short holder company identifier
  unsigned long ul_HolderCompanylD; //National holder company identifier
  unsigned long ul_HolderldNumber; //Holder identifier, within company
  clyApp_BYTE uc_HoiderProf1Code; //First profile code and validity ending date
  short sh_HoiderProf1Date; //Short holder company identifier
  clyApp_BYTE uc_HoiderProf2Code; //First profile code and validity ending date
  short sh_HoiderProf2Date; //Short holder company identifier
  clyApp_BYTE uc_HolderLanguage;//2 bits . Yoni 11.7.07
  clyApp_BYTE uc_HoiderRFU; //First profile code and validity ending date
}st_Internal_EnvAndHoldBinDataStruct;



typedef struct
{
  clyApp_BYTE uc_EventVersionNumber; //Event structure version number
  clyApp_BYTE uc_EventServiceProvider; //Provider identifier
  clyApp_BYTE uc_EventContractPointer; //Number of the contract used for this event (1 to 8, 0 if irrelevant)
  clyApp_BYTE uc_EventCode; //Type of the event
  unsigned long ul_EventDateTimeStamp; //Date and time of event (Number of seconds elapsed since 1/1/1997 at 0:00 GMT, (allows coding all instants until year 2031))
  clyApp_BOOL b_EventIsJourneylnterchange; //Set to 0 for a first boarding, set to 1 if interchange
  unsigned long ul_EventDataTimeFirstStamp; //Time of first boarding
  clyApp_BYTE uc_EventBestContractPriority1;//Priority of contract in Contracts file record #1 to #8
  clyApp_BYTE uc_EventBestContractPriority2;//Priority of contract in Contracts file record #1 to #8
  clyApp_BYTE uc_EventBestContractPriority3;//Priority of contract in Contracts file record #1 to #8
  clyApp_BYTE uc_EventBestContractPriority4;//Priority of contract in Contracts file record #1 to #8
  clyApp_BYTE uc_EventBestContractPriority5;//Priority of contract in Contracts file record #1 to #8
  clyApp_BYTE uc_EventBestContractPriority6;//Priority of contract in Contracts file record #1 to #8
  clyApp_BYTE uc_EventBestContractPriority7;//Priority of contract in Contracts file record #1 to #8
  clyApp_BYTE uc_EventBestContractPriority8;//Priority of contract in Contracts file record #1 to #8
  clyApp_BYTE uc_EventLocation; //Bitmap
  clyApp_BYTE uc_EventExtension; //Bitmap
}st_Internal_CardEventBinDataStruct;

//////////////////////////////////////////////////////////////////////////////
//GLOBAL
//////////////////////////////////////////////////////////////////////////////

#ifdef  VALIDATOR_DEVICE
// Here is manual allocation of external RAM to shadow copy of RavCard structure,
  // required for Calypso.
  // Next free area of external RAM is CORE_FLEX_CS3_ADDRESS + sizeof(st_AllAzmashRecords) + sizeof(st_clyApp_StateMachine);
static st_clyApp_StateMachine *p_st_Static_StateMachine=(st_clyApp_StateMachine*)(&CORE_FLEX_CS3_ADDRESS + sizeof(st_AllAzmashRecords));
#define st_Static_StateMachine (*p_st_Static_StateMachine)
#else
static st_clyApp_StateMachine st_Static_StateMachine;

#endif

static st_FieldDescriptor stEnvDesc[17];//static st_FieldDescriptor stEnvDesc[16];//Yoni changed 11.7.07
st_FieldDescriptor stEventMandatoryDataDesc[16];

static st_Internal_EnvAndHoldBinDataStruct st_Global_EnvAndHoldBinDataStruct;
st_Internal_CardEventBinDataStruct st_Global_CardEventBinDataStruct;

static const unsigned char uc_ArrDaysInMonth_[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static union_ClyApp_ContractRecord Global_union_ContractRecord;

//special events  //  transfer ticket
#if 0
static struct
{
  st_clyApp_CardEventDataStruct m_pSpecialEvents[SPECIAL_EVENTS_COUNT];
  clyApp_BOOL m_bReadSpecialEvents;
} SpecialEvent;
#endif
//////////////////////////////////////////////////////////////////////////////
//                            INTERNAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static unsigned short us_RemainingMinutesInFirstTrip(const st_clyApp_OptionalContractData* pContractOptionalData);
static eCalypsoErr e_ClyApp_WaitCard(int retries,int *p_IsCardReseleted);
static eCalypsoErr e_ClyApp_VerifyLoading(st_clyApp_CardEventDataStruct *p);
static void e_ClyApp_ShiftTransactionEvents(void);

static clyApp_BOOL b_CheckTransferTimeLimit(
  const st_clyApp_OptionalContractData* pContractOptionalData
  ,const st_clyApp_CardEventDataStruct* pSpEvent
  ,const st_Cly_DateAndTime *st_CurrentDateAndTime
  ,unsigned short* pRemainingMinutes
  );
void v_FlipBytes(unsigned char* ptr,unsigned char len)
{
  unsigned char tmp,i;
  for(i=0;i<len/2;i++)
  {
    tmp = ptr[i];
    ptr[i] = ptr[len - i-1];
    ptr[len - i-1] = tmp;
  }
}

static eCalypsoErr  e_Internal_Read(e_ClyApp_CardType e_CardType,//[IN]type - card \ ticket
  clyCard_BYTE RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
  e_clyCard_FileId FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
  void* StOut, //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
	eCalypsoErr* pOutConverError,
	clyCard_BYTE ForceRead);

#ifdef ENABLE_COMM
eCalypsoErr UpdateStateMachine(St_ClyApp_SmartCardData *pScData);
#endif
//===========================================
//               Date And Time functions
//===========================================

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:  l_ConvertStTime2SecFrom2000
//
// Discription: Converts time struct to seconds from 20000
//
//
// return: Sec From 2000. if errur return -1
//
// LOGIC:
//////////////////////////////////////////////////////////////////////////////

long l_Internal_ConvertStTime2SecFrom2000 (
  const st_Cly_DateAndTime *stp_TimeIn)// [IN] time struct needs to be converted

{
  st_Time st_TimeIn={0};
  if(stp_TimeIn)
  {
    st_TimeIn.ui_Year = stp_TimeIn->st_Date.Year;   ///* Year   : for example 20000  */
    st_TimeIn.uc_Month = stp_TimeIn->st_Date.Month; ///* Month.     [1-12] */
    st_TimeIn.uc_Day = stp_TimeIn->st_Date.Day;     ///* Day.       [1-31] */
    st_TimeIn.uc_Hour = stp_TimeIn->st_Time.hour;   ///* Hours.     [0-23] */
    st_TimeIn.uc_Minute = stp_TimeIn->st_Time.min;  ///* Minutes.   [0-59] */
    st_TimeIn.uc_Second = stp_TimeIn->st_Time.sec;  ///* Seconds.   [0-59] (1 leap second) */
    return l_TimeH_ConvertStTime2SecFrom2000 (&st_TimeIn);// [IN] time struct needs to be converted
  }
  return l_TimeH_ConvertStTime2SecFrom2000 (0);// [IN] time struct needs to be converted

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:  b_Convert2000Sec2StTime
//
// Discription: Convert secondes from 2000 input to Struct Time
//
//
// return: clyApp_TRUE - success, clyApp_FALSE - error
//
// LOGIC:
//////////////////////////////////////////////////////////////////////////////

clyApp_BOOL b_Internal_Convert2000Sec2StTime (
  const unsigned long l_SecIn,//[IN] secondes since 2000
  st_Cly_DateAndTime *stp_TimeOut)// [OUT] time struct needs to be filled

{
  clyApp_BOOL b;
  st_Time st_TimeOut;



  b= (clyApp_BOOL)b_TimeH_Convert2000Sec2StTime ( l_SecIn,//[IN] secondes since 2000
    &st_TimeOut);// [OUT] time struct needs to be filled

  stp_TimeOut->st_Date.Year = st_TimeOut.ui_Year ;        ///* Year   : for example 20000  */
  stp_TimeOut->st_Date.Month =    st_TimeOut.uc_Month ;       ///* Month.     [1-12] */
  stp_TimeOut->st_Date.Day = st_TimeOut.uc_Day;       ///* Day.       [1-31] */
  stp_TimeOut->st_Time.hour = st_TimeOut.uc_Hour;     ///* Hours.     [0-23] */
  stp_TimeOut->st_Time.min = st_TimeOut.uc_Minute ;   ///* Minutes.   [0-59] */
  stp_TimeOut->st_Time.sec = st_TimeOut.uc_Second;    ///* Seconds.   [0-59] (1 leap second) */

  return b;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                uc_CalcDayOfTheWeek
//
//DESCRITION:
//
//                 Calc Day Of The Week - sunday=1 to saturday=7
//
//RETURN:
//
//                unsigned short
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////

static unsigned char uc_CalcDayOfTheWeek(st_Cly_Date *st_Date)
{
  ////////////////////////////
  // START DAY 2000/1/1 = saturday
  ////////////////////////////
  unsigned int i_SumDaysSince2000=0;
  unsigned short CurrentYear=2000;
  unsigned char CurrentMonth=1;

  while(st_Date->Year > CurrentYear )
  {
    if(CurrentYear%4==0)
    {
      i_SumDaysSince2000+=DAYS_IN_LEAP_YEAR;
    }
    else
      i_SumDaysSince2000+= DAYS_IN_REGULAR_YEAR;
    CurrentYear++;
  }
  while(st_Date->Month > CurrentMonth )
  {
    i_SumDaysSince2000+=GET_NUM_OF_DAYS_IN_MONTH(st_Date->Year,CurrentMonth);
    CurrentMonth++;
  }
  i_SumDaysSince2000+=(st_Date->Day-1);

  return (i_SumDaysSince2000%7==0?7:i_SumDaysSince2000%7) ;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                l_GetDatef
//
//DESCRITION:
//                Get Datef
//                Date given as 8 BCD digits: 'yyyymmdd'h:  yyyy : year (e.g. '1968'h) , mm: month (1..12)  dd: day of the month (1..31)
//
//RETURN:
//
//                unsigned long
//LOGIC :
//////////////////////////////////////////////////////////////////////////////

static unsigned long l_GetDatef(st_Cly_Date *st_Date)
{
  long l_Datef=0,tmp;
  char str[32];

  sprintf(str,"%d",st_Date->Year);
  tmp =0;
  sscanf(str,"%x",(unsigned int*)&tmp);
  l_Datef= tmp<<16;
  sprintf(str,"%d",st_Date->Month);
  tmp =0;
  sscanf(str,"%x",(unsigned int*)&tmp);
  l_Datef|= tmp<<8;
  sprintf(str,"%d",st_Date->Day);
  tmp =0;
  sscanf(str,"%x",(unsigned int*)&tmp);
  l_Datef|= tmp;
  return l_Datef;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                st_GetDatef
//
//DESCRITION:
//                Get Datef
//                Date given as 8 BCD digits: 'yyyymmdd'h:  yyyy : year (e.g. '1968'h) , mm: month (1..12)  dd: day of the month (1..31)
//
//RETURN:
//
//                st_Cly_Date
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////

static st_Cly_Date st_GetDatef(unsigned long l_Date)
{
  st_Cly_Date Date;
  long tmp;
  char str[32];

  tmp = (l_Date &0xffff0000)>>16;
  sprintf(str,"%x",(int)tmp);
  sscanf(str,"%d",(unsigned int*)&tmp);
  Date.Year = (unsigned short)tmp;

  tmp = (l_Date &0x0000ff00)>>8;
  sprintf(str,"%x",(unsigned int)tmp);
  sscanf(str,"%d",(unsigned int*)&tmp);
  Date.Month = (unsigned char)tmp;

  tmp = (l_Date &0x000000ff);
  sprintf(str,"%x",(unsigned int)tmp);
  sscanf(str,"%d",(unsigned int*)&tmp);
  Date.Day = (unsigned char)tmp;

  return  Date;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                st_CalcEndDateByPeriod
//
//DESCRITION:
//               calc expected end date using contract start date Duration period and duration units
//RETURN:
//
//                st_Cly_Date
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////

st_Cly_Date CLYAPP_STDCALL st_CalcEndDateByPeriod(const st_Cly_Date *stp_StartDateCompact,//[IN] contract start date
  e_ClyApp_DurationType e_DurationType, //[IN] Duration period type
  unsigned char uc_DurationUnitCount) //[IN] duration units
{
  //updated by Yoni 11/2011: return (enddate - 1). monthly that starts on 1/11 ends on 30/12 and not 1/12
  //weekly that starts on tuesday ends on monday at 23:59
  //note that this is different from Egged, where the end date is the next day at 03:00

  st_Cly_DateAndTime st_DAndT;
  long l_Result;
  int MonthFrom2000;
  memset(&st_DAndT,0,sizeof(st_DAndT));
  st_DAndT.st_Time.hour = st_DAndT.st_Time.min= st_DAndT.st_Time.sec = 0;
  st_DAndT.st_Date = *stp_StartDateCompact;
  l_Result = l_Internal_ConvertStTime2SecFrom2000 (&st_DAndT);
  switch ( e_DurationType )
  {
  case e_DurationInMonths:
    //fixed by Yoni 11/2010: there was a bug in monthly contracts for December
    //count offset in month from 1/1/2000
    MonthFrom2000 = //month from 2000 = 12*years + currentmonth-1
      (st_DAndT.st_Date.Year-2000)*12  + st_DAndT.st_Date.Month -1;
    MonthFrom2000 += uc_DurationUnitCount;
    //now back to st_Date
    st_DAndT.st_Date.Year=MonthFrom2000/12+2000;
    st_DAndT.st_Date.Month = (MonthFrom2000%12)+1;
    //subtract 1  from day  (new logic on 11/2011)
    //do it by subtracting one second
    l_Result = l_Internal_ConvertStTime2SecFrom2000 (&st_DAndT);
    l_Result-=1;
    //and back to date
    b_Internal_Convert2000Sec2StTime (l_Result,&st_DAndT) ;

    break;
  case e_DurationInWeeks:
    l_Result+=uc_DurationUnitCount*SEC_IN_ONE_WEEK - 1;//minus 1 second to get the day before
    b_Internal_Convert2000Sec2StTime (l_Result,&st_DAndT) ;
    break;
  case e_DurationInDays:
    l_Result+=uc_DurationUnitCount*SEC_IN_ONE_DAY;
    b_Internal_Convert2000Sec2StTime (l_Result,&st_DAndT) ;
    break;
  case e_DurationInHalfHours:
    l_Result+=uc_DurationUnitCount*SEC_IN_HALF_HOURS;
    b_Internal_Convert2000Sec2StTime (l_Result,&st_DAndT) ;
    break;
  }


  return  st_DAndT.st_Date;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                st_CalcEndDateByTime
//
//DESCRITION:
//                Return date and time  = start date + Time to add in minuts
//RETURN:
//
//                st_Cly_DateAndTime
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////

static st_Cly_DateAndTime st_CalcEndDateByTime(st_Cly_Date *stp_StartDateCompact,//[IN] start date
  unsigned long ul_PeriodInMinutes) //[IN] Time to add in minuts 0- 1890  min
{
  st_Cly_DateAndTime st_DAndT;
  long l_Result;
  memset(&st_DAndT,0,sizeof(st_DAndT));
  st_DAndT.st_Time.hour = st_DAndT.st_Time.min= st_DAndT.st_Time.sec = 0;

  st_DAndT.st_Date = *stp_StartDateCompact;
  l_Result = l_Internal_ConvertStTime2SecFrom2000(&st_DAndT);
  l_Result+=(ul_PeriodInMinutes*60);
  b_Internal_Convert2000Sec2StTime (l_Result,&st_DAndT) ;

  return  st_DAndT;
}


//   get ETT for season by LSB
e_EttType GetETT_season_type(int TarifLSB)
{
  unsigned char c;

  c=20+TarifLSB;
  switch(c)
  {
  case e_EttMonthly:
  case e_EttWeekly:
  case e_Ettdaily:  
  case e_EttSemesterB:
	case e_EttSemesterA:
  case e_EttYearly:
  case e_EttHour:
    return (e_EttType)c;
  default :
    return (e_EttType)0;
  }

}

//   get ett type for kartisiya on ticket
e_EttType GetTktETT_by_count(int cnt)
{
  if(cnt==1)
    return e_EttSingle;
  if(cnt<=2)
    return e_EttMlt2;
  if(cnt<=5)
    return e_EttMlt5;
  if(cnt<=10)
    return e_EttMlt10;
  if(cnt<=11)
    return e_EttMlt11;
  if(cnt<=15)
    return e_EttMlt15;

  return e_EttMlt20;
}



//////////////////////////////////////////////////////////////////////////////
//FUNCTION:  GetETT_multy_type
//
//DESCRITION:get ETT for kartisiya by LSB
//                .
//RETURN:
//
//LOGIC :
//////////////////////////////////////////////////////////////////////////////
e_EttType GetETT_multy_type(int Tarif,int TarifLSB)
{
  unsigned char c;

  c=Tarif*10+TarifLSB;
  switch(c)
  {
  case e_EttSingle:
  case e_EttMlt2:
  case e_EttMlt5:
  case e_EttMlt10:
  case e_EttMlt11:
  case e_EttMlt15:
  case e_EttMlt20:
  case e_EttMlt4:
  case e_EttMlt6:
    return (e_EttType)c;
  default :
    return (e_EttType)0;
  }
}
e_EttType i_GetEttTypeFromCardContractRecord(const st_ClyApp_CardContractRecord *p_ClyApp_CardContractRecord)
{
  const st_ClyTkt_TicketSeasonPassTicket *pSeasonTkt=0;
  const st_ClyTkt_MultiRideTicket *pMulTkt=0;
  //int i_LocationCount = union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen;//Yoni
  int i_LocationCount = p_ClyApp_CardContractRecord->st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen;//Yoni
  int Tarif,TarifLSB=0;

  if(st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
  {   //Smart Card
    const st_ClyApp_ContractValidityLocation *p_LocationEnd;

    p_LocationEnd=  &p_ClyApp_CardContractRecord->st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i_LocationCount-1];
    TarifLSB=p_LocationEnd->union_ContractValidityLocation.st_PredefinedContract.uc_Tariff_Lsb;


    Tarif=p_ClyApp_CardContractRecord->st_CardContractIssuingData.st_ContractTariff.e_TariffAppType;
    if(Tarif==e_ClyApp_OneTimeOrMultiRideTicket || Tarif==e_ClyApp_OneTimeOrMultiRideTicket46)//if Tarif==1 it's always kartisia
    {
      //deleted by Yoni on 23/6/10 currently ignore interchange flag            //could be kartisia or transfer   - decide according to journeyinterchange
      //            if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.b_ContractIsJourneylnterchangesAllowed == 1)
      //               return e_EttPass;///e_transfer;
      //            else
      return GetETT_multy_type(Tarif, TarifLSB);///e_Kartisia;
    }


    if(Tarif==e_ClyApp_SeasonPass)//monthly or periodic
    {
      return GetETT_season_type(TarifLSB);
    }
    else if(Tarif==e_ClyApp_StoredValue)
        {
       switch(TarifLSB)
             {
                case 0://60
                    return e_EttStoreValue1_30;
                case 1://62
                    return e_EttStoreValue2_50;
                case 2:
                    return e_EttStoreValue3_100;
                case 3:
                    return e_EttStoreValue4_150;
                case 4:
                    return e_EttStoreValue5_200;
                case 5:
                    return e_EttStoreValue6_Special;
                case 6:
                    return e_EttStoreValue7_Temp;
                default:
                    return e_Ett_Unknown;
             }

        }
    return e_Ett_Unknown;///e_InvalidType;
  }

}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:  i_GetEttType
//
//DESCRITION: get ett of contract
//                .
//RETURN:
//
//LOGIC :
//////////////////////////////////////////////////////////////////////////////
e_EttType i_GetEttType(const union_ClyApp_ContractRecord *union_ContractRecord)
{
  const st_ClyTkt_TicketSeasonPassTicket *pSeasonTkt=0;
  const st_ClyTkt_MultiRideTicket *pMulTkt=0;
  int i_LocationCount = union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen;//Yoni
  int Tarif,TarifLSB=0;

  if(st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
  {   //Smart Card
    const st_ClyApp_ContractValidityLocation *p_LocationEnd;

    p_LocationEnd=&union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i_LocationCount-1];
    TarifLSB=p_LocationEnd->union_ContractValidityLocation.st_PredefinedContract.uc_Tariff_Lsb;


    Tarif=union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType;
    if(Tarif==e_ClyApp_OneTimeOrMultiRideTicket || Tarif==e_ClyApp_OneTimeOrMultiRideTicket46)//if Tarif==1 it's always kartisia
    {
      //deleted by Yoni on 23/6/10 currently ignore interchange flag            //could be kartisia or transfer   - decide according to journeyinterchange
      //            if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.b_ContractIsJourneylnterchangesAllowed == 1)
      //               return e_EttPass;///e_transfer;
      //            else
      return GetETT_multy_type(Tarif, TarifLSB);///e_Kartisia;
    }


    if(Tarif==e_ClyApp_SeasonPass)//monthly or periodic
    {
      return GetETT_season_type(TarifLSB);
    }
    else if(Tarif==e_ClyApp_StoredValue)
        {
       switch(TarifLSB)
             {
                case 0://60
                    return e_EttStoreValue1_30;
                case 1://62
                    return e_EttStoreValue2_50;
                case 2:
                    return e_EttStoreValue3_100;
                case 3:
                    return e_EttStoreValue4_150;
                case 4:
                    return e_EttStoreValue5_200;
                case 5:
                    return e_EttStoreValue6_Special;
                case 6:
                    return e_EttStoreValue7_Temp;
                default:
                    return e_Ett_Unknown;
             }

        }
    return e_Ett_Unknown;///e_InvalidType;
  }
  else
  {   //ticket
    switch(union_ContractRecord->struct_Ticket.st_TicketContractCommonData.st_Tariff)
    {
    case e_ClyTkt_TariffMultiRideFareCodeTicket://   vaf
    case e_ClyTkt_TariffPredefinedMultiRideTicket:
    case e_ClyTkt_TariffPointToPointMultiRideTicket:
      pMulTkt=&union_ContractRecord->struct_Ticket.union_TicketAppType.st_MultiRideTicket;


      return GetTktETT_by_count(pMulTkt->st_TicketMultiRideContractRec.uc_TMC_ValidityJourneys);///???                e_Kartisia;
    case e_ClyTkt_TariffPredefinedSeasonPass:
    case e_ClyTkt_TariffAreAeasonPass:///vaf
    case e_ClyTkt_TariffPointToPointSeasonPass:
      pSeasonTkt=&union_ContractRecord->struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket;
      if(pSeasonTkt->st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType==e_ClyTkt_DurationInDays)
        return e_Ettdaily;//               e_Period;//   yomi
      else
        return e_EttMonthly;///e_Monthly;///hodshi
    default:
      return e_Ett_Unknown;//                e_InvalidType;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                st_GetCurrentDateAndTime
//
//DESCRITION:
//                'yyyymmdd'h:  yyyy : year (e.g. '1968') , mm: month (1..12)  dd: day of the month (1..31) Time: sec = min= 0-59, hour 1 - 24
//RETURN:
//
//                st_Cly_DateAndTime
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////

static st_Cly_DateAndTime st_GetCurrentDateAndTime(void)
{
  st_Cly_DateAndTime st_DateAndTime;
  st_Static_StateMachine.st_UserCallbacks.fp_DateAndTimeCallBack(&st_DateAndTime);
  st_DateAndTime.stShortDate.Month = st_DateAndTime.st_Date.Month;
  st_DateAndTime.stShortDate.Year = st_DateAndTime.st_Date.Year;

  return st_DateAndTime;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                st_GetDateCompact
//
//DESCRITION:
//                Number of days since January 1 st, 1997 (being date 0). Last complete year is 2040.
//RETURN:
//
//                st_Cly_DateAndTime
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////

static st_Cly_Date st_GetDateCompact(short sh_DaysFrom1997)
{
  long l_Sec = (sh_DaysFrom1997 - DAYS_FROM_1997_TO_2000)*SEC_IN_ONE_DAY;
  st_Cly_DateAndTime st_DAndT;

  memset(&st_DAndT.st_Date,0,sizeof(st_DAndT.st_Date));
  if( sh_DaysFrom1997 == 0)
    return  st_DAndT.st_Date;


  b_Internal_Convert2000Sec2StTime (l_Sec,&st_DAndT) ;

  return  st_DAndT.st_Date;


}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                st_GetEndPeriodDate
//
//DESCRITION:
//                Get End Period Date
//RETURN:
//
//                st_Cly_DateAndTime
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////

static st_Cly_Date st_GetEndPeriodDate( e_ClyApp_PeriodType e_PeriodType,
  st_Cly_Date *stp_ContractLastValidityDate )
{
  st_Cly_DateAndTime st_DAndT;
  long l_sec1;
  unsigned char uc_DayOfTheWeek;
  st_Cly_DateAndTime  LastValidity;
  memset(&LastValidity,0,sizeof(LastValidity));
  LastValidity.st_Date = *stp_ContractLastValidityDate;

  memset(&st_DAndT,0,sizeof(st_DAndT));
  st_DAndT.st_Time.hour = st_DAndT.st_Time.min= st_DAndT.st_Time.sec = 0;
  st_DAndT.st_Date = *stp_ContractLastValidityDate;
  l_Internal_ConvertStTime2SecFrom2000 (&st_DAndT);
  switch ( e_PeriodType )
  {
  case e_PeriodIsAMonths:
    st_DAndT.st_Date.Day = GET_NUM_OF_DAYS_IN_MONTH(stp_ContractLastValidityDate->Year,stp_ContractLastValidityDate->Month);
    st_DAndT.st_Date.Month=stp_ContractLastValidityDate->Month;
    st_DAndT.st_Date.Year=stp_ContractLastValidityDate->Year;
    break;
  case e_PeriodIsAWeeks:
    uc_DayOfTheWeek = uc_CalcDayOfTheWeek(stp_ContractLastValidityDate);
    l_sec1 = l_Internal_ConvertStTime2SecFrom2000 (&LastValidity);
    l_sec1+= (7-uc_DayOfTheWeek)*SEC_IN_ONE_DAY;
    b_Internal_Convert2000Sec2StTime (l_sec1,&st_DAndT);
    break;
  case e_PeriodIsADay:
    //equal to last use date
    break;
  default: break;
  }


  return  st_DAndT.st_Date;

}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                st_GetTimeReal
//
//DESCRITION:
//                Get local time.
//RETURN:
//                st_Cly_DateAndTime
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////

static st_Cly_DateAndTime st_GetTimeReal(unsigned long ul_TimeReal
  , int IsGmtTime
  //1=input is in seconds from GMT. 1 only in e_Internal_convertBitSt2ApiSt!!
  )
{
  st_Cly_DateAndTime st_DateAndTime;
  long l_sec1;
  //Yoni 17/11/10 added ZoneTime
  int ZoneTime=0;
  if(IsGmtTime)
  {
    ZoneTime=g_time_zone_bias_minutes;
    ZoneTime*=60;
  }

  l_sec1 = ul_TimeReal - SEC_FROM_1997_TO_2000 + ZoneTime;

  memset(&st_DateAndTime,0,sizeof(st_DateAndTime));
  if( ul_TimeReal == 0 || (l_sec1<0))
    return st_DateAndTime;

  b_Internal_Convert2000Sec2StTime (l_sec1,&st_DateAndTime);
  return st_DateAndTime;

}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                ul_GetTimeReal
//
//DESCRITION:
//                convert struct to seconds from 1997
//RETURN:
//
//                st_Cly_DateAndTime
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////
unsigned long ul_GetTimeReal(const st_Cly_DateAndTime* stp_TimeReal
  ,int IsGmtTime //always 0 unless in e_Internal_convertBitSt2ApiSt
  )
{
  long l_sec1;
  int ZoneTime=0;

  if(IsGmtTime)
  {
    ZoneTime=g_time_zone_bias_minutes;
    ZoneTime*=60;
  }

  if( stp_TimeReal->st_Date.Year < 1997 )
    return 0;

  l_sec1 = l_Internal_ConvertStTime2SecFrom2000 (stp_TimeReal);
  return (l_sec1+SEC_FROM_1997_TO_2000-ZoneTime);
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                ush_GetDateCompact
//
//DESCRITION:
//
//                Number of days since January 1 st, 1997 (being date 0). Last complete year is 2040.
//
//RETURN:
//
//                unsigned short
//LOGIC :
//
////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned short ush_GetDateCompact(const st_Cly_Date* stp_Cly_Date)
{
  long l_secFrom2000=0;
  unsigned short  sh_DaysFrom1997;
  st_Cly_DateAndTime st_DateAndTime;

  memset(&st_DateAndTime,0,sizeof(st_DateAndTime));
  if( stp_Cly_Date->Year < 1997 )
    return 0;

  st_DateAndTime.st_Date = * stp_Cly_Date;
  l_secFrom2000 = l_Internal_ConvertStTime2SecFrom2000 (&st_DateAndTime);
  sh_DaysFrom1997 = (unsigned short)(l_secFrom2000/SEC_IN_ONE_DAY + DAYS_FROM_1997_TO_2000);

  return  sh_DaysFrom1997;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//                i_Internal_DateCmp
//DESCRITION:
//
//                compare the two dates
//
//RETURN:
//
//              Return Value :
//              < 0 Date1 less than Date2
//                0 Date1 identical to Date2
//              > 0 Date1 greater than Date2
//
//LOGIC :
//
//            convert to minuts and compare
///////////////////////////////////////////////////////////////////////////////////////////////////

static long l_Internal_DateCmp(st_Cly_Date *stp_Date1,st_Cly_Date *stp_Date2)
{
  unsigned long ul_Date1 = ush_GetDateCompact(stp_Date1);
  unsigned long ul_Date2 = ush_GetDateCompact(stp_Date2);

  return( ul_Date1 - ul_Date2);
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//                l_Internal_DateAndTimeCmp
//DESCRITION:
//
//                compare the two dates
//
//RETURN:
//
//              Return Value :
//              < 0 Date1 less than Date2
//                0 Date1 identical to Date2
//              > 0 Date1 greater than Date2
//
//LOGIC :
//
//            convert to minuts and compare
///////////////////////////////////////////////////////////////////////////////////////////////////

static long l_Internal_DateAndTimeCmp (st_Cly_DateAndTime *stp_Date1,st_Cly_DateAndTime *stp_Date2)
{
  unsigned long ul_Date1 = ul_GetTimeReal(stp_Date1,0);
  unsigned long ul_Date2 = ul_GetTimeReal(stp_Date2,0);

  return( ul_Date1 - ul_Date2);
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                uc_CalcExpectedContractAuthVal
//
//DESCRITION:
//
//                Count Num Of 0 bits In Buff  - for the use of the contract LRC authentication
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//
//Also for contractlist
///////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned char uc_CalcExpectedContractAuthVal(const unsigned char *ucp_BinDataInput,//[IN] binary data
  unsigned short ush_ByteLen)//[IN] binary data bit length
{

  unsigned int i,j;
  unsigned char strTmp[REC_SIZE]={0};
  unsigned char Bit0Cout=0;

  if(  ush_ByteLen != REC_SIZE )
    return 0;

  memcpy(strTmp,ucp_BinDataInput,REC_SIZE);
  //=================================
  //clear validity start date field - bit 4 to bit 17
  //=================================
  strTmp[0]&=~0x1f;
  strTmp[1]=0;
  strTmp[2]&=~0x80;

  //==============================================================================
  //count all ZERO bits of the first 28 bytes ( the AUTH filed is not included )
  //==============================================================================
  for(i=0;i<REC_SIZE-1;i++)
    for(j=0;j<8;j++)
      //if bit reset
      if( !( strTmp[i] & (1<<j) ) )
        Bit0Cout++;

  //==========================================
  //add 5 to result - as requested by Calypso
  //==========================================
  return (Bit0Cout+5);
}


//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_GetAppCardType
//
//DESCRITION:
//
//                get application card type - card or ticket
//
//RETURN:
//
//                clyApp_BOOL
//LOGIC :
//
////////////////////////////////////////////////////////////////////////////////////////////////////
static e_ClyApp_CardType e_Internal_GetAppCardType(e_7816_CardType e_7816CardType)
{

  //card
  if( e_7816CardType == e_7816_Cly_CDLIGHT ||
    e_7816CardType == e_7816_Cly_CD97BX ||
    e_7816CardType == e_7816_Cly_CD21 )
    return e_ClyApp_Card;

  //ticket
  if(e_7816CardType == e_7816_Cly_CTS256B)
    return e_ClyApp_Ticket;

  //unknown
  return e_ClyApp_UnKnown;


}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                v_Internal_GetKifVal
//
//DESCRITION:
//
//                 Get Key KIF Val accurding to the key type
//
//RETURN:
//
//                clyApp_BOOL
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static void v_Internal_GetKifVal(e_clyCard_KeyType e_KeyType,//[IN] the requested key type
  St_clySam_KIF_And_KVC *St_KIF_And_KVC )//[OUT] the requested key KVC && KIF
{
  St_KIF_And_KVC->KVC = 0x60;
  switch(e_KeyType)
  {
  case e_clyCard_KeyIssuer:// key1 -  Use to change other keys
    St_KIF_And_KVC->KIF = 0x21;
    break;

  case e_clyCard_KeyCredit:// key2 -  Credit Key
    St_KIF_And_KVC->KIF = 0x27;
    break;

  case e_clyCard_KeyDebit:    // key3 -  Debit Key
    St_KIF_And_KVC->KIF = 0x30;
    break;

  default: return ; // unexist value
  }
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_convertBitSt2ApiSt
//
//DESCRITION:
//
//                convert Bit St to Api St
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr e_Internal_convertBitSt2ApiSt( e_ConvertType e_Type,//[IN]convert direction - struct to binary Or binary to struct
  e_clyCard_FileId e_CardRecType,//[IN]Rec file type
  void* vp_BinSt,//[IN if e_BitStream2St OUT if e_St2BitStream]
  void* vp_ApiSt)//[OUT if e_BitStream2St IN if e_St2BitStream]
{

  st_Internal_EnvAndHoldBinDataStruct *stp_EnvAndHoldBinDataStruct = (st_Internal_EnvAndHoldBinDataStruct *)vp_BinSt;
  st_ClyApp_EnvAndHoldDataStruct *stp_EnvAndHoldDataStruct = (st_ClyApp_EnvAndHoldDataStruct *)vp_ApiSt;

  st_Internal_CardEventBinDataStruct *stp_CardEventBinDataStruct = (st_Internal_CardEventBinDataStruct *)vp_BinSt;
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct = (st_clyApp_CardEventDataStruct *)vp_ApiSt;


  if(e_Type == e_BitStream2St )
  {
    switch(e_CardRecType)
    {
    case e_clyCard_EnvironmentFile:
      stp_EnvAndHoldDataStruct->uc_EnvPayMethod = stp_EnvAndHoldBinDataStruct->uc_EnvPayMethod;
      stp_EnvAndHoldDataStruct->sh_EnvCountryld = stp_EnvAndHoldBinDataStruct->sh_EnvCountryld;
      stp_EnvAndHoldDataStruct->sh_HolderCompany = stp_EnvAndHoldBinDataStruct->sh_HolderCompany;
      stp_EnvAndHoldDataStruct->st_EnvEndDate = st_GetDateCompact( stp_EnvAndHoldBinDataStruct->sh_EnvEndDate);
      stp_EnvAndHoldDataStruct->st_EnvlssuingDate = st_GetDateCompact( stp_EnvAndHoldBinDataStruct->sh_EnvlssuingDate);
      stp_EnvAndHoldDataStruct->st_HoiderProf1.st_HoiderProfDate = st_GetDateCompact(stp_EnvAndHoldBinDataStruct->sh_HoiderProf1Date);
      stp_EnvAndHoldDataStruct->st_HoiderProf1.uc_HoiderProfCode = stp_EnvAndHoldBinDataStruct->uc_HoiderProf1Code;
      stp_EnvAndHoldDataStruct->st_HoiderProf2.st_HoiderProfDate = st_GetDateCompact(stp_EnvAndHoldBinDataStruct->sh_HoiderProf2Date);
      stp_EnvAndHoldDataStruct->st_HoiderProf2.uc_HoiderProfCode = stp_EnvAndHoldBinDataStruct->uc_HoiderProf2Code;
      stp_EnvAndHoldDataStruct->st_HolderBirthDate = st_GetDatef(stp_EnvAndHoldBinDataStruct->ul_HolderBirthDate);
      stp_EnvAndHoldDataStruct->uc_EnvApplicationVersionNumber = stp_EnvAndHoldBinDataStruct->uc_EnvApplicationVersionNumber;
      stp_EnvAndHoldDataStruct->uc_Envlssuerld = stp_EnvAndHoldBinDataStruct->uc_Envlssuerld;
      stp_EnvAndHoldDataStruct->ul_EnvApplicationNumber = stp_EnvAndHoldBinDataStruct->ul_EnvApplicationNumber;
      stp_EnvAndHoldDataStruct->ul_HolderCompanylD = stp_EnvAndHoldBinDataStruct->ul_HolderCompanylD;
      stp_EnvAndHoldDataStruct->ul_HolderldNumber = stp_EnvAndHoldBinDataStruct->ul_HolderldNumber;
      stp_EnvAndHoldDataStruct->uc_HolderLanguage = stp_EnvAndHoldBinDataStruct->uc_HolderLanguage;

      break;
    case e_clyCard_ContractsFile:
      break;
    case e_clyCard_CountersFile:
      break;
    case e_clyCard_EventLogFile:
    case e_clyCard_SpecialEventFile:
      stp_CardEventDataStruct->b_EventIsJourneylnterchange = stp_CardEventBinDataStruct->b_EventIsJourneylnterchange;
      stp_CardEventDataStruct->e_EventBestContractPriorityListArr[0] = (e_ClyApp_CardPriorityType)stp_CardEventBinDataStruct->uc_EventBestContractPriority1;
      stp_CardEventDataStruct->e_EventBestContractPriorityListArr[1] = (e_ClyApp_CardPriorityType)stp_CardEventBinDataStruct->uc_EventBestContractPriority2;
      stp_CardEventDataStruct->e_EventBestContractPriorityListArr[2] = (e_ClyApp_CardPriorityType)stp_CardEventBinDataStruct->uc_EventBestContractPriority3;
      stp_CardEventDataStruct->e_EventBestContractPriorityListArr[3] = (e_ClyApp_CardPriorityType)stp_CardEventBinDataStruct->uc_EventBestContractPriority4;
      stp_CardEventDataStruct->e_EventBestContractPriorityListArr[4] = (e_ClyApp_CardPriorityType)stp_CardEventBinDataStruct->uc_EventBestContractPriority5;
      stp_CardEventDataStruct->e_EventBestContractPriorityListArr[5] = (e_ClyApp_CardPriorityType)stp_CardEventBinDataStruct->uc_EventBestContractPriority6;
      stp_CardEventDataStruct->e_EventBestContractPriorityListArr[6] = (e_ClyApp_CardPriorityType)stp_CardEventBinDataStruct->uc_EventBestContractPriority7;
      stp_CardEventDataStruct->e_EventBestContractPriorityListArr[7] = (e_ClyApp_CardPriorityType)stp_CardEventBinDataStruct->uc_EventBestContractPriority8;

      stp_CardEventDataStruct->st_EventCode.e_CardEventTransportMean = (e_clyApp_CardEventTransportMean)((stp_CardEventBinDataStruct->uc_EventCode&0xf0)>>4);
      stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances = (e_clyApp_CardEventCircumstances)(stp_CardEventBinDataStruct->uc_EventCode&0x0f);


      //we read from GMT to local
      stp_CardEventDataStruct->st_EventDataTimeFirstStamp = st_GetTimeReal(stp_CardEventBinDataStruct->ul_EventDataTimeFirstStamp, 1);
      stp_CardEventDataStruct->st_EventDateTimeStamp = st_GetTimeReal(stp_CardEventBinDataStruct->ul_EventDateTimeStamp, 1);//

      stp_CardEventDataStruct->uc_EventContractPointer = stp_CardEventBinDataStruct->uc_EventContractPointer;
      stp_CardEventDataStruct->uc_EventServiceProvider = stp_CardEventBinDataStruct->uc_EventServiceProvider;
      stp_CardEventDataStruct->uc_EventVersionNumber = stp_CardEventBinDataStruct->uc_EventVersionNumber;


      break;
    default: return e_ClyApp_WrongParamErr;
    }
  }
  else // e_St2BitStream
  {
    switch(e_CardRecType)
    {
    case e_clyCard_EnvironmentFile:
      stp_EnvAndHoldBinDataStruct->uc_EnvPayMethod = stp_EnvAndHoldDataStruct->uc_EnvPayMethod;
      stp_EnvAndHoldBinDataStruct->sh_EnvCountryld = stp_EnvAndHoldDataStruct->sh_EnvCountryld;
      stp_EnvAndHoldBinDataStruct->sh_HolderCompany = stp_EnvAndHoldDataStruct->sh_HolderCompany;
      stp_EnvAndHoldBinDataStruct->sh_EnvEndDate = ush_GetDateCompact( &stp_EnvAndHoldDataStruct->st_EnvEndDate );
      stp_EnvAndHoldBinDataStruct->sh_EnvlssuingDate = ush_GetDateCompact( &stp_EnvAndHoldDataStruct->st_EnvlssuingDate );
      stp_EnvAndHoldBinDataStruct->sh_HoiderProf1Date = ush_GetDateCompact(&stp_EnvAndHoldDataStruct->st_HoiderProf1.st_HoiderProfDate);
      stp_EnvAndHoldBinDataStruct->uc_HoiderProf1Code = stp_EnvAndHoldDataStruct->st_HoiderProf1.uc_HoiderProfCode;
      stp_EnvAndHoldBinDataStruct->sh_HoiderProf2Date = ush_GetDateCompact(&stp_EnvAndHoldDataStruct->st_HoiderProf2.st_HoiderProfDate);
      stp_EnvAndHoldBinDataStruct->uc_HoiderProf2Code = stp_EnvAndHoldDataStruct->st_HoiderProf2.uc_HoiderProfCode;
      stp_EnvAndHoldBinDataStruct->ul_HolderBirthDate = l_GetDatef( &stp_EnvAndHoldDataStruct->st_HolderBirthDate );
      stp_EnvAndHoldBinDataStruct->uc_EnvApplicationVersionNumber = stp_EnvAndHoldDataStruct->uc_EnvApplicationVersionNumber;
      stp_EnvAndHoldBinDataStruct->uc_Envlssuerld = stp_EnvAndHoldDataStruct->uc_Envlssuerld;
      stp_EnvAndHoldBinDataStruct->ul_EnvApplicationNumber = stp_EnvAndHoldDataStruct->ul_EnvApplicationNumber;
      stp_EnvAndHoldBinDataStruct->ul_HolderCompanylD = stp_EnvAndHoldDataStruct->ul_HolderCompanylD;
      stp_EnvAndHoldBinDataStruct->ul_HolderldNumber = stp_EnvAndHoldDataStruct->ul_HolderldNumber;
      stp_EnvAndHoldBinDataStruct->uc_HolderLanguage = stp_EnvAndHoldDataStruct->uc_HolderLanguage;


      break;
    case e_clyCard_ContractsFile:
      break;
    case e_clyCard_CountersFile:
      break;
    case e_clyCard_EventLogFile:
    case e_clyCard_SpecialEventFile:
      stp_CardEventBinDataStruct->uc_EventVersionNumber = stp_CardEventDataStruct->uc_EventVersionNumber;
      stp_CardEventBinDataStruct->uc_EventServiceProvider = stp_CardEventDataStruct->uc_EventServiceProvider;
      stp_CardEventBinDataStruct->uc_EventContractPointer = stp_CardEventDataStruct->uc_EventContractPointer;
      stp_CardEventBinDataStruct->uc_EventCode = (stp_CardEventDataStruct->st_EventCode.e_CardEventTransportMean<<4&0xf0) | (stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances&0x0f);
      stp_CardEventBinDataStruct->b_EventIsJourneylnterchange = stp_CardEventDataStruct->b_EventIsJourneylnterchange ;

      //Yoni 11/2010: we wirte time in GMT
      stp_CardEventBinDataStruct->ul_EventDateTimeStamp = ul_GetTimeReal(&stp_CardEventDataStruct->st_EventDateTimeStamp,1 );
      stp_CardEventBinDataStruct->ul_EventDataTimeFirstStamp = ul_GetTimeReal(&stp_CardEventDataStruct->st_EventDataTimeFirstStamp,1 );
      /////////////////////////////////////////////////////////
      stp_CardEventBinDataStruct->uc_EventBestContractPriority1 = stp_CardEventDataStruct->e_EventBestContractPriorityListArr[0];
      stp_CardEventBinDataStruct->uc_EventBestContractPriority2 = stp_CardEventDataStruct->e_EventBestContractPriorityListArr[1];
      stp_CardEventBinDataStruct->uc_EventBestContractPriority3 = stp_CardEventDataStruct->e_EventBestContractPriorityListArr[2];
      stp_CardEventBinDataStruct->uc_EventBestContractPriority4 = stp_CardEventDataStruct->e_EventBestContractPriorityListArr[3];
      stp_CardEventBinDataStruct->uc_EventBestContractPriority5 = stp_CardEventDataStruct->e_EventBestContractPriorityListArr[4];
      stp_CardEventBinDataStruct->uc_EventBestContractPriority6 = stp_CardEventDataStruct->e_EventBestContractPriorityListArr[5];
      stp_CardEventBinDataStruct->uc_EventBestContractPriority7 = stp_CardEventDataStruct->e_EventBestContractPriorityListArr[6];
      stp_CardEventBinDataStruct->uc_EventBestContractPriority8 = stp_CardEventDataStruct->e_EventBestContractPriorityListArr[7];



      break;
    default: return e_ClyApp_WrongParamErr;
    }
  }
  return e_ClyApp_Ok;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                v_Internal_TranslateType
//
//DESCRITION:
//
//                Translate Data Type from st to bin && from bin to st
//
//RETURN:
//
//                -
//LOGIC:
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static void v_Internal_TranslateType( e_ConvertType ConvertType,//[IN] convert direction - bit stream to struct OR struct to bit stream
  e_ClyApp_StDateType e_DateType,//[IN] st data type
  unsigned char *ucp_BitStream,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
  void *vp_St)//[OUT if e_BitStream2St IN if e_St2BitStream]struct
{

  st_Cly_Date st_Date;


  switch(e_DateType)
  {
  case e_ClyApp_DateCompactType:
    if(ConvertType == e_BitStream2St)
    {
      st_Date = st_GetDateCompact(*(short*)ucp_BitStream);
      memcpy(vp_St, &st_Date,sizeof(st_Cly_Date));
    }
    else
      *(short*)ucp_BitStream = ush_GetDateCompact( (st_Cly_Date*)vp_St);

    break;

  case e_ClyApp_DateReverseType:
    //=====================================
    // date reverse = date compact ^ 0xffff
    //=====================================
    if(ConvertType == e_BitStream2St)
    {
      short sh_DateRev = *(short*)ucp_BitStream^0x3fff;
      st_Date = st_GetDateCompact(sh_DateRev);
      memcpy(vp_St, &st_Date,sizeof(st_Cly_Date));
    }
    else
    {
      *(short*)ucp_BitStream = ush_GetDateCompact( (st_Cly_Date*)vp_St);
      *(short*)ucp_BitStream^=0x3fff;
    }


    break;
  default: break;
  }
}



#if 0
//this is for Egged
///////////////////////////////////////////////////////////////
//
// Name:    v_BitsFixer
// Purpose: Support for Egged long sale device number
// Params:  ContractSaleDevice &  ContractSaleNumberDaily
// Returns:
//
///////////////////////////////////////////////////////////////

void v_BitsFixer( int ConvertType,unsigned short *m_ContractSaleDevice, unsigned short *m_ContractSaleNumberDaily)
{

#ifdef USE_EGGED_LONG_DEVICE_NUMBER


  unsigned long CSD      = (unsigned long)*m_ContractSaleDevice;
  unsigned long CSND     = (unsigned long)*m_ContractSaleNumberDaily;

  // When converting form binary back to struct
  if(ConvertType == 1)
  {
    if(GETBIT(CSND,9))
      SETBIT(CSD,12);

    CLRBIT(CSND,9);
  }
  else // Struct to binary conversion
  {
    if(CSND > 511)
      CSND = 511;

    if(CSD > 4095)
    {
      SETBIT(CSND,9);
      CLRBIT(CSD,12);
    }
  }

  *m_ContractSaleDevice = (unsigned short)CSD;
  *m_ContractSaleNumberDaily = (unsigned short)CSND;


#endif // #ifdef USE_EGGED_LONG_DEVICE_NUMBER
}
#endif


//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_Bit2Byte_Convert
//
//DESCRITION:
//
//                 Get Key KIF Val accurding to the key type
//
//RETURN:
//
//                clyApp_BOOL
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////


eCalypsoErr e_Internal_Bit2Byte_Convert( e_ConvertType ConvertType,//[IN] convert direction - bit stream to struct OR struct to bit stream
  e_ClyApp_CardType e_CardType,//[IN]type - card \ ticket
  e_clyCard_FileId e_CardFileType,//[IN] if not a ticket  - which record in the card
  unsigned char *ucp_BitStream,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
  void *vp_St)//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
{
  eCalypsoErr err;
  st_FieldDescriptor l_oContractListDescriptor={0};
  st_FieldDescriptor st_OptionalDataDesc;
  st_FieldDescriptor stEventTicketOptionalDataDesc[3],stTariffDesc[3],stValidityDurationDesc[2];
  union_ClyApp_ContractRecord *union_ContractRecord = (union_ClyApp_ContractRecord *)vp_St;
  st_clyApp_ContractListStruct* pContractList;
  unsigned short offset=0,sh_ContractValidityInfo=0;
  unsigned char uc_EventExtention,i,uc_EventLocation,uc_LocationArrLen;
  unsigned char ucp_Data[4];
  e_ClyApp_CardSpatialType e_CardSpatialType;
  char j;
  st_clyApp_CardEventDataStruct *st_CardEventDataStruct ;
  CalypsoBinTktType ucp_BinBuffOut;

  memset(&stEventTicketOptionalDataDesc,0,sizeof(stEventTicketOptionalDataDesc));
  memset(&stTariffDesc,0,sizeof(stTariffDesc));
  memset(&stValidityDurationDesc,0,sizeof(stValidityDurationDesc));

  //if convert from bin to staruct
  if(ConvertType == e_BitStream2St)
  {

    //if ticket
    if(e_CardType == e_ClyApp_Ticket)
      // Translate Ticket struct Binary buffer
      return (eCalypsoErr)e_ClyTkt_ConvertBinBuff2TktSt( (struct_ClyTkt_Ticket*)vp_St,//[OUT] Ticket struct output
      ucp_BitStream,//[IN] Binary buff to translate
      ucp_BinBuffOut);
    // if card
    switch(e_CardFileType)
    {
    case e_clyCard_EnvironmentFile:
      v_Bit2Byte_Convert(  e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        stEnvDesc,//[IN] Field Descriptor Array
        sizeof(stEnvDesc)/sizeof(stEnvDesc[0]),//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      //convert Bit St to Api St
      return e_Internal_convertBitSt2ApiSt(e_BitStream2St, e_CardFileType, (void*)&st_Global_EnvAndHoldBinDataStruct,(void*)vp_St);

    case e_clyCard_EventLogFile:
    case e_clyCard_SpecialEventFile:

      st_CardEventDataStruct = (st_clyApp_CardEventDataStruct*)vp_St;

      v_Bit2Byte_Convert(  e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        stEventMandatoryDataDesc,//[IN] Field Descriptor Array
        sizeof(stEventMandatoryDataDesc)/sizeof(stEventMandatoryDataDesc[0]),//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //convert Bit St to Api St

      err = e_Internal_convertBitSt2ApiSt( e_BitStream2St, e_CardFileType, (void*)&st_Global_CardEventBinDataStruct,(void*)vp_St);
      if(err!=e_ClyApp_Ok)
        return err;

      //==========================================================================
      //Read Optional data directly into the user pointer - don't use bin struct
      //==========================================================================

      ///////////////////////////////////////////////
      //Check which optional data exist and read it
      ///////////////////////////////////////////////
      offset = EVENT_START_OPTIONAL_DATA_OFFSET ;
      st_OptionalDataDesc.e_Fieltype = e_VariableType;

      //if Event Place Exist
      if(st_Global_CardEventBinDataStruct.uc_EventLocation & 1)
      {
        st_CardEventDataStruct->st_OptionalEventData.b_IsEventPlaceExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=16;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.ush_EventPlace;

        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }
      ///////////////////////////////////////////////
      //Read EventLocation optional data
      ///////////////////////////////////////////////

      //if Event Line Exist
      if(st_Global_CardEventBinDataStruct.uc_EventLocation & (1<<1))
      {
        st_CardEventDataStruct->st_OptionalEventData.b_IsEventLineExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=16;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.ush_EventLine;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //if Event Station Exist;
      if(st_Global_CardEventBinDataStruct.uc_EventLocation & (1<<2))
      {
        st_CardEventDataStruct->st_OptionalEventData.b_IsEventRFU1Exist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=8;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.uc_EventRFU1;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //if Event Run lD Exist;
      if(st_Global_CardEventBinDataStruct.uc_EventLocation & (1<<3))
      {
        st_CardEventDataStruct->st_OptionalEventData.b_IsEventRunlDExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=12;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.ush_EventRunlD;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //if Event Device Exist;
      if(st_Global_CardEventBinDataStruct.uc_EventLocation & (1<<4))
      {
        st_CardEventDataStruct->st_OptionalEventData.b_IsEventDevice4Exist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=14;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.ush_EventDevice4;

        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //if Event InterchangeRights Exist;
      if(st_Global_CardEventBinDataStruct.uc_EventLocation & (1<<5))
      {
        st_CardEventDataStruct->st_OptionalEventData.b_IsEventRFU2Exist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=4;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.uc_EventRFU2;

        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //if Event Device Exist;
      if(st_Global_CardEventBinDataStruct.uc_EventLocation & (1<<6))
      {
        st_CardEventDataStruct->st_OptionalEventData.b_IsEventInterchangeRightsExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=8;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.uc_EventInterchangeRights;

        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      ///////////////////////////////////////////////
      //Read Event Extention Mandatory data
      ///////////////////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=3;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &uc_EventExtention;

      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      //increment offset only if field exist
      offset+=st_OptionalDataDesc.uc_StreamBitCount;


      //if Event Place Exist
      ///////////////////////////////////////////////
      //Read EventExtention optional data
      ///////////////////////////////////////////////

      //if Event Ticket Exist;
      if(uc_EventExtention & 1)
      {
        st_CardEventDataStruct->st_OptionalEventData.b_IsEventTicketExist=clyApp_TRUE;
        stEventTicketOptionalDataDesc[0].e_Fieltype = e_VariableType;
        stEventTicketOptionalDataDesc[0].uc_StreamBitCount=10;
        stEventTicketOptionalDataDesc[0].us_StreamBitOffset= offset;
        stEventTicketOptionalDataDesc[0].vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.st_EventTicket.ush_EventTicketRoutesSystem;


        offset+=stEventTicketOptionalDataDesc[0].uc_StreamBitCount;

        stEventTicketOptionalDataDesc[1].e_Fieltype = e_VariableType;
        stEventTicketOptionalDataDesc[1].uc_StreamBitCount=8;
        stEventTicketOptionalDataDesc[1].us_StreamBitOffset= offset;
        stEventTicketOptionalDataDesc[1].vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.st_EventTicket.uc_EventTicketFareCode;

        offset+=stEventTicketOptionalDataDesc[1].uc_StreamBitCount;

        stEventTicketOptionalDataDesc[2].e_Fieltype = e_VariableType;
        stEventTicketOptionalDataDesc[2].uc_StreamBitCount=16;
        stEventTicketOptionalDataDesc[2].us_StreamBitOffset= offset;
        stEventTicketOptionalDataDesc[2].vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.st_EventTicket.ush_EventTicketDebitAmount;

        offset+=stEventTicketOptionalDataDesc[2].uc_StreamBitCount;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &stEventTicketOptionalDataDesc[0],//[IN] Field Descriptor Array
          sizeof(stEventTicketOptionalDataDesc)/sizeof(stEventTicketOptionalDataDesc[0]),//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        ////increment offset only if field exist
        //offset+=st_OptionalDataDesc.uc_StreamBitCount; deleted on 7/6/10 by Yoni
      }

      //if Event Passengers Number Exist;
      if(uc_EventExtention & (1<<1))
      {
        st_CardEventDataStruct->st_OptionalEventData.b_IsEventPassengersNumberExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=5;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.uc_EventPassengersNumber;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }
      ///////////////////////////////
      //  Check if legal structure
      ///////////////////////////////
      if(offset > (REC_SIZE*8))
        return e_ClyApp_CardContractErr;
      break;

    case e_clyCard_ContractsFile:

      union_ContractRecord = (union_ClyApp_ContractRecord*)vp_St;

      st_OptionalDataDesc.e_Fieltype = e_VariableType;

      //////////////////////////
      //Contract Version Number
      //////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=3;
      st_OptionalDataDesc.us_StreamBitOffset= (offset=0);
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractVersionNumber;
      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;

      /////////////////////////////////
      //Contract Validity Start Date
      /////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=14;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = ucp_Data;

      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //get date
      v_Internal_TranslateType(e_BitStream2St,e_ClyApp_DateReverseType,(unsigned char *)ucp_Data,&union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractValidityStartDate);
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;



      /////////////////////////////////
      //Contract Provider
      /////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=8;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractProvider;


      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;



      /////////////////////////////////
      //Contract tariff
      /////////////////////////////////
      /////////////////////////////////
      //Contract tariff
      /////////////////////////////////

      stTariffDesc[0].e_Fieltype = e_VariableType;
      stTariffDesc[0].uc_StreamBitCount=2;
      stTariffDesc[0].us_StreamBitOffset= offset;
      stTariffDesc[0].vp_StFieldPtr =  &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffTransportType;


      offset+=stTariffDesc[0].uc_StreamBitCount;

      stTariffDesc[1].e_Fieltype = e_VariableType;
      stTariffDesc[1].uc_StreamBitCount=3;
      stTariffDesc[1].us_StreamBitOffset= offset;
      stTariffDesc[1].vp_StFieldPtr =  &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType;

      offset+=stTariffDesc[1].uc_StreamBitCount;

      stTariffDesc[2].e_Fieltype = e_VariableType;
      stTariffDesc[2].uc_StreamBitCount=6;
      stTariffDesc[2].us_StreamBitOffset= offset;
      stTariffDesc[2].vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType;


      v_Bit2Byte_Convert(  e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        stTariffDesc,//[IN] Field Descriptor Array
        sizeof(stTariffDesc)/sizeof(stTariffDesc[0]),//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      //increment offset
      offset+=stTariffDesc[2].uc_StreamBitCount;

      /////////////////////////////////
      //Contract Sale Date
      /////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=14;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = ucp_Data;


      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //get date
      v_Internal_TranslateType(e_BitStream2St,e_ClyApp_DateCompactType,(unsigned char *)ucp_Data,&union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractSaleDate);
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;

      /////////////////////////////////
      //Contract Sale Device
      /////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=12;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.sh_ContractSaleDevice;


      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;


      /////////////////////////////////
      //Contract Sale Number Daily
      /////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=10;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.sh_ContractSaleNumberDaily;


      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;

      /////////////////////////////////
      //Contract Sale Number Daily
      /////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=1;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.b_ContractIsJourneylnterchangesAllowed;


      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;


      /////////////////////////////////
      //ContractValidityInfo
      /////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=9;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &sh_ContractValidityInfo;


      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;

      /////////////////////////////////
      //START READ OPTIONAL DATA
      /////////////////////////////////

      // Is Contract Restrict Time Code Exist;
      if(sh_ContractValidityInfo & 1)
      {
        union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_IsContractRestrictTimeCodeExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=5;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictTimeCode;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //is Contract Restrict Code Exist;
      if(sh_ContractValidityInfo & 1<<1)
      {
        union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictCodeExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=5;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictCode;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      // is Contract Restrict Duration Exist
      if(sh_ContractValidityInfo & 1<<2)
      {
        union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictDurationExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=6;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictDuration;

        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }


      // is Contract Validity End Date Exist
      if(sh_ContractValidityInfo & 1<<3)
      {
        union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityEndDateExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=14;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = ucp_Data;

        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
        //get date
        v_Internal_TranslateType(e_BitStream2St,e_ClyApp_DateCompactType,(unsigned char *)ucp_Data,&union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityEndDate);

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      // is Contract Validity Duration Exist;
      if(sh_ContractValidityInfo & 1<<4)
      {
        union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityDurationExist=clyApp_TRUE;


        stValidityDurationDesc[0].e_Fieltype = e_VariableType;
        stValidityDurationDesc[0].uc_StreamBitCount=2;
        stValidityDurationDesc[0].us_StreamBitOffset= offset;
        stValidityDurationDesc[0].vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.e_DurationType;

        offset+=stValidityDurationDesc[0].uc_StreamBitCount;

        stValidityDurationDesc[1].e_Fieltype = e_VariableType;
        stValidityDurationDesc[1].uc_StreamBitCount=6;
        stValidityDurationDesc[1].us_StreamBitOffset= offset;
        stValidityDurationDesc[1].vp_StFieldPtr =  &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.uc_DurationUnitCount;

        v_Bit2Byte_Convert(  e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          stValidityDurationDesc,//[IN] Field Descriptor Array
          sizeof(stValidityDurationDesc)/sizeof(stValidityDurationDesc[0]),//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream


        offset+=stValidityDurationDesc[1].uc_StreamBitCount;

      }

      // is Contract Period Journeys Exist;
      if(sh_ContractValidityInfo & 1<<5)
      {
        union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist=clyApp_TRUE;

        stValidityDurationDesc[0].e_Fieltype = e_VariableType;
        stValidityDurationDesc[0].uc_StreamBitCount=2;
        stValidityDurationDesc[0].us_StreamBitOffset= offset;
        stValidityDurationDesc[0].vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.e_PeriodType;

        offset+=stValidityDurationDesc[0].uc_StreamBitCount;

        stValidityDurationDesc[1].e_Fieltype = e_VariableType;
        stValidityDurationDesc[1].uc_StreamBitCount=6;
        stValidityDurationDesc[1].us_StreamBitOffset= offset;
        stValidityDurationDesc[1].vp_StFieldPtr =  &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.uc_MaxNumOfTripsInPeriod;

        v_Bit2Byte_Convert(  e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          stValidityDurationDesc,//[IN] Field Descriptor Array
          sizeof(stValidityDurationDesc)/sizeof(stValidityDurationDesc[0]),//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream


        offset+=stValidityDurationDesc[1].uc_StreamBitCount;

      }

      // is Contract Customer Profile Exist;
      if(sh_ContractValidityInfo & 1<<6)
      {
        union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractCustomerProfileExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=6;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractCustomerProfile;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      // is Contract Passengers Number Exist;
      if(sh_ContractValidityInfo & 1<<7)
      {
        union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPassengersNumberExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=5;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractPassengersNumber;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      // is Contract RFU Number Exist;
      if(sh_ContractValidityInfo & 1<<8 )
      {
        union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRFUExist=clyApp_TRUE;
        st_OptionalDataDesc.uc_StreamBitCount=32;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.ul_ContractRFUval;

        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      ///////////////////////////////
      // GET FIRST VALIDITY LOCATION
      ///////////////////////////////
      union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen=0;
      e_CardSpatialType=e_CardSpatialTypeZones;
      st_OptionalDataDesc.uc_StreamBitCount=4;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &e_CardSpatialType;


      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;
      i=0;
      while(e_CardSpatialType != e_CardSpatialTypeEndLocationList
				&& i < MAX_VALIDITY_LOCATION)
      {
        ///////////////////////////////////////////////////////////
        //  Check if legal structure - last byte reserved for AUTH
        ///////////////////////////////////////////////////////////
        if(offset>((REC_SIZE-1)*8))
          return e_ClyApp_CardContractErr;

        switch(e_CardSpatialType)
        {
        case e_CardSpatialTypeZones:
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypeZones;
          st_OptionalDataDesc.uc_StreamBitCount=10;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Zones.ush_SpatialRoutesSystem;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Zones.ush_SpatialZones;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;
          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;

        case e_CardSpatialTypeFareCode:
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypeFareCode;
          st_OptionalDataDesc.uc_StreamBitCount=10;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_FareCode.ush_SpatialRoutesSystem;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Zones.ush_SpatialZones;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;
          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;

        case e_CardSpatialTypeLinesList:
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypeLinesList;
          // get number of lines
          st_OptionalDataDesc.uc_StreamBitCount=4;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_LinesList.uc_SpatiaiLineArrLen;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;
          //read all lines
          for(j=0;j<union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_LinesList.uc_SpatiaiLineArrLen;j++)
          {

            st_OptionalDataDesc.uc_StreamBitCount=16;
            st_OptionalDataDesc.us_StreamBitOffset= offset;
            st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_LinesList.ush_SpatiaiLineArr[(int)j];


            v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
              &st_OptionalDataDesc,//[IN] Field Descriptor Array
              1,//[IN]Field Descriptor Array len
              ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

            offset+=st_OptionalDataDesc.uc_StreamBitCount;
          }
          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;

        case e_CardSpatialTypeRide:
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypeRide;
          st_OptionalDataDesc.uc_StreamBitCount=16;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Ride.ush_SpatialLine;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Ride.uc_SpatialStationOrigin;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Ride.uc_SpatialStationDestination;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;
          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;

        case e_CardSpatialTypeRideAndRunType:
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypeRideAndRunType;
          st_OptionalDataDesc.uc_StreamBitCount=16;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunType.ush_SpatialLine;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunType.uc_SpatialStationOrigin;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunType.uc_SpatialStationDestination;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=4;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunType.uc_SpatialRunType;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;

        case e_CardSpatialTypeRideAndRunID:
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypeRideAndRunID;
          st_OptionalDataDesc.uc_StreamBitCount=16;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunID.ush_SpatialLine;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunID.uc_SpatialStationOrigin;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunID.uc_SpatialStationDestination;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunID.ush_SpatialRunlD;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;
          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;

        case e_CardSpatialTypeRideRunAndSeat:
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypeRideRunAndSeat;
          st_OptionalDataDesc.uc_StreamBitCount=16;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.ush_SpatialLine;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.uc_SpatialStationOrigin;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.uc_SpatialStationDestination;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.ush_SpatialRunlD;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=4;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.uc_SpatialVehicleCoach;
          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=7;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.uc_SpatialSeat;
          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;

        case e_CardSpatialTypeRideZones:
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypeRideZones;
          st_OptionalDataDesc.uc_StreamBitCount=10;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideZones.ush_SpatialRoutesSystemFrom;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideZones.ush_SpatialZonesFrom;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=10;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideZones.ush_SpatialRoutesSystemTo;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideZones.ush_SpatialZonesTo;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;

        case e_CardSpatialTypeParking:
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypeParking;
          st_OptionalDataDesc.uc_StreamBitCount=6;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Parking.uc_SpatialParkingDataSize;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          //check value
          if(!(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Parking.uc_SpatialParkingDataSize>=12 &&
            union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Parking.uc_SpatialParkingDataSize<=75))
            return e_ClyApp_CardContractErr;

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12 + union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Parking.uc_SpatialParkingDataSize;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Parking.uc_SpatialParkingData;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;
          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;

        case e_CardSpatialTypePredefinedContract:
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypePredefinedContract;
          st_OptionalDataDesc.uc_StreamBitCount=3;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_PredefinedContract.uc_Tariff_Lsb;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=11;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_PredefinedContract.ush_SpetailCode;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;


          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;


          //Yoni 08/2011 handle e_CardSpatialTypeRouteSystemsList and e_CardSpatialTypeFareCodeExtension
        case e_CardSpatialTypeRouteSystemsList:
          //RouteSystemsNumber- 4bits- Number of Route Systems in the list
          //Route System1- 10bits -Identifier (1 to 1023) of the first route system where the ticket is valid
          //...
          //Route SystemN- 10bits -Identifier (1 to 1023) of the last route system where the ticket is valid
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypeRouteSystemsList;
          // get number of routesystems
          st_OptionalDataDesc.uc_StreamBitCount=4;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RouteSystemsList.uc_SpatiaiRouteSystemArrLen;


          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;
          //read all routesystems
          for(j=0;j<union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RouteSystemsList.uc_SpatiaiRouteSystemArrLen;j++)
          {

            st_OptionalDataDesc.uc_StreamBitCount=10;
            st_OptionalDataDesc.us_StreamBitOffset= offset;
            st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RouteSystemsList.ush_SpatiaiRouteSystemArr[(int)j];

            v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
              &st_OptionalDataDesc,//[IN] Field Descriptor Array
              1,//[IN]Field Descriptor Array len
              ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

            offset+=st_OptionalDataDesc.uc_StreamBitCount;
          }
          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;

        case e_CardSpatialTypeFareCodeExtension:
          //SpatialRoutesSystem- 10bits -Identifier (1 to 1023) of the routes system where the ticket is valid. 0 � means any route system.
          //FareRestrictionCode- 3bits - Restriction code with the following values:
          //  0 � any fare code
          //  1 � only specified fare code
          //  2 � any fare code up to the specified value (included)
          //  3 � any fare code above the specified value (included)
          //  4 � preferred fare code to be used, unless another code was specified.
          //  5�7 � RFU
          //SpatialFareCode- 8bits -Fare code (1 to 255) for the ticket, within SpatialRoutesSystem
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypeFareCodeExtension;
          st_OptionalDataDesc.uc_StreamBitCount=10;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_FareCodeExtension.ush_SpatialRoutesSystem;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=3;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_FareCodeExtension.uc_FareRestrictionCode;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;


          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_FareCodeExtension.uc_SpatialFareCode;

          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          //add one to list
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen++;

          break;

        default: return e_ClyApp_CardContractErr; //if data structur is unknow - return err
        }

        //==========================
        //Get next validity location
        //==========================
        st_OptionalDataDesc.uc_StreamBitCount=4;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &e_CardSpatialType;

        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //Increment offset
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
        i++;
      }

      ////////////////////////////////////////////////////////////////////////////////
      //get Contract Authenticator value - WRITTEN IN THE LAST 8 BITS OF THE RECORD
      ////////////////////////////////////////////////////////////////////////////////
      offset = ((REC_SIZE*8)-8);
      st_OptionalDataDesc.uc_StreamBitCount=8;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractAuthenticator;

      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      break;
    case e_clyCard_CountersFile:
      /////////////////////////////////////////////
      // if counter as Date And Remaining Journeys - data recevided from the contract data
      ////////////////////////////////////////////
      offset =0;
      if(union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.e_CardCounterRecordType == e_ClyApp_CardCounter_DateAndRemainingJourneys)
      {

        st_OptionalDataDesc.uc_StreamBitCount=14;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = ucp_Data;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //get data
        v_Internal_TranslateType(e_BitStream2St,e_ClyApp_DateReverseType,(unsigned char *)ucp_Data,&union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.st_CounterDate);


        //Increment offset
        offset+=st_OptionalDataDesc.uc_StreamBitCount;

        st_OptionalDataDesc.uc_StreamBitCount=10;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.CounterValue;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream


      }
      else // counter as Number Of Tokens Or Amount
      {

        if( union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.e_CardCounterRecordType !=e_ClyApp_CardCounter_NumberOfTokensOrAmount )
          return e_ClyApp_WrongParamErr;

        st_OptionalDataDesc.uc_StreamBitCount=24;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue;


        v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      }
      break;

    case e_clyCard_ContractListFile://Yoni 10/2011
      pContractList = (st_clyApp_ContractListStruct*)vp_St;
      memset(pContractList, 0, sizeof(*pContractList));

      ////////  ver
      l_oContractListDescriptor.uc_StreamBitCount=3;
      l_oContractListDescriptor.us_StreamBitOffset=offset=0;
      l_oContractListDescriptor.vp_StFieldPtr=&pContractList->uc_Ver;
      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
              &l_oContractListDescriptor,//[IN] Field Descriptor Array
              1,//[IN]Field Descriptor Array len
              ucp_BitStream);
      offset+=l_oContractListDescriptor.uc_StreamBitCount;
      ////////  bitmap

      l_oContractListDescriptor.uc_StreamBitCount=16;
      l_oContractListDescriptor.us_StreamBitOffset=offset;
      l_oContractListDescriptor.vp_StFieldPtr=&pContractList->us_Bitmap;
      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
              &l_oContractListDescriptor,//[IN] Field Descriptor Array
              1,//[IN]Field Descriptor Array len
              ucp_BitStream);
      offset+=l_oContractListDescriptor.uc_StreamBitCount;
      ////////  code array
      for(j=0;j<8;j++)
      {
        if((pContractList->us_Bitmap) & (1<<j))
        {
          l_oContractListDescriptor.uc_StreamBitCount=16;
          l_oContractListDescriptor.us_StreamBitOffset=offset;
          l_oContractListDescriptor.vp_StFieldPtr=&pContractList->ContractListAuthorizationCodeArr[j];
          v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
                  &l_oContractListDescriptor,//[IN] Field Descriptor Array
                  1,//[IN]Field Descriptor Array len
                  ucp_BitStream);
          offset+=l_oContractListDescriptor.uc_StreamBitCount;

        }
      }

      ////////  authenticator
      l_oContractListDescriptor.uc_StreamBitCount=8;
      l_oContractListDescriptor.us_StreamBitOffset=(29*8-8);//there's padding and auth is last 8 bits
      l_oContractListDescriptor.vp_StFieldPtr=&pContractList->ContractListAuthenticator;
      v_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
              &l_oContractListDescriptor,//[IN] Field Descriptor Array
              1,//[IN]Field Descriptor Array len
              ucp_BitStream);

      break;

    default: return e_ClyApp_WrongParamErr;
    }

  }
  else //if convert from staruct to bin  -> ConvertType == e_St2BitStream)
  {
    //if ticket
    if(e_CardType == e_ClyApp_Ticket)
      // Convert Ticket struct to Binary buffer
      return (eCalypsoErr)e_ClyTkt_ConvertTktSt2BinBuff( (struct_ClyTkt_Ticket*)vp_St,//[IN] Ticket struct input for translation
      ucp_BitStream);//[OUT] Binary buff result

    // if card
    switch(e_CardFileType)
    {

    case e_clyCard_EnvironmentFile:
      //convert Api St to Bit St
      err = e_Internal_convertBitSt2ApiSt(e_St2BitStream, e_CardFileType, (void*)&st_Global_EnvAndHoldBinDataStruct,(void*)vp_St);
      if( err!=e_ClyApp_Ok )
        return err;

      v_Bit2Byte_Convert(  e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        stEnvDesc,//[IN] Field Descriptor Array
        sizeof(stEnvDesc)/sizeof(stEnvDesc[0]),//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      break;

    case e_clyCard_EventLogFile:
    case e_clyCard_SpecialEventFile:

      st_CardEventDataStruct = (st_clyApp_CardEventDataStruct*)vp_St;
      //convert Bit St to Api St
      err = e_Internal_convertBitSt2ApiSt( e_St2BitStream,e_CardFileType, (void*)&st_Global_CardEventBinDataStruct,(void*)vp_St);
      if( err!=e_ClyApp_Ok)
        return err;

      v_Bit2Byte_Convert(  e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        stEventMandatoryDataDesc,//[IN] Field Descriptor Array
        (sizeof(stEventMandatoryDataDesc)/sizeof(stEventMandatoryDataDesc[0]))-1,//[IN]Field Descriptor Array len - last field not included
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      //==================================================================
      //build uc_EventLocation field the - using the optional data exist
      //==================================================================
      uc_EventLocation=0;
      if( st_CardEventDataStruct->st_OptionalEventData.b_IsEventPlaceExist )
        uc_EventLocation|= 1;
      if( st_CardEventDataStruct->st_OptionalEventData.b_IsEventLineExist )
        uc_EventLocation|= 1<<1;
      if( st_CardEventDataStruct->st_OptionalEventData.b_IsEventRFU1Exist)
        uc_EventLocation|= 1<<2;
      if( st_CardEventDataStruct->st_OptionalEventData.b_IsEventRunlDExist )
        uc_EventLocation|= 1<<3;
      if( st_CardEventDataStruct->st_OptionalEventData.b_IsEventDevice4Exist )
        uc_EventLocation|= 1<<4;
      if( st_CardEventDataStruct->st_OptionalEventData.b_IsEventRFU2Exist )
        uc_EventLocation|= 1<<5;
      if( st_CardEventDataStruct->st_OptionalEventData.b_IsEventInterchangeRightsExist )
        uc_EventLocation|= 1<<6;
      //uc_EventLocation field offset
      offset = (EVENT_START_OPTIONAL_DATA_OFFSET -7);

      st_OptionalDataDesc.uc_StreamBitCount=7;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &uc_EventLocation;
      st_OptionalDataDesc.e_Fieltype = e_VariableType;


      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;

      //////////////////////////////////////////////////////
      //Check which optional data exist add them to buffer
      //////////////////////////////////////////////////////

      //if Event Place Exist
      if(st_CardEventDataStruct->st_OptionalEventData.b_IsEventPlaceExist)
      {

        st_OptionalDataDesc.uc_StreamBitCount=16;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.ush_EventPlace;

        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }
      ///////////////////////////////////////////////
      //Read EventLocation optional data
      ///////////////////////////////////////////////

      //if Event Line Exist
      if(st_CardEventDataStruct->st_OptionalEventData.b_IsEventLineExist)
      {
        st_OptionalDataDesc.uc_StreamBitCount=16;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.ush_EventLine;

        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //if Event Station Exist;
      if(st_CardEventDataStruct->st_OptionalEventData.b_IsEventRFU1Exist)
      {
        st_OptionalDataDesc.uc_StreamBitCount=8;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.uc_EventRFU1;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //if Event Run lD Exist;
      if(st_CardEventDataStruct->st_OptionalEventData.b_IsEventRunlDExist)
      {
        st_OptionalDataDesc.uc_StreamBitCount=12;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.ush_EventRunlD;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //if Event Device Exist;
      if(st_CardEventDataStruct->st_OptionalEventData.b_IsEventDevice4Exist)
      {
        st_OptionalDataDesc.uc_StreamBitCount=14;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.ush_EventDevice4;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //if Event RFU2 Exist;
      if(st_CardEventDataStruct->st_OptionalEventData.b_IsEventRFU2Exist)
      {
        st_OptionalDataDesc.uc_StreamBitCount=4;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.uc_EventRFU2;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //if Event InterchangeRights Exist;
      if(st_CardEventDataStruct->st_OptionalEventData.b_IsEventInterchangeRightsExist)
      {
        st_OptionalDataDesc.uc_StreamBitCount=8;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.uc_EventInterchangeRights;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //==================================================================
      //build Event Extention Mandatory field - using the optional data flags
      //==================================================================
      uc_EventExtention=0;
      if( st_CardEventDataStruct->st_OptionalEventData.b_IsEventTicketExist )
        uc_EventExtention|= 1;
      if( st_CardEventDataStruct->st_OptionalEventData.b_IsEventPassengersNumberExist )
        uc_EventExtention|= 1<<1;

      st_OptionalDataDesc.uc_StreamBitCount=3;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &uc_EventExtention;

      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      //increment offset only if field exist
      offset+=st_OptionalDataDesc.uc_StreamBitCount;


      //if Event Place Exist
      ///////////////////////////////////////////////
      //build EventExtention optional data
      ///////////////////////////////////////////////

      //if Event Ticket Exist;
      if(st_CardEventDataStruct->st_OptionalEventData.b_IsEventTicketExist)
      {
        stEventTicketOptionalDataDesc[0].e_Fieltype = e_VariableType;
        stEventTicketOptionalDataDesc[0].uc_StreamBitCount=10;
        stEventTicketOptionalDataDesc[0].us_StreamBitOffset= offset;
        stEventTicketOptionalDataDesc[0].vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.st_EventTicket.ush_EventTicketRoutesSystem;


        offset+=stEventTicketOptionalDataDesc[0].uc_StreamBitCount;

        stEventTicketOptionalDataDesc[1].e_Fieltype = e_VariableType;
        stEventTicketOptionalDataDesc[1].uc_StreamBitCount=8;
        stEventTicketOptionalDataDesc[1].us_StreamBitOffset= offset;
        stEventTicketOptionalDataDesc[1].vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.st_EventTicket.uc_EventTicketFareCode;


        offset+=stEventTicketOptionalDataDesc[1].uc_StreamBitCount;

        stEventTicketOptionalDataDesc[2].e_Fieltype = e_VariableType;
        stEventTicketOptionalDataDesc[2].uc_StreamBitCount=16;
        stEventTicketOptionalDataDesc[2].us_StreamBitOffset= offset;
        stEventTicketOptionalDataDesc[2].vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.st_EventTicket.ush_EventTicketDebitAmount;


        offset+=stEventTicketOptionalDataDesc[2].uc_StreamBitCount;
        ///////////////////////////////
        //  Check if legal structure
        ///////////////////////////////
        if(offset > (REC_SIZE*8))
          return e_ClyApp_CardContractErr;

        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &stEventTicketOptionalDataDesc[0],//[IN] Field Descriptor Array
          sizeof(stEventTicketOptionalDataDesc)/sizeof(stEventTicketOptionalDataDesc[0]),//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        // offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //if Event Passengers Number Exist;
      if(st_CardEventDataStruct->st_OptionalEventData.b_IsEventPassengersNumberExist)
      {
        st_OptionalDataDesc.uc_StreamBitCount=5;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &st_CardEventDataStruct->st_OptionalEventData.uc_EventPassengersNumber;


        ///////////////////////////////
        //  Check if legal structure
        ///////////////////////////////
        if((offset+st_OptionalDataDesc.uc_StreamBitCount) > (REC_SIZE*8))
          return e_ClyApp_CardContractErr;

        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      break;

    case e_clyCard_ContractsFile:

      union_ContractRecord = (union_ClyApp_ContractRecord*)vp_St;

      st_OptionalDataDesc.e_Fieltype = e_VariableType;

      //////////////////////////
      //Contract Version Number
      //////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=3;
      st_OptionalDataDesc.us_StreamBitOffset= (offset=0);
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractVersionNumber;


      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;

      /////////////////////////////////
      //Contract Validity Start Date
      /////////////////////////////////
      //get date
      memset(ucp_Data,0,sizeof(ucp_Data));
      v_Internal_TranslateType(e_St2BitStream,e_ClyApp_DateReverseType,(unsigned char *)ucp_Data,&union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractValidityStartDate);

      st_OptionalDataDesc.uc_StreamBitCount=14;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = ucp_Data;


      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;



      /////////////////////////////////
      //Contract Provider
      /////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=8;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractProvider;


      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;



      /////////////////////////////////
      //Contract tariff
      /////////////////////////////////


      stTariffDesc[0].e_Fieltype = e_VariableType;
      stTariffDesc[0].uc_StreamBitCount=2;
      stTariffDesc[0].us_StreamBitOffset= offset;
      stTariffDesc[0].vp_StFieldPtr =  &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffTransportType;


      offset+=stTariffDesc[0].uc_StreamBitCount;

      stTariffDesc[1].e_Fieltype = e_VariableType;
      stTariffDesc[1].uc_StreamBitCount=3;
      stTariffDesc[1].us_StreamBitOffset= offset;
      stTariffDesc[1].vp_StFieldPtr =  &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType;


      offset+=stTariffDesc[1].uc_StreamBitCount;

      stTariffDesc[2].e_Fieltype = e_VariableType;
      stTariffDesc[2].uc_StreamBitCount=6;
      stTariffDesc[2].us_StreamBitOffset= offset;
      stTariffDesc[2].vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType;


      v_Bit2Byte_Convert(  e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        stTariffDesc,//[IN] Field Descriptor Array
        sizeof(stTariffDesc)/sizeof(stTariffDesc[0]),//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream



      //increment offset
      offset+=stTariffDesc[2].uc_StreamBitCount;


      /////////////////////////////////
      //Contract Sale Date
      /////////////////////////////////
      //get date
      memset(ucp_Data,0,sizeof(ucp_Data));
      v_Internal_TranslateType(e_St2BitStream,e_ClyApp_DateCompactType,(unsigned char *)ucp_Data,&union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractSaleDate);

      st_OptionalDataDesc.uc_StreamBitCount=14;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = ucp_Data;


      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;

      /////////////////////////////////
      //Contract Sale Device
      /////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=12;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.sh_ContractSaleDevice;

      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;


      /////////////////////////////////
      //Contract Sale Number Daily
      /////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=10;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.sh_ContractSaleNumberDaily;


      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;

      /////////////////////////////////
      //Contract Sale Number Daily
      /////////////////////////////////
      st_OptionalDataDesc.uc_StreamBitCount=1;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.b_ContractIsJourneylnterchangesAllowed;


      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;


      /////////////////////////////////
      //build ContractValidityInfo
      /////////////////////////////////
      sh_ContractValidityInfo=0;
      if( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_IsContractRestrictTimeCodeExist )
        sh_ContractValidityInfo|= 1;
      if( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictCodeExist )
        sh_ContractValidityInfo|= 1<<1;
      if( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictDurationExist)
        sh_ContractValidityInfo|= 1<<2;
      if( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityEndDateExist)
        sh_ContractValidityInfo|= 1<<3;
      if( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityDurationExist )
        sh_ContractValidityInfo|= 1<<4;
      if( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist )
        sh_ContractValidityInfo|= 1<<5;
      if( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractCustomerProfileExist )
        sh_ContractValidityInfo|= 1<<6;
      if( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPassengersNumberExist )
        sh_ContractValidityInfo|= 1<<7;
      if( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRFUExist )
        sh_ContractValidityInfo|= 1<<8;

      st_OptionalDataDesc.uc_StreamBitCount=9;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &sh_ContractValidityInfo;


      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      //increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;

      /////////////////////////////////
      //START READ OPTIONAL DATA
      /////////////////////////////////

      // Is Contract Restrict Time Code Exist;
      if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_IsContractRestrictTimeCodeExist)
      {

        st_OptionalDataDesc.uc_StreamBitCount=5;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictTimeCode;

        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //is Contract Restrict Code Exist;
      if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictCodeExist)
      {
        st_OptionalDataDesc.uc_StreamBitCount=5;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictCode;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      // is Contract Restrict Duration Exist
      if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictDurationExist)
      {

        st_OptionalDataDesc.uc_StreamBitCount=6;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictDuration;

        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }


      // is Contract Validity End Date Exist
      if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityEndDateExist)
      {
        //get date
        memset(ucp_Data,0,sizeof(ucp_Data));
        v_Internal_TranslateType(e_St2BitStream,e_ClyApp_DateCompactType,(unsigned char *)ucp_Data,&union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityEndDate);

        st_OptionalDataDesc.uc_StreamBitCount=14;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = ucp_Data;

        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      // is Contract Validity Duration Exist;
      if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityDurationExist)
      {


        stValidityDurationDesc[0].e_Fieltype = e_VariableType;
        stValidityDurationDesc[0].uc_StreamBitCount=2;
        stValidityDurationDesc[0].us_StreamBitOffset= offset;
        stValidityDurationDesc[0].vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.e_DurationType;


        offset+=stValidityDurationDesc[0].uc_StreamBitCount;

        stValidityDurationDesc[1].e_Fieltype = e_VariableType;
        stValidityDurationDesc[1].uc_StreamBitCount=6;
        stValidityDurationDesc[1].us_StreamBitOffset= offset;
        stValidityDurationDesc[1].vp_StFieldPtr =  &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.uc_DurationUnitCount;



        v_Bit2Byte_Convert(  e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          stValidityDurationDesc,//[IN] Field Descriptor Array
          sizeof(stValidityDurationDesc)/sizeof(stValidityDurationDesc[0]),//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream


        offset+=stValidityDurationDesc[1].uc_StreamBitCount;
      }

      // is Contract Period Journeys Exist;
      if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist)
      {

        stValidityDurationDesc[0].e_Fieltype = e_VariableType;
        stValidityDurationDesc[0].uc_StreamBitCount=2;
        stValidityDurationDesc[0].us_StreamBitOffset= offset;
        stValidityDurationDesc[0].vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.e_PeriodType;

        offset+=stValidityDurationDesc[0].uc_StreamBitCount;

        stValidityDurationDesc[1].e_Fieltype = e_VariableType;
        stValidityDurationDesc[1].uc_StreamBitCount=6;
        stValidityDurationDesc[1].us_StreamBitOffset= offset;
        stValidityDurationDesc[1].vp_StFieldPtr =  &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.uc_MaxNumOfTripsInPeriod;

        v_Bit2Byte_Convert(  e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          stValidityDurationDesc,//[IN] Field Descriptor Array
          sizeof(stValidityDurationDesc)/sizeof(stValidityDurationDesc[0]),//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream



        offset+=stValidityDurationDesc[1].uc_StreamBitCount;
      }

      // is Contract Customer Profile Exist;
      if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractCustomerProfileExist)
      {

        st_OptionalDataDesc.uc_StreamBitCount=6;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractCustomerProfile;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      // is Contract Passengers Number Exist;
      if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPassengersNumberExist)
      {

        st_OptionalDataDesc.uc_StreamBitCount=5;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractPassengersNumber;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }
      // is Contract RFU Number Exist;
      if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRFUExist)
      {

        st_OptionalDataDesc.uc_StreamBitCount=32;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.ul_ContractRFUval;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset only if field exist
        offset+=st_OptionalDataDesc.uc_StreamBitCount;
      }

      //////////////////////////////////////
      // WRITE ALL VALIDITY LOCATION LIST
      //////////////////////////////////////

      uc_LocationArrLen = union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen;

      for(i=0;i<uc_LocationArrLen;i++)
      {
        ///////////////////////////////////////////////////////////
        //  Check if legal structure - last byte reserved for AUTH
        ///////////////////////////////////////////////////////////
        if(offset>((REC_SIZE-1)*8))
          return e_ClyApp_CardContractErr;

        e_CardSpatialType = union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType;
        st_OptionalDataDesc.uc_StreamBitCount=4;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &e_CardSpatialType;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //increment offset
        offset+=st_OptionalDataDesc.uc_StreamBitCount;

        switch(e_CardSpatialType)
        {
        case e_CardSpatialTypeZones:

          st_OptionalDataDesc.uc_StreamBitCount=10;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Zones.ush_SpatialRoutesSystem;

          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Zones.ush_SpatialZones;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          break;

        case e_CardSpatialTypeFareCode:
          st_OptionalDataDesc.uc_StreamBitCount=10;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_FareCode.ush_SpatialRoutesSystem;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_FareCode.uc_SpatialFareCode;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          break;

        case e_CardSpatialTypeLinesList:
          // get number of lines
          st_OptionalDataDesc.uc_StreamBitCount=4;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_LinesList.uc_SpatiaiLineArrLen;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;
          //read all lines
          for(j=0;j<union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_LinesList.uc_SpatiaiLineArrLen;j++)
          {

            st_OptionalDataDesc.uc_StreamBitCount=16;
            st_OptionalDataDesc.us_StreamBitOffset= offset;
            st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[(int)i].union_ContractValidityLocation.st_LinesList.ush_SpatiaiLineArr[(int)j];

            v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
              &st_OptionalDataDesc,//[IN] Field Descriptor Array
              1,//[IN]Field Descriptor Array len
              ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

            offset+=st_OptionalDataDesc.uc_StreamBitCount;
          }

          break;

        case e_CardSpatialTypeRide:
          st_OptionalDataDesc.uc_StreamBitCount=16;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Ride.ush_SpatialLine;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Ride.uc_SpatialStationOrigin;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Ride.uc_SpatialStationDestination;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          break;

        case e_CardSpatialTypeRideAndRunType:
          st_OptionalDataDesc.uc_StreamBitCount=16;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunType.ush_SpatialLine;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunType.uc_SpatialStationOrigin;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunType.uc_SpatialStationDestination;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=4;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunType.uc_SpatialRunType;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;


          break;

        case e_CardSpatialTypeRideAndRunID:
          st_OptionalDataDesc.uc_StreamBitCount=16;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunID.ush_SpatialLine;

          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunID.uc_SpatialStationOrigin;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunID.uc_SpatialStationDestination;

          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideAndRunID.ush_SpatialRunlD;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          break;

        case e_CardSpatialTypeRideRunAndSeat:
          st_OptionalDataDesc.uc_StreamBitCount=16;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.ush_SpatialLine;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.uc_SpatialStationOrigin;

          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.uc_SpatialStationDestination;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.ush_SpatialRunlD;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=4;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.uc_SpatialVehicleCoach;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=7;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideRunAndSeat.uc_SpatialSeat;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;


          break;

        case e_CardSpatialTypeRideZones:
          st_OptionalDataDesc.uc_StreamBitCount=10;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideZones.ush_SpatialRoutesSystemFrom;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideZones.ush_SpatialZonesFrom;

          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=10;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideZones.ush_SpatialRoutesSystemTo;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RideZones.ush_SpatialZonesTo;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;


          break;

        case e_CardSpatialTypeParking:
          //check value
          if(!(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Parking.uc_SpatialParkingDataSize>=12 &&
            union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Parking.uc_SpatialParkingDataSize<=75))
            return e_ClyApp_CardContractErr;

          st_OptionalDataDesc.uc_StreamBitCount=6;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Parking.uc_SpatialParkingDataSize;

          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=12 + union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Parking.uc_SpatialParkingDataSize;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_Parking.uc_SpatialParkingData;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          break;

        case e_CardSpatialTypePredefinedContract:
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].e_CardSpatialType = e_CardSpatialTypePredefinedContract;
          st_OptionalDataDesc.uc_StreamBitCount=3;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_PredefinedContract.uc_Tariff_Lsb;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=11;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_PredefinedContract.ush_SpetailCode;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;



          break;

          //Yoni 08/2011 handle e_CardSpatialTypeRouteSystemsList and e_CardSpatialTypeFareCodeExtension
        case e_CardSpatialTypeRouteSystemsList:
          //RouteSystemsNumber- 4bits- Number of Route Systems in the list
          //Route System1- 10bits -Identifier (1 to 1023) of the first route system where the ticket is valid
          //...
          //Route SystemN- 10bits -Identifier (1 to 1023) of the last route system where the ticket is valid
          // get number of routesystems
          st_OptionalDataDesc.uc_StreamBitCount=4;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RouteSystemsList.uc_SpatiaiRouteSystemArrLen;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;
          //read all routesystems
          for(j=0;j<union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RouteSystemsList.uc_SpatiaiRouteSystemArrLen;j++)
          {

            st_OptionalDataDesc.uc_StreamBitCount=10;
            st_OptionalDataDesc.us_StreamBitOffset= offset;
            st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_RouteSystemsList.ush_SpatiaiRouteSystemArr[(int)j];

            v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
              &st_OptionalDataDesc,//[IN] Field Descriptor Array
              1,//[IN]Field Descriptor Array len
              ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

            offset+=st_OptionalDataDesc.uc_StreamBitCount;
          }
          break;

        case e_CardSpatialTypeFareCodeExtension:
          //SpatialRoutesSystem- 10bits -Identifier (1 to 1023) of the routes system where the ticket is valid. 0 � means any route system.
          //FareRestrictionCode- 3bits - Restriction code with the following values:
          //  0 � any fare code
          //  1 � only specified fare code
          //  2 � any fare code up to the specified value (included)
          //  3 � any fare code above the specified value (included)
          //  4 � preferred fare code to be used, unless another code was specified.
          //  5�7 � RFU
          //SpatialFareCode- 8bits -Fare code (1 to 255) for the ticket, within SpatialRoutesSystem
          st_OptionalDataDesc.uc_StreamBitCount=10;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_FareCodeExtension.ush_SpatialRoutesSystem;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          st_OptionalDataDesc.uc_StreamBitCount=3;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_FareCodeExtension.uc_FareRestrictionCode;
          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;


          st_OptionalDataDesc.uc_StreamBitCount=8;
          st_OptionalDataDesc.us_StreamBitOffset= offset;
          st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i].union_ContractValidityLocation.st_FareCodeExtension.uc_SpatialFareCode;


          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
            &st_OptionalDataDesc,//[IN] Field Descriptor Array
            1,//[IN]Field Descriptor Array len
            ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

          offset+=st_OptionalDataDesc.uc_StreamBitCount;

          break;



        default: return e_ClyApp_CardContractErr; //if data structur is unknow - return err
        }


      }
      ///////////////////////////////////////////////////////////
      //  Check if legal structure - last byte reserved for AUTH
      ///////////////////////////////////////////////////////////
      if(offset>((REC_SIZE-1)*8))
        return e_ClyApp_CardContractErr;

      //==================================
      //WRITE END VALIDITY END LIST VALUE
      //==================================

      e_CardSpatialType = e_CardSpatialTypeEndLocationList;

      st_OptionalDataDesc.uc_StreamBitCount=4;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &e_CardSpatialType;


      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      //Increment offset
      offset+=st_OptionalDataDesc.uc_StreamBitCount;

      ////////////////////////////////////////////////////////////////////////////////
      //WRITE Contract Authenticator value - WRITTEN IN THE LAST 8 BITS OF THE RECORD
      ////////////////////////////////////////////////////////////////////////////////
      union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractAuthenticator = uc_CalcExpectedContractAuthVal( ucp_BitStream,//[IN] binary data
        REC_SIZE);//[IN] binary data bit length


      offset = ((REC_SIZE*8)-8);
      st_OptionalDataDesc.uc_StreamBitCount=8;
      st_OptionalDataDesc.us_StreamBitOffset= offset;
      st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractAuthenticator;


      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
        &st_OptionalDataDesc,//[IN] Field Descriptor Array
        1,//[IN]Field Descriptor Array len
        ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream


      break;
    case e_clyCard_CountersFile:
      /////////////////////////////////////////////
      // if counter as Date And Remaining Journeys
      ////////////////////////////////////////////
      offset =0;
      if(union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.e_CardCounterRecordType == e_ClyApp_CardCounter_DateAndRemainingJourneys)
      {
        //get data
        memset(ucp_Data,0,sizeof(ucp_Data));
        v_Internal_TranslateType(e_St2BitStream,e_ClyApp_DateReverseType,(unsigned char *)ucp_Data,&union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.st_CounterDate);

        st_OptionalDataDesc.uc_StreamBitCount=14;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = (unsigned short*)ucp_Data;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

        //Increment offset
        offset+=st_OptionalDataDesc.uc_StreamBitCount;

        st_OptionalDataDesc.uc_StreamBitCount=10;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.CounterValue;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream


      }
      else // counter as Number Of Tokens Or Amount
      {
        if(union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.e_CardCounterRecordType !=e_ClyApp_CardCounter_NumberOfTokensOrAmount   )
          return e_ClyApp_WrongParamErr;
        st_OptionalDataDesc.uc_StreamBitCount=24;
        st_OptionalDataDesc.us_StreamBitOffset= offset;
        st_OptionalDataDesc.vp_StFieldPtr = &union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue;


        v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
          &st_OptionalDataDesc,//[IN] Field Descriptor Array
          1,//[IN]Field Descriptor Array len
          ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream

      }
      break;

    case e_clyCard_ContractListFile://Yoni 10/2011
      pContractList = (st_clyApp_ContractListStruct*)vp_St;
      offset=0;
      memset(ucp_Data,0,sizeof(ucp_Data));
      ////////  ver
      l_oContractListDescriptor.uc_StreamBitCount=3;
      l_oContractListDescriptor.us_StreamBitOffset=offset=0;
      l_oContractListDescriptor.vp_StFieldPtr=&pContractList->uc_Ver;
      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
              &l_oContractListDescriptor,//[IN] Field Descriptor Array
              1,//[IN]Field Descriptor Array len
              ucp_BitStream);
      offset+=l_oContractListDescriptor.uc_StreamBitCount;
      ////////  bitmap

      l_oContractListDescriptor.uc_StreamBitCount=16;
      l_oContractListDescriptor.us_StreamBitOffset=offset;
      l_oContractListDescriptor.vp_StFieldPtr=&pContractList->us_Bitmap;
      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
              &l_oContractListDescriptor,//[IN] Field Descriptor Array
              1,//[IN]Field Descriptor Array len
              ucp_BitStream);
      offset+=l_oContractListDescriptor.uc_StreamBitCount;
      ////////  code array
      for(j=0;j<8;j++)
      {
        if((pContractList->us_Bitmap) & (1<<j))
        {
          l_oContractListDescriptor.uc_StreamBitCount=16;
          l_oContractListDescriptor.us_StreamBitOffset=offset;
          l_oContractListDescriptor.vp_StFieldPtr=&pContractList->ContractListAuthorizationCodeArr[j];
          v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
                  &l_oContractListDescriptor,//[IN] Field Descriptor Array
                  1,//[IN]Field Descriptor Array len
                  ucp_BitStream);
          offset+=l_oContractListDescriptor.uc_StreamBitCount;

        }
      }

      pContractList->ContractListAuthenticator = uc_CalcExpectedContractAuthVal( ucp_BitStream,//[IN] binary data
        REC_SIZE);//[IN] binary data bit length


      ////////  authenticator
      l_oContractListDescriptor.uc_StreamBitCount=8;
      l_oContractListDescriptor.us_StreamBitOffset=(29*8-8);//there's padding and auth is last 8 bits
      l_oContractListDescriptor.vp_StFieldPtr=&pContractList->ContractListAuthenticator;
      v_Bit2Byte_Convert( e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
              &l_oContractListDescriptor,//[IN] Field Descriptor Array
              1,//[IN]Field Descriptor Array len
              ucp_BitStream);

      break;

    default: return e_ClyApp_WrongParamErr;
    }
  }


  return e_ClyApp_Ok;
}


//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                b_Internal_IsContractSecurity0BitCoutOK
//
//DESCRITION:
//
//                Is Contract Security 0 Bit Cout OK - AUTH check
//
//RETURN:
//
//                clyApp_BOOL
//LOGIC :
//                check if value found == value calculated
///////////////////////////////////////////////////////////////////////////////////////////////////

static clyApp_BOOL b_Internal_IsContractSecurity0BitCoutOK(char c_RecNumber)//[IN]Rec Number
{

  unsigned char  uc_ExpectedResult;
  //get bin record
  unsigned char * Rec = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractRecArr[(int)c_RecNumber];

  //count the number of zero in the binary buffer
  uc_ExpectedResult = uc_CalcExpectedContractAuthVal( Rec,//[IN] binary data
    REC_SIZE);//[IN] binary data bit length ( always full record )

  //check if calculate result is equal to the value found in the record
  if( uc_ExpectedResult == st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_RecNumber].st_CardContractIssuingData.uc_ContractAuthenticator )
    return clyApp_TRUE;
  return clyApp_FALSE;


}

static clyApp_BOOL b_Internal_IsContractListAuthOk()
{
  unsigned char  uc_ExpectedResult;
  //get bin record
  unsigned char * Rec = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractList;

  //count the number of zero in the binary buffer
  uc_ExpectedResult = uc_CalcExpectedContractAuthVal( Rec,//[IN] binary data
    REC_SIZE);//[IN] binary data bit length ( always full record )

  //check if calculate result is equal to the value found in the record
  if( uc_ExpectedResult == st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractList.ContractListAuthenticator)
    return clyApp_TRUE;

  return clyApp_FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                b_Internal_IsValidCounterType
//
//DESCRITION:
//
//                Is Valid Counter Type - check if legal counter type for the contract
//
//RETURN:
//
//                -
//LOGIC:
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static clyApp_BOOL b_Internal_IsValidCounterType( union_ClyApp_ContractRecord *union_ContractRecord )
{

  //get counter type fron contract tariff
  e_ClyApp_TariffCounterType e_TariffCounterType = union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType;


  switch(e_TariffCounterType)
  {
  case e_ClyApp_CounterNotUsed:
    if( ( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType == e_ClyApp_OneTimeOrMultiRideTicket ) ||
      ( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType == e_ClyApp_TransferTick ) ||
      ( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType == e_ClyApp_StoredValue ) ||
      ( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist  ) ||
      ( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType == e_ClyApp_OneTimeOrMultiRideTicket46 )
      )
      return clyApp_FALSE;
    break;

  case e_ClyApp_CounterAsDateAndRemainingNumOfJourneys:
  case e_ClyApp_CounterAsNumOfToken:
  case e_ClyApp_CounterAsMonetaryAmount:

    //if tariff equal in contract and in counter
    if( (union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType == e_ClyApp_CounterAsDateAndRemainingNumOfJourneys) &&
      (union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.e_CardCounterRecordType != e_ClyApp_CardCounter_DateAndRemainingJourneys)
      )
      return clyApp_FALSE;

    if( (union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType == e_ClyApp_CounterAsNumOfToken || union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType == e_ClyApp_CounterAsMonetaryAmount   ) &&
      (union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.e_CardCounterRecordType != e_ClyApp_CardCounter_NumberOfTokensOrAmount)
      )
      return clyApp_FALSE;


    //check counter start value - if not Period Journeys type - must be greater then Zero
    if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist)
    {
                        //Yoni 03/2013 allow e_PeriodKartisiaHemshech wuth e_ClyApp_CounterAsNumOfToken
                        if(e_PeriodKartisiaHemshech!=union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.e_PeriodType)
                        {
      //if Period Journeys field Exist && tariff is not "date + number" - return err
      if(  e_TariffCounterType != e_ClyApp_CounterAsDateAndRemainingNumOfJourneys )
        return clyApp_FALSE;
                        }

    }
                    /*
                    else
                    {
                        //check legal value
                        if ( (e_TariffCounterType == e_ClyApp_CounterAsNumOfToken || e_TariffCounterType == e_ClyApp_CounterAsMonetaryAmount ) && union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue == 0)
                            return clyApp_FALSE;                                    
                    }
                    */
                    

    break;

  }
  return clyApp_TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                b_Internal_IsLegalProfile
//
//DESCRITION:
//
//                Check if contract Profile is legal - if found in ENV file
//
//RETURN:
//
//                clyApp_BOOL
//LOGIC :  contract profile has to match one of the still valid env profiles
//         Fixed by Yoni on 28/7/10
//         1. input paramter is current date, doesn't return the profile end date
//         2. handle case in which 2 identical profiles exist and the second
//            one has later expiration date, then return the second date
//
///////////////////////////////////////////////////////////////////////////////////////////////////
static clyApp_BOOL b_Internal_IsLegalProfile(e_ClyApp_CardPriorityType e_ContractProfile,//[IN] profile to check
  const st_Cly_Date* st_CurrentDate //[IN] current date //changed on 28/7/10
  )

{

  long l;
  st_Cly_Date st_EndDateCompact;

  //Check if ENV file exist
  if( !st_Static_StateMachine.st_TransactionData.b_IsEnvRecExist)
    return clyApp_FALSE;//err

  //if contratc profile == env profile1 and prof1 is still valid => return true
  if(e_ContractProfile == st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec.st_HoiderProf1.uc_HoiderProfCode)
  {
    st_EndDateCompact = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec.st_HoiderProf1.st_HoiderProfDate;
    l=l_Internal_DateCmp((st_Cly_Date*)st_CurrentDate,&st_EndDateCompact);
    //if profile end date >= current date then return true
    if( l <=0 )
    {
      return clyApp_TRUE;
    }
    //else check if the second profile is valid-it could be the same profile
  }
  //if contratc profile == env profile2 and prof2 is still valid => return true
  if(e_ContractProfile == st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec.st_HoiderProf2.uc_HoiderProfCode )
  {
    st_EndDateCompact = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec.st_HoiderProf2.st_HoiderProfDate;
    l=l_Internal_DateCmp((st_Cly_Date*)st_CurrentDate,&st_EndDateCompact);
    //if profile end date >= current date then return true
    if( l <=0 )
      return clyApp_TRUE;
  }

  //either contract profle doesn't match any of env profiles or they are expired
  return clyApp_FALSE;
}



//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_BasicCalypsoCheckIfRecValid
//
//DESCRITION:
//
//                 check basic Calypso informatio to conform that the record is valid
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////////////////////////

static eCalypsoErr e_Internal_BasicCalypsoCheckIfRecValid(e_ClyApp_CardType e_CardType,//[IN]type - card \ ticket
                                                          e_clyCard_FileId e_CardRecType,//[IN]Rec file type
                                                          char c_RecNumber,//[IN]Rec Number
                                                          const void *vp_St)//[IN] the rec to check
{
  st_ClyApp_EnvAndHoldDataStruct *stp_EnvAndHoldDataStruct = (st_ClyApp_EnvAndHoldDataStruct *)vp_St;
  union_ClyApp_ContractRecord *union_ContractRecord = (union_ClyApp_ContractRecord*)vp_St;
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct = (st_clyApp_CardEventDataStruct *)vp_St;
  st_Cly_DateAndTime st_DateAndTime;
  long l_DateCurrent,l_DateEnvEnd;
  //    st_Cly_Date st_EndDateCompact;
  //    long l;
  //get current date from the application
  if(!st_Static_StateMachine.st_UserCallbacks.fp_DateAndTimeCallBack(&st_DateAndTime))
    return e_ClyApp_UnknownErr;


  //if card
  if(e_CardType == e_ClyApp_Card)
  {

    //st_ClyApp_Params stp_Params;
    //if(!st_Static_StateMachine.st_UserCallbacks.fp_b_GetParams(&stp_Params))
    //{
    //  return e_ClyApp_UnknownErr;
    //}
    // if card
    switch(e_CardRecType)
    {
    case e_clyCard_EnvironmentFile:
      if( !(
        (stp_EnvAndHoldDataStruct->uc_EnvPayMethod & PAY_METHOD_PRE_PAYMENT) &&
        (stp_EnvAndHoldDataStruct->sh_EnvCountryld == ISRAEL_COUNTRY_ISO_IDENTIFICATION) &&
        (stp_EnvAndHoldDataStruct->uc_EnvApplicationVersionNumber == ENV_VERSION_NUM)
        )
        )
        return e_ClyApp_CardEnvErr;


      //get current long date
      l_DateCurrent = ush_GetDateCompact(&st_DateAndTime.st_Date);
      l_DateEnvEnd = ush_GetDateCompact(&stp_EnvAndHoldDataStruct->st_EnvEndDate);

      //check if the card end date has not yet expired
      if( l_DateCurrent > l_DateEnvEnd )
        return e_ClyApp_CardEnvEndDateErr;
      break;

    case e_clyCard_EventLogFile:
    case e_clyCard_SpecialEventFile:
      if( stp_CardEventDataStruct->uc_EventVersionNumber != EVENT_VERSION_NUM )
        return e_ClyApp_CardEventErr;

      break;


    case e_clyCard_ContractsFile:
      //check record number only if exist
      if(c_RecNumber)
        if(! b_Internal_IsContractSecurity0BitCoutOK( c_RecNumber ) )
          return e_ClyApp_CardContractLRCErr;
        //////////////////////////////////////////////////////////////////////
        //Check tariff - expecting TransportAccessOnly or ParkingAndTransport
        //////////////////////////////////////////////////////////////////////

        if(!(
          (union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractVersionNumber == CONTRACT_VERSION_NUM) && // check contract Version Number
          ((union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffTransportType == e_ClyApp_TransportAccessOnly) || (union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffTransportType == e_ClyApp_ParkingAndTransport) ) ) // check for Transport application
          )
          return e_ClyApp_CardContractErr;

        //check is the contract has a valid counter type
        if( !b_Internal_IsValidCounterType(union_ContractRecord ) )
          return e_ClyApp_WrongParamErr;

        ///////////////////////////////////////////////
        //Check contract profile && profile end date
        ///////////////////////////////////////////////

        // if Customer Profile Exist && !=0 - check if legal type and date
        if(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractCustomerProfileExist &&
          union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractCustomerProfile)
        {
          //can not load card with un exist profile - The contract profile must exist in ENV && contract end date can not exceed the end date of the profile
          //fixed by Yoni 28/7/10: check date validity inside function
          if(!b_Internal_IsLegalProfile((e_ClyApp_CardPriorityType)union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractCustomerProfile,
            &st_DateAndTime.st_Date) )
            return e_ClyApp_CardProfileErr;
        }

        break;
    case e_clyCard_CountersFile:
      break;

    case e_clyCard_ContractListFile:
      //check auth
      if(b_Internal_IsContractListAuthOk() == clyApp_FALSE)
      {
        return e_ClyApp_CardContractListLRCErr;
      }

    default: return e_ClyApp_WrongParamErr;
    }
  }
  else// ticket
  {
    struct_ClyTkt_Ticket *struct_Ticket  = (struct_ClyTkt_Ticket *)vp_St;

    //check common data
    if( struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex != TKT_CONTRACT_LOAD_KEY ||
      struct_Ticket->st_TicketContractCommonData.uc_ReloadCount > MAX_RELOAD_COUT ||
      struct_Ticket->st_TicketContractCommonData.st_Tariff == e_ClyTkt_TariffRFU1 ||
      struct_Ticket->st_TicketContractCommonData.st_Tariff >= e_ClyTkt_TariffRFU2 )
      return e_ClyApp_WrongParamErr;

    //check multi - ride
    if( IS_MULTI_TKT( struct_Ticket->st_TicketContractCommonData.st_Tariff ) )
    {
      //check multi - ride data
      if ( struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.e_TMF_Direction > e_ClyTkt_Backward ||
        struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys > struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.uc_TMC_ValidityJourneys )
        return e_ClyApp_WrongParamErr;

    }
    else // season pass
    {
      //===============================================================================================
      // if Duration is In Hours Validity CAN NOT Starts At FirstUse ( can not be slidind start date )
      //===============================================================================================
      if( struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.e_TSC_Sliding == e_ClyTkt_ValidityStartsAtFirstUse &&
        struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType == e_ClyTkt_DurationInHours)
        return e_ClyApp_WrongParamErr;

      //=========================================================================================================
      //if sliding contract OR contract in hours && contract was used  -> check that stat date of first use exist
      //=========================================================================================================
      if( (struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.e_TSC_Sliding == e_ClyTkt_ValidityStartsAtFirstUse ||
        struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType == e_ClyTkt_DurationInHours)  &&
        (struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.TSL_IsVirginFlag == clyApp_FALSE &&
        ush_GetDateCompact( &struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start.st_Date) == 0 ) )
        return e_ClyApp_WrongParamErr;

      //check duration type - not RFU
      if(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType == e_ClyTkt_DurationRFU )
        return e_ClyApp_WrongParamErr;
    }

  }

  return e_ClyApp_Ok;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                b_IsContractValidBasic
//DESCRITION:
//
//                Check: Contract version, Contract authenticator, e_TariffTransportType
//RETURN:
//
//LOGIC :
//////////////////////////////////////////////////////////////////////////////////////////////////
clyApp_BOOL b_IsContractValidBasic(const union_ClyApp_ContractRecord *union_ContractRecord, char c_RecNumber)
{
  // Check: Contract version, Contrat authenticator, e_TariffTransportType
  if (e_ClyApp_Ok != e_Internal_BasicCalypsoCheckIfRecValid(e_ClyApp_Card, e_clyCard_ContractsFile, c_RecNumber, union_ContractRecord))
  {
    return clyApp_FALSE;
  }
  return clyApp_TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_ConvertTariffCounter2CounterType
//
//DESCRITION:
//
//
//                 Convert Tariff Counter enume to Counter Type enume
//
//RETURN:
//
//                e_ClyApp_CardCounterRecordType
//
//LOGIC :
//////////////////////////////////////////////////////////////////////////////////////////////////

static e_ClyApp_CardCounterRecordType e_Internal_ConvertTariffCounter2CounterType ( e_ClyApp_TariffCounterType e_TariffCounterType)//[IN] Tariff Counter enume
{
  switch(e_TariffCounterType)
  {
  case e_ClyApp_CounterNotUsed:
  case e_ClyApp_CounterAsDateAndRemainingNumOfJourneys:
    return e_ClyApp_CardCounter_DateAndRemainingJourneys;
    //  case e_ClyApp_CounterAsNumOfToken:
    //  case e_ClyApp_CounterAsMonetaryAmount:
    //      return e_ClyApp_CardCounter_NumberOfTokensOrAmount;
  default: break;
  }
  return e_ClyApp_CardCounter_NumberOfTokensOrAmount;

}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_UpdateCardSateMachin
//
//DESCRITION:
//
//                Update Card Sate Machine data
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static eCalypsoErr  e_Internal_UpdateCardSateMachin(   e_clyCard_FileId FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                                    clyCard_BYTE RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
                                                    clyCard_BYTE ucp_RecData[REC_SIZE],//[IN]bit stream
                                                    void* StOut, //[IN] St data
                                                    eCalypsoErr isRecordConversionValid) // if we convert the record from bits to a
                                                    // data structure - is covertion OK
{
  union_ClyApp_ContractRecord *union_ContractRecord = (union_ClyApp_ContractRecord*)StOut;
  //=======================================================
  //update state machine - bin and struct data information
  //=======================================================
  switch(FileToSelect)
  {
  case e_clyCard_EnvironmentFile:
    //rise exist flag
    st_Static_StateMachine.st_TransactionData.b_IsEnvRecExist = clyApp_TRUE;
    //copy  bin data
    memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EnvRec,ucp_RecData,REC_SIZE);
    //copy  struct data
    memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec,StOut,sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec));

    break;

  case e_clyCard_EventLogFile:
    //rise exist flag
    st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[RecNum]=clyApp_TRUE;
    //copy  bin data
    memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[RecNum],ucp_RecData,REC_SIZE);
    //copy  struct data
    memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[RecNum],
      StOut,sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[RecNum]));
    break;

  case e_clyCard_SpecialEventFile:
    //rise exist flag
    st_Static_StateMachine.st_TransactionData.b_IsSpecialEventRecExistArr[RecNum]=clyApp_TRUE;
    //copy  bin data
    memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_SpecialEventRecArr[RecNum],ucp_RecData,REC_SIZE);
    //copy  struct data
    memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[RecNum],
      StOut,sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[RecNum]));




    //b_Internal_UpdateSpecialEvent(RecNum, StOut);
    break;
  case e_clyCard_ContractsFile:
    //rise exist flag
    st_Static_StateMachine.st_TransactionData.b_IsContractRecExistArr[RecNum] =clyApp_TRUE;
    st_Static_StateMachine.st_TransactionData.isContractConversionValid[RecNum] = isRecordConversionValid;
    //copy  bin data
    memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractRecArr[RecNum],ucp_RecData,REC_SIZE);
    //copy  struct data
    memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardContractIssuingData,&union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData,sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardContractIssuingData));

    //store couner type
    st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardCounterRecord.e_CardCounterRecordType = e_Internal_ConvertTariffCounter2CounterType(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType );
    union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.e_CardCounterRecordType = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardCounterRecord.e_CardCounterRecordType;

    //we check authentication here
    st_Static_StateMachine.st_TransactionData.isContractAuthOk[RecNum]=b_Internal_IsContractSecurity0BitCoutOK(RecNum);

    break;

  case e_clyCard_CountersFile:
    //raise exist flag
    st_Static_StateMachine.st_TransactionData.b_IsCounterRecExistArr[RecNum] =clyApp_TRUE;
    st_Static_StateMachine.st_TransactionData.isCounterConversionValid[RecNum] = isRecordConversionValid;

    //copy  bin data if exist
    if( ucp_RecData != NULL)
      //each counter is three bytes size and all counters are reside is the counter record number 1 and placed one after the another
      memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec + COUNTER_OFFSET(RecNum),ucp_RecData,COUTER_SIZE);
    //copy  struct data
    memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardCounterRecord,&union_ContractRecord->st_CardContractRecord.st_CardCounterRecord,sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardCounterRecord));
    break;
  case e_clyCard_ContractListFile://Yoni 10/2011
    st_Static_StateMachine.st_TransactionData.b_IsContractListExist=clyApp_TRUE;
    //copy bin data and struct data
	  memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractList,ucp_RecData,REC_SIZE);
    memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractList, StOut, sizeof(st_clyApp_ContractListStruct));//struct
    break;

  default: return e_ClyApp_WrongParamErr;

  }
  return e_ClyApp_Ok; // return OK
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_ConvertTktMediaType
//
//DESCRITION:
//
//                 Convert Ticket Media Type from 7816 type to tktOs type
//
//RETURN:
//
//                e_ClyTkt_TicketMediaTypes
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////
static e_ClyTkt_TicketMediaTypes e_Internal_ConvertTktMediaType( e_7816_CardType e_7816CardType)
{
  switch(  e_7816CardType )
  {
  case e_7816_Cly_CTS256B:
    return e_ClyTkt_CTS256B;
  default: return e_ClyTkt_LastMediaType;
  }
}





//////////////////////////////////////////////////////////////////////////////
//  Yoni 11/2011
//
//FUNCTION:
//                e_Internal_ContractLogicalOR_Write
//
//DESCRITION:
//                does WriteRecord for contract (logical OR)
//                call this from UseContract because this action is enabled also in sam CV  (for sliding contract, first use)
//
//RETURN:
//                eCalypsoErr
//LOGIC :
//                convert to bin record, call pSt_ClyCard_WriteRecord
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static eCalypsoErr  e_Internal_ContractLogicalOR_Write(e_ClyApp_CardType e_CardType,//[IN]type - card \ ticket
                                     clyCard_BYTE RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
                                     void* StIn, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
                                     clyCard_BYTE uc_Len2Write//[IN] len to Write - 1 to record size
                                     )
{
  eCalypsoErr err;
  clyCard_BYTE ucp_RecData[REC_SIZE]={0};
  RESPONSE_OBJ* Obj;

  if( e_CardType != e_ClyApp_Card )
    return e_ClyApp_CardWriteErr;


  memset(ucp_RecData,0,sizeof(ucp_RecData));



  err = e_Internal_Bit2Byte_Convert(e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
    e_CardType,//[IN]type - card \ ticket
    e_clyCard_ContractsFile,//[IN] if not a ticket  - which record in the card
    ucp_RecData,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
    StIn);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
  if( err!=e_ClyApp_Ok)
    return err;


  if( e_CardType == e_ClyApp_Card )
    Obj =  pSt_ClyCard_WriteRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
    RecNum,//[IN] //record number to read - 1 is always the first record
    e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    uc_Len2Write,//[IN] len to read - 1 to record size
    ucp_RecData); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  if( !IS_RES_OBJ_OK(Obj) )
    return e_ClyApp_CardWriteErr;

  //=======================================================
  //update state machine - bin and struct data information
  //=======================================================
  return  e_Internal_UpdateCardSateMachin( e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    ucp_RecData,//[IN]bit stream
    StIn, //[IN] St data
    e_ClyApp_Ok);

}



//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_Write
//
//DESCRITION:
//                write Record to the card (UpdateReocrd). not supported with sam CV
//
//RETURN:
//                eCalypsoErr
//LOGIC :
//                only if data dont exist in the state machene read it from the card
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static eCalypsoErr  e_Internal_Write(e_ClyApp_CardType e_CardType,//[IN]type - card \ ticket
  clyCard_BYTE RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
  e_clyCard_FileId FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
  void* StIn, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
  clyCard_BYTE uc_Len2Write//[IN] len to Write - 1 to record size
  )
{
  //    unsigned long l;  //Yoni 21/6/10 had to delete this variable because of extremely strange bug that new param  bSignContract caused bad signature. dont know why it happened and why this var affects this.
  //    unsigned long p_Sign;
  //    unsigned long p_SamSerNum;
  //    unsigned short count_val=0;
  //    st_Cly_Date tmp;
  //    union_ClyApp_ContractRecord *cntr=(union_ClyApp_ContractRecord *)StIn;
  // union_ClyApp_ContractRecord cntr2;
  eCalypsoErr err;
  clyCard_BYTE ucp_RecData[REC_SIZE]={0};
  RESPONSE_OBJ* Obj;
  //    enmReaderError samerr;

  if( e_CardType == e_ClyApp_Card )
  {

    memset(ucp_RecData,0,sizeof(ucp_RecData));



    err = e_Internal_Bit2Byte_Convert(e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
      e_CardType,//[IN]type - card \ ticket
      FileToSelect,//[IN] if not a ticket  - which record in the card
      ucp_RecData,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      StIn);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
    if( err!=e_ClyApp_Ok)
      return err;



    //=========================================
    //write the recod to the card - binary format
    //=========================================
    if( FileToSelect == e_clyCard_CountersFile )
    {
      //=======================================================
      //update state machine - bin and struct data information - add the counter to the record file
      //=======================================================
      //if counter file - copy the record data into it's place in the location in record 1 and reupdate all recoed 1 ( all counters ) in the card
      err = e_Internal_UpdateCardSateMachin( FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        ucp_RecData,//[IN]bit stream
        StIn, //[IN] St data
        e_ClyApp_Ok);
      if( err!=e_ClyApp_Ok)
        return err;

      if( e_CardType == e_ClyApp_Card )
        Obj =  ClyApp_Virtual_UpdateRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
        1,//[IN] all counters reside in record numer 1
        FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        (clyCard_BYTE)(RecNum*COUTER_SIZE),//[IN] len to read - update the minimum len possible
        st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
      if( !IS_RES_OBJ_OK(Obj) )
      {
        //marke record as unexist so that the real value will be read back from the card
        st_Static_StateMachine.st_TransactionData.b_IsCounterRecExistArr[RecNum] =clyApp_FALSE;
        return e_ClyApp_CardWriteErr;
      }

      return err;

    }
    else
    {

      if( e_CardType == e_ClyApp_Card )
        Obj =  ClyApp_Virtual_UpdateRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
        RecNum,//[IN] //record number to read - 1 is always the first record
        FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        uc_Len2Write,//[IN] len to read - 1 to record size
        ucp_RecData); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
      if( !IS_RES_OBJ_OK(Obj) )
        return e_ClyApp_CardWriteErr;

      //if Event Log cyclic file update - don't run over the data found in rec 1 - copy data to rec 0
      if(FileToSelect == e_clyCard_EventLogFile )
        RecNum = 0;
      //=======================================================
      //update state machine - bin and struct data information
      //=======================================================
      return  e_Internal_UpdateCardSateMachin( FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        ucp_RecData,//[IN]bit stream
        StIn, //[IN] St data
        e_ClyApp_Ok);
    }
  }
  else//ticket
  {
    e_ClyTkt_ERR  TktErr;
    struct_ClyTkt_Ticket *struct_Ticket = (struct_ClyTkt_Ticket *)StIn;
    e_ClyTkt_TicketRecordType e_TicketRecordType = (e_ClyTkt_TicketRecordType)FileToSelect;
    CalypsoBinTktType ucp_BinBuffOut;
    e_ClyTkt_TicketMediaTypes e_TicketMediaTypes;
    clyTkt_BYTE ucp_WordStartAddInCard;
    clyTkt_BYTE ucp_WordEndAddInCard;
    clyTkt_BYTE ucp_WordStartOffsetInBuff;
    clyTkt_BYTE ucp_WordRecLenInBuff;

#ifdef MICKY
    a
    e_ContacLessErr TrErr;
#else
    //        e_ClyTkt_ERR TrErr;
#endif


    //=======================================
    // Convert ticket struct to binary Buff
    //=======================================
    memset(ucp_BinBuffOut,0,sizeof(ucp_BinBuffOut));
    TktErr =  e_ClyTkt_ConvertTktSt2BinBuff( struct_Ticket,//[IN] Ticket struct input for translation
      ucp_BinBuffOut);//[OUT] Binary buff result
    if( TktErr )
      return e_ClyApp_WrongParamErr;
#ifdef DEBUG_MULTI
    {
      CalypsoBinTktType BinBuff;
      union_ClyApp_ContractRecord DBG_union_ContractRecord;
      memset(SaveSignesPtr,0,87);
      TrErr  = e_ClyTkt_ConvertBinBuff2TktSt( &DBG_union_ContractRecord.struct_Ticket,//[OUT] Ticket struct output
        ucp_BinBuffOut ,//[IN] Binary buff to translate
        BinBuff);//[OUT]



      if( TrErr  )
        return e_ClyApp_UnknownErr;
    }

#endif

    //Convert Ticket Media Type from 7816 type to tktOs type
    e_TicketMediaTypes = e_Internal_ConvertTktMediaType(st_Static_StateMachine.e_7816CardType);

    //=======================================
    // Get Ticket record Address - Translate Ticket Record type to physical Address
    //=======================================
    TktErr =  e_ClyTkt_GetTktRecAddress(e_TicketMediaTypes,//[IN] Ticket Media Types
      e_TicketRecordType,//[IN] Record Name
      &ucp_WordStartAddInCard,//[OUT] Record start physicl address
      &ucp_WordEndAddInCard,//[OUT] Record end physicl address
      &ucp_WordStartOffsetInBuff,//[OUT] the record offset in the translated buffer
      &ucp_WordRecLenInBuff);//[OUT] the len of the record in the translated buffer
    if( TktErr )
      return e_ClyApp_WrongParamErr;

    //=========================================
    //write the recod to the ticket - binary format
    //=========================================
#ifdef MICKY
    //len to write in bytes
    BlockSize = (ucp_WordEndAddInCard - ucp_WordStartAddInCard)*2;

    TrErr = e_CticketWrite(
      ///&e_AorL,//indicates the location of the reader
      c_EnVrf, // writing verification: 0-disabled; otherwise - enabled;
      ucp_WordStartAddInCard,//the number of block to write into ( 0 - 15 )
      BlockSize ,//the size of block (in bytes)
      ucp_BinBuffOut+ (ucp_WordStartOffsetInBuff*2) );//ptr to data to writen

    if( TrErr)// || e_AorL != e_App )
    {
      //======================================================
      //update state machine - clear ticket data from memory
      //======================================================
      st_Static_StateMachine.st_TransactionData.b_IsTktDataExist =0;
      return e_ClyApp_CardWriteErr;
    }
#endif

    //=======================================================
    //update state machine - bin and struct data information
    //=======================================================
    //update existing  bin data with the new record data
    memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData+(ucp_WordStartOffsetInBuff*2),ucp_BinBuffOut+ (ucp_WordStartOffsetInBuff*2),ucp_WordRecLenInBuff*2);

    switch( e_TicketRecordType )
    {
    case e_ClyTkt_MultiRideContractRec:
      memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket.st_TicketContractCommonData,&Global_union_ContractRecord.struct_Ticket.st_TicketContractCommonData,sizeof(Global_union_ContractRecord.struct_Ticket.st_TicketContractCommonData));
      memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec,&Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec,sizeof(Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec));
      break;
    case e_ClyTkt_MultiRideFirstValidationRec:
      memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec,&Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec,sizeof(Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec));
      break;

    case e_ClyTkt_MultiRideLocationRec:
      memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket.union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec,&Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec,sizeof(Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec));
      break;

    case e_ClyTkt_SeasonPassContractRec:
      memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket.st_TicketContractCommonData,&Global_union_ContractRecord.struct_Ticket.st_TicketContractCommonData,sizeof(Global_union_ContractRecord.struct_Ticket.st_TicketContractCommonData));
      memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec,&Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec,sizeof(Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec));
      break;

    case e_ClyTkt_SeasonPassInitialRec:
      memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec,&Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec,sizeof(Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec));
      break;

    case e_ClyTkt_SeasonPassValidationRe:
      memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec,&Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec,sizeof(Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec));
      break;
    default: break;

    }


    return e_ClyApp_Ok;
  }

}


//////////////////////////////////////////////////////////////////////////////
//  Yoni 11/2011
//  e_Internal_CardDescreaseCounter
//  decrease counter and update state machine binary counter record
//  for contractuse (support sv sam)
//////////////////////////////////////////////////////////////////////////////
static eCalypsoErr  e_Internal_CardDescreaseCounter(
  clyCard_BYTE RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
  unsigned long ulDebit
  )
{
//  eCalypsoErr err;
  RESPONSE_OBJ* Obj;
  //    enmReaderError samerr;
  clyCard_BYTE cpDecrease[3]={0};
  clyCard_BYTE OutNewCountData[3]={0};

  //copy ulDebit into 3 bytes  in reverse
  char* p = (char*)&ulDebit;
  cpDecrease[0]=p[2];
  cpDecrease[1]=p[1];
  cpDecrease[2]=p[0];


  Obj = pSt_ClyCard_IncreaseDecrease(
    st_Static_StateMachine.CardReaderId,/// (IN)reader id
    RecNum,///(IN) counter number
    e_clyCard_CountersFile,///(IN) file name enum value
    cpDecrease,///   data to the card
    OutNewCountData,///   out new couner value
    0);/// 1 for increase 0 for decrease

  if( !IS_RES_OBJ_OK(Obj) )
  {
    //marke record as unexist so that the real value will be read back from the card
    st_Static_StateMachine.st_TransactionData.b_IsCounterRecExistArr[RecNum] =clyApp_FALSE;
    return e_ClyApp_CardWriteErr;
  }

  //set the counter bin data in the state machine (this is used only for binary data for transaction record)
  {
    unsigned char* pc=st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec;
    memcpy(pc+3*(RecNum-1),OutNewCountData,3);
  }


  return e_ClyApp_Ok;

}



//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                v_Internal_MemsetSt
//
//DESCRITION:
//
//                clear objects befor use
//
//RETURN:
//
//                -
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static void v_Internal_MemsetSt(void* StOut,e_clyCard_FileId FileId)
{
  union_ClyApp_ContractRecord *union_ContractRecord = (union_ClyApp_ContractRecord*)StOut;
  switch(FileId)
  {
  case e_clyCard_EnvironmentFile: memset(StOut,0,sizeof(st_ClyApp_EnvAndHoldDataStruct));
    break;
  case e_clyCard_EventLogFile: 
  case e_clyCard_SpecialEventFile:
	  memset(StOut,0,sizeof(st_clyApp_CardEventDataStruct));
    break;
  case e_clyCard_ContractsFile :
    memset(&union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData,0,sizeof(union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData));
    break;
  case e_clyCard_CountersFile:
    memset(&union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord,0,sizeof(union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord));
    break;
  default: break;
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// e_Internal_IsAllSpecialEventsExist
// return value: 1= all exist,  0= Not all exist
//////////////////////////////////////////////////////////////////////////////////////////////////
static unsigned char e_Internal_IsAllSpecialEventsExist(void)
{
  unsigned char ret;
  unsigned int i;

  ret = 1; 
  for (i=1; i < SPECIAL_EVENTS_COUNT+1; i++)
  {
    ret &= (unsigned char)st_Static_StateMachine.st_TransactionData.b_IsSpecialEventRecExistArr[i];
  }
  return ret;

}


//////////////////////////////////////////////////////////////////////////////////////////////////
//Read whole special events file
//////////////////////////////////////////////////////////////////////////////////////////////////
static eCalypsoErr e_Internal_ReadAllSpecialEvents()
{
  eCalypsoErr OutConverError;
  int i;
  //clyCard_BYTE ucp_RecDataOut[REC_SIZE];
  st_clyApp_CardEventDataStruct st_CardEventDataStruct;
  volatile eCalypsoErr err;
  

  for (i=1; i < SPECIAL_EVENTS_COUNT+1; i++)
  {
	err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
      (clyCard_BYTE)i,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_SpecialEventFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts

	if(err != e_ClyApp_Ok)
      return e_ClyApp_CardReadErr;//return error only if ClyApp_Virtual_ReadRecord failed
  }

  //SpecialEvent.m_bReadSpecialEvents = clyApp_TRUE;

  return e_ClyApp_Ok;//no matter the result of e_Internal_Bit2Byte_Convert

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//Yoni 11/2010
//get index of SpecialEvent associated with contract (by contract numnber)
//Return Special Event record number or -1 if doesn't exist
///////////////////////////////////////////////////////////////////////////////////////////////////
static int  i_Internal_FindSpecialEvent(int iContract) // 1-8
{
  int i;
  if(iContract >= 1  && iContract <= 8)
  {
	  //read special events file if didn't read yet
	  if(e_Internal_IsAllSpecialEventsExist() == 0)
		  e_Internal_ReadAllSpecialEvents();

	   //looking for record
	  for (i=1; i < SPECIAL_EVENTS_COUNT+1; i++)
	  {
		if  (st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[i].uc_EventContractPointer == iContract)
		{
		  //SpecialEvent.m_iIndex = i;
		  return i;
		  //break;
		}
	  }
  }
  return -1;//didn't find
}

///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
void  b_Internal_UpdateSpecialEvent(int iRec, void* StOut)
{
  //read special events file if didn't read yet
  if (SpecialEvent.m_bReadSpecialEvents && iRec && (iRec <= SPECIAL_EVENTS_COUNT) )
    memcpy(&SpecialEvent.m_pSpecialEvents[iRec-1],  StOut, sizeof(SpecialEvent.m_pSpecialEvents[0]));
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
static int i_Internal_FindEmptySpEvent(st_Cly_DateAndTime *st_CurrentDateAndTime)
{
  int i;
  unsigned short usRemainingMinutes;
  st_clyApp_CardEventDataStruct* pEv;
  st_clyApp_CardContractIssuingData* pContractData ;

  //read special events file if didn't read yet
  if(e_Internal_IsAllSpecialEventsExist() == 0)
	  e_Internal_ReadAllSpecialEvents();

  //looking for record
  for (i=1; i < SPECIAL_EVENTS_COUNT+1; i++)
  {
    pEv = &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[i];

    //looking for expired events
    pContractData = &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[pEv->uc_EventContractPointer].st_CardContractIssuingData;
    if  ((!pEv->uc_EventContractPointer) ||                             //empty event
      !pContractData->b_ContractIsJourneylnterchangesAllowed ||     //is not transfer contract
      !b_CheckTransferTimeLimit(&(pContractData->st_OptionalContractData),  //expired special event
      pEv,
      st_CurrentDateAndTime,
      &usRemainingMinutes))
      return i;

  }
  return -1;
}
//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_Read
//
//DESCRITION:
//
//                Read Record and return the struct output - update state machine with the read information
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//                only if data dont exist in the state machene read it from the card
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static eCalypsoErr  e_Internal_Read(e_ClyApp_CardType e_CardType,//[IN]type - card \ ticket
  clyCard_BYTE RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
  e_clyCard_FileId FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
  void* StOut, //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
	eCalypsoErr* pOutConverError,
	clyCard_BYTE ForceRead)
{
  clyCard_BYTE ucp_RecDataOut[REC_SIZE],RecNum2Read,offset;
  RESPONSE_OBJ* Obj;
  eCalypsoErr err, err2 = e_ClyApp_Ok;
  clyApp_BOOL b_RecExistInMem=clyApp_FALSE;
  union_ClyApp_ContractRecord *union_ContractRecord = (union_ClyApp_ContractRecord *)StOut;
  int iii;
	*pOutConverError = e_ClyApp_NotOk;

  if( e_CardType == e_ClyApp_Card )
  {
    //===============================================
    //if record already exist in memory  - return it
    //===============================================
    switch(FileToSelect)
    {
    case e_clyCard_EnvironmentFile:
      //if exist flag
      if( st_Static_StateMachine.st_TransactionData.b_IsEnvRecExist)
      {
        //copy  struct data
        memcpy(StOut,&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec,sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec));
            //rise flag - rec found
        b_RecExistInMem=clyApp_TRUE;
      }
      break;

    case e_clyCard_EventLogFile:
      //rise exist flag
      if(st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[RecNum])
      {
        //copy  struct data
        memcpy(StOut,&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[RecNum],sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[RecNum]));
        //rise flag - rec found
        b_RecExistInMem=clyApp_TRUE;
      }

      break;
    case e_clyCard_SpecialEventFile:
      //rise exist flag
      if(st_Static_StateMachine.st_TransactionData.b_IsSpecialEventRecExistArr[RecNum])
      {
        //copy  struct data
        memcpy(StOut,&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[RecNum],sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[RecNum]));
        //rise flag - rec found
        b_RecExistInMem=clyApp_TRUE;
      }
      break;

    case e_clyCard_ContractsFile:
      //rise exist flag
      if(st_Static_StateMachine.st_TransactionData.b_IsContractRecExistArr[RecNum])
      {
        //copy  struct data
        memcpy(&union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData,&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardContractIssuingData,sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardContractIssuingData));
        union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.e_CardCounterRecordType = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardCounterRecord.e_CardCounterRecordType;
        //rise flag - rec found
        b_RecExistInMem=clyApp_TRUE;
      }
      break;

    case e_clyCard_CountersFile:
      //rise exist flag
      if(st_Static_StateMachine.st_TransactionData.b_IsCounterRecExistArr[RecNum])
      {
        //copy  struct data
        memcpy(&union_ContractRecord->st_CardContractRecord.st_CardCounterRecord,&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardCounterRecord,sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum].st_CardCounterRecord));
        //rise flag - rec found
        b_RecExistInMem=clyApp_TRUE;
      }
      break;

    case e_clyCard_ContractListFile:
      if(st_Static_StateMachine.st_TransactionData.b_IsContractListExist)
      {
        memcpy(StOut, &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractList, sizeof(st_clyApp_ContractListStruct));
        b_RecExistInMem=clyApp_TRUE;
      }
      break;
    default: return e_ClyApp_WrongParamErr;

    }
    //rec found
    if( b_RecExistInMem )
    {
        switch(FileToSelect)
      {
      case e_clyCard_ContractsFile:
        return st_Static_StateMachine.st_TransactionData.isContractConversionValid[RecNum];
      case e_clyCard_CountersFile:
        return st_Static_StateMachine.st_TransactionData.isCounterConversionValid[RecNum];
      }
      return e_ClyApp_Ok;
    }


    //=========================================
    //Read the recod from card - binary format
    //=========================================
    RecNum2Read = RecNum;
    offset = 0;

    //all counters reside in record numer 1
    if( FileToSelect == e_clyCard_CountersFile )
    {
      //if counter file - copy the record data into it's place in the location in record 1 and reupdate all recoed 1 ( all counters ) in the card
      offset = (RecNum-1)*COUTER_SIZE;
      RecNum2Read=1;
    }



    if( e_CardType == e_ClyApp_Card )
    {
      iii=0;
      do
      {
        if (iii)
          CoreDelay(5);
        iii++;
        Obj =  ClyApp_Virtual_ReadRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
          RecNum2Read,//[IN] //record number to read - 1 is always the first record
          FileToSelect, //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
          REC_SIZE,//[IN] len to read - 1 to record size
          ucp_RecDataOut,ForceRead); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    } while(iii<1 && Obj->sw1_sw2[0]==0);
    }

    //@@TODO - if sw1,sw2 is 0x6A,0x83 RECORD_NOT_FOUND it is not an io error
    //         maybe e_ClyApp_NoValidContractErr should be returned
    //         need to go over the possible switches & return error accordingly.
    if (IS_RES_RECORD_NOT_FOUND(Obj))
      return e_ClyApp_RecordNotFoundErr;
    if( !IS_RES_OBJ_OK(Obj) )
      return e_ClyApp_CardReadErr;


    #ifdef WIN32
    {
      //print the binary
      char record[20];
      const char* enuToName[e_clyCard_ContractListFile+1]={0};
      enuToName[e_clyCard_TicketingDF]="TicketingDF";
      enuToName[e_clyCard_EnvironmentFile]="Env";
      enuToName[e_clyCard_ContractsFile]="Contract";
      enuToName[e_clyCard_CountersFile]="Counter";
      enuToName[e_clyCard_EventLogFile]="Event";
      enuToName[e_clyCard_SpecialEventFile]="SpecialEvent";
      enuToName[e_clyCard_ContractListFile]="ContractList";
      /*
      e_clyCard_TicketingDF,      //DF ID = "1TIC.ICA" = 0x315449432e494341
      e_clyCard_EnvironmentFile=0x07, //Linear,>= 1 records
      e_clyCard_ContractsFile=0x09, //Linear,>= 4 records
      e_clyCard_CountersFile=0x19,  //Linear/Counter >=9
      e_clyCard_EventLogFile=0x08,  //Cyclic>=3 records
      e_clyCard_SpecialEventFile=0x1d,//Linear,>= 1 records
      e_clyCard_ContractListFile=0x1e //Linear,>= 1 records
      */
      sprintf(record,"%s %d", enuToName[FileToSelect], RecNum2Read);
      UIPrintHex(record, ucp_RecDataOut, 29);
    }
    #endif
    //=========================================
    //translate the bin data to API struct
    //=========================================
    v_Internal_MemsetSt(StOut,FileToSelect);
    *pOutConverError=e_ClyApp_Ok;
    err = e_Internal_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
      e_CardType,//[IN]type - card \ ticket
      FileToSelect,//[IN] if not a ticket  - which record in the card
      ucp_RecDataOut+offset,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      StOut);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream

    *pOutConverError=err;
    //all counters reside in record numer 1
    if( FileToSelect == e_clyCard_CountersFile )
    {
      //each counter is three bytes size and all counters are reside is the counter record number 1 and placed one after the another
      memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec,ucp_RecDataOut,REC_SIZE);
      //=======================================================
      //update state machine - ONLY struct data information
      //=======================================================
      err2 = e_Internal_UpdateCardSateMachin( FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        NULL,//ucp_RecDataOut,//[IN]bit stream
        StOut,//[IN] St data
        err); // convert status
    }
    else
    {
      //=======================================================
      //update state machine - bin and struct data information
      //=======================================================
      err2 = e_Internal_UpdateCardSateMachin( FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        ucp_RecDataOut,//[IN]bit stream
        StOut,//[IN] St data
        err); // convert status
    }
    if (err != e_ClyApp_Ok)
      return err;
    else
            return err2;

  }
  else // ticket
  {

#ifdef CORE_SUPPORT_TICKET

        e_ContacLessErr TrErr;
    ///e_AppOrLoader e_AorL;
    CalypsoBinTktType BinBuff;
    e_ClyTkt_ERR TktErr;
    struct_ClyTkt_Ticket *struct_Ticket  = (struct_ClyTkt_Ticket *)StOut;

    CalypsoBinTktType ucp_BinBuffOut;
    //===============================================
    //if ticket already exist in memory  - return it
    //===============================================
    if( st_Static_StateMachine.st_TransactionData.b_IsTktDataExist)
    {
      //copy  struct data
      memcpy(StOut,&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket,sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket));
      //return
      return e_ClyApp_Ok;
    }

    //============================
    //if data not exist in memory
    //============================

    //initial value - no rehabilitation was made
    st_Static_StateMachine.b_WasTktRehabilitate=0;

    memset(BinBuff,0,sizeof(BinBuff));
    //read all ticket
    TrErr = ContactlessTicketRead(
      //                             &e_AorL ,
      0,
      FULL_TKT_BYTE_COUNT , //16
      BinBuff
      );
    if(TrErr)//                || e_AorL != e_App)
      return e_ClyApp_ReaderErr;

    //if binary data exist - copy  bin data to stae machine
    memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData,BinBuff,32);


    // convert bin Ticket data to Struct
    TktErr =   e_ClyTkt_ConvertBinBuff2TktSt( struct_Ticket,//[OUT] Ticket struct output
      BinBuff,//[IN] Binary buff to translate
      ucp_BinBuffOut);

    if( TktErr != e_ClyTkt_NO_ERROR)
    {
      switch(TktErr)
      {
      case e_ClyTkt_MEDIA_INCORRECT: // unknown card type
        return e_ClyApp_WrongCardTypeErr;

      case e_ClyTkt_RECORD_TYPE_INCORECT://ilegal record type
        return e_ClyApp_WrongParamErr;

      case e_ClyTkt_CHECK_SIGN_FAIL://contract sign fail
        return e_ClyApp_CardContractErr;

      case e_ClyTkt_CHECK_SEASON_THERD_SIGN_FAIL://season pass - therd sign
      case e_ClyTkt_DECRYPT_FAIL:// internal error
      case e_ClyTkt_ENCRYPT_FAIL:// internal error
      case e_ClyTkt_SECOND_MULTI_SIGN_FAIL_BUT_BKPFLAG_FALSE:
      case e_ClyTkt_ILLEGAL_DATE://not used
      case e_ClyTkt_ILLEGAL_TIME://not used
      case e_ClyTkt_KEY_LRC_FAIL://key coraption
        return e_ClyApp_CardSecurityErr;

      case e_ClyTkt_KEY_NOT_EXIST://internal
        return e_ClyApp_WrongCardTypeErr;


      case e_ClyTkt_DATE_PLUS_H_D_CALLBACK_NULL://null
      case e_ClyTkt_NO_SIGN_CALBACK://null
      case e_ClyTkt_DATE_TIME_CALLBACK_NULL:
      case e_ClyTkt_SIGN_CALLBACK_NULL:
        return e_ClyApp_InterfaceNotInitErr;
      case e_ClyTkt_BKFLGTRU_SECMULSIGNFAIL:
        {
          struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys = struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.uc_JourneysBck;
          struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.ush_TMF_LocationStamp = struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.ush_TML_LocationId;
          struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.us_TMF_Sig = struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.ush_SignatureBkp;

          //============================================
          //WRITE TO CARD - first validity record
          //============================================
          err = e_Internal_Write( e_ClyApp_Ticket,//[IN]type - card \ ticket
            0,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
            (e_clyCard_FileId)e_ClyTkt_MultiRideFirstValidationRec, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
            struct_Ticket, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
            0 // not relevat for ticket
            );
          if(err!=e_ClyApp_Ok)
            return err;
          return e_ClyApp_CardSecurityErr;
        }
      default : break;
      }


    }

    //=====================
    //update state mashine
    //=====================

    //rise exist flag
    st_Static_StateMachine.st_TransactionData.b_IsTktDataExist = clyApp_TRUE;
    //copy  bin data
    memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData,BinBuff,sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData));
    //copy  struct data
    memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket,StOut,sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket));

    return e_ClyApp_Ok;
#else
   return e_ClyApp_WrongCardTypeErr;
#endif
  }
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                ul_Internal_GetFirstUseDateAndTimeFromEventRecords
//
//DESCRITION:
//                 Get First Use Date And Time From Special Event. local time, not GMT
//RETURN:
//
//                0 if no special event found
//LOGIC :
///////////////////////////////////////////////////////////////////////////////////////////////////
static unsigned long  ul_Internal_GetFirstUseDateAndTimeFromEventRecordsSecsFrom2000( char c_ContractRecNum //[IN] contract rec number to check
  )//[OUT] Date And Time of first use
{

  const st_clyApp_CardEventDataStruct* pSpecialEvent;
  int iSpecialEventRecNum=i_Internal_FindSpecialEvent(c_ContractRecNum);
  if(iSpecialEventRecNum>=1)
  {
    pSpecialEvent= &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[iSpecialEventRecNum];

    return  (ul_GetTimeReal(&pSpecialEvent->st_EventDataTimeFirstStamp,0)-SEC_FROM_1997_TO_2000);

  }

  return 0;

#if 0



  st_clyApp_CardEventDataStruct st_CardEventDataStruct;
  unsigned long ul_DataTimeFirstStamp;
  char i;
  //first record index
  i=1;

  //===============================================================
  //if no date exist - refer to current date and time as first use
  //===============================================================
  *stp_EndDateAndTime = st_GetCurrentDateAndTime();
  //Read event file while last use not foud. if found check ( TatalHalfHourPassFromFirstUse = currentDateAndTime - EventFirstUseDateAndTime ) > Duration
  do
  {
    //===================
    // Read event Record
    //===================
    if( !e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
      (clyCard_BYTE)i,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &st_CardEventDataStruct)) //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
      //if record can not be read - exist
      break;
    //===================
    // Read found
    //===================
    //check if it is the right contract
    if(st_CardEventDataStruct.uc_EventContractPointer == c_ContractRecNum )
    {

      //=================================================
      // check if DataTimeFirstStamp  exist
      //=================================================
      // get contract Data and Time of First Stamp as long
      ul_DataTimeFirstStamp = ul_GetTimeReal(&st_CardEventDataStruct.st_EventDataTimeFirstStamp,0);

      //=================================================
      // if Data and Time of First Stamp == 0  -> contract has not been used yet or not yet valid
      // use current date as first stamp
      //=================================================
      if( ul_DataTimeFirstStamp == 0 )
        break;


      //if date exist - store first use date and time
      *stp_EndDateAndTime = st_CardEventDataStruct.st_EventDataTimeFirstStamp;
      //end
      break;
    }
    else
      //=================================================
      //while not found - store last valid date and time
      //==================================================
      *stp_EndDateAndTime =st_CardEventDataStruct.st_EventDataTimeFirstStamp;
    //next rec index
    i++;
    //=================================================
    //while not found - read all records
    //==================================================
  }while(i<=MAX_EVENT_COUT);

  //store date in global data
  st_Static_StateMachine.st_EventDataTimeFirstStampOfContract2Use = *stp_EndDateAndTime;
#endif
}




//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//                b_Internal_IsCurrentDateIsPartOfLastUsePeriod
//DESCRITION:
//
//                check if current date is part of last use period
//              b_Internal_IsCurrentDateIsPartOfLastUsePeriod
//RETURN:
//
//                -
//LOGIC :
//
////////////////////////////////////////////////////////////////////////////////////////////////////

static clyApp_BOOL  b_Internal_IsCurrentDateIsPartOfLastUsePeriod(st_Cly_Date *stp_ContractLastValidityDate, //[IN] contract start validity date
  e_ClyApp_PeriodType e_PeriodType) //[IN] period type
{
  long l;
  //Get Current Date and time - struct
  st_Cly_DateAndTime st_CurrentDateAndTime = st_GetCurrentDateAndTime();
  st_Cly_Date st_EndPeriodDate =  st_GetEndPeriodDate( e_PeriodType,stp_ContractLastValidityDate );

  l = l_Internal_DateCmp(&st_CurrentDateAndTime.st_Date,&st_EndPeriodDate);
  //if current date is not grater then the end of the period - return clyApp_TRUE
  if( l<=0)
    return clyApp_TRUE;
  return clyApp_FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Yoni 10/2010
//  return true if continue/maavar trip is valid (without need for nikuv)
//  return remaining minutes
///////////////////////////////////////////////////////////////////////////////////////////////////
#if 0 //not used in Metronit
/*
static clyApp_BOOL b_GetContinueTripValidity(
                                             const st_clyApp_CardContractIssuingData* pContractData //[IN] contract
                                             ,const st_clyApp_CardEventDataStruct* pSpecialEventData//[IN] SpecialEvent associated with this contract
                                             ,const st_clyApp_CardEventDataStruct* pRegularEventData//[IN]last regular event
                                             ,const st_Cly_DateAndTime* pCurrDateAndTime//[IN] current date and time
                                             ,TR_USHORT* pMaavarValidity//[OUT] remaining minutes. if "end of day" 9998. if "end of service" 9999.
                                             ,TR_BYTE* pPrevProvider //[OUT] prev provider (from regular event)
                                             ,TR_BYTE *pPassengerCount //[OUT] passenger count cound be 1 or more
                                             ,TR_USHORT* pMaavarPrevLine//[OUT] prev makat line (from regular event). 0 if not egged
                                             ,TR_USHORT  *pEventRunID
                                             ,TR_BYTE *pFareCode
                                             )
{

  clyApp_BOOL bRes;
  //first check if continue/maavar is valid (without need for nikuv)
  if(pContractData->b_ContractIsJourneylnterchangesAllowed)
  {
    //calc end time of continue
    bRes=b_CheckTransferTimeLimit(
      &pContractData->st_OptionalContractData
      ,pSpecialEventData //[IN]
      ,pCurrDateAndTime //[IN]
      ,pMaavarValidity//[OUT]
      );
    if(bRes) //maavar/continue is valid
    {
      //set prev provider,shilut, according to last regular event
      *pPrevProvider=pSpecialEventData->uc_EventServiceProvider;//from special, not regular Yoni 8/3/11

      if(*pPrevProvider == g_Params.uc_ProviderId)
        //get shilut from vehicle field
        *pMaavarPrevLine=pSpecialEventData->st_OptionalEventData.ush_EventDevice4;//pRegularEventData->st_OptionalEventData.ush_EventLine;

      if(pSpecialEventData->st_OptionalEventData.b_IsEventPassengersNumberExist)      // if bit is on
        *pPassengerCount = pSpecialEventData->st_OptionalEventData.uc_EventPassengersNumber;

      // Run ID ..
      if(pSpecialEventData->st_OptionalEventData.b_IsEventRunlDExist)      // if bit is on
        *pEventRunID = pSpecialEventData->st_OptionalEventData.ush_EventRunlD;


      // Fare Code
      if(pSpecialEventData->st_OptionalEventData.b_IsEventTicketExist)      // if bit is on
        *pFareCode = pSpecialEventData->st_OptionalEventData.st_EventTicket.uc_EventTicketFareCode;

      //else dont update passenger number


      return clyApp_TRUE;
    }
  }


  return clyApp_FALSE;

}
*/
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//  b_ClyApp_GetContinueValidity
//  API
//  return true of continue/maavar trip is valid (also return byref relevant data)
///////////////////////////////////////////////////////////////////////////////////////////////////
#if 0 //not used in metronit
/*
clyApp_BOOL b_ClyApp_GetContinueValidity(int ContractIndex //[IN]
  ,const st_clyApp_CardContractIssuingData* pContractData
  ,const st_clyApp_CardEventDataStruct* pRegularEventData
  ,const st_Cly_DateAndTime* pCurrDateAndTime//[IN] current date and time
  ,TR_USHORT* pMaavarValidity//[OUT] remaining minutes. if "end of day" 9998. if "end of service" 9999.
  ,TR_BYTE* pPrevProvider //[OUT] prev provider (from regular event)
  ,TR_BYTE *pPassengerCount //[OUT] passenger count cound be 1 or more
  ,TR_USHORT* pMaavarPrevLine//[OUT] prev makat line (from regular event). 0 if not egged
  ,TR_USHORT *pEventRunID //[OUT] Run ID
  ,TR_BYTE *pFareCode
  )
{
  int iSpecialEventIndex;
  st_clyApp_CardEventDataStruct* pSpecialEvent;
  //get specialevent connected to this contract
  iSpecialEventIndex=i_Internal_FindSpecialEventIndex(ContractIndex+1);
  if(iSpecialEventIndex>=0)
  {
    pSpecialEvent= &SpecialEvent.m_pSpecialEvents[iSpecialEventIndex];
    if(b_GetContinueTripValidity(pContractData
      ,pSpecialEvent
      ,pRegularEventData
      ,pCurrDateAndTime
      ,pMaavarValidity
      ,pPrevProvider
      ,pPassengerCount
      ,pMaavarPrevLine
      ,pEventRunID
      ,pFareCode

      )
      )
    {
      return clyApp_TRUE;
    }
  }

  return clyApp_FALSE;
}
*/
#endif

//in event written before 2.01 get from field4, else get from field runid
unsigned short usGetShilutFromEventAccordingToVersion(const st_clyApp_CardEventDataStruct* pRegularEventData)
{
    if(!pRegularEventData)
        return 0;
    //determine if we're in old or new version   
          //from runid
        if(pRegularEventData->st_OptionalEventData.b_IsEventRunlDExist)
        {
            //runid is only 12 bit so we have to return it in the same format as before - 14 bit
            //we add 00 in bit 2,3
            short res =  pRegularEventData->st_OptionalEventData.ush_EventRunlD;
            short lsb = pRegularEventData->st_OptionalEventData.ush_EventRunlD & 1;
            res *= 4;//add 2 bits
            res |= lsb; //set lsb
            return res;
        }
    
    
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_CheckContractValidity
//
//DESCRITION:
//
//                Is Contract No Longer Valid
//                This function must be called after e_Internal_BasicCalypsoCheckIfRecValid
//
//RETURN:
//
//                e_ClyApp_ContractNoLongerValid = Contract No Longer Valid
//                e_ClyApp_ContractValidButNotInThisPeriod = Contract Valid But Not In This Period
//                e_ClyApp_ContractValid = Contract Valid
//                e_ClyApp_ContractDataErr = Contract Data Err - can bot by used or change
//
//LOGIC :
//                contract is no longer valid if :
//                end date only - end rights priority chang is done during use operation
//
//                validity end date
//                     current date > end date
//
//                validity duration : if validity statr date != 0 && // have been used or alreay valid
//                    if if not in hours  - current date > expected end date ||   current date > expected end dateCompact = ( start date + duration )
//                    if duration in half hours and defently not valid: ( start date + duration/48 < current date ) || ( start date + duration/48 == current date ) read event file while last use not foud. if found check ( TatalHalfHourPassFromFirstUse = currentDateAndTime - EventFirstUseDateAndTime ) > Duration
//                    if duration in half hours valid ( start date + duration/48 > current date ) exist
//
//                    services needed : GetEndDate( dateCompact startDate , timeinHalfHours
//
//                period journeys
//                    if tariff is not "date + number" - return err
//                     if(tariff->date.X == currentDate.X  &&  tariff->Number >= period Max Counter )
//                        return e_ClyApp_ContractValidButNotInThisPeriod;
//
//                for all contracts:
//                if future validity statr date - return 0
//////////////////////////////////////////////////////////////////////////////////////////////////////
const st_Cly_Time stTimeOfEndDay={59,59,23};//sec,min,hour
static e_clyApp_ValidityType e_ClyApp_CheckContractValidity( char c_ContractRecNum , //[IN] contract rec number to check
                                                            st_ProcessedContractValidityInfo* pProcessedValidityInfo//[OUT]
                                                            )
{

  //new logic 10/2011
  //first get: start, end, counter, interchange
  static st_ClyApp_CardContractRecord st_CardContractRecord;
  long l;
  clyApp_BOOL b_CounterPeriodJourniesValid=clyApp_FALSE;//if true than means contract is valid even if counter==0
  int iSpecialEventRecNum;
  st_Cly_Date st_CurrentDateCompact;
  unsigned long  ul_StartDate;
  st_Cly_DateAndTime st_CurrentDateAndTime;//,st_EndDateAndTime;
  e_ClyApp_TariffCounterType e_TariffCounterType;
  //clyApp_BOOL bInterchangeValid=clyApp_FALSE;
  //  st_Cly_Date  st_EndDateCompact;
  st_clyApp_CardContractIssuingData* pContractData= &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData;
  st_clyApp_CardEventDataStruct* pSpEvent;

  //Get Current Date and time - struct
  st_CurrentDateAndTime = st_GetCurrentDateAndTime();
  st_CurrentDateCompact = st_CurrentDateAndTime.st_Date;
  st_CardContractRecord = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum];



  memset(pProcessedValidityInfo, 0, sizeof(*pProcessedValidityInfo));

  pProcessedValidityInfo->ContractIndex = c_ContractRecNum-1;
  /////////////////////////
  //validity start date
  /////////////////////////
  // get contract start date
  //special case: year==2041 means there's no start date (for sliding)
  if(st_CardContractRecord.st_CardContractIssuingData.st_ContractValidityStartDate.Year == stSlidingZeroDate.Year)
  {
    ul_StartDate = 0;
    memset(&pProcessedValidityInfo->ClyDtm_StartDate.st_Date ,0, sizeof(pProcessedValidityInfo->ClyDtm_StartDate.st_Date));
  }
  else //normal
  {
    ul_StartDate = ush_GetDateCompact( &st_CardContractRecord.st_CardContractIssuingData.st_ContractValidityStartDate);
    //set pProcessedValidityInfo->ClyDtm_StartDate
    pProcessedValidityInfo->ClyDtm_StartDate.st_Date = st_CardContractRecord.st_CardContractIssuingData.st_ContractValidityStartDate;

  }
  memset(&pProcessedValidityInfo->ClyDtm_StartDate.st_Time, 0, sizeof(pProcessedValidityInfo->ClyDtm_StartDate.st_Time));

  if( st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityEndDateExist)
  {
    //day end at 23:59
    pProcessedValidityInfo->ClyDtm_EndDate.st_Date = st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityEndDate;
    pProcessedValidityInfo->ClyDtm_EndDate.st_Time = stTimeOfEndDay;
  }

  ///////////////////////////////////////////////////
  //validity duration - calc end only if start exists
  ///////////////////////////////////////////////////
  if(ul_StartDate > 0 &&  st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityDurationExist ) // if Contract Validity Duration field Exist
  {

    st_Cly_DateAndTime ov_EndDateTime={0};
    ov_EndDateTime =  stCalcEndDateByValidityDuration(&st_CardContractRecord, c_ContractRecNum);

    pProcessedValidityInfo->ClyDtm_EndDate = ov_EndDateTime;

  }


  /////////////////////////
  //period journeys
  /////////////////////////
  // in period jurnys the counter can by ZERO but the contract can still be valid - for all othere cases check the counter
    if(  st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist
        &&   (e_GetInterchangeType(&st_CardContractRecord.st_CardContractIssuingData) != e_OneHemshech) //Yoni 03/2013 
				 &&   (e_GetInterchangeType(&st_CardContractRecord.st_CardContractIssuingData) != e_TwoHemshech) //Yoni 03/2013 
				)         
         // if Contract Validity Duration field Exist and not hemshech
    {
    //==================================================================
    // Read Contract's counter ( from memory )and check if the same period && Max counter
    //===================================================================
		eCalypsoErr OutConverError;
    e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
      c_ContractRecNum,//[IN] //not relevat for ticket - record number to read : 1 is always the first record
      e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &st_CardContractRecord, &OutConverError,0) ;//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts

    //====================================================================================================
    // In this point the contract MUST be valid - but check if not all Trips In Period has been used yet
    //====================================================================================================
    //Yoni 10.7.07- fix bug. If counter==0 &&  same day/period , then not valid         //
    if(st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.CounterValue <= 0
      && b_Internal_IsCurrentDateIsPartOfLastUsePeriod(&st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.st_CounterDate, //[IN] Last use date
      st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.e_PeriodType) ) //[IN] period type)
    {
      b_CounterPeriodJourniesValid=clyApp_FALSE;//not valid
    }
    else{ //else it's valid (counter>0 || new day/period)
      b_CounterPeriodJourniesValid=clyApp_TRUE;
    }
  }
  else
  {

    st_CurrentDateAndTime = st_GetCurrentDateAndTime();

    ///////////////////////////////////////////////////////////////////////////////
    //get counter value according to type
    ///////////////////////////////////////////////////////////////////////////////
    e_TariffCounterType = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType;
    if(e_TariffCounterType != e_ClyApp_CounterNotUsed)
    {

      if(e_TariffCounterType == e_ClyApp_CounterAsDateAndRemainingNumOfJourneys)
      {
        pProcessedValidityInfo->Counter = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.CounterValue;//Yoni 10/2011
      }
      else if(e_TariffCounterType == e_ClyApp_CounterAsNumOfToken)
      {
        pProcessedValidityInfo->Counter = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue;
      }
      else if(e_TariffCounterType == e_ClyApp_CounterAsMonetaryAmount)
      {
        pProcessedValidityInfo->Counter = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue;//Yoni 10/2011
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  //  now decide validity status
  //  1) curr < start date => e_ClyApp_ContractValidButNotInThisPeriod
  //  2) curr > endDate  => e_ClyApp_ContractNoLongerValid
  //  3) if counter is used, if 0 and no valid interchange or not new period in period journys => e_ClyApp_ContractNoLongerValid
  ///////////////////////////////////////////////////////////////////////////////////////////////////
  //  check start date
  l = l_Internal_DateCmp(&st_CurrentDateCompact, &pProcessedValidityInfo->ClyDtm_StartDate.st_Date); //&st_CardContractRecord.st_CardContractIssuingData.st_ContractValidityStartDate) ;
  if(l < 0)
  {
    return e_ClyApp_ContractValidButNotInThisPeriod;//start date is in the future
  }

  //check end date and time if exists

  if(pProcessedValidityInfo->ClyDtm_EndDate.st_Date.Year>0 && l_Internal_DateAndTimeCmp (&pProcessedValidityInfo->ClyDtm_EndDate, &st_CurrentDateAndTime) < 0)
  {
    return e_ClyApp_ContractNoLongerValid;
  }


  //for contracts with counter check counter and interchange
  if(e_TariffCounterType == e_ClyApp_CounterNotUsed)
    return e_ClyApp_ContractValid;
  //else check counter/interchange

	pProcessedValidityInfo->IsValidInterchange=clyApp_FALSE;//default
  if(pContractData->b_ContractIsJourneylnterchangesAllowed)
  {

	  unsigned short usRemainingMinutes=0;

    //is there a SpecialEvent connected to this contract?
    iSpecialEventRecNum=i_Internal_FindSpecialEvent(c_ContractRecNum);
	pSpEvent = &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[iSpecialEventRecNum]; 
    if(iSpecialEventRecNum>=1)
    {
			 //now check if continue trip is valid
			pProcessedValidityInfo->IsValidInterchange = b_CheckTransferTimeLimit(
																								&pContractData->st_OptionalContractData //const st_clyApp_OptionalContractData* pContractOptionalData
																								,pSpEvent//const st_clyApp_CardEventDataStruct* pSpEvent //[IN]
																								,&st_CurrentDateAndTime//const st_Cly_DateAndTime *st_CurrentDateAndTime //[IN]
																								,&usRemainingMinutes//unsigned short* pRemainingMinutes//[OUT]
																								);

     
      if(!pProcessedValidityInfo->IsValidInterchange && (pProcessedValidityInfo->Counter==0))
      {
        return e_ClyApp_ContractNoLongerValid;//interchange is not valid and counter is 0
      }
      else
      { 

	    // hime 19/04/15 update m_MaavarPrevProvider 
	    pProcessedValidityInfo->m_MaavarPrevProvider=pSpEvent->uc_EventServiceProvider;
	   // hime 19/04/15 update m_MaavarPrevLine 
	    pProcessedValidityInfo->m_MaavarPrevLine=usGetShilutFromEventAccordingToVersion(pSpEvent);

        pProcessedValidityInfo->usRemainingInterchangeMinutes = usRemainingMinutes;
      }

			//set InterchangeRights 03/2013
			if(pSpEvent->st_OptionalEventData.b_IsEventInterchangeRightsExist)
			{
				pProcessedValidityInfo->ucInterchangeRIghts = pSpEvent->st_OptionalEventData.uc_EventInterchangeRights;
			}
			//03/2013 set psngr num
			if(pSpEvent->st_OptionalEventData.b_IsEventPassengersNumberExist)
				pProcessedValidityInfo->ucPsngrCount = pSpEvent->st_OptionalEventData.uc_EventPassengersNumber;
    } 
		//Yoni 10/2012
		if(!pProcessedValidityInfo->IsValidInterchange)
		{
			//set usRemainingInterchangeMinutes for first use
			pProcessedValidityInfo->usRemainingInterchangeMinutes = us_RemainingMinutesInFirstTrip(&pContractData->st_OptionalContractData);
			//interchangevalid is false
		}

		if(pProcessedValidityInfo->IsValidInterchange)
		{
			if(pSpEvent->st_OptionalEventData.b_IsEventTicketExist)
			{
				//also set code from EventTicket  
				pProcessedValidityInfo->ucEventTicketFareCode = pSpEvent->st_OptionalEventData.st_EventTicket.uc_EventTicketFareCode;
			}
		}

  }
  if(pProcessedValidityInfo->Counter==0 && !pProcessedValidityInfo->IsValidInterchange)
  {

    if(b_CounterPeriodJourniesValid==clyApp_TRUE)
    {
      return e_ClyApp_ContractValid;
    }
    else return e_ClyApp_ContractNoLongerValid;
  }

  return e_ClyApp_ContractValid;

}


//////////////////////////////////////////////////////////////////////////////////////////////
//  Yoni 11/2011
//  Metronit
//  Calculate the end date of contract according to start date and validity duration
//  for card
//////////////////////////////////////////////////////////////////////////////////////////////
st_Cly_DateAndTime stCalcEndDateByValidityDuration(const st_ClyApp_CardContractRecord* pst_CardContractRecord,
                                                   unsigned char RecNum //[IN]
                                                   )
{

  st_Cly_Date  stEndDateCompact={0};
  st_Cly_DateAndTime st_EndDateAndTime={0};
  unsigned long ul_FirstUseStampReal=0, ul_EndTime=0;
  unsigned char uc_DurationUnitCount=0;
  //if validity in months/weeks, resolution in in days
  e_ClyApp_DurationType eDurationTp=pst_CardContractRecord->st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.e_DurationType;


  switch(eDurationTp)
  {

  //case e_DurationInDays:
  //case e_DurationInHalfHours:
  //  ul_FirstUseStampReal = ul_Internal_GetFirstUseDateAndTimeFromEventRecordsSecsFrom2000(RecNum);
  //  uc_DurationUnitCount = pst_CardContractRecord->st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.uc_DurationUnitCount;
  //  //add minutes/days and convert to st_Cly_DateAndTime
  //  if(eDurationTp == e_DurationInDays)
  //  {

  //    ul_EndTime = ul_FirstUseStampReal+ uc_DurationUnitCount*SEC_IN_ONE_DAY;
  //  }
  //  else if(eDurationTp == e_DurationInHalfHours)
  //  {
  //    ul_EndTime = ul_FirstUseStampReal+ uc_DurationUnitCount*SEC_IN_HALF_HOURS;
  //  }
  //  //convert
  //  b_Internal_Convert2000Sec2StTime (ul_EndTime,&st_EndDateAndTime);

  //  break;

  default:
    //e_DurationInMonths:
    //e_DurationInWeeks:
    stEndDateCompact = st_CalcEndDateByPeriod( &pst_CardContractRecord->st_CardContractIssuingData.st_ContractValidityStartDate,//[IN] start date
      pst_CardContractRecord->st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.e_DurationType, //[IN] Duration period type
      pst_CardContractRecord->st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.uc_DurationUnitCount);//[IN] duration units


    st_EndDateAndTime.st_Date = stEndDateCompact;
    st_EndDateAndTime.st_Time = stTimeOfEndDay;//23:59:59

    break;


  }

  return st_EndDateAndTime;

}


//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//                b_Internal_IsContractNoLongerValidAndChangedPriority2Invalid
//DESCRITION:
//
//                if Contract No Longer Valid - Changed it Priority to Invalid and return clyApp_TRUE
//
//RETURN:
//
//                clyApp_TRUE -  Priority was Changed to invalid
//                clyApp_FALSE - invalid Contract not found
//LOGIC :
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static clyApp_BOOL b_Internal_IsContractNoLongerValidAndChangedPriority2Invalid(char c_RecNum,
  union_ClyApp_ContractRecord *union_ContractRecord)
{

  st_ProcessedContractValidityInfo ProcessedValidityInfo;//not used
  //===================================================================
  //if contract is no longer valid - update Next priority list in memory
  //===================================================================
  //Check basic Calypso informatio to conform that the record is valid - since for example it is not possible to update recored which is not used for tranportation
  if(e_ClyApp_Ok==e_Internal_BasicCalypsoCheckIfRecValid(e_ClyApp_Card,e_clyCard_ContractsFile,c_RecNum,(void *)union_ContractRecord))
  {
    if( e_ClyApp_CheckContractValidity( c_RecNum, &ProcessedValidityInfo ) == e_ClyApp_ContractNoLongerValid)
    {
        st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[c_RecNum-1] = e_CardPriorityInvalid;
        // Contract No Longer Valid found
        return clyApp_TRUE;
    }
  }
  //invalid Contract not found
  return clyApp_FALSE;
}


//Yoni 11/2009
//Check if any of the priorities in BCPL need to be updated
static void v_UpdateNextBCPL()
{
  int i;
  //copy current priority list to the next event to be written   (copy from st_EventRecArr[1])
  memcpy( st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr,
    st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[1].e_EventBestContractPriorityListArr,
    sizeof(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr));
  //for all list - check if a contract is no longer valid  -> if so update next priority list
  for(i=0;i<MAX_CONTRACT_COUT;i++)
  {

    if(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i]<e_CardPriorityErasable)
    {
      //get contract from memory
      memcpy(&Global_union_ContractRecord,
        &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[i+1],
        sizeof(Global_union_ContractRecord));


      //=====================================================================
      //if contract is no longer valid - update Next priority list in memory
      //=====================================================================
      b_Internal_IsContractNoLongerValidAndChangedPriority2Invalid((char)(i+1),&Global_union_ContractRecord);
    }
  }

}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                v_Internal_FillEventData
//
//DESCRITION:
//
//                 Fill Event Data before write 2 card
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static void v_Internal_FillEventData( st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[OUT] event to fill
  unsigned char uc_EventContractPointer)//[IN]Event Contract Pointer
{
  st_Cly_DateAndTime st_CurrentDateAndTime;
  //Get Current Date and time - struct
  st_CurrentDateAndTime = st_GetCurrentDateAndTime();

  //Event VersionNumber
  stp_CardEventDataStruct->uc_EventVersionNumber = EVENT_VERSION_NUM;
  //Service Provider
  stp_CardEventDataStruct->uc_EventServiceProvider = g_Params.uc_ProviderId;
  //Event Contract Pointer
  stp_CardEventDataStruct->uc_EventContractPointer = uc_EventContractPointer;

  //Event Date and Time  = current time
  stp_CardEventDataStruct->st_EventDateTimeStamp = st_CurrentDateAndTime;

  //copy the most updated priority list to the envent
  memcpy(stp_CardEventDataStruct->e_EventBestContractPriorityListArr,st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr,sizeof(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr));

}

//convert gmt to local
clyApp_BOOL  ConvertGmtToLocal(const st_Cly_DateAndTime *stp_TimeGmtIn,//[IN] datetime in gmt
  st_Cly_DateAndTime *stp_TimeLocalOut //[OUT] datetime in local
  )
{
  long l_sec1;
  int ZoneTime;

  ZoneTime=g_time_zone_bias_minutes;
  ZoneTime*=60;

  l_sec1 = l_Internal_ConvertStTime2SecFrom2000 (stp_TimeGmtIn);

  l_sec1+=ZoneTime;

  return b_Internal_Convert2000Sec2StTime(l_sec1, stp_TimeLocalOut);
}

//Yoni 11/2010
//calculate end of day for given datetime in GMT
//alg: convert to local, put 23:59 and convert to seconds from 1997
long l_GetEndOfDayInSecondsFrom1997(const st_Cly_DateAndTime *stp_TimeIn) //[IN] GMT
{
  st_Cly_DateAndTime Local;
  //convert gmt to local
  ConvertGmtToLocal(stp_TimeIn, &Local);

  //end of day
  Local.st_Time.hour=23;
  Local.st_Time.min=59;
  Local.st_Time.sec=59;

  //get time real without subtracting time zone
  return ul_GetTimeReal(&Local, 0);
}

////////////////////////////////////////////////////////////////////////////
//	Yoni 
//	Modified for Metronit  03/2013
//	return number of seconds from 1997 for next day in 03:00
//	for restrictduration calculations
//	input in GMT
////////////////////////////////////////////////////////////////////////////
static long lv_CalcEndOfServiceInSecondsFrom1997(const st_Cly_DateAndTime* pClyDateTime) //in GMT
{
  //end of day + hours from param 
	//the param is in minutes so multiply by 60 to get seconds
	long lEndOfServiceSeconds = 60L*(long)g_Params.us_EndOfServiceHour;
	long res = l_GetEndOfDayInSecondsFrom1997(pClyDateTime);
  return lEndOfServiceSeconds+res;

}


////////////////////////////////////////////////////////////////////////////
//	Yoni 10/2012
//	us_RemainingMinutesInFirstTrip
//	determine the validty of interchange before first use (no valid interchange)
//	return resolution*units or
//		 RESTRICT_DURATION_END_OF_SERVICE or
//		 RESTRICT_DURATION_END_OF_DAY
////////////////////////////////////////////////////////////////////////////
static unsigned short us_RemainingMinutesInFirstTrip(const st_clyApp_OptionalContractData* pContractOptionalData)
{
  if (pContractOptionalData->b_ContractRestrictDurationExist &&
			pContractOptionalData->uc_ContractRestrictDuration  )
  {

    switch(pContractOptionalData->uc_ContractRestrictDuration)
    {
			case 62://Until end of exploitation (end of public opening) /end of day
				return RESTRICT_DURATION_END_OF_DAY;//9998;    

		 case 63://Until end of service
        return RESTRICT_DURATION_END_OF_SERVICE;//9999;     

      default:
      if(pContractOptionalData->uc_ContractRestrictCode == RESTRICT_CODE_5_MINUTE_RESOLUTION)//0x10
      {
        return pContractOptionalData->uc_ContractRestrictDuration*5;//x5 minutes
      }
      else
      {
        return pContractOptionalData->uc_ContractRestrictDuration*30;//x30 minutes
      }
		}
	}
	else return 0;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:       updated by Yoni 10/2010
//                b_CheckTransferTimeLimit
//
//DESCRITION:
//
//                Check that use of next segment in transfer contract is within time limit
//RETURN:
//
//                clyApp_TRUE=within time limit, use is allowed
//                clyApp_FALSE=passed time limit
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////
static clyApp_BOOL b_CheckTransferTimeLimit(
  const st_clyApp_OptionalContractData* pContractOptionalData
  ,const st_clyApp_CardEventDataStruct* pSpEvent //[IN]
  ,const st_Cly_DateAndTime *st_CurrentDateAndTime //[IN]
  ,unsigned short* pRemainingMinutes//[OUT]
  )
{
  long lv_now;// current time in seconds from 2000
  long lv_end;//end datetime of contract in seconds from 2000
  long RestrictDurationInSeconds;
  *pRemainingMinutes = 0;
  if(!pContractOptionalData || !pSpEvent)
    return clyApp_FALSE;
  if (pContractOptionalData->b_ContractRestrictDurationExist &&
    pContractOptionalData->uc_ContractRestrictDuration  )
  {



    switch(pContractOptionalData->uc_ContractRestrictDuration)
    {
    case 62://Until end of exploitation (end of public opening) /end of day
      lv_now = ul_GetTimeReal(st_CurrentDateAndTime, 0);//lv_now in local
      //calc end of day in seconds
      //date is same as event, time is end of day 23:59
      lv_end = l_GetEndOfDayInSecondsFrom1997(&pSpEvent->st_EventDataTimeFirstStamp);
      if(lv_end>=lv_now)
      {
        *pRemainingMinutes=RESTRICT_DURATION_END_OF_DAY;//9998;
        return clyApp_TRUE;
      }
      break;

    case 63://Until end of service
      {
        lv_now = ul_GetTimeReal(st_CurrentDateAndTime, 0);//lv_now in local
        lv_end = lv_CalcEndOfServiceInSecondsFrom1997(&pSpEvent->st_EventDataTimeFirstStamp);
        if(lv_end>=lv_now)
        {
          *pRemainingMinutes=RESTRICT_DURATION_END_OF_SERVICE;//9999;
          return clyApp_TRUE;
        }

      }
      break;

    default:
      lv_now = ul_GetTimeReal(st_CurrentDateAndTime, 0);//calc now in seconds in Local 17/11/10
      if(pContractOptionalData->uc_ContractRestrictCode == RESTRICT_CODE_5_MINUTE_RESOLUTION)//0x10
      {
        RestrictDurationInSeconds=pContractOptionalData->uc_ContractRestrictDuration*300;//x5 minutes
      }
      else
      {
        RestrictDurationInSeconds=pContractOptionalData->uc_ContractRestrictDuration*1800;//x30 minutes
      }
      //get time in seconds from event- no need to subtract time zone bias
      lv_end = ul_GetTimeReal(&pSpEvent->st_EventDataTimeFirstStamp, 0)
        + RestrictDurationInSeconds;//end of validity

      if( lv_end >= lv_now) //validity > now
      {
        //set the diff in minutes
        *pRemainingMinutes= (unsigned short)((lv_end-lv_now)/60);
        //fix by Yoni 8/3/11 if 0 return 1 because TIM doesn't "understand" 0
        if(*pRemainingMinutes==0)
          *pRemainingMinutes=1;
        return clyApp_TRUE;
      }
      break;

    }
  }
  return clyApp_FALSE;
}



//Yoni 11/2010
//check that counter value is ok for use
//call this in e_Internal_UseContract
static clyApp_BOOL bCounterHasEnoughRights(const st_ClyApp_CardContractRecord* pContract
  ,e_ClyApp_TariffCounterType e_TariffCounterType
  ,clyApp_BYTE uc_NumOfPassengers
  ,unsigned long ul_DebitAmount
  ,clyApp_BOOL b_IsFirstInterchange //if 0 then no need to decrease counter
  )
{
  switch(e_TariffCounterType)
  {

  case e_ClyApp_CounterAsDateAndRemainingNumOfJourneys:
    //check that counter >= uc_NumOfPassengers
    return (clyApp_BOOL)(pContract->st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.CounterValue
      >= uc_NumOfPassengers);

  case e_ClyApp_CounterAsNumOfToken:
    if(b_IsFirstInterchange)
    {
      //check that counter >= uc_NumOfPassengers
      return (clyApp_BOOL)(pContract->st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue
        >= uc_NumOfPassengers);

    }
    else return clyApp_TRUE;

  case e_ClyApp_CounterAsMonetaryAmount:
    //check that counter >= Debit Amount
    return (clyApp_BOOL)(pContract->st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue
      >= (long)ul_DebitAmount);
  default:
    return clyApp_FALSE;
  }

}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// b_IsNeedToWriteSpecialEventInUse
// return true if in use of this contract we need to update special event
// (for card)
///////////////////////////////////////////////////////////////////////////////////////////////////////
static clyApp_BOOL b_IsNeedToWriteSpecialEventInUse(const st_clyApp_CardContractIssuingData* pContractData)
{
  e_ClyApp_DurationType e_DurationTp= (e_ClyApp_DurationType)0;
  //if interchange contract return true
  if(pContractData->b_ContractIsJourneylnterchangesAllowed)
    return clyApp_TRUE;

  e_DurationTp = pContractData->st_OptionalContractData.st_ContractValidityDuration.e_DurationType;

  //if first use daily/hour sliding (date==0) return true
  if(e_DurationTp == e_DurationInDays || e_DurationTp == e_DurationInHalfHours)
  {
    if(pContractData->st_ContractValidityStartDate.Year == 0) //start date not set yet
      return clyApp_TRUE;
  }

  return clyApp_FALSE;
}


//#define MAX_LOCATIONS 10
//deleted on 10/2010. undo deleted when special tickets are implemented
//static clyApp_BOOL b_IsLastSegment(st_clyApp_OptionalContractData* pContractOptionalData, st_clyApp_OptionalEventData* pSpEvent)
//{
//    const TR_USHORT a[MAX_LOCATIONS+1] = {0, 2, 6, 14, 30, 62, 126, 254, 510, 1022, 2046};
//    int iLen =  pContractOptionalData->uc_LocationArrLen;
//
//    //boundary check
//    if ( --iLen <= 0 || iLen >  MAX_LOCATIONS)
//         return clyApp_FALSE;
//
//    return (clyApp_BOOL)(pSpEvent->b_IsEventPlaceExist &&  (a[iLen] == pSpEvent->ush_EventPlace));
//}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_UseContract
//
//DESCRITION:
//
//                UseContract
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//
//    during use operation:
//    ---------------------
//    contract update:
//        if startDate = 0 -> update to current date
//    counter update:
//        if counter exist:
//
//        e_ClyApp_CounterNotUsed =0 -> do nothing
//        e_ClyApp_CounterAsDateAndRemainingNumOfJourneys  -> if Period journey Exist ->update current date + decrease counter by uc_NumOfPassengers
//        e_ClyApp_CounterAsNumOfToken =2, -> decrease counter by uc_NumOfPassengers
//        e_ClyApp_CounterAsMonetaryAmount =3, -> decrease counter by ush_DebitAmount
//
//    event update:
//
//        EventDateFirstStamp
//            date exist in state machine - use it if not call getdatefromevent
//
//        bestPriorityList - change priority only for the contract that was used
//             e_CardPriorityHighestLevel - (current season pass):
//                dont change
//
//             e_CardPriorityOneBelowHighestLevel - (next season pass):
//                set to e_CardPriorityHighestLevel
//
//             e_CardPriorityTwoBelowHighestLevel - (current restricted season pass):
//                check if still valid  if valid dont change
//
//             e_CardPriorityThreeBelowHighestLevel - (next restricted season pass )
//                check if still valid  if valid change to e_CardPriorityTwoBelowHighestLevel
//
//             e_CardPriorityFourBelowHighestLevel - (current one time or multi-ride ticket)
//                check if still valid  if valid dont change
//
//             e_CardPriorityFiveBelowHighestLevel - (next one time or multi-ride ticket )
//                check if still valid  if valid change to e_CardPriorityFourBelowHighestLevel
//
//        event number of passengers - RFU
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////

static eCalypsoErr  e_Internal_UseContract( char c_ContractRecNum,//[IN]current recurd to use
  clyApp_BYTE uc_NumOfPassengers,//[IN] to be recored in the event record
  unsigned long ul_DebitAmount,//[IN] Amount (0 to 65535) of the contract counter - for MultiRide / stored value. Not relevalt for Season Pass
  unsigned long ul_StoredValueCredit, //[IN] Add Amount (0 to 65535) to the contract counter - for stored value only!!!
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card
  clyApp_BOOL b_IsFirstInterchange //[IN] if true then decrease counter (in interchange)
  )
{
  //    unsigned short usRemainingMinutes;
  int Tariff = 0;
  st_clyApp_CardContractIssuingData* pContractData ;
  st_Cly_Date st_CurrentDateCompact;
  e_ClyApp_TariffCounterType e_Tariff;
//  unsigned long  ul_StartDate;
  st_Cly_DateAndTime st_CurrentDateAndTime;
  eCalypsoErr err=e_ClyApp_NotOk;
  st_clyApp_CardEventDataStruct st_CardEventDataStruct;
  St_clySam_KIF_And_KVC St_KIF_And_KVC;
  e_clyCard_KeyType KeyType;
  int iSpecialEventRecNum;
  clyApp_BOOL bNeedSpecialEvent=clyApp_FALSE;

  unsigned long ulDecrease=0;

  //Get Current Date and time - struct
  st_CurrentDateAndTime = st_GetCurrentDateAndTime();
  st_CurrentDateCompact = st_CurrentDateAndTime.st_Date;



  //==============================
  //check card type  - if card
  //==============================
  if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
  {
    //===============================================================================================================================================================
    //read two last history files ( since they need to be send back to the user in the end of the transaction - third record will be written by the use operation )
    //===============================================================================================================================================================

    ////////////////////////
    // Read event Record 1
    ///////////////////////
		eCalypsoErr OutConverError;
    err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
      (clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
      //if record can not be read - exist
      return err;

    ////////////////////////
    // Read event Record 2
    ////////////////////////
    err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
      (clyCard_BYTE)2,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
      //if record can not be read - exist
      return err;


    //==============================================================================================
    //Before open session - in case the contract is transfer ticket - check if it is the first use - get information from last event's record
    //==============================================================================================
    pContractData = &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData;

    //we need to do the following check before we change the contract
    bNeedSpecialEvent = b_IsNeedToWriteSpecialEventInUse(pContractData);

    //==================
    //Contract update
    //==================

    //=========================================
    //open session
    //=========================================

    //KeyType = (st_Static_StateMachine.e_SamType == e_ClyApp_SamCL)? e_clyCard_KeyCredit:e_clyCard_KeyDebit;
		//Yoni 03/2013
		KeyType = e_clyCard_KeyDebit;
    v_Internal_GetKifVal(KeyType,&St_KIF_And_KVC);

    err = ClyApp_Virtual_OpenSecureSession(  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
      KeyType,//[IN] Key Type to use for the session
      NULL); //[IN]Rec Num 2 Return: if read not requested send NULL

    if( err!=e_ClyApp_Ok)
      return err;

    // get contract start date
    //ul_StartDate = ush_GetDateCompact(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_ContractValidityStartDate);

    //if startDate doesn't exist (year==2041) -> update to current date
    if(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_ContractValidityStartDate.Year == stSlidingZeroDate.Year)
    {
      //store FirstStamp data for the EVENT file
      st_Static_StateMachine.st_EventDataTimeFirstStampOfContract2Use = st_CurrentDateAndTime;
      st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_ContractValidityStartDate = st_CurrentDateCompact;

      //============================================
      //WRITE TO CARD - only the date - 3 byte only
      //============================================
      if (!pContractData->b_ContractIsJourneylnterchangesAllowed || b_IsFirstInterchange)  //exclude transfer contracts
      {

        err = e_Internal_ContractLogicalOR_Write(  e_ClyApp_Card,//[IN]type - card \ ticket
          c_ContractRecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
          &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum], //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
          REC_SIZE//////!!!!!!!hren BYTE_COUNT_TO_UPDATE_STATR_DATE); // write only 3 byte version(3 bits ) + start date ( 14 bit ) -> totaly 17 but = 3 BYTES
          );
        if( err!=e_ClyApp_Ok)
          return err;
      }
    }

    //==================
    //Counter update
    //==================

    Tariff = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType;
    e_Tariff = (e_ClyApp_TariffCounterType)Tariff;
    //if counter exist
    if(e_Tariff != e_ClyApp_CounterNotUsed )
    {
      /////////////////////////////////////////
      // CASE Contract Period Journeys Exist
      /////////////////////////////////////////
      //e_ClyApp_CounterAsDateAndRemainingNumOfJourneys  -> if Period journey Exist ->update current date + decrease counter by uc_NumOfPassengers
      if( (e_Tariff == e_ClyApp_CounterAsDateAndRemainingNumOfJourneys) &&
        (st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist) )
      {
        return e_ClyApp_NotOk;//currently not supported todo 11/2011 because we now do "decrease" instead of updaterecord
        /////////////////////////////////////////////////////////////
        //basic Check if use is possible - there are enough rights for use
        /////////////////////////////////////////////////////////////

                #if 0

                if(uc_NumOfPassengers > st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.uc_MaxNumOfTripsInPeriod  )
          return e_ClyApp_NotEnoughRightsForUseErr;


        //if current date is part of last use period - increase current couner value
        if( b_Internal_IsCurrentDateIsPartOfLastUsePeriod(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.st_CounterDate, //[IN] Last use date
          st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.e_PeriodType) ) //[IN] period type
        {
          //if not enough rights
          //if(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.CounterValue + uc_NumOfPassengers > st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.uc_MaxNumOfTripsInPeriod )
          if(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.CounterValue < uc_NumOfPassengers )
            return e_ClyApp_NotEnoughRightsForUseErr;
          else
          {
            //update counter
            st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.CounterValue -= uc_NumOfPassengers;

            ulDecrease = uc_NumOfPassengers;
          }

        }
        else //first use for this period
        {
          //update counter
          st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.CounterValue = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.uc_MaxNumOfTripsInPeriod - uc_NumOfPassengers;

          ulDecrease = uc_NumOfPassengers;
        }

        //update date
        st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.st_CounterDate = st_CurrentDateCompact;
                #endif

            }
      else
      {
        /////////////////////////////////////
        // CASE COUNTER NEED TO BE INCREASE
        /////////////////////////////////////
        //Stored Value Credit only
        if( (e_Tariff == e_ClyApp_CounterAsMonetaryAmount && ul_StoredValueCredit>0)&&
          (st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_ContractTariff.e_TariffAppType == e_ClyApp_StoredValue) )
        {
          //credit && debit can not exist together
          if(ul_DebitAmount!=0)
            return e_ClyApp_WrongParamErr;

          //check that the stored value result will not exceed it's ceiling
          if(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue + ul_StoredValueCredit > g_Params.lv_StoredValueCeiling )
            return e_ClyApp_CounterCeilingErr;

          //e_ClyApp_CounterAsMonetaryAmount =3, -> decrease counter by ush_DebitAmount
          st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue += ul_StoredValueCredit;
        }
        else
        {
          //if this is transfer contract, and this is not the first segment and within time limit
          //then we will not decrease the counter      //Yoni 20.9.07
          if (!pContractData->b_ContractIsJourneylnterchangesAllowed || b_IsFirstInterchange )
          {

            /////////////////////////////////////////
            // CASE COUNTER NEED TO BE DECREASED
            /////////////////////////////////////////

            //if Stored Value Credit && debit = 0 -> return error
            if( (e_Tariff== e_ClyApp_CounterAsMonetaryAmount && ul_DebitAmount==0)&&
              (st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_ContractTariff.e_TariffAppType == e_ClyApp_StoredValue) )
              return e_ClyApp_WrongParamErr;

            /////////////////////////////////////////////////////////////
            //Check if use is possible - there are enough rights for use
            /////////////////////////////////////////////////////////////

            if(!bCounterHasEnoughRights(
              &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum]
            ,e_Tariff
              ,uc_NumOfPassengers
              ,ul_DebitAmount
              ,b_IsFirstInterchange))//Yoni 11/2010
            {
              return e_ClyApp_NotEnoughRightsForUseErr;
            }

            if( e_Tariff ==  e_ClyApp_CounterAsDateAndRemainingNumOfJourneys)
            {
              //update current date
              st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.st_CounterDate = st_CurrentDateCompact;
              //decrease rights
              st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.CounterValue -= uc_NumOfPassengers;

              ulDecrease = uc_NumOfPassengers;


              //currently not supported
              return e_ClyApp_NotOk;

            }
            //e_ClyApp_CounterAsNumOfToken =2, -> decrease counter by uc_NumOfPassengers
            else if( e_Tariff ==  e_ClyApp_CounterAsNumOfToken)
            {
              //decrease rights
              st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue -= uc_NumOfPassengers;

              ulDecrease = uc_NumOfPassengers;

            }
            else if(e_Tariff == e_ClyApp_CounterAsMonetaryAmount) //=3, -> decrease counter by ush_DebitAmount
            {

              if((long)ul_DebitAmount > st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue)
                return e_ClyApp_StoredValueNotEnoughCreditForUSe;

              st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue -= ul_DebitAmount;

              ulDecrease = ul_DebitAmount;

            }
          }
        }
      }

      if (!pContractData->b_ContractIsJourneylnterchangesAllowed || b_IsFirstInterchange )
      {


        err = e_Internal_CardDescreaseCounter(c_ContractRecNum,
                        ulDecrease);
        if( err!=e_ClyApp_Ok)
          return err;
      }
    }
    //==================
    //EVENT UPDATE
    //==================

    /////////////////////
    //bestPriorityList
    /////////////////////

    //Event Circumstances  is defined by the user  - check if legal value given
    if( !(stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances >= e_CardEventCircumEntry &&
      stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances <= e_CardEventCircumInterchangeExit) )
      return e_ClyApp_WrongParamErr;

    ////////////////////////////////////////////////////////////////////////
    //if contract is no longer valid after use - update Next priority list in memory
    ////////////////////////////////////////////////////////////////////////
    if(!bNeedSpecialEvent) //dont do this for contracts that use special event because this check is done before we write it
    {
      b_Internal_IsContractNoLongerValidAndChangedPriority2Invalid(c_ContractRecNum,
        (union_ClyApp_ContractRecord *)&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum]);
    }

    //Event Journey lnterchange - relevant only for transfer ticket
    stp_CardEventDataStruct->b_EventIsJourneylnterchange =(clyApp_BOOL)( pContractData->b_ContractIsJourneylnterchangesAllowed && (clyApp_BOOL)(!b_IsFirstInterchange)); //b_IsTransferTicketInterchage;


  }


  // fill event: VersionNumber,Service Provider,Event Contract Pointer,Event Date and Time,priority list
  v_Internal_FillEventData( stp_CardEventDataStruct,//[OUT] event to fill
    c_ContractRecNum);//[IN]Event Contract Pointer

  //////////////////////
  // EventDateFirstStamp
  //////////////////////
  stp_CardEventDataStruct->st_EventDataTimeFirstStamp =stp_CardEventDataStruct->st_EventDateTimeStamp; //  st_CurrentDateAndTime;
  //if interchange trip then get firststamp from specialevent
  if (stp_CardEventDataStruct->b_EventIsJourneylnterchange)
  {
    iSpecialEventRecNum=i_Internal_FindSpecialEvent(c_ContractRecNum);
    if(iSpecialEventRecNum >= 1)
    {
		stp_CardEventDataStruct->st_EventDataTimeFirstStamp = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[iSpecialEventRecNum].st_EventDataTimeFirstStamp;
    }
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////
  // if contract has "next" periority -> change to "current"
  //e_CardPriorityHighestLevel - (current season pass) -> dont change
  //e_CardPriorityTwoBelowHighestLevel - (current restricted season pass) ->  dont change
  //if e_CardPriorityFourBelowHighestLevel - (current one time or multi-ride ticket) ->dont change
  /////////////////////////////////////////////////////////////////////////////////////////////////

  //e_CardPriorityOneBelowHighestLevel - (next season pass) -> set to e_CardPriorityHighestLevel
  if(stp_CardEventDataStruct->e_EventBestContractPriorityListArr[c_ContractRecNum-1] == e_CardPriorityOneBelowHighestLevel)
    stp_CardEventDataStruct->e_EventBestContractPriorityListArr[c_ContractRecNum-1] = e_CardPriorityHighestLevel;
  else
  {
    // if e_CardPriorityThreeBelowHighestLevel - (next restricted season pass ) ->change to e_CardPriorityTwoBelowHighestLevel
    if(stp_CardEventDataStruct->e_EventBestContractPriorityListArr[c_ContractRecNum-1] == e_CardPriorityThreeBelowHighestLevel)
      stp_CardEventDataStruct->e_EventBestContractPriorityListArr[c_ContractRecNum-1] = e_CardPriorityTwoBelowHighestLevel;

    else
      //if e_CardPriorityFiveBelowHighestLevel - (next one time or multi-ride ticket ) ->change to e_CardPriorityFourBelowHighestLevel
      if(stp_CardEventDataStruct->e_EventBestContractPriorityListArr[c_ContractRecNum-1] == e_CardPriorityFiveBelowHighest)
        stp_CardEventDataStruct->e_EventBestContractPriorityListArr[c_ContractRecNum-1] = e_CardPriorityFourBelowHighestLevel;

  }

  //always update passenger number
  stp_CardEventDataStruct->st_OptionalEventData.b_IsEventPassengersNumberExist = clyApp_TRUE;
  stp_CardEventDataStruct->st_OptionalEventData.uc_EventPassengersNumber = uc_NumOfPassengers;

  if (e_Tariff == e_ClyApp_CounterAsMonetaryAmount && ul_DebitAmount!=0)
  {
    stp_CardEventDataStruct->st_OptionalEventData.b_IsEventTicketExist = clyApp_TRUE;
    stp_CardEventDataStruct->st_OptionalEventData.st_EventTicket.ush_EventTicketDebitAmount = (unsigned short)ul_DebitAmount;
  }
	if(stp_CardEventDataStruct->st_OptionalEventData.st_EventTicket.uc_EventTicketFareCode>0)
	{
		stp_CardEventDataStruct->st_OptionalEventData.b_IsEventTicketExist = clyApp_TRUE;
	}
	if(stp_CardEventDataStruct->st_OptionalEventData.st_EventTicket.ush_EventTicketRoutesSystem>0)
	{
		stp_CardEventDataStruct->st_OptionalEventData.b_IsEventTicketExist = clyApp_TRUE;
	}


  // Eitanm - Use bits overflow problem - must be fixed!
  // Stored Value
  if( (e_Tariff == e_ClyApp_CounterAsMonetaryAmount && ul_DebitAmount>0)&&
    (st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)c_ContractRecNum].st_CardContractIssuingData.st_ContractTariff.e_TariffAppType == e_ClyApp_StoredValue) )

  {
    
		//todo check this 03/2013
		//stp_CardEventDataStruct->st_OptionalEventData.b_IsEventInterchangeRightsExist = clyApp_FALSE;
    //stp_CardEventDataStruct->st_OptionalEventData.uc_EventInterchangeRights = 0;

    stp_CardEventDataStruct->st_OptionalEventData.b_IsEventRFU1Exist = clyApp_FALSE;
    stp_CardEventDataStruct->st_OptionalEventData.uc_EventRFU1 = 0;

  } // End of bits fix

  //WRITE TO CARD
  err = e_Internal_Write(  e_ClyApp_Card,//[IN]type - card \ ticket
    1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    stp_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
    );

  ////////////////////////////////////////////////////////////////////////////////////////
  //write special event in every use of continue/maavar contract
  // or in case of first use of sliding daily/hour (11/2011)
  if(bNeedSpecialEvent)
  {
    //rec = SpecialEvent.m_iIndex;
    //get the special event index associated with this contract
    iSpecialEventRecNum = i_Internal_FindSpecialEvent(c_ContractRecNum);
    //if
    if (iSpecialEventRecNum<0)
      iSpecialEventRecNum = i_Internal_FindEmptySpEvent(&st_CurrentDateAndTime);


    if ( iSpecialEventRecNum >= 1)
      err = e_Internal_Write(  e_ClyApp_Card,//[IN]type - card \ ticket
      (char)(iSpecialEventRecNum),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_SpecialEventFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      stp_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
      );//Yoni 14/6/10

  }
  return err;
}


//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_CalcContractPriority
//
//DESCRITION:
//
//                determine the priority of the contract
//
//RETURN:
//
//                clyApp_BOOL
//LOGIC :
//            check if contract is currently valid or has future validity start date
//            if contract if OneTimeOrMultiRideTicket  or SeasonPass and have restriction:[2],[5]
//
//    e_ClyApp_OneTimeOrMultiRideTicket =1,
//    e_ClyApp_SeasonPass =2, // e_CardPriorityHighestLevel , e_CardPriorityOneBelowHighestLevel
//    e_ClyApp_TransferTick =3,
//    e_ClyApp_FreeCertificate =4,
//    e_ClyApp_ParkAndRideSeasonPass =5,
//    e_ClyApp_StoredValue =6
//
//                if the requested priority is found in only one contract :
//                return the contract record index
//
//                if the requested priority is found in more then one contract :
//
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static e_ClyApp_CardPriorityType e_Internal_CalcContractPriority(union_ClyApp_ContractRecord *union_ContractRecord)
{
  clyApp_BOOL b_IsContractCurrentlyValid = clyApp_FALSE;
  unsigned long  ul_StartDate;
  st_Cly_Date st_CurrentDateCompact,st_ContractStartDate;
  st_Cly_DateAndTime st_CurrentDateAndTime;
  e_ClyApp_CardTariffAppType e_CardTariffAppType;
  long l;
  st_clyApp_CardContractIssuingData*  pcI = &union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData;

  //Get Current Date and time - struct
  st_CurrentDateAndTime = st_GetCurrentDateAndTime();
  st_CurrentDateCompact = st_CurrentDateAndTime.st_Date;

  // get contract start date
  st_ContractStartDate = union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractValidityStartDate;

  //========================================================================
  //check if contract is currently valid or has future validity start date
  //========================================================================
  // get contract start date
  ul_StartDate = ush_GetDateCompact( &st_ContractStartDate);

  // if validity statr date == 0  -  have not been used yet or not yet valid
  if( ul_StartDate != 0 )
  {
    l = l_Internal_DateCmp(&st_CurrentDateCompact,&st_ContractStartDate);
    //if current date is not grater then the end of the period - return clyApp_TRUE
    if( l >=0)
      b_IsContractCurrentlyValid = clyApp_TRUE;
  }

  //===========================================================================================
  //check if contract is OneTimeOrMultiRideTicket  or SeasonPass and have restriction:[2],[5]
  //===========================================================================================
  //get contract tariff
  e_CardTariffAppType = union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType;

  switch(e_CardTariffAppType)
  {
  case e_ClyApp_OneTimeOrMultiRideTicket:
  case e_ClyApp_TransferTick:
  case e_ClyApp_StoredValue: // StoredValue will treated as MultiRide
  case e_ClyApp_OneTimeOrMultiRideTicket46:

    //Time restriced MultiRideTicket will be treated as restriced season pass
    if(pcI->b_ContractIsJourneylnterchangesAllowed ||!pcI->st_OptionalContractData.b_ContractRestrictDurationExist )
    {
      if(b_IsContractCurrentlyValid)
        //(current one time or multi-ride ticket)
        return e_CardPriorityFourBelowHighestLevel;
      else
        //(next one time or multi-ride ticket )
        return e_CardPriorityFiveBelowHighest;
    }

  case e_ClyApp_SeasonPass:
  case e_ClyApp_FreeCertificate:
  case e_ClyApp_ParkAndRideSeasonPass:

    //check if restricted contract
    if(!union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictDurationExist &&
      !union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist)
    {
      if(b_IsContractCurrentlyValid)
        //(current season pass):
        return e_CardPriorityHighestLevel;
      else
        //(next season pass):
        return e_CardPriorityOneBelowHighestLevel;
    }

  }

  //all time restricted contracts
  if(b_IsContractCurrentlyValid)
    //(current restricted season pass):
    return e_CardPriorityTwoBelowHighestLevel;
  else
    // - (next restricted season pass )
    return e_CardPriorityThreeBelowHighestLevel;


  //   //debug value
  //    return e_CardPriorityLowestLevelOfPriority;//contract not for transport
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                v_Internal_ForgetCard
//
//DESCRITION:
//
//                Forget Card - eject the card from the RF field ( to avoid retification ) && delete card from state machine
//
//RETURN:
//
//                -
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static void v_Internal_ForgetCard(e_7816_DEVICE i_ReaderId)//[IN] the reader ID)
{
  //todo what's the difference between forget and release
  // make sure that the reader will not receive deselect by RF shutdown
   st_Static_StateMachine.b_IsCardEmptyEnv =  0; // Card empty flag reset
  ContactlessForgetCard();//  &e_AorL);
  e_ClyApp_ReleaseCard(i_ReaderId);//[IN] the reader ID

}

////////////////////////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_OpenSecureSession
//
//DESCRITION:
//
//                Open Secure Session
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//                open secure session and read env
//                update state machine that session is opened
//                if record was read - update state machine with the file data
//
////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr e_Internal_OpenSecureSession( St_clySam_KIF_And_KVC  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
  e_clyCard_KeyType KeyType, //[IN] Key Type to use for the session
  st_ClyApp_EnvAndHoldDataStruct *stp_EnvAndHoldDataStruct)//[OUT] the Event Data Struct read - if read not requested send NULL
{
  RESPONSE_OBJ* Obj;
  eCalypsoErr err;
  St_clyCard_OpenSessionInput St_OpenSessionInput;
  Union_clySam_WorKeyParamsAcess SessionWorkKey;
  St_clyCard_OpenSessionOutput St_OpenSessionOutput;

  //=========================================
  //open secure session and read env
  //=========================================
  //Open Session input
  St_OpenSessionInput.b_Is2ReturnKeyKvc=clyCard_TRUE;
  St_OpenSessionInput.FileToSelect = e_clyCard_NoFile2Select;//e_clyCard_EnvironmentFile;
  St_OpenSessionInput.KeyType = KeyType;

  St_OpenSessionInput.RecNum2Return =0;//stp_EnvAndHoldDataStruct?1:0; // read ENV file

  //Work Key input
  SessionWorkKey.KifAndKvc.KIF=St_KIF_And_KVC.KIF;//
  SessionWorkKey.KifAndKvc.KVC=St_KIF_And_KVC.KVC;//   0x60; // use the KVC return by the card

  Obj = pSt_ClySession_OpenSecureSession( st_Static_StateMachine.CardReaderId,//[IN]  card reader id
    st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
    &St_OpenSessionInput,//[IN] Open Session Input parameters
    e_clySam_KeyKIFandKVC,//[IN] choose SAM access type -  index \ KIF+KVC  are only available
    SessionWorkKey,//[IN] the SAM session work key ( index \ KIF+KVC ) found in the sam work keys
    &St_OpenSessionOutput);//[OUT]Open Session output parameters

  if( !IS_RES_OBJ_OK(Obj) )
    return e_ClyApp_CardReadErr;

  //check if last session was not retified.
  if(St_OpenSessionOutput.St_Ratif.b_IsRatifExist)
    st_Static_StateMachine.b_retification=(clyApp_BOOL)1;

  //================================================================
  //update state machine update state machine that session is opened
  //================================================================
  //rise exist flag
  st_Static_StateMachine.e_TransactionState=e_clyApp_SessionOpenOk;

  //=============================================================
  //if record was read - update state machine with the file data
  //=============================================================
  if(stp_EnvAndHoldDataStruct)
  {

    st_Static_StateMachine.st_TransactionData.b_IsEnvRecExist=clyApp_TRUE;
    //copy ENV bin data
    memcpy(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EnvRec,St_OpenSessionOutput.RecDataRead,REC_SIZE);
    //store retification data
    st_Static_StateMachine.St_Ratif = St_OpenSessionOutput.St_Ratif;

    //if record was read - translate it into binary data and stor it into global data
    //=========================================
    //translate the bin data to API stract
    //=========================================
    err = e_Internal_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
      e_ClyApp_Card,//[IN]type - card \ ticket
      e_clyCard_EnvironmentFile,//[IN] if not a ticket  - which record in the card
      St_OpenSessionOutput.RecDataRead,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      (void *)stp_EnvAndHoldDataStruct);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
    if( err!=e_ClyApp_Ok)
      return err;
    //=========================================
    //update state machine
    //=========================================
    //rise exist flag
    st_Static_StateMachine.st_TransactionData.b_IsEnvRecExist =clyApp_TRUE;
    //copy ENV struct data
    st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec =*stp_EnvAndHoldDataStruct;
  }

  return e_ClyApp_Ok;
}




//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                v_Internal_ClearLastSessionData
//
//DESCRITION:
//
//                Clear Last Session Data
//
//RETURN:
//
//                -
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////////////////////////

static void v_Internal_ClearLastSessionData()
{
  st_Static_StateMachine.b_FreeRecFoundForLoad =clyApp_FALSE;
  st_Static_StateMachine.b_isManufacturerModeTkt =clyApp_FALSE;
  st_Static_StateMachine.b_retification=clyApp_FALSE;
  st_Static_StateMachine.c_ContractRecNumberForUse=0;

    memset(&st_Static_StateMachine.CardVirtualImage,0,sizeof(st_ClyApp_TransactionVirtualData));  // Virtual data card cleanup
    st_Static_StateMachine.CardWriteMode        = e_NormalMode;

    memset(&st_Static_StateMachine.st_EventDataTimeFirstStampOfContract2Use,0,sizeof(st_Static_StateMachine.st_EventDataTimeFirstStampOfContract2Use));
  memset(&st_Static_StateMachine.St_Ratif,0,sizeof(st_Static_StateMachine.St_Ratif));
  memset(&st_Static_StateMachine.st_TransactionData,0,sizeof(st_Static_StateMachine.st_TransactionData));
  //clear user callback
  v_ClyCard_SetSessionCallBack((SESSION_CALLBACK)0);
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                v_Internal_ClearLastCardGlobalDataStateMachine
//
//DESCRITION:
//
//                Clear State Machine Data of last card info
//
//RETURN:
//
//                -
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

void CLYAPP_STDCALL v_Internal_ClearLastCardGlobalDataStateMachine()
{

  //clear special events file  //  transfer ticket
  //memset(&SpecialEvent,0, sizeof(SpecialEvent));
  //    m_i_lastSegment = 0;

  //Mark card as unexist
  st_Static_StateMachine.e_TransactionState = e_clyApp_NoCardExist;
  st_Static_StateMachine.e_7816CardType = e_7816_UNKNOWN_CARD;
  st_Static_StateMachine.e_AppCardType = e_ClyApp_UnKnown;

//#ifdef WIN32
//  st_Static_StateMachine.CardReaderId = _READER_NON;
//#else
//  st_Static_StateMachine.CardReaderId = INTERNALe_7816_LAST;
//
//#endif
  st_Static_StateMachine.uc_CardSnLen = 0;
  memset(st_Static_StateMachine.ucp_CardSn, 0, sizeof(st_Static_StateMachine.ucp_CardSn));
  v_Internal_ClearLastSessionData();
  //memset(&st_Static_StateMachine.st_CanceledStoredValueSnapshot, 0, sizeof(st_Static_StateMachine.st_CanceledStoredValueSnapshot));//23/7/13
}


//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                st_GetShortDate
//
//DESCRITION:
//
//                converting from WORD to struct
//                short date:
//                    number of month since January first ,1997(being date 0)
//                    Last complete year is 2039
//
//RETURN:
//            st_Cly_DateShort
//
//LOGIC :
//
////////////////////////////////////////////////////////////////////////////////////////////////////

st_Cly_DateShort st_GetShortDate( clyTkt_WORD sh_MonthsSince1997 )//[IN]
{
  st_Cly_DateShort st_DateShort;
  st_DateShort.Month=1;
  st_DateShort.Year=1997;

  st_DateShort.Month+=sh_MonthsSince1997%12;
  st_DateShort.Year+=sh_MonthsSince1997/12;

  return st_DateShort;

}
//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                ush_GetShortDate
//
//DESCRITION:
//
//                converting from struct to WORD
//
//                short date:
//                    number of month since January first ,1997(being date 0)
//                    Last complete year is 2039
//RETURN:
//
//                 clyTkt_WORD
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

clyTkt_WORD ush_GetShortDate(st_Cly_DateShort* st_date)
{

  clyTkt_WORD sh_MonthsSince1997 = st_date->Month-1 +  (st_date->Year-1997)*12;

  if( st_date->Year < 1997 )
    return 0;
  return sh_MonthsSince1997;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//            st_GetCompactTime
//
//DESCRITION:
//
//             converting from WORD to struct
//
//             compact time:
//                    duration,or time of the day:
//                    0-287 time since 0:00, in 5mn units(1/12 of an hour)
//
//
//RETURN:
//
//            st_Cly_Time
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

st_Cly_Time st_GetCompactTime( clyTkt_WORD sh_5MinUnitsCount )//[IN]
{
  clyTkt_WORD Min = sh_5MinUnitsCount*5;
  st_Cly_Time st_Time={0};
  st_Time.hour = Min/60;
  st_Time.min = Min%60;
  st_Time.sec = 0;

  return st_Time;
}
//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//            ush_GetCompactTime
//
//DESCRITION:
//
//             converting from WORD to struct
//
//             compact time:
//                    duration,or time of the day:
//                    0-287 time since 0:00, in 5mn units(1/12 of an hour)
//
//RETURN:
//
//            clyTkt_WORD
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

clyTkt_WORD ush_GetCompactTime(st_Cly_Time * st_Time)
{
  clyTkt_WORD Min = (st_Time->hour*60 + st_Time->min )/5;

  return Min;

}

////////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//            e_Tkt_TimeAndDateCallback
//DESCRITION:
//            Callback to ticket layer
//
//             converting from WORD to struct (short date,compact date,compact time)
//             or from struct to WORD,convertation by last parametr
//             not used structs parametrs will be 0
//             short date:
//                    number of month since January first ,1997(being date 0)
//                    Last complete year is 2039
//             compact date:
//                    number of days since January first , 1997 (being date 0)
//                    Last complete year is 2040
//             compact time:
//                    duration,or time of the day:
//                    0-287 time since 0:00, in 5mn units(1/12 of an hour)
//
//RETURN:
//            e_ClyTkt_ERR
//
//LOGIC :
//
////////////////////////////////////////////////////////////////////////////////////////////////////

e_ClyTkt_ERR e_Tkt_TimeAndDateCallback(
  clyTkt_WORD *date_timeINOUT,///short of time/date INOUT
  st_Cly_DateShort *shdateINOUT,// short date struct INOUT
  st_Cly_Time *comptimeINOUT,// compact time struct INOUT
  st_Cly_Date *compdateINOUT,///compact date struct INOUT
  e_ClyTkt_TktDateTimeConv Convtype)// type of convertation
{

  switch(Convtype)
  {
  case e_ClyTkt_TktDTConv_Bit2ShortDate:
    *shdateINOUT = st_GetShortDate(*date_timeINOUT);
    break;
  case e_ClyTkt_TktDTConv_ShortDate2Bit:
    *date_timeINOUT = ush_GetShortDate(shdateINOUT);
    break;

  case e_ClyTkt_TktDTConv_Bit2CompactDate:
    *compdateINOUT = st_GetDateCompact(*date_timeINOUT);
    break;
  case e_ClyTkt_TktDTConv_CompactDate2Bit:
    *date_timeINOUT = ush_GetDateCompact(compdateINOUT);
    break;

  case e_ClyTkt_TktDTConv_Bit2CompactTime:
    *comptimeINOUT = st_GetCompactTime(*date_timeINOUT);
    break;

  case e_ClyTkt_TktDTConv_CompactTime2Bit:
    *date_timeINOUT = ush_GetCompactTime(comptimeINOUT);
    break;
  default: return (e_ClyTkt_ERR)e_ClyApp_WrongParamErr;
  }
  return (e_ClyTkt_ERR)e_ClyTkt_NO_ERROR;//   e_ClyApp_Ok;

}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//            st_DayInWORD2stDate
//DESCRITION:
//
//            fill the struct date&time(par 1) by struct com date(par 3) plus number of days
//
//RETURN:
//
//            st_Cly_DateAndTime
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

st_Cly_DateAndTime st_DayInWORD2stDate( clyTkt_WORD ush_Days2Add2startDate,//[IN]days to add to start date
  st_Cly_Date *st_StartDate)//[IN]start date
{
  st_Cly_DateAndTime st_DateAndTimeResult;
  unsigned long ul_PeriodInMinutes = (ush_Days2Add2startDate*MINUTS_IN_ONE_DAY);
  //Return date and time  = start date + Time to add in minuts
  st_DateAndTimeResult =  st_CalcEndDateByTime(st_StartDate,//[IN] start date
    ul_PeriodInMinutes);//[IN] Time to add in minuts 0- 1890  min
  return st_DateAndTimeResult;

}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//            st_HourInWORD2stDate
//
//DESCRITION:
//
//            fill the struct date&time(par 1) by struct com date(par 3) plus number of hours
//
//RETURN:
//
//            st_Cly_DateAndTime
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

st_Cly_DateAndTime st_HourInWORD2stDate( clyTkt_WORD ush_Hours2Add2startDate,//[IN]days to add to start date
  st_Cly_Date *st_StartDate)//[IN]start date
{
  st_Cly_DateAndTime st_DateAndTimeResult;
  unsigned long ul_PeriodInMinutes = ush_Hours2Add2startDate*60;
  //Return date and time  = start date + Time to add in minuts
  st_DateAndTimeResult =  st_CalcEndDateByTime(st_StartDate,//[IN] start date
    ul_PeriodInMinutes);//[IN] Time to add in minuts 0- 1890  min
  return st_DateAndTimeResult;

}
//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//            ush_stDate2DayInWORD
//
//DESCRITION:
//
//            return the number of days between struct date&time(par 1) minus struct com date(par 3)
//
//
//RETURN:
//
//            clyTkt_WORD
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

clyTkt_WORD ush_stDate2DayInWORD(st_Cly_DateAndTime * st_Cly_EndDateAndTime,//[IN]end date
  st_Cly_Date *st_SartDate )//[IN]statr date
{
  st_Cly_DateAndTime stp_StartTimeReal;
  unsigned long ul_StartDateInSec,ul_EndDateInSec;


  memset(&stp_StartTimeReal,0,sizeof(stp_StartTimeReal));
  stp_StartTimeReal.st_Date = *st_SartDate;


  ul_StartDateInSec = ul_GetTimeReal(&stp_StartTimeReal,0);
  ul_EndDateInSec = ul_GetTimeReal(st_Cly_EndDateAndTime,0);

  //check result
  if(ul_StartDateInSec>ul_EndDateInSec)
    return 0; // error

  return (clyTkt_WORD)((ul_EndDateInSec-ul_StartDateInSec)/SEC_IN_ONE_DAY);

}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//            ush_stDate2HourInWORD
//
//DESCRITION:
//
//            return the number of Hours between struct date&time(par 1) minus struct com date(par 3)
//
//RETURN:
//
//            clyTkt_WORD
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

clyTkt_WORD ush_stDate2HourInWORD(st_Cly_DateAndTime * st_Cly_EndDateAndTime,//[IN]end date
  st_Cly_Date *st_SartDate )//[IN]statr date
{
  st_Cly_DateAndTime stp_StartTimeReal;
  unsigned long ul_StartDateInSec,ul_EndDateInSec;
  stp_StartTimeReal.st_Date = *st_SartDate;

  memset(&stp_StartTimeReal,0,sizeof(stp_StartTimeReal));
  ul_StartDateInSec = ul_GetTimeReal(&stp_StartTimeReal,0);
  ul_EndDateInSec = ul_GetTimeReal(st_Cly_EndDateAndTime,0);

  //check result
  if(ul_StartDateInSec>ul_EndDateInSec)
    return 0; // error

  return (clyTkt_WORD)((ul_EndDateInSec-ul_StartDateInSec)/MINUTS_IN_ONE_HOUR);


}
//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//                e_Tkt_Add2DateCallback
//DESCRITION:
//                callback to ticket layer
//                fill the struct date&time(par 1) by struct com date(par 3) plus number of days/hours
//                or fill the number of days/hours by struct date&time(par 1) minus struct com date(par 3)
//
//RETURN:
//                e_ClyTkt_ERR
//
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

e_ClyTkt_ERR e_Tkt_Add2DateCallback(
  st_Cly_DateAndTime *st_date_timeINOUT,// date & time struct INOUT
  clyTkt_WORD *ush_DayOrHourINOUT,// number of days or hours (by next parametr) in short INOUT
  st_Cly_Date *st_TSCdateIN,// struct of type compact date IN
  e_ClyTkt_TktDateTimeConv e_Convtype)//   type of convertation
{

  switch(e_Convtype)
  {


  case e_ClyTkt_TktDTConv_DayInWORD2stDate:
    *st_date_timeINOUT = st_DayInWORD2stDate( *ush_DayOrHourINOUT,//[IN]days to add
      st_TSCdateIN);//[IN]start date
    break;

  case e_ClyTkt_TktDTConv_HourInWORD2stDate:
    *st_date_timeINOUT = st_HourInWORD2stDate(*ush_DayOrHourINOUT,//[IN]Hour to add
      st_TSCdateIN);//[IN]start date
    break;

  case e_ClyTkt_TktDTConv_stDate2DayInWORD:
    *ush_DayOrHourINOUT = ush_stDate2DayInWORD(st_date_timeINOUT,//[IN]end date
      st_TSCdateIN);//[IN]statr date
    break;


  case e_ClyTkt_TktDTConv_stDate2HourInWORD:
    *ush_DayOrHourINOUT = ush_stDate2HourInWORD(st_date_timeINOUT,//[IN]end date
      st_TSCdateIN);//[IN]statr date
    break;
  default: return (e_ClyTkt_ERR)e_ClyApp_WrongParamErr;

  }
  return (e_ClyTkt_ERR)e_ClyTkt_NO_ERROR;///e_ClyApp_Ok;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//                e_Internal_GetContractValidityStatus
//DESCRITION:
//
//                accurding to the contract start and end date - return its status - ContractValid ,NoLongerValid,ValidButNotInThisPeriod,
//
//RETURN:
//                e_clyApp_ValidityType
//
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

e_clyApp_ValidityType  e_Internal_GetContractValidityStatus(st_Cly_DateAndTime *st_StartDateAndTime,//[IN] contract start date && time
  st_Cly_DateAndTime *st_EndDateAndTime)//[IN]contract end date && time
{
  st_Cly_DateAndTime st_CurrentDateAndTime = st_GetCurrentDateAndTime();
  long l;

  l = l_Internal_DateAndTimeCmp(st_StartDateAndTime,&st_CurrentDateAndTime) ;
  //if futue start date
  if( l>0)
    return e_ClyApp_ContractValidButNotInThisPeriod;
  l = l_Internal_DateAndTimeCmp(&st_CurrentDateAndTime ,st_EndDateAndTime);
  //if expired
  if( l >0)
    return e_ClyApp_ContractNoLongerValid;

  return e_ClyApp_ContractValid;
}




////////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//                e_ClyApp_CheckTktContractValidity
//DESCRITION:
//
//                Check if the tickey Contract is Validity for use
//
//RETURN:
//                number of days (0 to 511)
//
//LOGIC :
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

static e_clyApp_ValidityType e_ClyApp_CheckTktContractValidity( struct_ClyTkt_Ticket *struct_Ticket) //[IN] check if the ticket contract is valid
{

  st_Cly_DateAndTime st_StartDateAndTime,st_EndDateAndTime;
  e_ClyApp_DurationType e_DurationType;
  unsigned long l_Date;
  // st_Cly_Date  st_EndDateCompact;

  //st_EndDateAndTime = st_GetCurrentDateAndTime();


  memset(&st_StartDateAndTime,0,sizeof(st_StartDateAndTime));
  memset(&st_EndDateAndTime,0,sizeof(st_EndDateAndTime));

  if( IS_MULTI_TKT( struct_Ticket->st_TicketContractCommonData.st_Tariff ) )
  {
    if( struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys > 0 )
      return e_ClyApp_ContractValid;
    return e_ClyApp_ContractNoLongerValid;
  }
  else //season pass
  {

    e_DurationType = struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType == e_ClyTkt_DurationInMonths ? e_DurationInMonths : e_DurationInDays;

    //if fix date
    if( struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.e_TSC_Sliding == e_ClyTkt_ValidityStartsAtIssueDate )
    {
      //if duration NOT in hour - calc end date
      if(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType != e_ClyTkt_DurationInHours )
      {

        st_EndDateAndTime.st_Date = st_CalcEndDateByPeriod(&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date,//[IN] contract start date
          e_DurationType, //[IN] Duration period type
          struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.uc_DurationValue); //[IN] duration units
        st_StartDateAndTime.st_Date = struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date;

      }
      else
      {
        l_Date = ul_GetTimeReal(&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start,0);
        l_Date += (struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.uc_DurationValue * SEC_IN_HOURS);
        st_EndDateAndTime  = st_GetTimeReal(l_Date, 0);

        st_StartDateAndTime = struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start;

      }
    }
    else // sliding
    {
      //if vigin - always valid
      if( struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.TSL_IsVirginFlag)
        return e_ClyApp_ContractValid;


      st_EndDateAndTime.st_Date = st_CalcEndDateByPeriod(&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start.st_Date,//[IN] contract start date
        e_DurationType, //[IN] Duration period type
        struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.uc_DurationValue); //[IN] duration units

      st_StartDateAndTime = struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start;

      // st_EndDateCompact =  st_EndDateAndTime.st_Date;
    }

    //return the contract status
    return e_Internal_GetContractValidityStatus(&st_StartDateAndTime,//[IN] contract start date && time
      &st_EndDateAndTime);//contract end date && time
  }


}
//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//                us_Internal_GetDateOffset
//DESCRITION:
//
//                  offset of the days, from the start day to end date
//
//RETURN:
//                number of days (0 to 511)
//
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned short us_Internal_GetDateOffset( st_Cly_Date *StartDate, st_Cly_Date *EndDate)
{
  return (ush_GetDateCompact(EndDate) - ush_GetDateCompact(StartDate));
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//                e_Internal_ConvertTo7816TktType
//DESCRITION:
//
//                convet type from TR1000 type to 7816 type
//
//RETURN:
//                7816 type
//
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

e_7816_CardType e_Internal_ConvertTo7816TktType(  e_ClCardType e_ClType)//[IN] TR1000 type
{
  switch(e_ClType)
  {
  case e_C_TICKET: return e_7816_Cly_CTS256B;
  default: return e_7816_UNKNOWN_CARD;
  }
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//                b_Internal_IsKeyNotIssued
//DESCRITION:
//
//                chaeck is the Key is Not Issued
//
//RETURN:
//                clyApp_BOOL
//
//LOGIC :
//                open and close session - if fail return TRUE
///////////////////////////////////////////////////////////////////////////////////////////////////

clyApp_BOOL b_Internal_IsKeyNotIssued(e_clyCard_KeyType e_KeyType)
{
  St_clySam_KIF_And_KVC St_KIF_And_KVC;
  RESPONSE_OBJ *Obj;
  eCalypsoErr err;

  v_Internal_GetKifVal(e_KeyType, &St_KIF_And_KVC);

  err = ClyApp_Virtual_OpenSecureSession(St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
    e_KeyType,//[IN] Key Type to use for the session
    NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
  if( err!=e_ClyApp_Ok)
    return clyApp_TRUE;

  //==================
  // Close Session
  //==================
  Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
    st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
    clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified

  if (!IS_RES_OBJ_OK(Obj))
    return clyApp_TRUE;

  return clyApp_FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//
//                b_Internal_IsManufacturerModeTkt
//DESCRITION:
//
//                check if the ticket is in Manufacturer Mode  - all data is cleared
//
//RETURN:
//                clyApp_BOOL
//
//LOGIC :
//                check data from byte 10 ( block 5 ) to byte 32 (block 15) that all are cleared
////////////////////////////////////////////////////////////////////////////////////////////////////

clyApp_BOOL b_Internal_IsManufacturerModeTkt(unsigned char* ucp_BinTktData)
{
  int i;
  for(i=10;i<32;i++)
  {
    if(ucp_BinTktData[i] !=0 )
      return clyApp_FALSE;
  }
  return clyApp_TRUE;
}
static short TranslateShilutTo12Bit(unsigned short shilut)
{
    short res = shilut;
    short lsb = shilut & 1;
    res /= 4;
    return res | lsb;
}

///////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                v_FillCommonFieldsOfEvent
//
//DESCRITION:
//
//                Write Event Code - for lock operation which need event code update only
//
//RETURN:
//
//                eCalypsoErr
//
//LOGIC :
//                check card type  - if card
//                read two last history files + ENV file ( since they need to be send back to the user in the end of the transaction - third record will be written by the use operation )
//                copy last event to new event and fill the new enent with some new data
//                add the new event to the card
////////////////////////////////////////////////////////////////////////////////////////////////////


void v_FillCommonFieldsOfEvent(st_clyApp_CardEventDataStruct *stp_CardEventDataStruct/*IN/OUT*/)
{


	stp_CardEventDataStruct->st_EventCode.e_CardEventTransportMean = e_CardEventTransUrbanBus;//Yoni 05/2013
	//device
	stp_CardEventDataStruct->st_OptionalEventData.b_IsEventDevice4Exist=(clyApp_BOOL)1;
	stp_CardEventDataStruct->st_OptionalEventData.ush_EventDevice4=us_GetSaleDeviceNumber(g_Params.lv_DeviceNumber); //set 12LSBs of device number 1-4095

#ifndef TIM7020
	//place
	stp_CardEventDataStruct->st_OptionalEventData.b_IsEventPlaceExist=(clyApp_BOOL)1;
	stp_CardEventDataStruct->st_OptionalEventData.ush_EventPlace=g_Params.us_PlaceUniqueId;
#else
	if(g_Params.us_StationNumber)
	{
		stp_CardEventDataStruct->st_OptionalEventData.b_IsEventPlaceExist= (clyApp_BOOL)1;
		stp_CardEventDataStruct->st_OptionalEventData.ush_EventPlace     = g_Params.us_StationNumber;
	}
#endif
	//Line MAKAT number
	stp_CardEventDataStruct->st_OptionalEventData.b_IsEventLineExist = (clyApp_BOOL)0;
	// fill line,cluster,shilut
	if(g_TripInfo.m_MotLine)
	{
		stp_CardEventDataStruct->st_OptionalEventData.b_IsEventLineExist = (clyApp_BOOL)1;
		// line
		stp_CardEventDataStruct->st_OptionalEventData.ush_EventLine      = g_TripInfo.m_MotLine;
		if(g_TripInfo.m_Clustrer)
		{
         // cluster
		 stp_CardEventDataStruct->st_OptionalEventData.b_IsEventTicketExist=(clyApp_BOOL)1;
		 stp_CardEventDataStruct->st_OptionalEventData.st_EventTicket.ush_EventTicketRoutesSystem=g_TripInfo.m_Clustrer;
		 // shilut
		 stp_CardEventDataStruct->st_OptionalEventData.b_IsEventRunlDExist=(clyApp_BOOL)1;
		 stp_CardEventDataStruct->st_OptionalEventData.ush_EventRunlD=TranslateShilutTo12Bit(g_TripInfo.shilut);
		 
		}

	}
	//RFU
	stp_CardEventDataStruct->st_OptionalEventData.b_IsEventRFU1Exist = (clyApp_BOOL)0;
	stp_CardEventDataStruct->st_OptionalEventData.b_IsEventRFU2Exist = (clyApp_BOOL)0;



}

///////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_WriteEventCode
//
//DESCRITION:
//
//                Write Event Code - for lock operation which need event code update only
//
//RETURN:
//
//                eCalypsoErr
//
//LOGIC :
//                check card type  - if card
//                read two last history files + ENV file ( since they need to be send back to the user in the end of the transaction - third record will be written by the use operation )
//                copy last event to new event and fill the new enent with some new data
//                add the new event to the card
////////////////////////////////////////////////////////////////////////////////////////////////////

static eCalypsoErr CLYAPP_STDCALL e_Internal_WriteEventCode(e_7816_DEVICE i_ReaderId, //[IN]reader ID
  e_clyApp_CardEventCircumstances e_CardEventCircumstances)//[IN]Event Circums to write
{
  eCalypsoErr err;
  e_clyCard_KeyType e_KeyType;
  St_clySam_KIF_And_KVC St_KIF_And_KVC;
  RESPONSE_OBJ* Obj;

  st_clyApp_CardEventDataStruct st_CardEventDataStruct;
  st_ClyApp_EnvAndHoldDataStruct st_EnvAndHoldDataStruct;
  CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()


    //==============================
    //check card type  - if card
    //==============================
    if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
    {
			eCalypsoErr OutConverError;
      memset(&st_CardEventDataStruct,0,sizeof(st_CardEventDataStruct));

      //===============================================================================================================================================================
      //read two last history files + ENV file ( since they need to be send back to the user in the end of the transaction - third record will be written by the use operation )
      //===============================================================================================================================================================

      //=====================
      // Read Environment Record
      //=====================
			
      err =e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
        (clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        e_clyCard_EnvironmentFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        &st_EnvAndHoldDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
      if( err!=e_ClyApp_Ok)
        return err;

      //raise exist flag
      st_Static_StateMachine.e_TransactionState=e_clyApp_SessionOpenOk;


      ////////////////////////
      // Read event Record 2
      ////////////////////////
      err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
        (clyCard_BYTE)2,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        (void*) &st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
      if( err!=e_ClyApp_Ok)
        //if record can not be read - exist
        return err;

      ////////////////////////
      // Read event Record 1
      ///////////////////////
      err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
        (clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        (void*) &st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
      if( err!=e_ClyApp_Ok)
        //if record can not be read - exist
        return err;


      //======================================================================
      //copy last event to new event and fill the new enent with some new data
      //=======================================================================
      //fill next priority list - copy the must updated priority list fount in event 1 for the card

      memcpy(&st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr,&st_CardEventDataStruct.e_EventBestContractPriorityListArr,sizeof(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr));

      // fill event: VersionNumber,Service Provider,Event Contract Pointer,Event Date and Time,priority list
      v_Internal_FillEventData( &st_CardEventDataStruct,//[OUT] event to fill
        (unsigned char)0);//[IN]Event Contract Pointer



      //Event Data Time First Stamp = 0
      memset(&st_CardEventDataStruct.st_EventDataTimeFirstStamp,0,sizeof(st_CardEventDataStruct.st_EventDataTimeFirstStamp));

      //Event Circumstances  =  Invalidation
      st_CardEventDataStruct.st_EventCode.e_CardEventCircumstances = e_CardEventCircumstances;
			
			v_FillCommonFieldsOfEvent(&st_CardEventDataStruct);

      //=========================================
      //open session
      //=========================================
      e_KeyType = e_clyCard_KeyDebit;
      v_Internal_GetKifVal(e_KeyType,&St_KIF_And_KVC);

      err = ClyApp_Virtual_OpenSecureSession(  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
        e_KeyType,//[IN] Key Type to use for the session
        NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
      if( err!=e_ClyApp_Ok)
        return err;


      //WRITE TO CARD
      err = e_Internal_Write(  e_ClyApp_Card,//[IN]type - card \ ticket
        1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        &st_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
        );//Yoni 14/6/10
      if( err!=e_ClyApp_Ok)
        return err;

      //==================
      // Close Session
      //==================
      Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
        st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
        clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified

      if( !IS_RES_OBJ_OK(Obj) )
      {
        //if session close fail - make sure that the reader will not receive deselect by RF shutdown
        v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
        return e_ClyApp_CardWriteErr;
      }

      st_Static_StateMachine.e_TransactionState = e_clyApp_SessionCloseOk;

      return e_ClyApp_Ok;

    }
    else // ticket
    {
    }

    return e_ClyApp_CardLockedErr;
}

//=======================================================================
//             API   FUNCTIONS
//=======================================================================




//=======================================================================
//              MANDATORY FUNCTIONS
//=======================================================================

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_ReleaseCard
//
//DESCRITION:
//
//                Release Card - Forget the Card when the application finishes working with it
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//                check if priority list need to be updateded -> if so update and
//                releas card
//                mark card as unexist
//
//////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_ReleaseCard(e_7816_DEVICE i_ReaderId)//[IN] the reader ID
{
  //release card
  if( !b_ClyCard_EjectCard(i_ReaderId) )
    return e_ClyApp_UnknownErr;

  //clear State Machine global data from last card info
  v_Internal_ClearLastCardGlobalDataStateMachine();
  return e_ClyApp_Ok;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_StartWorkWithCard
//
//DESCRITION:
//
//                Start Work With Card
//                ask the provider to start work with the following card - This command is mandatory
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//            �   Check with 7816 if the number is correct
//            �   Get card info from 7816 and update state machine
//            �   check if reader is init
//            �   if card update card state machine - e_ClyCrd_StartWorkWithCard
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void v_LoadTr1000Dll_SetActiveReader(e_7816_DEVICE i_ReaderId);

static eCalypsoErr CLYAPP_STDCALL PRV_e_ClyApp_StartWorkWithCard(e_7816_DEVICE i_ReaderId,//[IN] the reader ID in which to detect
  unsigned char  *ucp_SNum, /*[IN]*/ //The card SN
  unsigned char  uc_SNumLen);//[IN] Serial Number Len

eCalypsoErr CLYAPP_STDCALL e_ClyApp_StartWorkWithCard(e_7816_DEVICE i_ReaderId,//[IN] the reader ID in which to detect
  unsigned char  *ucp_SNum, /*[IN]*/ //The card SN
  unsigned char  uc_SNumLen)//[IN] Serial Number Len
{
  eCalypsoErr e;
  e=PRV_e_ClyApp_StartWorkWithCard(i_ReaderId,ucp_SNum,uc_SNumLen);
#if ENABLE_TIME_MONITORING
  v_MonitorCheckPoint(1);
#endif
  return e;

}


eCalypsoErr CLYAPP_STDCALL PRV_e_ClyApp_StartWorkWithCard(e_7816_DEVICE i_ReaderId,//[IN] the reader ID in which to detect
  unsigned char  *ucp_SNum, /*[IN]*/ //The card SN
  unsigned char  uc_SNumLen)//[IN] Serial Number Len
{
  st_7816_CardResetInfo  st_CardResetInfo;
  St_clyCard_SN St_SN;
  long SN;
  e_7816_CardType e_CardType;
#if 0 //TBD:yoram
  const ContactlessInfo *sInfo;// =  ContactlessGetCardInfo();
#endif

  //=========================================
  //Check with 7816 if the number is correct
  //=========================================
  //cleare State Machine global data from last card info
  v_Internal_ClearLastCardGlobalDataStateMachine();

  //clear ticket OS state machine
//  memset(SaveSignesPtr,0,87);
#if 0 //TBD:yoram
  sInfo =  ContactlessGetCardInfo();
  e_CardType = e_7816_UNKNOWN_CARD;
  if( sInfo )
    e_CardType =e_Internal_ConvertTo7816TktType( sInfo->e_ClType);
  if( IS_CL_READER(i_ReaderId  ) && IS_TICKET_CARD (e_CardType) )
  {
    memcpy(&SN,ucp_SNum,sizeof(long));
    //if(  memcmp(sInfo->cp_ClUid,ucp_SNum,2)!=0 || memcmp(sInfo->cp_ClUid+4,ucp_SNum+2,6)!=0)

    if(memcmp(sInfo->cp_ClUid , ucp_SNum,uc_SNumLen)==0)

      //if(  memcmp(sInfo->cp_ClUid+6,ucp_SNum,4)!=0 )
      return e_ClyApp_CardErr;
  }
  else // card
#endif
  {
    _7816_GetCardResetInfo((e_7816_DEVICE)i_ReaderId,//[IN] the reader id
      &st_CardResetInfo);//[OUT] the card info

    memcpy(&SN,ucp_SNum,sizeof(long));
    //  v_FlipBytes((unsigned char *)&SN,sizeof(long));
    if( (st_CardResetInfo.c_UidLen != uc_SNumLen) ||  memcmp(st_CardResetInfo.cp_ClUid,ucp_SNum,uc_SNumLen)!=0 )
      return e_ClyApp_CardErr;

    e_CardType =(e_7816_CardType) st_CardResetInfo.e_CardType;

    //=========================================
    //Update Card OS Layer
    //=========================================
    memcpy(&St_SN.p_SerNum4,ucp_SNum,4);
    //in case the card reset was made outside this layer - update the layer state machine with it's require information
    b_ClyCard_StartWorkWithCard(i_ReaderId,//[IN]   card reader id
      st_Static_StateMachine.SamReaderId,//[IN]    sam reader id
      &St_SN);//[IN]  serial number in the

  }
  //=========================================
  //Check if legal card type
  //=========================================
  st_Static_StateMachine.e_AppCardType = e_Internal_GetAppCardType(e_CardType);
  if( st_Static_StateMachine.e_AppCardType == e_ClyApp_UnKnown )
    return e_ClyApp_WrongCardTypeErr;



  //=========================================
  //Get card info from 7816 and update state machine
  //=========================================
  st_Static_StateMachine.e_TransactionState = e_clyApp_CardExist;
  st_Static_StateMachine.e_7816CardType = e_CardType;
  //st_Static_StateMachine.CardReaderId = i_ReaderId;
  st_Static_StateMachine.uc_CardSnLen = uc_SNumLen;
  memcpy(st_Static_StateMachine.ucp_CardSn,ucp_SNum,uc_SNumLen);
  return e_ClyApp_Ok;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_IsCardDepositRefund
//
//DESCRITION:
//
//                Was Card Deposit Refund
//                This function gives the ability to check if a card Card Deposit was already Refund and therefor  it is locked for future use
//
//                IMPORTANT : befor any call to this function RESET MUST be done!!!
//
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//                if last Event Code = e_CardEventCircumTest -> the Card Deposit was Refund
///////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_IsCardDepositRefund(clyApp_BOOL *b_WasCardDepositRefund)//[OUT]1=DepositRefund ,0= Deposit not Refund
{


  eCalypsoErr err;
  st_clyApp_CardEventDataStruct st_CardEventDataStruct;
  CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()

    *b_WasCardDepositRefund=clyApp_FALSE;


  //==============================
  //check card type  - if card
  //==============================
  if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
  {
		eCalypsoErr OutConverError;
    memset(&st_CardEventDataStruct,0,sizeof(st_CardEventDataStruct));
    //read last event && copy last event data to new event_sem
    ////////////////////////
    // Read event Record 1
    ///////////////////////
    err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
      (clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
      //if record can not be read - exist
      return err;

    //chaeck if Event Circumstances  =  Invalidation

    if(st_CardEventDataStruct.st_EventCode.e_CardEventCircumstances == e_CardEventCircumTest )
      *b_WasCardDepositRefund=clyApp_TRUE;

  }
  return e_ClyApp_Ok;
}



//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_UseContract
//
//DESCRITION:
//
//                Use Contract found by e_ClyApp_IsValidContractExist
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//                �   check (interface init, card exist, type=known card , session open )
//                .   read two last history files ( since they need to be send back to the user in the end of the transaction)
//                �   update best priority list
//                �   write event
//                �   close session
///////////////////////////////////////////////////////////////////////////////////////////////////

static eCalypsoErr CLYAPP_STDCALL PRV_e_ClyApp_UseContract(clyApp_BYTE uc_NumOfPassengers,//[IN] to be recored in the event record
  unsigned long ul_DebitAmount, //[IN] Amount (0 to 65535) of the contract counter - for MultiRide / stored value. Not relevalt for Season Pass
  unsigned long ul_StoredValueCredit, //[IN] Add Amount (0 to 65535) to the contract counter - for stored value only!!!
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] user event data - for card
  struct_ClyTkt_Ticket *stp_TicketEvent,//[IN] Ticket event data
  TR_St_CancelData *union_BinDataForCancle,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
  clyApp_BOOL *b_Result,unsigned char cRecNum,//[OUT]1=use ok,0=use fail
  clyApp_BOOL b_IsFirstInterchange //[IN] false if nikuv is needed
  );
////////////////////////////////////////////
eCalypsoErr CLYAPP_STDCALL e_ClyApp_UseContract(clyApp_BYTE uc_NumOfPassengers,//[IN] to be recored in the event record
  unsigned long ul_DebitAmount, //[IN] Amount (0 to 65535) of the contract counter - for MultiRide / stored value. Not relevalt for Season Pass
  unsigned long ul_StoredValueCredit, //[IN] Add Amount (0 to 65535) to the contract counter - for stored value only!!!
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] user event data - for card
  struct_ClyTkt_Ticket *stp_TicketEvent,//[IN] Ticket event data
  TR_St_CancelData *union_BinDataForCancle,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
  clyApp_BOOL *b_Result,unsigned char cRecNum,//[OUT]1=use ok,0=use fail
  clyApp_BOOL b_IsFirstInterchange, //[IN]
  union_ClyApp_TransactionBinData *p_union_TransactionBinData //[OUT]
  )
{
  eCalypsoErr e;

  e=PRV_e_ClyApp_UseContract(uc_NumOfPassengers,ul_DebitAmount,ul_StoredValueCredit,stp_CardEventDataStruct,//[IN] user event data - for card
    stp_TicketEvent,union_BinDataForCancle,b_Result,cRecNum,//[OUT]1=use ok,0=use fail
    b_IsFirstInterchange);


  //fill TR_St_TransactionData
  if(e == e_ClyApp_Ok)
  {
    e_ClyApp_GetTransactionBinaryData(p_union_TransactionBinData);//[OUT] get the Transaction Binary Data
  }


#if ENABLE_TIME_MONITORING
  v_MonitorCheckPoint(1);
#endif

  return e;

}

eCalypsoErr CLYAPP_STDCALL e_ClyApp_UseContractT(clyApp_BYTE uc_NumOfPassengers,//[IN] to be recored in the event record
  clyApp_WORD ush_DebitAmount, //[IN] Amount (0 to 65535) of the contract counter - for MultiRide / stored value. Not relevalt for Season Pass
  clyApp_WORD ush_StoredValueCredit, //[IN] Add Amount (0 to 65535) to the contract counter - for stored value only!!!
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] user event data - for card
  struct_ClyTkt_Ticket *stp_TicketEvent,//[IN] Ticket event data
  TR_St_CancelData *union_BinDataForCancle,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
  clyApp_BOOL *b_Result,
  clyApp_BOOL b_IsFirstInterchange //10/2010
  )//[OUT]1=use ok,0=use fail
{
  eCalypsoErr e;

  //    m_i_lastSegment = *i_lastSegment;
  e=PRV_e_ClyApp_UseContract(uc_NumOfPassengers,ush_DebitAmount,ush_StoredValueCredit,stp_CardEventDataStruct,//[IN] user event data - for card
    stp_TicketEvent,union_BinDataForCancle,b_Result,0, //[OUT]1=use ok,0=use fail
    b_IsFirstInterchange);

  return e;

}


static eCalypsoErr CLYAPP_STDCALL PRV_e_ClyApp_UseContract(clyApp_BYTE uc_NumOfPassengers,//[IN] to be recored in the event record
  unsigned long ul_DebitAmount, //[IN] Amount (0 to 65535) of the contract counter - for MultiRide / stored value. Not relevalt for Season Pass
  unsigned long ul_StoredValueCredit, //[IN] Add Amount (0 to 65535) to the contract counter - for stored value only!!!
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] user event data - for card
  struct_ClyTkt_Ticket *stp_TicketEvent,//[IN] Ticket event data
  TR_St_CancelData *union_BinDataForCancle,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
  clyApp_BOOL *b_Result,unsigned char cRecNum,//[OUT]1=use ok,0=use fail
  clyApp_BOOL b_IsFirstInterchange
  )

{
  RESPONSE_OBJ*    Obj;
  eCalypsoErr err;
  st_Cly_DateAndTime st_DateAndTime;
  CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()

    *b_Result=clyApp_FALSE;


  st_DateAndTime = st_GetCurrentDateAndTime();

  //======================================================================
  //check that a Valid Contract exist - Contract Rec Number For Use !=0
  //======================================================================
  st_Static_StateMachine.c_ContractRecNumberForUse=cRecNum;
  if(!st_Static_StateMachine.c_ContractRecNumberForUse)
    return e_ClyApp_NoContractSelectedErr;

  //==============================
  //check card type  - if card
  //==============================
  if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
  {

    //===============================================================
    // Copy the contract Bin Data before use - for cancel operation
    //===============================================================
    memset(union_BinDataForCancle,0,sizeof(*union_BinDataForCancle));
    union_BinDataForCancle->st_CardCancelData.uc_ContractRecNumBefore = union_BinDataForCancle->st_CardCancelData.uc_ContractRecNumAfter = st_Static_StateMachine.c_ContractRecNumberForUse;///???
    memcpy(union_BinDataForCancle->st_CardCancelData.ucp_ContractDataBeforeAction,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractRecArr[(int)st_Static_StateMachine.c_ContractRecNumberForUse],sizeof(CalypsoFileType));
    memcpy(union_BinDataForCancle->st_CardCancelData.ucp_CounterDataBeforeAction,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec + COUNTER_OFFSET(st_Static_StateMachine.c_ContractRecNumberForUse),COUTER_SIZE);
    memcpy(union_BinDataForCancle->st_CardCancelData.ucp_Event1BeforeAction,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[1],sizeof(CalypsoFileType));
    union_BinDataForCancle->st_CardCancelData.Action=BIN_DATA_OF_USE;

    //==================
    // Use contract
    //==================

    err = e_Internal_UseContract( st_Static_StateMachine.c_ContractRecNumberForUse,//[IN]current recurd to use
      uc_NumOfPassengers,//[IN] to be recored in the event record
      ul_DebitAmount,//[IN] Amount (0 to 65535) of the contract counter - for MultiRide / stored value. Not relevalt for Season Pass
      ul_StoredValueCredit, //[IN] Add Amount (0 to 65535) to the contract counter - for stored value only!!!
      stp_CardEventDataStruct,//[IN] user event data
      b_IsFirstInterchange //10/2010
      );
    if( err!=e_ClyApp_Ok)
      return err;

    //==================
    // Close Session
    //==================
    Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
      st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
      clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified

    if( !IS_RES_OBJ_OK(Obj) )
    {
      //if session close fail - make sure that the reader will not receive deselect by RF shutdown
      v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
      return e_ClyApp_CardWriteErr;
    }

    ////Yoni 07/2011
    //memcpy(union_BinDataForCancle->st_CardCancelData.ucp_Event1AfterAction,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[1],sizeof(CalypsoFileType));

  }
  else // ticket
  {
		eCalypsoErr OutConverError;
    //=========================================
    //read all ticket - from memory
    //=========================================
    e_Internal_Read(e_ClyApp_Ticket,//[IN]type - card \ ticket
      0,//[IN] //not relevat for ticket
      e_clyCard_TicketingDF, //[IN]not relevat for ticket
      (void*) &Global_union_ContractRecord, &OutConverError,0); //[OUT] data read

    //===================================================
    //check if ticket data is legal values - Basic check
    //===================================================
    err = e_Internal_BasicCalypsoCheckIfRecValid(e_ClyApp_Ticket,e_clyCard_TicketingDF,(clyCard_BYTE)0,(void*) &Global_union_ContractRecord.struct_Ticket);
    if( err!=e_ClyApp_Ok)
      return err;

    //===============================================================
    // Copy the contract Bin Data before use - for cancel operation
    //===============================================================
    memset(union_BinDataForCancle,0,sizeof(*union_BinDataForCancle));
    memcpy(union_BinDataForCancle->ucp_BinTktDataBeforUse,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData,sizeof(CalypsoBinTktType));

    //copy the contract to the user buffer + SN
    memcpy(&stp_TicketEvent->st_TicketContractCommonData,&Global_union_ContractRecord.struct_Ticket.st_TicketContractCommonData,sizeof(Global_union_ContractRecord.struct_Ticket.st_TicketContractCommonData));
    memcpy(&stp_TicketEvent->ucp_Sn,&Global_union_ContractRecord.struct_Ticket.ucp_Sn,sizeof(TKT_SN_TYPE));

    //if multi-ride
    if( IS_MULTI_TKT( st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket.st_TicketContractCommonData.st_Tariff) )
    {
      memcpy(&stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec,&Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec,sizeof(Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec));

      //==============================================
      //FIRST VALIDATION BACKUP- for backup purposes only
      //==============================================

      stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.uc_FirstFlag=1;
      stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.ush_TML_LocationId=Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.ush_TMF_LocationStamp;
      stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.uc_JourneysBck=Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys;
      stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.ush_SignatureBkp = Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.us_TMF_Sig;

      //check backup TotalJourneys value before write
      if ( stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.uc_JourneysBck > stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.uc_TMC_ValidityJourneys ||
        stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.uc_JourneysBck != Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys )
        return e_ClyApp_WrongParamErr;

      stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_DateStamp = st_DateAndTime.st_Date;
      stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_TimeStamp = st_DateAndTime.st_Time;

      //============================================================
      //WRITE TO CARD - first location backup data record ( first flag=1 )
      //============================================================
      err = e_Internal_Write( e_ClyApp_Ticket,//[IN]type - card \ ticket
        0,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        (e_clyCard_FileId)e_ClyTkt_MultiRideLocationRec, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        stp_TicketEvent, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
        0 // not relevat for ticket
        );//Yoni 14/6/10
      if( err!=e_ClyApp_Ok)
        return err;
      //==================
      //CONTINUE FIRST
      //==================

      stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_ServiceProvider = g_Params.uc_ProviderId;
      stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys = Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys -uc_NumOfPassengers;

      //check First Validation TotalJourneys  value before write
      if ( stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys >= stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.uc_TMC_ValidityJourneys ||
        stp_TicketEvent->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys != Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys - uc_NumOfPassengers)
        return e_ClyApp_WrongParamErr;

      //============================================================
      //WRITE TO CARD - first validation record
      //============================================================
      err = e_Internal_Write( e_ClyApp_Ticket,//[IN]type - card \ ticket
        0,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        (e_clyCard_FileId)e_ClyTkt_MultiRideFirstValidationRec, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        stp_TicketEvent, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
        0 // not relevat for ticket
        );//Yoni 14/6/10
      if( err!=e_ClyApp_Ok)
        return err;
    }
    else //season pass
    {
      memcpy(&stp_TicketEvent->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec,&Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec,sizeof(Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec));

      //============================================================
      //if FIRST USE of sliding contract OR contract in hours
      //============================================================
      if( (Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.e_TSC_Sliding == e_ClyTkt_ValidityStartsAtFirstUse ||
        Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType == e_ClyTkt_DurationInHours)  &&
        ( Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.TSL_IsVirginFlag == clyApp_TRUE) )
      {
        stp_TicketEvent->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start = st_DateAndTime;

        //============================================================
        //WRITE TO CARD - Initial record
        //============================================================
        err = e_Internal_Write( e_ClyApp_Ticket,//[IN]type - card \ ticket
          0,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
          (e_clyCard_FileId)e_ClyTkt_SeasonPassInitialRec, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
          stp_TicketEvent, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
          0 // not relevat for ticket
          );//Yoni 14/6/10
        if( err!=e_ClyApp_Ok)
          return err;
      }

      //============================================================
      //prepare validation record
      //============================================================
      stp_TicketEvent->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.st_TSLL_TimeStamp = st_DateAndTime.st_Time;
      stp_TicketEvent->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.TSL_IsVirginFlag =(ClyTkt_BOOL)0;
      stp_TicketEvent->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.uc_TSLL_ServiceProvider = g_Params.uc_ProviderId;
      if(!Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.e_TSC_Sliding)
        stp_TicketEvent->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.ush_TSSL_DateOffset = us_Internal_GetDateOffset(&st_DateAndTime.st_Date,&Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date);
      else
        stp_TicketEvent->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.ush_TSSL_DateOffset = us_Internal_GetDateOffset(&Global_union_ContractRecord.struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start.st_Date,&st_DateAndTime.st_Date);

      //============================================================
      //WRITE TO CARD - validation record
      //============================================================
      err = e_Internal_Write( e_ClyApp_Ticket,//[IN]type - card \ ticket
        0,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        (e_clyCard_FileId)e_ClyTkt_SeasonPassValidationRe, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        stp_TicketEvent, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
        0 // not relevat for ticket
        );//Yoni 14/6/10
      if( err!=e_ClyApp_Ok)
        return err;

    }

  }
  //======================
  // Update state machine
  //======================
  st_Static_StateMachine.e_TransactionState = e_clyApp_SessionCloseOk;
  *b_Result=clyApp_TRUE;
  return e_ClyApp_Ok; // OK

}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_GetTransactionBinaryData
//
//DESCRITION:
//
//                Get Transaction Binary Data
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//                return the transaction data stored in memory
////////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetTransactionBinaryData(union_ClyApp_TransactionBinData *union_TransactionBinData)//[OUT] get the Transaction Binary Data
{
  CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()

  //check that the transaction was close successfully so the data exist
  if(st_Static_StateMachine.e_TransactionState != e_clyApp_SessionCloseOk)
    return e_ClyApp_NoValidContractErr;

  if(st_Static_StateMachine.e_AppCardType == e_ClyApp_Card )
  {

    // in this state all data must already be stored in memory
    if(!(st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[0] &&
      st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[1] &&
      st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[2] &&
      st_Static_StateMachine.st_TransactionData.b_IsEnvRecExist))//TBD:yoram contractlist?
      return e_ClyApp_UnknownErr;

    //copy environment record
    memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_EnvironmentData,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EnvRec,REC_SIZE);

    //copy contract record
    memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_ContractData,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractRecArr[(int)st_Static_StateMachine.c_ContractRecNumberForUse],REC_SIZE);

    //copy event 1
    memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_Event1,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[0],REC_SIZE);
    //copy event 2
    memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_Event2,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[1],REC_SIZE);
    //copy event 3
    memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_Event3,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[2],REC_SIZE);

    //and counter
    memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_Counter, st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec,REC_SIZE);
    
		//and contractlist
    memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_ContractList, st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractList,REC_SIZE);

  }
  else //e_ClyApp_Ticket
  {
    // in this state all data must already be stored in memory
    if(!(st_Static_StateMachine.st_TransactionData.b_IsTktDataExist) )
      return e_ClyApp_UnknownErr;

    //copy environment record
    memcpy(union_TransactionBinData->ucp_CalypsoBinTkt,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData,sizeof(union_TransactionBinData->ucp_CalypsoBinTkt));

  }

  return e_ClyApp_Ok;
}
void v_ClyApp_EmptyStoreValueTransactionData(void )
{
	memset(&st_Static_StateMachine.st_CanceledStoredValueSnapshot,0,sizeof(st_Static_StateMachine.st_CanceledStoredValueSnapshot));
}

static int i_AllValueIs(int val,void *Data,int size)
{
	int i;
	unsigned char *ptr=(unsigned char *)Data;
	
	for(i=0;i<size;i++)
	 if(ptr[i]!=val)
		 return 0;
	 
	 return 1;
}
//
////////////////////////////////////////////////////////////////////////////////////////////////////
// Yoni 24/7/13
// e_ClyApp_GetSnapshotAfterStoredValueCancel
// get st_Static_StateMachine.ov_CardSnapshotAfterStoredValueCancel
// call this after loading of stored value, to get the saved snapshot of card after cancel
////////////////////////////////////////////////////////////////////////////////////////////////////
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetSnapshotAfterStoredValueCancel(unsigned char* pucCancelledRecNumOut, //out
																union_ClyApp_TransactionBinData *union_TransactionBinData
,int *p_IsAllEmpty)//[OUT] get the Transaction Binary Data
{
	*pucCancelledRecNumOut = st_Static_StateMachine.st_CanceledStoredValueSnapshot.uc_CancelledContractRecNum;
	if(st_Static_StateMachine.st_CanceledStoredValueSnapshot.uc_CancelledContractRecNum)//  1 to 8
	{
		union_TransactionBinData->st_CardTransactionBinData = st_Static_StateMachine.st_CanceledStoredValueSnapshot.ov_CardSnapshotAfterStoredValueCancel;
		*p_IsAllEmpty = 0;
	}
	else
	{
		*p_IsAllEmpty = 1;
	}
	return e_ClyApp_Ok;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Yoni 7/9/10
// e_ClyApp_GetTransactionBinaryDataForEmptyCardOnly
// like  e_ClyApp_GetTransactionBinaryDataForEmptySale but for emptycard sale in which there isn't
// any transaction so the statemachine is differnet (for example events are stored in indexes 1-3
// instead of 0-2)
////////////////////////////////////////////////////////////////////////////////////////////////////
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetTransactionBinaryDataForEmptySale(union_ClyApp_TransactionBinData *union_TransactionBinData)//[OUT] get the Transaction Binary Data
{
  CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()

    if(st_Static_StateMachine.e_AppCardType == e_ClyApp_Card )
    {

      // in this state all data must already be stored in memory
      if(!(st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[1] &&
        st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[2] &&
        st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[3] &&
        st_Static_StateMachine.st_TransactionData.b_IsEnvRecExist))
        return e_ClyApp_UnknownErr;

      //copy environment record
      memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_EnvironmentData,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EnvRec,REC_SIZE);

      //copy contract record
      memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_ContractData,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractRecArr[(int)st_Static_StateMachine.c_ContractRecNumberForUse],REC_SIZE);

      //copy event 1
      memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_Event1,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[1],REC_SIZE);
      //copy event 2
      memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_Event2,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[2],REC_SIZE);
      //copy event 3
      memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_Event3,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[3],REC_SIZE);

			//and contractlist
	    memcpy(union_TransactionBinData->st_CardTransactionBinData.ucp_ContractList, st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractList,REC_SIZE);

    }
    else //e_ClyApp_Ticket
    {
      // in this state all data must already be stored in memory
      if(!(st_Static_StateMachine.st_TransactionData.b_IsTktDataExist) )
        return e_ClyApp_UnknownErr;

      //copy environment record
      memcpy(union_TransactionBinData->ucp_CalypsoBinTkt,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData,sizeof(union_TransactionBinData->ucp_CalypsoBinTkt));

    }

    return e_ClyApp_Ok;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	api
//	us_GetAuthorizationCodeAsciiFromContractList
//	translate 29 bytes of ContractList and get AuthorizationCode for contract in given RecNum
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned short us_GetAuthorizationCodeAsciiFromContractList(const CalypsoFileType uc_ContractList, unsigned char RecNum/*1-8*/)
{
	
	unsigned short Bitmap=0;

	//convert to struct
	st_clyApp_ContractListStruct ov_ContractList;
	memset(&ov_ContractList, 0, sizeof(ov_ContractList));
	e_Internal_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
    e_ClyApp_Card,//[IN]type - card \ ticket
    e_clyCard_ContractListFile,//[IN] if not a ticket  - which record in the card
    (unsigned char *)uc_ContractList,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
    &ov_ContractList);//[OUT]

  //check that ov_Contractlist.ContractListAuthenticator is ok
	if(uc_CalcExpectedContractAuthVal( uc_ContractList,//[IN] binary data
        REC_SIZE) == ov_ContractList.ContractListAuthenticator) 
	{
		unsigned char index = RecNum-1;
		Bitmap = ov_ContractList.us_Bitmap;
		if(RecNum>=1 && RecNum<=8 && //verify RecNum
			(Bitmap & (1<<index)) //bit is set for this contract ?
			)
		{
			return ov_ContractList.ContractListAuthorizationCodeArr[index];
		}
	}

	return 0;

}

//Yoni 3/2011
//for given address and len check if all zero's
clyApp_BOOL bIsAllZero(const void* pStartFrom, int len)
{
  int i;
  const char* p =  (const char*)pStartFrom;
  for(i=0;i<len;i++)
  {
    if(p[i]!= 0)
      return clyApp_FALSE;
  }
  return clyApp_TRUE;
}
//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_GetTransactionBinaryData
//
//DESCRITION:
//
//                Find free record  ( for load operation )
//                Find if exist free record for a new Contract loading operation
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//                �   Read last event - if not read yet
//                �   Check best priority  list for a free record RFU type
//                �   Check best priority  list for a free record INVALID  priority and check if it can be change to erasable
//                �   Check current valid contracts if still valid
//
///////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_IsFreeRecExist(                clyApp_BOOL *b_Result,//[OUT]1=free record exist ok,0=no free record
  unsigned char * first_free_index//,///[out] first free index or ff if not found  
  )
{

  eCalypsoErr err;
  e_clyApp_ValidityType errv;
  st_clyApp_CardEventDataStruct st_CardEventDataStruct;
  clyApp_BYTE i;
  *b_Result=clyApp_FALSE;

  (*first_free_index)=0xff;//   not found
  CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()

    //==============================
    //check card type  - if card
    //==============================
    if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
    {
			eCalypsoErr OutConverError;
        //=================
        //read Last event
        //=================
        //if Event File was updated - read the data from the card and not from the memory in case the update has fail
        if( st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[0])
          memset( st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr,0,sizeof(st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr));

      //raed last record
      err =e_Internal_Read(  e_ClyApp_Card,//[IN]type - card \ ticket
        1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        (void*) &st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
      if( err!=e_ClyApp_Ok)
        return err;
      //copy current priority list to Next priority list
      memcpy(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr,
        st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[1].e_EventBestContractPriorityListArr,
        sizeof(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[1].e_EventBestContractPriorityListArr));


      //====================================================
      //Change RFU or INVALID priority to ERAESED priority
      //====================================================
      for(i=0;i<MAX_CONTRACT_COUT;i++)
      {
        //if contract is ERASABLE - marke space found!!!
        if( st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] == e_CardPriorityErasable)
        {
          *b_Result=clyApp_TRUE;
          continue;//Yoni 03/2011 (go to next contract, no need to go to next 'if')
        }
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Read all existing contracts even those which are marked as invalid to check if periority change is needed
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        if( ((int)(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i])  >= e_CardPriorityHighestLevel &&
          st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] <= e_CardPriorityFiveBelowHighest ) ||
          (st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] == e_CardPriorityInvalid )
          )
        {
          err = e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
            (clyCard_BYTE)(i+1),//[IN] //not relevat for ticket - record number to read : 1 is always the first record
            e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
            (void*) &Global_union_ContractRecord, &OutConverError,0);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
          // if( err!=e_ClyApp_Ok)   deleted 3/2011
          //        return err;
          //fix bug 1490
          //if contract all 0 + not IO problem => erase contract, else skip
          if( err!=e_ClyApp_Ok)
          {
            clyApp_BOOL bIOProblem=clyApp_FALSE;
            //check if contract is all zero
            clyApp_BOOL bContractAllZero =
              bIsAllZero(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractRecArr[i+1]
            ,sizeof(CalypsoFileType));
            //check if IO problem
            if(err == e_ClyApp_RecordNotFoundErr || err==e_ClyApp_CardReadErr)
              bIOProblem=clyApp_TRUE;

            if(bContractAllZero && !bIOProblem)
            {
              //mark as 14
              *b_Result=clyApp_TRUE;
              st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i]=e_CardPriorityErasable;
              continue;
            }
            else
            {
              //io problem or !zero - goto next contract
              continue;
            }
          }
          //else check contract validity
          //======================================================
          //if the contract is related to a counter - read it too
          //======================================================
          if( Global_union_ContractRecord.st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType != e_ClyApp_CounterNotUsed )
            err = e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
            (clyCard_BYTE)(i+1),//[IN] //not relevat for ticket - record number to read : 1 is always the first record
            e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
            (void*) &Global_union_ContractRecord, &OutConverError,0);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
          if( err!=e_ClyApp_Ok)
            return err;

          //////////////////////////////////////////////////////////////////////////////////
          //if contract is no longer valid - update Next priority list in memory to invalid
          ////////////////////////////////////////////////////////////////////////////////////
          if(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] != e_CardPriorityInvalid)
            b_Internal_IsContractNoLongerValidAndChangedPriority2Invalid((clyCard_BYTE)(i+1),//contract record number
            &Global_union_ContractRecord);
          /////////////////////////////////////////////////////////////////////////////////////////////
          //if the invalid record was issued by the same provaider - change the priority to Erasable
          /////////////////////////////////////////////////////////////////////////////////////////////
          if( (st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] == e_CardPriorityInvalid ) &&
            (Global_union_ContractRecord.st_CardContractRecord.st_CardContractIssuingData.uc_ContractProvider == g_Params.uc_ProviderId ) )
          {
            
						//03/2013
						//also check that this is not a stored value with counter>0 (marked as invalid by the logic few lines below)
						if((e_ClyApp_StoredValue == Global_union_ContractRecord.st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType)
							 && (0 < Global_union_ContractRecord.st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue)
							)	
						{
							//do nothing
						}
						else //normal
						{
            st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i]=e_CardPriorityErasable;
            *b_Result=clyApp_TRUE;
          }
          }

					/////////////////////////////////////////////////////////////////////////////////
					//Yoni 03/2013 handle stored value contract where profile is invalid. mark as 13
					/////////////////////////////////////////////////////////////////////////////////
					if(e_ClyApp_StoredValue == Global_union_ContractRecord.st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType)
					{
						//if contract is marked as valid
						if(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] < e_CardPriorityInvalid)
						{
							//check that profile is valid, and if not mark it as invalid (13)
							//this contract is not to be erased
							//check profile with b_IsContractValidBasic which does a few other things
							if (clyApp_FALSE == b_IsContractValidBasic(&Global_union_ContractRecord, (char)(i + 1)))
							{
								//contract is not valid - mark 13
								st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] = e_CardPriorityInvalid;
							}

						}
					}

        }
        else
        {
          ////////////////////////////////////////////////////////////////////////
          //change  RFU1 to RFU6 to ERAESED without reading the contract
          ////////////////////////////////////////////////////////////////////////
          if( st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] >= e_CardPriorityRFU1 &&
            st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] <= e_CardPriorityRFU6 )
          {
            st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i]=e_CardPriorityErasable;
            //space to load contract was found
            *b_Result=clyApp_TRUE;
          }

        }
      }






      //============================================================================================================================
      //if no erasable priority was found check if ivalid contracts of other providers exist - if so change one of them to Erasable
      //=============================================================================================================================
      if(*b_Result == 0)
        for(i=0;i<MAX_CONTRACT_COUT;i++)
          if(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] == e_CardPriorityInvalid)
          {
						//also check that this is not a stored value with counter>0 (marked as invalid)  03/2013
						if((e_ClyApp_StoredValue == st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)(i+1)].st_CardContractIssuingData.st_ContractTariff.e_TariffAppType)
							 && (0 < st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[(int)(i+1)].st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue))
						{
							//do nothing
						}
						else
						{
            *b_Result=clyApp_TRUE;
            st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i]=e_CardPriorityErasable;
            break; // change only one and exist
          }
          }
          //==================
          // Update state machine
          //==================
          st_Static_StateMachine.b_FreeRecFoundForLoad = *b_Result;


          if(*b_Result==1)
            for(i=0;i<MAX_CONTRACT_COUT;i++)
              if(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i]==e_CardPriorityErasable ||
                st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] == e_CardPriorityInvalid)
              {
                (*first_free_index)=i;//   not found
                break;
              }

              return e_ClyApp_Ok;
    }
    else // ticket
    {
			eCalypsoErr OutConverError;
      if(st_Static_StateMachine.e_AppCardType != e_ClyApp_Ticket)
        return e_ClyApp_WrongCardTypeErr ;

      //=========================================
      //read all ticket - internal_read
      //=========================================
      err =e_Internal_Read(e_ClyApp_Ticket,//[IN]type - card \ ticket
        0,//[IN] //not relevat for ticket
        e_clyCard_TicketingDF, //[IN]not relevat for ticket
        (void*) &Global_union_ContractRecord, &OutConverError,0); //[OUT] data read
      if( err!=e_ClyApp_Ok)
      {
        //
        // in case the ticket is NEW ( manufacturer mode )  - contract loading is allowed
        if( err  == e_ClyApp_CardContractErr && b_Internal_IsManufacturerModeTkt(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData))
        {
          *b_Result=clyApp_TRUE;
          st_Static_StateMachine.b_isManufacturerModeTkt = clyApp_TRUE;
          err = e_ClyApp_Ok;
          goto MARK_REC_FOUND;
        }
        return err;
      }

      //===================================================
      //check if ticket data is legal values - Basic check
      //===================================================
      err = e_Internal_BasicCalypsoCheckIfRecValid(e_ClyApp_Ticket,e_clyCard_TicketingDF,(clyCard_BYTE)0,(void*) &Global_union_ContractRecord.struct_Ticket);
      //can not load if contact is ilagal
      if( err!=e_ClyApp_Ok)
        return err;

      //==============================================================
      //check if Contract loading is possible - reload counter value
      //==============================================================
      if(Global_union_ContractRecord.struct_Ticket.st_TicketContractCommonData.uc_ReloadCount >= MAX_RELOAD_COUT)
        return e_ClyApp_TicketMaxReloadCounterErr;

      //==================================================================
      //check if card is empty - if so reloading of contract is possible
      //==================================================================
      errv = e_ClyApp_CheckTktContractValidity( &Global_union_ContractRecord.struct_Ticket); //[IN] check if the ticket contract is valid
      if(errv == e_ClyApp_ContractNoLongerValid)
        *b_Result=clyApp_TRUE;

MARK_REC_FOUND:
      //=======================
      // Update state machine
      //=======================
      st_Static_StateMachine.b_FreeRecFoundForLoad = *b_Result;

    }
    return e_ClyApp_Ok;
}

#ifdef BUILD_ZABAD_TOOL
void  StartSession()
{



    e_clyCard_KeyType e_KeyType;
    St_clySam_KIF_And_KVC St_KIF_And_KVC;
    eCalypsoErr err;
//    union_ClyApp_ContractRecord union_ContractRecord;




    if(st_Static_StateMachine.e_SamType == e_ClyApp_SamCL || st_Static_StateMachine.e_SamType == e_ClyApp_SamCP)
        e_KeyType = e_clyCard_KeyCredit;
    else
        return ;

    v_Internal_GetKifVal(e_KeyType,&St_KIF_And_KVC);


    ///////////////////////////////////////////////////////
    //
    // DELETE EVENT 1
    //
    ///////////////////////////////////////////////////////

    err = ClyApp_Virtual_OpenSecureSession(  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
        e_KeyType,//[IN] Key Type to use for the session
        NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
   

}

void  CloseSession()
{



    e_clyCard_KeyType e_KeyType;
    St_clySam_KIF_And_KVC St_KIF_And_KVC;






    if(st_Static_StateMachine.e_SamType == e_ClyApp_SamCL || st_Static_StateMachine.e_SamType == e_ClyApp_SamCP)
        e_KeyType = e_clyCard_KeyCredit;
    else
        return ;

    v_Internal_GetKifVal(e_KeyType,&St_KIF_And_KVC);

 ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
        st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
        clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified
}
#endif

///////////////////////////////////////////////////////////////////////////////
//  Yoni 10/2011
//  read 8 contract records and their counters - almost no logic
//  card only
//  reutrn ok if all existing records were read
//  return readerr if existing record failed to read (io)
//  return e_ClyApp_InterfaceNotInitErr if interface not init
//  todo ticket
///////////////////////////////////////////////////////////////////////////////
eCalypsoErr e_ClyApp_SimpleReadAllContracts(union_ClyApp_ContractRecord
                                                     union_ContractRecordArr[MAX_CONTRACT_COUT])
{
    eCalypsoErr err;
	eCalypsoErr OutConverError;
#ifdef BUILD_ZABAD_TOOL
	static int i_debug=0;
#endif

    int i;
    for (i = 0; i < MAX_CONTRACT_COUT; i++)
        {
           err = e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
                                      (clyCard_BYTE)(i+1),//[IN] //not relevat for ticket - record number to read : 1 is always the first record
                                      e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                      (void*)&union_ContractRecordArr[i], &OutConverError,0);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
          if ((e_ClyApp_CardReadErr == err) || (e_ClyApp_InterfaceNotInitErr == err))
          {
              return err;
          }
#ifdef BUILD_ZABAD_TOOL
if(i_debug)
{
	StartSession();
	e_Internal_Write(e_ClyApp_Card,(clyCard_BYTE)(i+1),e_clyCard_ContractsFile,(void*)&union_ContractRecordArr[i],29);
    CloseSession();


}
#endif

      if (union_ContractRecordArr[i].st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType != e_ClyApp_CounterNotUsed)
      {
        err=e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
          (clyCard_BYTE)(i+1),//[IN] //not relevat for ticket - record number to read : 1 is always the first record
          e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
          (void*)&union_ContractRecordArr[i], &OutConverError,0);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
        if ((e_ClyApp_CardReadErr == err) || (e_ClyApp_InterfaceNotInitErr == err))
        {
          return err;
        }
        if (e_ClyApp_RecordNotFoundErr == err)
        {
          break;
        }
        if (e_ClyApp_Ok != err)
        {
          continue;
        }
      }
    }
      return  e_ClyApp_Ok;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_GetAllContracts
//
//DESCRITION:
//
//                Get All Contract fond in the card/ticket - even the contract which are invalid
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//             �  Read last event - if not read yet
//             �  Read all contract which do not marked as absent or RFU
//
///////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetAllContracts(union_ClyApp_ContractRecord
  union_ContractRecordArr[MAX_CONTRACT_COUT])//[OUT]Array memory allocation of contracts to fill
{
  eCalypsoErr err;
  st_clyApp_CardEventDataStruct st_CardEventDataStruct;
  clyApp_BYTE i;
  // e_ClyApp_CardPriorityType e_NextEventPriorityListArr[MAX_CONTRACT_COUT+1];
  e_ClyApp_CardPriorityType *pNextEventPriorityListArr;
  CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()

    memset(union_ContractRecordArr, 0, MAX_CONTRACT_COUT * sizeof(union_ClyApp_ContractRecord));

  //==============================
  //check card type  - if card
  //==============================
  if (st_Static_StateMachine.e_AppCardType == e_ClyApp_Card)
  {
		eCalypsoErr OutConverError;
    CHECK_SESSION_OPEN()

      //=================
      //read Last event
      //=================
      //if Event File was updated - read the data from the card and not from the memory in case the update has fail
      if( st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[0])
        memset(st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr,0,sizeof(st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr));

    //raed last record
    err =e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
      1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*)&st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if (err!=e_ClyApp_Ok)
      return err;

    //copy current priority list to Next priority list
    //Yoni 11/2009
    pNextEventPriorityListArr = st_CardEventDataStruct.e_EventBestContractPriorityListArr;

    for (i = 0; i < MAX_CONTRACT_COUT; i++)
    {

      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Read all existing contracts even those which are marked as invalid
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      if ((pNextEventPriorityListArr[i] -1 > e_CardPriorityHighestLevel &&
        pNextEventPriorityListArr[i]  <= e_CardPriorityFiveBelowHighest) ||
        pNextEventPriorityListArr[i]==e_CardPriorityInvalid ||
        pNextEventPriorityListArr[i]==e_CardPriorityErasable //Yoni 22/2/10
        )
      {
        err = e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
          (clyCard_BYTE)(i+1),//[IN] //not relevat for ticket - record number to read : 1 is always the first record
          e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
          (void*)&union_ContractRecordArr[i], &OutConverError,0);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
        if ((e_ClyApp_CardReadErr == err) || (e_ClyApp_InterfaceNotInitErr == err))
        {
          return err;
        }
        if (e_ClyApp_RecordNotFoundErr == err)
        {
          break;
        }
        if (e_ClyApp_Ok != err)
        {
          continue;
        }

        if (union_ContractRecordArr[i].st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType != e_ClyApp_CounterNotUsed)
        {
          err=e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
            (clyCard_BYTE)(i+1),//[IN] //not relevat for ticket - record number to read : 1 is always the first record
            e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
            (void*)&union_ContractRecordArr[i], &OutConverError,0);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
          if ((e_ClyApp_CardReadErr == err) || (e_ClyApp_InterfaceNotInitErr == err))
          {
            return err;
          }
          if (e_ClyApp_RecordNotFoundErr == err)
          {
            break;
          }
          if (e_ClyApp_Ok != err)
          {
            continue;
          }
        }
      }
    }


    //start = OS_Tick_Count;
    if(e_Internal_ReadAllSpecialEvents() != e_ClyApp_Ok)
      return e_ClyApp_CardReadErr;

    v_UpdateNextBCPL();

    return e_ClyApp_Ok;
  }
  else //other
  {
    if (st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Ticket)// ticket
    {
			eCalypsoErr OutConverError;
      // read last record
      err=e_Internal_Read(e_ClyApp_Ticket,//[IN]type - card \ ticket
        0,//[IN] //not relevat for ticket
        e_clyCard_TicketingDF, //[IN]not relevat for ticket
        &union_ContractRecordArr[0].struct_Ticket, &OutConverError,0); //[OUT] data read [0]);


      if (err!=e_ClyApp_Ok)
        return err;
      //increase array result len

    }
    else// not ticket & not card
      return e_ClyApp_NoCardErr;
  }

  return e_ClyApp_Ok;

}


//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_GetContract
//
//DESCRITION:
//
//                read a requested contract  - issuing data + counter
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :

//
///////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetContract( union_ClyApp_ContractRecord *union_ContractRecord,//[OUT]
  unsigned char uc_ContractNumber)//[IN] contract Number to read: can take the values 1 to 8 - NOT RELEVAT FOR TICKET
{
  eCalypsoErr err;
	eCalypsoErr OutConverError;

  CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()

    //==============================
    //read card \ ticket contract
    //==============================
    err =  e_Internal_Read(st_Static_StateMachine.e_AppCardType,//[IN]type - card \ ticket
    uc_ContractNumber,//[IN] //not relevat for ticket - record number to read : 1 is always the first record
    e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    union_ContractRecord, &OutConverError,0);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  if( err!=e_ClyApp_Ok)
    return err;

  //==============================
  //check card type  - if card
  //==============================
  if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card )
  {

    //======================================================
    //if the contract is related to a counter - read it too
    //======================================================
    if( union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType != e_ClyApp_CounterNotUsed )
      err = e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
      uc_ContractNumber,//[IN] //not relevat for ticket - record number to read : 1 is always the first record
      e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      union_ContractRecord, &OutConverError,0);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts

  }

  return err;

}



//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_GetSpecialEvent
//
//DESCRITION:
//
//                get the special event related to given contract num
//
//RETURN:
//
//                if found = e_ClyApp_Ok and with the special event by ref.
//								else e_ClyApp_NotOk
//
//LOGIC :

//PRE:
//								special events were read
///////////////////////////////////////////////////////////////////////////////////////////////////
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetSpecialEvent(unsigned char ContractRecNum // 1-8
																										,st_clyApp_CardEventDataStruct* pSpecialEvent /*out*/)
{
	int iSpecialEventRecNum = 0;
	memset(pSpecialEvent, 0, sizeof(*pSpecialEvent));    
  iSpecialEventRecNum= i_Internal_FindSpecialEvent(ContractRecNum);
  if(iSpecialEventRecNum>=1)
  {
    //we have a special event associated with  contract    
    *pSpecialEvent= st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[iSpecialEventRecNum];	    
			return e_ClyApp_Ok;
  }

	return e_ClyApp_NotOk;

}





#if 0
10/2011
static eCalypsoErr e_ClyApp_LoadContractToTicket(todo params)
{
  //fill missing contract common data
  p_union_ContractRecord->struct_Ticket.st_TicketContractCommonData.uc_TC_KeyIndex=TKT_CONTRACT_LOAD_KEY;
  p_union_ContractRecord->struct_Ticket.st_TicketContractCommonData.uc_TC_Provider = st_Static_StateMachine.st_UserCallbacks.fp_uch_GetProviderId();

  //fill card SN into the struct
  memcpy(&p_union_ContractRecord->struct_Ticket.ucp_Sn,&st_Static_StateMachine.ucp_CardSn,sizeof(TKT_SN_TYPE));

  // update Reload Count value
  if ( st_Static_StateMachine.b_isManufacturerModeTkt == clyApp_TRUE )
    //if first time loading - set counter to 1
    p_union_ContractRecord->struct_Ticket.st_TicketContractCommonData.uc_ReloadCount = 1;
  else
    //add one to the existing reload counter
    p_union_ContractRecord->struct_Ticket.st_TicketContractCommonData.uc_ReloadCount = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Ticket.st_TicketContractCommonData.uc_ReloadCount + 1;


  //if multi-ride
  if ( IS_MULTI_TKT(p_union_ContractRecord->struct_Ticket.st_TicketContractCommonData.st_Tariff ) )
  {
    //==============================================================================================
    //prepare first validation record - note that only date, time and validityJourny need to exist
    //==============================================================================================
    memset(&p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec,0,sizeof(p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec));

    p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_DateStamp = st_DateAndTime.st_Date;
    p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_TimeStamp = st_DateAndTime.st_Time;
    p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys = p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.uc_TMC_ValidityJourneys;

    //fill missing multiride contract data
    p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.st_TMC_SaleDate = st_DateAndTime.stShortDate;

    //===================================================
    //check if ticket data is legal values - Basic check
    //===================================================
    err = e_Internal_BasicCalypsoCheckIfRecValid(e_ClyApp_Ticket,e_clyCard_TicketingDF,(clyCard_BYTE)0,&p_union_ContractRecord->struct_Ticket);

    if( err!=e_ClyApp_Ok)
      return err;

    //============================================================
    //store current ticket data befor write - for recovery purpose
    //============================================================
    memcpy(st_TktRecoveryDate.upc_FullBinTktData,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData,sizeof(CalypsoBinTktType));
    memcpy(st_TktRecoveryDate.SerialNum,st_Static_StateMachine.ucp_CardSn,sizeof(SN8));
    st_Static_StateMachine.st_UserCallbacks.fp_b_StoreTktRecoveryDate(&st_TktRecoveryDate);
    //store data for cancel
    memcpy(union_BinDataForCancle->ucp_BinTktDataBeforUse, st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData,sizeof(CalypsoBinTktType));
    //============================================
    //WRITE TO CARD - clear location record
    //============================================
    err = e_Internal_Write( e_ClyApp_Ticket,//[IN]type - card \ ticket
      0,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      (e_clyCard_FileId)e_ClyTkt_MultiRideLocationRec, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      &p_union_ContractRecord->struct_Ticket, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
      0, // not relevat for ticket
      clyApp_FALSE);//Yoni 14/6/10
    if( err!=e_ClyApp_Ok)
      return err;

    //============================================
    //WRITE TO CARD - first validity record
    //============================================
    err = e_Internal_Write( e_ClyApp_Ticket,//[IN]type - card \ ticket
      0,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      (e_clyCard_FileId)e_ClyTkt_MultiRideFirstValidationRec, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      &p_union_ContractRecord->struct_Ticket, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
      0, // not relevat for ticket
      clyApp_FALSE);//Yoni 14/6/10
    if( err!=e_ClyApp_Ok)
      return err;

    //============================================
    //WRITE TO CARD - contract record
    //============================================
    err = e_Internal_Write( e_ClyApp_Ticket,//[IN]type - card \ ticket
      0,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      (e_clyCard_FileId)e_ClyTkt_MultiRideContractRec, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      &p_union_ContractRecord->struct_Ticket, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
      0, // not relevat for ticket
      clyApp_FALSE);//Yoni 14/6/10
    if( err!=e_ClyApp_Ok)
      return err;


  }
  else //season pass
  {
    if ( IS_SEASON_TKT(p_union_ContractRecord->struct_Ticket.st_TicketContractCommonData.st_Tariff) )
    {
      //Validation Record
      memset(&p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec,0,sizeof(p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec));
      p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.TSL_IsVirginFlag=(ClyTkt_BOOL)1;

      //contract record
      //if sliding date contract - write the issuing date to start date field
      if(p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.e_TSC_Sliding)
        p_union_ContractRecord->struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date = st_DateAndTime.st_Date;

      //===================================================
      //check if ticket data is legal values - Basic check
      //===================================================
      err = e_Internal_BasicCalypsoCheckIfRecValid(e_ClyApp_Ticket,e_clyCard_TicketingDF,(clyCard_BYTE)0,&p_union_ContractRecord->struct_Ticket);

      if( err!=e_ClyApp_Ok)
        return err;

      //============================================================
      //store current ticket data befor write - for recovery purpose
      //============================================================
      memcpy(st_TktRecoveryDate.upc_FullBinTktData,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData,sizeof(CalypsoBinTktType));
      memcpy(st_TktRecoveryDate.SerialNum,st_Static_StateMachine.ucp_CardSn,sizeof(SN8));
      st_Static_StateMachine.st_UserCallbacks.fp_b_StoreTktRecoveryDate(&st_TktRecoveryDate);

      //store data for cancel
      memcpy(union_BinDataForCancle->ucp_BinTktDataBeforUse,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData,sizeof(CalypsoBinTktType));

      //============================================
      //WRITE TO CARD - clear initial record
      //============================================
      err = e_Internal_Write( e_ClyApp_Ticket,//[IN]type - card \ ticket
        0,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        (e_clyCard_FileId)e_ClyTkt_SeasonPassInitialRec, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        &p_union_ContractRecord->struct_Ticket, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
        0, // not relevat for ticket
        clyApp_FALSE);//Yoni 14/6/10
      if( err!=e_ClyApp_Ok)
        return err;

      //============================================
      //WRITE TO CARD - first validity record
      //============================================
      err = e_Internal_Write( e_ClyApp_Ticket,//[IN]type - card \ ticket
        0,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        (e_clyCard_FileId)e_ClyTkt_SeasonPassValidationRe, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        &p_union_ContractRecord->struct_Ticket, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
        0, // not relevat for ticket
        clyApp_FALSE);//Yoni 14/6/10
      if( err!=e_ClyApp_Ok)
        return err;

      //============================================
      //WRITE TO CARD - contract record
      //============================================
      err = e_Internal_Write( e_ClyApp_Ticket,//[IN]type - card \ ticket
        0,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        (e_clyCard_FileId)e_ClyTkt_SeasonPassContractRec, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        &p_union_ContractRecord->struct_Ticket, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
        0, // not relevat for ticket
        clyApp_FALSE);//Yoni 14/6/10
      if( err!=e_ClyApp_Ok)
        return err;
    }
    else
      return e_ClyApp_WrongParamErr;


  //if load OK - delete recovery information
  if(err == e_ClyApp_Ok)
  {
    memset(&st_TktRecoveryDate,0,sizeof(st_TktRecoveryDate));
    st_Static_StateMachine.st_UserCallbacks.fp_b_StoreTktRecoveryDate(&st_TktRecoveryDate);

  }
}
#endif//if 0


unsigned short us_GetPredefineCode(const union_ClyApp_ContractRecord *p_union_ContractRecord);
//pre: last event and contracts were all read
///////////////////////////////////////////////////////////////////////////////////////////
//	GetValidSVContractsCountForGivenPredefine
//	Count how many stored value contracts with given predefine code
///////////////////////////////////////////////////////////////////////////////////////////
static int GetValidSVContractsCountForGivenPredefine(unsigned short ContractPredefineCode
																		,union_ClyApp_ContractRecord *pSVContract //[OUT]
																		,int *pIndex //[OUT]
																		)
{

  int i,ett,contract_prof;
  int NumberOfStoredValue =0;

  if(pIndex)
    *pIndex = -1;

  if(pSVContract)
    memset(pSVContract,0,sizeof(union_ClyApp_ContractRecord));


  if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card )
    {
    for(i=0;i<MAX_CONTRACT_COUT;i++)
        {

      const st_ClyApp_CardContractRecord* pCurrContract = &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[i+1];


      if(!isLegalContract(i+1))
                continue; //skip

      if(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] == e_CardPriorityErasable ||
      st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] == e_CardPriorityInvalid)
        continue;

            // Get ett of contract
            ett = i_GetEttType((const union_ClyApp_ContractRecord*)pCurrContract);
      if(IsStoredValue(ett))  // Select only stored value contracts
      {


        if(us_GetPredefineCode((const union_ClyApp_ContractRecord *)pCurrContract) == ContractPredefineCode)
        {


          // Check that it is attached to a valid profile
          contract_prof = pCurrContract->st_CardContractIssuingData.st_OptionalContractData.uc_ContractCustomerProfile;
           if(clyapp_bIsProfileValid(contract_prof)== TR_TRUE)
           {
            //trace("Event BCPL %d:%d is a stored value (Prof: %d)",i,stGlobalDataObject.EvntArr[stGlobalDataObject.EvntLastIndex].st_CardEventDataStruct.e_EventBestContractPriorityListArr[i],contract_prof);
            if(NumberOfStoredValue == 0)
            {
              if(pSVContract)
                memcpy(pSVContract, pCurrContract, sizeof(*pSVContract));
              if(pIndex)
                *pIndex = i;
            }

            NumberOfStoredValue++;
          }


       }

      }

    }
  }

  //trace("GetValidSVContractsCount() Got %d stored value contracts",NumberOfStoredValue);
  return NumberOfStoredValue;
}

//10/2011
//pre: card was read
eCalypsoErr e_ClyApp_CheckConditionsForStoredValue(unsigned short ContractPredefineCode,
																									 long lAmountToLoad, 
																									 unsigned long* pSVTotalAmount/*out*/, 
																									 long* pSVIndex/*out*/)
{

  union_ClyApp_ContractRecord SVContract;
  long SVCount;
  *pSVIndex=-1;
  *pSVTotalAmount = lAmountToLoad,


  SVCount = GetValidSVContractsCountForGivenPredefine(ContractPredefineCode, &SVContract,(int*)pSVIndex);//todo
//  if((SVCount < 0) || (SVCount > 1))
//    return e_ClyApp_MoreThanOneStoredValueAlready;

  if(SVCount == 1)
    *pSVTotalAmount += SVContract.st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue;

  // Check for overflow..
  if(*pSVTotalAmount > g_Params.lv_StoredValueCeiling)
    return e_ClyApp_StoredValueOverflow;

  return e_ClyApp_Ok;
}


#define CONTRACT_LIST_VERSION 0
static void v_SetContractListCodeInPosition(st_clyApp_ContractListStruct* pContractlistStruct/*in/out*/, int index, unsigned short code)
{
  if(index >= MAX_CONTRACT_COUT) //illegal index
    return;
  //if code > 0 then set code in array and bitmap, else close bitmap and set code = 0
  if(code == 0)
  {
    pContractlistStruct->us_Bitmap &= ~(1<<index);//clr
  }
  else
  {
    pContractlistStruct->us_Bitmap |= (1<<index);//set
  }

  pContractlistStruct->ContractListAuthorizationCodeArr[index] = code;
  pContractlistStruct->uc_Ver = CONTRACT_LIST_VERSION;

  //Authenticator is calculated in write

}


/*
eCalypsoErr HowManyWritesInSession()
just a temporary tester
{
  RESPONSE_OBJ*    Obj;
  eCalypsoErr err;
  static int max=7,i;
  union_ClyApp_ContractRecord temp_union_ContractRecord;
  e_clyCard_KeyType e_KeyType;
  St_clySam_KIF_And_KVC St_KIF_And_KVC;
  e_KeyType = (st_Static_StateMachine.e_SamType == e_ClyApp_SamCL)? e_clyCard_KeyCredit:e_clyCard_KeyDebit;

  v_Internal_GetKifVal(e_KeyType,&St_KIF_And_KVC);

  err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
    1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    (void*) &temp_union_ContractRecord); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  //   not exit because convert
  if( err!=e_ClyApp_Ok && read_convert_err==e_ClyApp_Ok)
    return err;

  err = ClyApp_Virtual_OpenSecureSession(  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
    e_KeyType,//[IN] Key Type to use for the session
    NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
  if( err!=e_ClyApp_Ok)
    return err;

  for(i=0;i<max;i++)
  {
    err = e_Internal_Write( e_ClyApp_Card,//[IN]type - card \ ticket
      i+1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      &temp_union_ContractRecord,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
      );
    if( err!=e_ClyApp_Ok)
      return err;

  }

  Obj =   pSt_ClySession_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
    st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
    clyCard_FALSE);//[IN] //1= the session will be immediately Ratified

  if( !IS_RES_OBJ_OK(Obj) )
  {
    //if session close fail - make sure that the reader will not receive deselect by RF shutdown
    v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
    return e_ClyApp_CardWriteErr;
  }


  return 1;
}
*/



#if ENABLE_DEBUG_LOAD

 //#define DEBUG_LOAD_BEFOR_REGULER_MODE 0
 //#define DEBUG_LOAD_BEFOR_CLOSE_SESSION 1
 //#define DEBUG_AFTER_CLOSE_SESSION 2
// #define DEBUG_BEFOR_OPEN_SESSION 3


St_ClyDebugSettin St_CurrSetting;
 int i_CountDebugOpAfterX=2;
 int i_CurrCount=0;
 int i_DebugValue=DEBUG_LOAD_BEFOR_REGULER_MODE;
#endif

static eCalypsoErr e_ClyApp_LoadContractToCard(union_ClyApp_ContractRecord *p_union_ContractRecord,//[IN]the contract record to load
                                              st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card
																							long SVRequestedAmount, //how much to add to stored value. 0 if not stored value
											                        unsigned short usWhiteListId,  //for contract list record
												                      TR_St_CancelData *pBinDataForCancel,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
                                              clyApp_BOOL *b_Result,
																							int * index,
																							st_clyApp_CardEventDataStruct *p_Save_FirstEvent	//[OUT]																				
																							)
{

  RESPONSE_OBJ*    Obj;
  clyApp_BYTE i,RecNum;
  st_Cly_DateAndTime st_DateAndTime;
  e_ClyApp_TariffCounterType e_TariffCounterType;
  st_clyApp_CardEventDataStruct st_CardEventDataStruct;
  st_clyApp_CardEventDataStruct ov_EmptySpecialEvent;
  CalypsoFileType temp_CounterData;
  union_ClyApp_ContractRecord temp_union_ContractRecord;
  clyApp_BOOL bCounterSaved;
  int iSpecialEventRecNum;
	eCalypsoErr OutConverError;
  e_clyCard_KeyType e_KeyType;
  St_clySam_KIF_And_KVC St_KIF_And_KVC;
  //char  c_ExistingCntract;//index of existing stored value contract on card
  eCalypsoErr err;
//  long SVCount,SVIndex = -1,SVTotalAmount = 0;
  unsigned long SVTotalAmount=0;
  long SVIndex=-1;
//  union_ClyApp_ContractRecord SVContract;



  unsigned char bIsStoredValue = IsStoredValue(i_GetEttType(p_union_ContractRecord));

  CHECK_INERFACE_INIT()

  CHECK_CARD_EXIST()

  *b_Result = clyApp_FALSE;

  st_DateAndTime = st_GetCurrentDateAndTime();

  bCounterSaved =clyApp_FALSE;
	
	if(p_Save_FirstEvent)
		memset(p_Save_FirstEvent,0,sizeof(*p_Save_FirstEvent));


  if(bIsStoredValue)
  {
		unsigned short usPredefineCode = us_GetPredefineCode(p_union_ContractRecord);
    err = e_ClyApp_CheckConditionsForStoredValue(usPredefineCode, SVRequestedAmount, &SVTotalAmount, &SVIndex);//todo
    if(err != e_ClyApp_Ok)
    {
      return err;
    }
  }

  //======================================================================
  //check that Rec Found For Load - using e_ClyApp_IsFreeRecExist
  //======================================================================
  if(!st_Static_StateMachine.b_FreeRecFoundForLoad)
    return e_ClyApp_NoContractSelectedErr;

  CHECK_SESSION_OPEN()

  //===============================================================================================================================================================
  //read two last history files ( since they need to be send back to the user in the end of the transaction - third record will be written by the use operation )
  //===============================================================================================================================================================

  ////////////////////////
  // Read event Record 1
  ///////////////////////
  err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
  (clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
  e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
  (void*) &st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  if( err!=e_ClyApp_Ok)
    //if record can not be read - exist
    return err;
	
	// Save for future comparison To understand whether the  contract
   if(p_Save_FirstEvent)
		*p_Save_FirstEvent=st_CardEventDataStruct;
  ////////////////////////
  // Read event Record 2
  ////////////////////////
  err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
    (clyCard_BYTE)2,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    (void*) &st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  if( err!=e_ClyApp_Ok)
    //if record can not be read - exist
    return err;



  //==============================
  // Find Free contract Record
  //==============================
  if(bIsStoredValue)
  {
    // Update Tarrif
    p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType  =   e_ClyApp_CounterAsMonetaryAmount;
    p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType    = e_ClyApp_StoredValue;

  }

  //if(RecNum<1)//if not a stored value contract or a stored value that we need to load in a new place -find a free record
  //{

  //find free space
  RecNum=0;
  for(i=0;i<MAX_CONTRACT_COUT;i++)
  {
    if(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[i] == e_CardPriorityErasable)
    {
      RecNum = i+1;
      break; // change only one and exit
    }
  }
  //}

  //check if no contract found - return error
  if(RecNum <= 0)
    return e_ClyApp_UnknownErr;

  //==================
  // load contract
  //==================
  ///////////////////   1020 problem : close sess. fail

  err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
    RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    (void*) &temp_union_ContractRecord, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  //   not exit because convert
  if( err!=e_ClyApp_Ok && OutConverError==e_ClyApp_Ok)
    return err;




  err =e_Internal_Read(  e_ClyApp_Card,//[IN]type - card \ ticket
    RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    (void*) &temp_union_ContractRecord, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  //   not exit because convert
  if( err!=e_ClyApp_Ok && OutConverError==e_ClyApp_Ok)
    return err;

  ///////////////////////////////////////////////////////////

  //=========================================
  //open session
  //=========================================

    if(st_Static_StateMachine.CardWriteMode == e_NormalMode) // Note: check for session status only when actually writing (bypass in virtual mode)
    {
        if(st_Static_StateMachine.e_SamType == e_ClyApp_SamCL || st_Static_StateMachine.e_SamType == e_ClyApp_SamCP)
      {
        e_KeyType = e_clyCard_KeyCredit;
      }
      else
      {
        return e_ClyApp_NotOk;
      }
  }
    v_Internal_GetKifVal(e_KeyType,&St_KIF_And_KVC);

// for debug
#if ENABLE_DEBUG_LOAD
	i_CurrCount++;
	if(i_DebugValue==DEBUG_BEFOR_OPEN_SESSION && i_CurrCount<=i_CountDebugOpAfterX)
		return e_ClyApp_NotOk;

#endif

  err = ClyApp_Virtual_OpenSecureSession(  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
    e_KeyType,//[IN] Key Type to use for the session
    NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
  if( err!=e_ClyApp_Ok)
    return err;

  // Case this is a stored value and have to remove prev contract [Eitanm: 6/6/2011]
  if(bIsStoredValue && (SVTotalAmount > 0)) // Set amount
    p_union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue = SVTotalAmount;


  //update contract version
  p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractVersionNumber = CONTRACT_VERSION_NUM;

  //==========================================
  //Fill contract and event data missing data
  //==========================================
  //sale date
  p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractSaleDate = st_DateAndTime.st_Date;

  //update contract provider
  p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractProvider = g_Params.uc_ProviderId;

  //check is the contract has a valid counter type
  if( !b_Internal_IsValidCounterType(p_union_ContractRecord ) )
    return e_ClyApp_WrongParamErr;



  //=================================================================================================
  //store data for cancel - contract data before load is not relevat - set to 0 - only the event state ( counter is update later when read )
  //=================================================================================================
  memset(pBinDataForCancel->st_CardCancelData.ucp_ContractDataBeforeAction,0,REC_SIZE );

  // If SV, update contract data  (for cancel of stored value contract)
  //but only if this is not the first loading on this record number
  if(p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType==e_ClyApp_StoredValue
    && (SVIndex >=0))
  {
    //read contract
    err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
      (char)(SVIndex+1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &temp_union_ContractRecord, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts




    err =e_Internal_Read(  e_ClyApp_Card,//[IN]type - card \ ticket
      (char)(SVIndex+1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &temp_union_ContractRecord, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts


    //======================================================
    //if the contract is related to a counter - read it too
    //======================================================
    //convert to byte stream in order to save for cancel
    err = e_Internal_Bit2Byte_Convert(  e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
      e_ClyApp_Card,//[IN]type - card \ ticket
      e_clyCard_ContractsFile,//[IN] if not a ticket  - which record in the card
      pBinDataForCancel->st_CardCancelData.ucp_ContractDataBeforeAction,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      (void*) &temp_union_ContractRecord);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream



    err = e_Internal_Bit2Byte_Convert(  e_St2BitStream,//[IN] convert direction - bit stream to struct OR struct to bit stream
      e_ClyApp_Card,//[IN]type - card \ ticket
      e_clyCard_CountersFile,//[IN] if not a ticket  - which record in the card
      temp_CounterData,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      (void*) &temp_union_ContractRecord);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream




    memcpy(pBinDataForCancel->st_CardCancelData.ucp_CounterDataBeforeAction
      ,&temp_CounterData,
      REC_SIZE);

    bCounterSaved=clyApp_TRUE;//to prevent updatding of pBinDataForCancel->st_CardUseBinData.ucp_CounterData later

    //if this is the first loading on this record number- we'll get error in the convert
    //so in case of error don't return
  }

  memcpy(pBinDataForCancel->st_CardCancelData.ucp_Event1BeforeAction, &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[1],REC_SIZE);
  //st_CardCancelData.uc_ContractRecNumBefore for storec value will be changed later if needed
  pBinDataForCancel->st_CardCancelData.uc_ContractRecNumBefore = pBinDataForCancel->st_CardCancelData.uc_ContractRecNumAfter = RecNum;
  pBinDataForCancel->st_CardCancelData.Action=BIN_DATA_OF_LOAD;

  //==============================================================
  // event update - moved some fields here, for stored value cancel 
  //			   to prevent code repetition
  // 24/7/13
  //==============================================================
  //Event Data Time First Stamp = 0
  memset(&stp_CardEventDataStruct->st_EventDataTimeFirstStamp,0,sizeof(stp_CardEventDataStruct->st_EventDataTimeFirstStamp));
  //Event Circumstances  = Contract Loading
  //stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances = e_CardEventCircumContractLoading;
  //set event 
  stp_CardEventDataStruct->st_OptionalEventData.b_IsEventInterchangeRightsExist = clyApp_FALSE;
  stp_CardEventDataStruct->st_OptionalEventData.uc_EventInterchangeRights = 0;
  stp_CardEventDataStruct->st_OptionalEventData.b_IsEventRFU1Exist = clyApp_FALSE;
  stp_CardEventDataStruct->st_OptionalEventData.uc_EventRFU1 = 0;

  //Yoni 24/7/13  
  if(bIsStoredValue) 
  {
	  //if needed, write cancel event before all other changes. after which we'll save the card's snapshot (binary)
	  if(SVIndex!=-1) //if erase previos contract needed 
	  {
		  v_Internal_FillEventData( stp_CardEventDataStruct,//[OUT] event to fill
				RecNum);//[IN]Event Contract Pointer
		  //  the cancel event is very similar  to load event  
		  st_CardEventDataStruct=*stp_CardEventDataStruct;
		  // operation is cancel 
		  st_CardEventDataStruct.st_EventCode.e_CardEventCircumstances = e_CardEventCircumCancellation;
		  // the pointer is the previos contract pointer
		  st_CardEventDataStruct.uc_EventContractPointer =(clyApp_BYTE)SVIndex+1;
		  // the new bcpl for the new contract  is not enable yet
		  st_CardEventDataStruct.e_EventBestContractPriorityListArr[SVIndex]=e_CardPriorityErasable;
		  // close all ticket information 
		  st_CardEventDataStruct.st_OptionalEventData.b_IsEventTicketExist=clyApp_FALSE;

		  // write cancel event 
		  err = e_Internal_Write(  e_ClyApp_Card,//[IN]type - card \ ticket
			  1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
			  e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
			  &st_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
			  );

		  //save card's snapshot	
		  st_Static_StateMachine.st_CanceledStoredValueSnapshot.uc_CancelledContractRecNum = (clyApp_BYTE)SVIndex + 1;
		  //copy environment record
		  memcpy(st_Static_StateMachine.st_CanceledStoredValueSnapshot.ov_CardSnapshotAfterStoredValueCancel.ucp_EnvironmentData,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EnvRec,REC_SIZE);
		  //copy contract record
		  memcpy(st_Static_StateMachine.st_CanceledStoredValueSnapshot.ov_CardSnapshotAfterStoredValueCancel.ucp_ContractData,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractRecArr[SVIndex+1],REC_SIZE);
		  //copy event 1
		  memcpy(st_Static_StateMachine.st_CanceledStoredValueSnapshot.ov_CardSnapshotAfterStoredValueCancel.ucp_Event1,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[0],REC_SIZE);
		  //copy event 2
		  memcpy(st_Static_StateMachine.st_CanceledStoredValueSnapshot.ov_CardSnapshotAfterStoredValueCancel.ucp_Event2,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[1],REC_SIZE);
		  //copy event 3
		  memcpy(st_Static_StateMachine.st_CanceledStoredValueSnapshot.ov_CardSnapshotAfterStoredValueCancel.ucp_Event3,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[2],REC_SIZE);
		  //and counter
		  memcpy(st_Static_StateMachine.st_CanceledStoredValueSnapshot.ov_CardSnapshotAfterStoredValueCancel.ucp_Counter, st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec,REC_SIZE);
		  //and contractlist
		  memcpy(st_Static_StateMachine.st_CanceledStoredValueSnapshot.ov_CardSnapshotAfterStoredValueCancel.ucp_ContractList, st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractList,REC_SIZE);

		  //we need to update the state machine
		  st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[1] = clyApp_FALSE;
		  st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[2] = clyApp_FALSE;
		  st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[3] = clyApp_FALSE;

		  err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
			  (clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
			  e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
			  (void*) &st_CardEventDataStruct, &OutConverError,1); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
		  if( err!=e_ClyApp_Ok)
			  return err;		 
		  err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
			  (clyCard_BYTE)2,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
			  e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
			  (void*) &st_CardEventDataStruct, &OutConverError,1); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
		  if( err!=e_ClyApp_Ok)
			  return err;		  
		  err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
			  (clyCard_BYTE)3,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
			  e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
			  (void*) &st_CardEventDataStruct, &OutConverError,1); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
		  if( err!=e_ClyApp_Ok)
			  return err;
	  }
  } 



  //WRITE TO CARD
  err = e_Internal_Write( e_ClyApp_Card,//[IN]type - card \ ticket
    RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    p_union_ContractRecord,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
    );//Yoni 14/6/10 Always put tw sign in load
  if( err!=e_ClyApp_Ok)
    return err;


  //============================================
  //Counter update - only if counter is in use
  //============================================
  //get counter type from contract tariff
  e_TariffCounterType = p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType;

  if(e_TariffCounterType != e_ClyApp_CounterNotUsed )
  {
    //copy the original type of the counter from p_union_ContractRecord to the temporary Global_union_ContractRecord for the convertor use
    memcpy(&Global_union_ContractRecord,p_union_ContractRecord,sizeof(Global_union_ContractRecord));
    if (e_TariffCounterType == e_ClyApp_CounterAsDateAndRemainingNumOfJourneys)
    {
      //clear the date
      memset(&p_union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.st_CounterDate,0,sizeof(p_union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.st_CounterDate));
      //if b_ContractPeriodJourneysExist  - set value to max value in period
      if(p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist)
        p_union_ContractRecord->st_CardContractRecord.st_CardCounterRecord.union_CardCounterRecord.st_CardCounter_DateAndRemainingJourneys.CounterValue =p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.uc_MaxNumOfTripsInPeriod;
    }


    //READ CURRENT COUNTERS VALUES - becase the update operation rewrite the existing counters value
    err =e_Internal_Read(  e_ClyApp_Card,//[IN]type - card \ ticket
      RecNum,  //[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &Global_union_ContractRecord, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
      return err;

    //store counter file state before update
    if(bCounterSaved==clyApp_FALSE)
      memcpy(pBinDataForCancel->st_CardCancelData.ucp_CounterDataBeforeAction,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec,REC_SIZE);

    //WRITE TO CARD
    err = e_Internal_Write( e_ClyApp_Card,//[IN]type - card \ ticket
      RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      p_union_ContractRecord,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
      );
    if( err!=e_ClyApp_Ok)
      return err;


  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //check if the contract + counter loaded are legal - check after WRITE  and before close session so that contart + counter will be present in memory
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  err =  e_Internal_BasicCalypsoCheckIfRecValid(e_ClyApp_Card,e_clyCard_ContractsFile,RecNum,(void *)p_union_ContractRecord);
  //load contract even if profile end date is less then end contract date
  if( err!=e_ClyApp_Ok)
    return err;

  //==================
  //event update
  //==================
  /////////////////////
  //bestPriorityList upadte
  //////////////////////
  ////update priority level of the new contract
  st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[RecNum-1] = e_Internal_CalcContractPriority(p_union_ContractRecord);  
  // fill event: VersionNumber,Service Provider,Event Contract Pointer,Event Date and Time,priority list
  v_Internal_FillEventData( stp_CardEventDataStruct,//[OUT] event to fill
    RecNum);//[IN]Event Contract Pointer
  ////Event Circumstances  = Contract Loading
  //stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances = e_CardEventCircumContractLoading; moved up
  // Case this is a stored value and have to remove prev contract [Eitanm: 6/6/2011]
  if(bIsStoredValue && (SVIndex != -1)) // Remove prev contract
  {
    stp_CardEventDataStruct->e_EventBestContractPriorityListArr[SVIndex] = e_CardPriorityErasable;
    pBinDataForCancel->st_CardCancelData.uc_ContractRecNumBefore = (unsigned char)(SVIndex+1);//Yoni 07/2011
  }

  //moved  up
  ////Event Data Time First Stamp = 0  
  //memset(&stp_CardEventDataStruct->st_EventDataTimeFirstStamp,0,sizeof(stp_CardEventDataStruct->st_EventDataTimeFirstStamp));
  //moved up
  //if(bIsStoredValue)
  //{
  //
	 // stp_CardEventDataStruct->st_OptionalEventData.b_IsEventInterchangeRightsExist = clyApp_FALSE;
	 // stp_CardEventDataStruct->st_OptionalEventData.uc_EventInterchangeRights = 0;
	 // stp_CardEventDataStruct->st_OptionalEventData.b_IsEventRFU1Exist = clyApp_FALSE;
	 // stp_CardEventDataStruct->st_OptionalEventData.uc_EventRFU1 = 0;
  //} 

  //WRITE TO CARD
  err = e_Internal_Write(  e_ClyApp_Card,//[IN]type - card \ ticket
    1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    stp_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
    );
  if( err!=e_ClyApp_Ok)
    return err;



  // in case of stored value contract only, we dont delete the special event associated with the old contract,
  // but we change the contract pointer to the point to the new stored value contract
  if(bIsStoredValue && SVIndex != -1)
  {
    st_clyApp_CardEventDataStruct* pSpecialEvent;
    iSpecialEventRecNum = i_Internal_FindSpecialEvent(SVIndex+1); //SVIndex+1 = recnum of old contract
    if(iSpecialEventRecNum >=1)
    {
      //we have a special event associated with old contract
      //get current special event
      pSpecialEvent= &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[iSpecialEventRecNum];
      //change contract pointer
      pSpecialEvent->uc_EventContractPointer = RecNum;//pointer to new contract
      //write special event again
      err = e_Internal_Write(e_ClyApp_Card,//[IN]
        (char)(iSpecialEventRecNum),//[IN]
        e_clyCard_SpecialEventFile, //[IN]
        pSpecialEvent
        ,(char)REC_SIZE
        );
      if( err!=e_ClyApp_Ok)
        return err;

    }
  }
  else
  {
    //if not stored value: if there's a special event associated with this contract,
    //then we need to delete the special event because it belonged
    //to the prev contract and not to the new
    iSpecialEventRecNum=i_Internal_FindSpecialEvent(RecNum);
    if(iSpecialEventRecNum>=1)
    {
      memset(&ov_EmptySpecialEvent,0, sizeof(ov_EmptySpecialEvent));
      err = e_Internal_Write(e_ClyApp_Card,//[IN]
        (char)(iSpecialEventRecNum),//[IN]
        e_clyCard_SpecialEventFile, //[IN]
        &ov_EmptySpecialEvent
        ,(char)REC_SIZE
        );
      if( err!=e_ClyApp_Ok)
        return err;

    }
  }



  //CONTRACT LIST (WHITE LIST) 10/2011
  //if usWhiteListId==0 and current record is also 0 then no change
  //else we have to update the contract list
  //1) read current record
  //2) if whitelist code for new contract > 0 then we need to update
  //3) if whitelist code is 0 but in existing bitmap some code exists, we need to clear it
  {

    clyApp_BOOL bUpdateWhiteList=clyApp_FALSE;
    st_clyApp_ContractListStruct ov_Contractlist={0};
    //read existing
    err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
      1,  //[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_ContractListFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &ov_Contractlist, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err == e_ClyApp_CardReadErr)
    {
      return err;//only if io error
    }

    if(usWhiteListId == 0)
    {
      //if bit is on for this RecNum then we need to clear this code from list
      //if(ov_Contractlist.us_Bitmap & (1<<(RecNum-1)))
      if(IS_CONTRACT_LIST_ON(ov_Contractlist.us_Bitmap, RecNum))
      {

        bUpdateWhiteList=clyApp_TRUE;
      }
    }
    else
    {
      bUpdateWhiteList=clyApp_TRUE;
    }
    //update record only if necessary
    if(bUpdateWhiteList == clyApp_TRUE)
    {
      //set code in the relevant place in list
      v_SetContractListCodeInPosition(&ov_Contractlist, RecNum-1, usWhiteListId);
      //write
      err = e_Internal_Write(e_ClyApp_Card,//[IN]
        1,//[IN]
        e_clyCard_ContractListFile, //[IN]
        &ov_Contractlist
        ,(char)REC_SIZE
        );
      if( err!=e_ClyApp_Ok)
        return err;
    }


  }




  //==================
  // Close Session
  //==================

  // for debug
#if ENABLE_DEBUG_LOAD
	
	if(i_DebugValue==DEBUG_LOAD_BEFOR_CLOSE_SESSION && i_CurrCount<=i_CountDebugOpAfterX)
		return e_ClyApp_NotOk;
#endif

	
	
  Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
    st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
    clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified

#if ENABLE_DEBUG_LOAD
	
	if(i_DebugValue==DEBUG_AFTER_CLOSE_SESSION && i_CurrCount<=i_CountDebugOpAfterX)
		goto NEXT;
#endif

  if( !IS_RES_OBJ_OK(Obj) )
  {
#if ENABLE_DEBUG_LOAD		
NEXT:		
#endif		
    //if session close fail - make sure that the reader will not receive deselect by RF shutdown
     b_ClyCard_EjectCard(st_Static_StateMachine.CardReaderId);
  
   (*index)=RecNum-1;
	  st_Static_StateMachine.e_TransactionState = e_clyApp_SessionCloseOk;
    st_Static_StateMachine.c_ContractRecNumberForUse = RecNum;

    return e_ClyApp_FailedOnCloseSession;
  }

  //==================
  // Update state machine
  //==================
  st_Static_StateMachine.e_TransactionState = e_clyApp_SessionCloseOk;
  st_Static_StateMachine.c_ContractRecNumberForUse = RecNum;
  (*index)=RecNum-1;


  return e_ClyApp_Ok;
}


//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_LoadContract
//
//DESCRITION:
//
//                Load Contract
//                For a card fill the contract, for a ticket fill only the contract related information ( common data + contract data )
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//            �   Check if contract loading is passable
//            �   Fill contract and event data missing data
//            �   update best priority list in event
//            �   write event
//            �   close session
//            �   store bin files bin data
//            �   can not load card with un exist profile.
//            �   The profile must exist in ENV
//            �   Profile must have valid date
//
////////////////////////////////////////////////////////////////////////////////////////////////////


eCalypsoErr CLYAPP_STDCALL e_ClyApp_LoadContract(
												 union_ClyApp_ContractRecord *p_union_ContractRecord,//[IN]the contract record to load
												 st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card
												 long SVRequestedAmount, //how much to add to stored value
												 unsigned short usWhiteListId,  //for contract list record
												 TR_St_CancelData *pBinDataForCancel,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
												 union_ClyApp_TransactionBinData *p_union_TransactionBinData, //[OUT]
                                                 clyApp_BOOL *b_Result,
												 int * index
												 )
{
	
	
	
	eCalypsoErr err;
	int i;
	st_clyApp_CardEventDataStruct Save_FirstEvent;
	
	
	//==============================
	//check card type  - if card
	//==============================
	if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card )
	{
#if ENABLE_DEBUG_LOAD
		//i_CurrCount=0;
#endif
		for(i=0;i<MAX_LOAD_RETRIES  ;i++)
		{
			err = e_ClyApp_LoadContractToCard(p_union_ContractRecord,//[IN]the contract record to load
				stp_CardEventDataStruct,//[IN] parameter relevant only for card
				SVRequestedAmount, //how much to add to stored value
				usWhiteListId, //for contract list record
				pBinDataForCancel,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
				b_Result,
				index,&Save_FirstEvent
				);

			if(err==e_ClyApp_Ok || err==e_ClyApp_NoContractSelectedErr)
				break;
		
			// case of session fail test if data writen 
			else
			{
				
				
				int Save_c_ContractRecNumberForUse=st_Static_StateMachine.c_ContractRecNumberForUse;
				v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
				//Eject();
				//	unsigned char SaveFirstEvent[29];
				// Keep data that may be  erase  by function e_ClyApp_WaitCard
				
				
				
				// select card again 
				err=e_ClyApp_WaitCard(6,NULL);
				// Checks if the data is written 
				if(err==e_ClyApp_Ok)
				{
					err=e_ClyApp_VerifyLoading(&Save_FirstEvent);
					if(err==e_ClyApp_Ok)
					{
						st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[0]=(clyApp_BOOL)1;
						st_Static_StateMachine.c_ContractRecNumberForUse=Save_c_ContractRecNumberForUse;
						e_ClyApp_ShiftTransactionEvents();
						break;// verify ok repit not needed 
					}
				}
				else
				{
					break;// read card fail -> loading again forbidden 
				}
				
				if(err!=e_ClyApp_Ok)
				{
//#if ENABLE_DEBUG_LOAD
  //    i_DebugValue=0;
//#endif
					st_Static_StateMachine.b_FreeRecFoundForLoad=(clyApp_BOOL)1;
				
					continue;
					//	return err;
				}
				else
				{
				//	 e_ClyApp_ShiftTransactionEvents();
					// restore  data that may be  erase  by function e_ClyApp_WaitCard
					//st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[0]= (clyApp_BOOL)Save_IsEventRecExistArr;
					//st_Static_StateMachine.c_ContractRecNumberForUse=Save_c_ContractRecNumberForUse;
					//	  memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[0],
					//		   SaveFirstEvent,29);
					
					
					
					
				}
				
				
			}
		}
		
		if(err!=e_ClyApp_Ok)
		{
#if ENABLE_DEBUG_LOAD
		  i_CurrCount=0;
#endif
			st_Static_StateMachine.b_FreeRecFoundForLoad=(clyApp_BOOL)0;
			return err;
		}
				 
		
			
			
			
			/*
			char Contract[29];
			char Env[29];
			char Event1[29];
			char Event2[29];
			char Event3[29];
			char Counter[29];
			*/
		}
		
		else //ticket
		{
			
			return e_ClyApp_NotOk;//currently not supported
		}
		
		*b_Result = clyApp_TRUE;
		st_Static_StateMachine.e_TransactionState = e_clyApp_SessionCloseOk;
		
		//fill TR_St_TransactionData
		e_ClyApp_GetTransactionBinaryData(p_union_TransactionBinData);//[OUT] get the Transaction Binary Data
		
#if ENABLE_DEBUG_LOAD
		  i_CurrCount=0;
#endif
		
		st_Static_StateMachine.b_FreeRecFoundForLoad=(clyApp_BOOL)0;
		return e_ClyApp_Ok; // OK
		

		}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Yoni 07/2011 Seperate card and ticket for code clarity
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static eCalypsoErr e_ClyApp_CancelTicketOperation(const unsigned char ucp_BinTktDataBeforUse[32],//[IN]
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card. defines the parameters of the cancle operation.
  clyApp_BOOL *b_Result)//[OUT]1=Cancel OK ,0=Cancel fail )
{
  return e_ClyApp_NotOk;
#if 0  //todo put back
  eCalypsoErr err;
  st_ClyApp_TktRecoveryDate st_TktRecoveryDate;
  e_ContacLessErr TrErr;
  e_ClyTkt_ERR  TktErr;
  CalypsoBinTktType ucp_BinBuffOut;
  ///e_AppOrLoader e_AorL;

  //=========================================
  //read all ticket - internal_read
  //=========================================
  err = e_Internal_Read(e_ClyApp_Ticket,//[IN]type - card \ ticket
    0,//[IN] //not relevat for ticket
    e_clyCard_TicketingDF, //[IN]not relevat for ticket
    &Global_union_ContractRecord); //[OUT] data read
  if( err!=e_ClyApp_Ok )
    return err;

  //force application to read the ticket after it has been recoverd
  st_Static_StateMachine.st_TransactionData.b_IsTktDataExist = clyApp_FALSE;


  //============================================================
  //store current ticket data befor write - for recovery purpose
  //============================================================
  memcpy(st_TktRecoveryDate.upc_FullBinTktData,st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.ucp_TktBinData,sizeof(CalypsoBinTktType));
  memcpy(st_TktRecoveryDate.SerialNum,st_Static_StateMachine.ucp_CardSn,sizeof(SN8));
  st_Static_StateMachine.st_UserCallbacks.fp_b_StoreTktRecoveryDate(&st_TktRecoveryDate);

  //check if legal data before write
  if( !b_Internal_IsManufacturerModeTkt(pBinDataForCancel->ucp_BinTktDataBeforUse) )
  {
    // convert bin Ticket data to Struct
    TktErr =   e_ClyTkt_ConvertBinBuff2TktSt( &Global_union_ContractRecord.struct_Ticket,//[OUT] Ticket struct output
      pBinDataForCancel->ucp_BinTktDataBeforUse,//[IN] Binary buff to translate
      ucp_BinBuffOut);
    if( TktErr  )
      return e_ClyApp_WrongParamErr;
  }

  //============================================
  //WRITE ALL TICKET CANCEL DATA
  //============================================
  TrErr = e_CticketWrite(
    ///&e_AorL,//indicates the location of the reader
    1, // writing verification: 0-disabled; otherwise - enabled;
    5,//the number of block to write into ( 0 - 15 )
    22 ,//the size of block (in bytes)
    pBinDataForCancel->ucp_BinTktDataBeforUse+10);//ptr to data to writen


  if( TrErr)
    return e_ClyApp_CardWriteErr;

  *b_Result = clyApp_TRUE;
  //========================
  // Update state machine
  //========================
  st_Static_StateMachine.e_TransactionState = e_clyApp_SessionCloseOk;

  return e_ClyApp_Ok;
#endif
}



static eCalypsoErr  e_ClyApp_CancelCardLoad(const st_ClyApp_CardCancelData* st_CardCancelData,//[IN]
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card. defines the parameters of the cancle operation.
  clyApp_BOOL *b_Result)
{
  //in cancel of load - in cancel of stored value reload (a previous contract existed) we need to delete the new one and set back a valid bcpl for old one
  //          - in all other cases we just set invalid bcpl for new contract
  // if a special event is connected to new contract, change it to point to old contract
  eCalypsoErr err;
	eCalypsoErr OutConverError;
  RESPONSE_OBJ* Obj;
  st_clyApp_CardEventDataStruct* pSpecialEvent;
  int iSpecialEventRecNum;
  e_clyCard_KeyType e_KeyType;
  St_clySam_KIF_And_KVC St_KIF_And_KVC;
  union_ClyApp_EventRecord union_EventRecordBeforeAction;
  unsigned char usOldContractNumber = st_CardCancelData->uc_ContractRecNumBefore;
  unsigned char usNewContractNumber = st_CardCancelData->uc_ContractRecNumAfter;
  st_clyApp_ContractListStruct ov_Contractlist={0};

  if(usOldContractNumber < 1 || usOldContractNumber > 8 || usNewContractNumber < 1 || usNewContractNumber>8)
    return e_ClyApp_NotOk;

  //convert event before action
  err = e_Internal_Bit2Byte_Convert(  e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
    e_ClyApp_Card,//[IN]type - card \ ticket
    e_clyCard_EventLogFile,//[IN] if not a ticket  - which record in the card
    (unsigned char*)st_CardCancelData->ucp_Event1BeforeAction,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
    (void*) &union_EventRecordBeforeAction.st_CardEventDataStruct);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
  if( err!=e_ClyApp_Ok)
    return err;

  //convert contract before action
  //could be all 0 so we dont check result. data is releveant only for SV
  e_Internal_Bit2Byte_Convert(  e_BitStream2St,
    e_ClyApp_Card,
    e_clyCard_ContractsFile,
    (unsigned char*)st_CardCancelData->ucp_ContractDataBeforeAction,
    (void*)&Global_union_ContractRecord);

  //////////////////////////////////////////////
  //READ EXISTING CONTRACT LIST
  err =e_Internal_Read( e_ClyApp_Card,//[IN]
    1,  //[IN]
    e_clyCard_ContractListFile,
    (void*) &ov_Contractlist, &OutConverError,0); //[OUT]
  if( err == e_ClyApp_CardReadErr)
  {
    return err;//only if io error
  }
  //////////////////////////////////////////////

  //=========================================
  //open session
  //=========================================
  e_KeyType = (st_Static_StateMachine.e_SamType == e_ClyApp_SamCL)? e_clyCard_KeyCredit:e_clyCard_KeyDebit;
  v_Internal_GetKifVal(e_KeyType,&St_KIF_And_KVC);

  err = ClyApp_Virtual_OpenSecureSession( St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
    e_KeyType,//[IN] Key Type to use for the session
    NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
  if( err!=e_ClyApp_Ok)
    return err;


  //so now decide if this is cancel of sv reload, according to contract tariff and contracts rec numbers before and after
  if( Global_union_ContractRecord.st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType == e_ClyApp_CounterAsMonetaryAmount
    && usOldContractNumber != usNewContractNumber)
  {
    //set counter as 'not updated'
    st_Static_StateMachine.st_TransactionData.b_IsCounterRecExistArr[union_EventRecordBeforeAction.st_CardEventDataStruct.uc_EventContractPointer]=clyApp_FALSE;

    //set the reloaded contract as invalid
    st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[usNewContractNumber -1] = e_CardPriorityErasable;
    //set the old contract as valid again (get the priority from event before loading)
    st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[usOldContractNumber -1] = union_EventRecordBeforeAction.st_CardEventDataStruct.e_EventBestContractPriorityListArr[usOldContractNumber -1];

    //update state machine with the contract data - force reading from the card - to be available for e_ClyApp_GetTransactionBinaryData command
    err = e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
      usNewContractNumber,//[IN] //not relevat for ticket - record number to read : 1 is always the first record
      e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &Global_union_ContractRecord, &OutConverError,0);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
      return err;

    //======================================================
    //if the contract is related to a counter - read it too and update
    //======================================================
    if( Global_union_ContractRecord.st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType != e_ClyApp_CounterNotUsed )
    {
      //restore counter value
      memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec + COUNTER_OFFSET(usOldContractNumber), st_CardCancelData->ucp_CounterDataBeforeAction ,COUTER_SIZE);
      Obj =  ClyApp_Virtual_UpdateRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
        1,//[IN] //for counter rec allways write record 1
        e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        REC_SIZE,//[IN] len to read - 1 to record size
        st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
      if( !IS_RES_OBJ_OK(Obj) )
        return e_ClyApp_CardWriteErr;
    }

    //if a special event is connected to new contract, change it to point to old contract
    iSpecialEventRecNum = i_Internal_FindSpecialEvent(usNewContractNumber); // recnum of old contract
    if(iSpecialEventRecNum>=1)
    {
      //we have a special event associated with old contract
      //get current special event
      pSpecialEvent= &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[iSpecialEventRecNum];
      //change contract pointer
      pSpecialEvent->uc_EventContractPointer = usOldContractNumber;//pointer to new contract
      //write special event again
      err = e_Internal_Write(e_ClyApp_Card,//[IN]
        (char)(iSpecialEventRecNum),//[IN]
        e_clyCard_SpecialEventFile, //[IN]
        pSpecialEvent
        ,(char)REC_SIZE
        );
      if( err!=e_ClyApp_Ok)
        return err;
    }

  }
  else //normal cancel of load - simple
  {
    //Only mark record as un exist in the priority list
    st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[usOldContractNumber -1] = e_CardPriorityErasable;
    //no need to restore counter
  }

  //==================
  //event update
  //==================

  // fill event: VersionNumber,Service Provider,Event Contract Pointer,Event Date and Time,priority list
  v_Internal_FillEventData( stp_CardEventDataStruct,//[OUT] event to fill
    usNewContractNumber);//[IN]Event Contract Pointer


  //Event Circumstances  = Contract  Cancellation
  stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances = e_CardEventCircumCancellation;

  //WRITE TO CARD
  err = e_Internal_Write(  e_ClyApp_Card,//[IN]type - card \ ticket
    1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    stp_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
    );
  if( err!=e_ClyApp_Ok)
    return err;


  //CONTRACT LIST : CHECK IF WE NEED TO ERASE RECORD IN CONTRACT LIST (WHITE LIST)
  if(IS_CONTRACT_LIST_ON(ov_Contractlist.us_Bitmap, usNewContractNumber))
  {
    //set 0 in the relevant place in contract list
    v_SetContractListCodeInPosition(&ov_Contractlist, usNewContractNumber-1, 0);
    //write
    err = e_Internal_Write(e_ClyApp_Card,//[IN]
      1,//[IN]
      e_clyCard_ContractListFile, //[IN]
      &ov_Contractlist
      ,(char)REC_SIZE
      );
    if( err!=e_ClyApp_Ok)
      return err;

  }


  //==================
  // Close Session
  //==================
  Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
    st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
    clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified

  if( !IS_RES_OBJ_OK(Obj) )
  {
    //if session close fail - make sure that the reader will not receive deselect by RF shutdown
    v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
    return e_ClyApp_CardWriteErr;
  }
  *b_Result = clyApp_TRUE;
  //========================
  // Update state machine
  //========================
  st_Static_StateMachine.e_TransactionState = e_clyApp_SessionCloseOk;

  return e_ClyApp_Ok;
}




//Yoni 07/2011
static eCalypsoErr  e_ClyApp_CancelCardUse(const st_ClyApp_CardCancelData* st_CardCancelData,//[IN]
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card. defines the parameters of the cancle operation.
  clyApp_BOOL *b_Result,
  const st_clyApp_CardEventDataStruct* stp_LastCardEvent //event1 which is currently on card
  )
{
  int iSpecialEventRecNum;
  eCalypsoErr err;
  RESPONSE_OBJ* Obj;
  e_clyCard_KeyType e_KeyType;
  St_clySam_KIF_And_KVC St_KIF_And_KVC;
  unsigned char ucContractNum =  st_CardCancelData->uc_ContractRecNumBefore;
  //in cancel of use: restore counter, erase special event if required and write cancel event
  st_clyApp_CardEventDataStruct ov_EventBefore;
  unsigned char cpContractBuffer[29];


  if(ucContractNum < 1 || ucContractNum > 8)
    return e_ClyApp_NotOk;

  if(stp_LastCardEvent->b_EventIsJourneylnterchange) //can't cancel maavar (no need to)
    return e_ClyApp_NotOk;

  //convert event before action
  err = e_Internal_Bit2Byte_Convert(  e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
    e_ClyApp_Card,//[IN]type - card \ ticket
    e_clyCard_EventLogFile,//[IN] if not a ticket  - which record in the card
    (unsigned char*)st_CardCancelData->ucp_Event1BeforeAction,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
    (void*)&ov_EventBefore);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
  if( err!=e_ClyApp_Ok)
    return err;

  //convert counter before action


  //=========================================
  //open session
  //=========================================
  //e_KeyType = (st_Static_StateMachine.e_SamType == e_ClyApp_SamCL)? e_clyCard_KeyCredit:e_clyCard_KeyDebit;
  e_KeyType = e_clyCard_KeyCredit;
  v_Internal_GetKifVal(e_KeyType,&St_KIF_And_KVC);

  err = ClyApp_Virtual_OpenSecureSession( St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
    e_KeyType,//[IN] Key Type to use for the session
    NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
  if( err!=e_ClyApp_Ok)
    return err;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //1. restore counter and contract
  st_Static_StateMachine.st_TransactionData.b_IsCounterRecExistArr[ov_EventBefore.uc_EventContractPointer]=clyApp_FALSE;

  //get old contract before use
  err = e_Internal_Bit2Byte_Convert(  e_BitStream2St,
    e_ClyApp_Card,
    e_clyCard_ContractsFile,
    (unsigned char*)st_CardCancelData->ucp_ContractDataBeforeAction,
    (void*) &Global_union_ContractRecord);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
  if( err!=e_ClyApp_Ok)
    return err;

  //Restore the contract
  memcpy(cpContractBuffer, st_CardCancelData->ucp_ContractDataBeforeAction, sizeof(cpContractBuffer));
  Obj =  ClyApp_Virtual_UpdateRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
    ucContractNum,//[IN] //record number to read - 1 is always the first record
    e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    REC_SIZE,//[IN] len to read - 1 to record size
    cpContractBuffer); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  if( !IS_RES_OBJ_OK(Obj) )
    return e_ClyApp_CardWriteErr;

  //    //update state machine with the contract data - force reading from the card - to be available for e_ClyApp_GetTransactionBinaryData command
  //    //moved this up Yoni 11/2009:  st_Static_StateMachine.st_TransactionData.b_IsCounterRecExistArr[union_EventRecord.st_CardEventDataStruct.uc_EventContractPointer]=clyApp_FALSE;
  //    err = e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
  //                           ucContractNum,//[IN] //not relevat for ticket - record number to read : 1 is always the first record
  //                           e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
  //                           (void*) &Global_union_ContractRecord);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  //    if( err!=e_ClyApp_Ok)
  //        return err;

  //counter
  if(Global_union_ContractRecord.st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType != e_ClyApp_CounterNotUsed)
  {
    //restore counter value
    memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec + COUNTER_OFFSET(ucContractNum), st_CardCancelData->ucp_CounterDataBeforeAction ,COUTER_SIZE);
    Obj =  ClyApp_Virtual_UpdateRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
      1,//[IN] //for counter rec allways write record 1
      e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      REC_SIZE,//[IN] len to read - 1 to record size
      st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( !IS_RES_OBJ_OK(Obj) )
      return e_ClyApp_CardWriteErr;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //2. check if we need to erase special event
  iSpecialEventRecNum= i_Internal_FindSpecialEvent(ucContractNum); // recnum of old contract
  if(iSpecialEventRecNum>=1)
  {

    //we have a special event associated with contract

    //only if this special event was just updated
    if(memcmp(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[iSpecialEventRecNum], stp_LastCardEvent, sizeof(*stp_LastCardEvent)) == 0)
    {
      //event of action and special event are the same
      //delete it
      memset(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[iSpecialEventRecNum], 0, sizeof(st_clyApp_CardEventDataStruct));
      err = e_Internal_Write(e_ClyApp_Card,//[IN]
        (char)(iSpecialEventRecNum),//[IN]
        e_clyCard_SpecialEventFile, //[IN]
        &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[iSpecialEventRecNum]
      ,(char)REC_SIZE
        );
      if( err!=e_ClyApp_Ok)
        return err;
    }


  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //3. write regular event
  //restor periority
  st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[ucContractNum-1]=ov_EventBefore.e_EventBestContractPriorityListArr[ucContractNum-1];

  //Get EventRunId, EventTicket for last event (the event of the use op we are cancelling)
  //The assumption is that cancel use is only for last operation on card
  stp_CardEventDataStruct->st_OptionalEventData.b_IsEventTicketExist = stp_LastCardEvent->st_OptionalEventData.b_IsEventTicketExist;
  stp_CardEventDataStruct->st_OptionalEventData.st_EventTicket =  stp_LastCardEvent->st_OptionalEventData.st_EventTicket;
  stp_CardEventDataStruct->st_OptionalEventData.b_IsEventRunlDExist= stp_LastCardEvent->st_OptionalEventData.b_IsEventRunlDExist;
  stp_CardEventDataStruct->st_OptionalEventData.ush_EventRunlD= stp_LastCardEvent->st_OptionalEventData.ush_EventRunlD;
  stp_CardEventDataStruct->st_OptionalEventData.b_IsEventPassengersNumberExist= stp_LastCardEvent->st_OptionalEventData.b_IsEventPassengersNumberExist;
  stp_CardEventDataStruct->st_OptionalEventData.uc_EventPassengersNumber= stp_LastCardEvent->st_OptionalEventData.uc_EventPassengersNumber;
  // fill event: VersionNumber,Service Provider,Event Contract Pointer,Event Date and Time,priority list
  v_Internal_FillEventData( stp_CardEventDataStruct,//[OUT] event to fill
    ucContractNum);//[IN]Event Contract Pointer
  //Event Circumstances  = Contract  Cancellation
  stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances = e_CardEventCircumCancellation;

  //WRITE TO CARD
  err = e_Internal_Write(  e_ClyApp_Card,//[IN]type - card \ ticket
    1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    stp_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
    );//Yoni 14/6/10
  if( err!=e_ClyApp_Ok)
    return err;

  //==================
  // Close Session
  //==================
  Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
    st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
    clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified

  if( !IS_RES_OBJ_OK(Obj) )
  {
    //if session close fail - make sure that the reader will not receive deselect by RF shutdown
    v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
    return e_ClyApp_CardWriteErr;
  }
  *b_Result = clyApp_TRUE;
  //========================
  // Update state machine
  //========================
  st_Static_StateMachine.e_TransactionState = e_clyApp_SessionCloseOk;

  return e_ClyApp_Ok;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Yoni 07/2011 Seperate card and ticket for code clarity
//  Totally new impl
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static eCalypsoErr e_ClyApp_CancelCardOperation(const st_ClyApp_CardCancelData* st_CardCancelData,//[IN]
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card. defines the parameters of the cancle operation.
  clyApp_BOOL *b_Result)//[OUT]1=Cancel OK ,0=Cancel fail )
{

  eCalypsoErr err;
	eCalypsoErr OutConverError;

  union_ClyApp_EventRecord union_EventRecord;

  //st_clyApp_CardEventDataStruct ov_SaveLastEvent;

  CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()

    *b_Result = clyApp_FALSE;

  CHECK_SESSION_OPEN()


    //  //first determine whether we cancel use or loading according to ucp_Event1AfterAction
    //  //convert ucp_Event1AfterAction
    //    err = e_Internal_Bit2Byte_Convert(  e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
    //                                        e_ClyApp_Card,//[IN]type - card \ ticket
    //                                        e_clyCard_EventLogFile,//[IN] if not a ticket  - which record in the card
    //                                        (unsigned char*)st_CardCancelData->ucp_Event1AfterAction,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
    //                                        (void*)&union_EventRecord.st_CardEventDataStruct);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
    //    if( err!=e_ClyApp_Ok)
    //        return err;


    ///////////////////////////////////////////////////////////////////////////////////////
    // Read event Record 2 - to be available for e_ClyApp_GetTransactionBinaryData command
    // 2 before 1 because we pass event1 to canceluse
    ///////////////////////////////////////////////////////////////////////////////////////
    err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
    2,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    (void*) &union_EventRecord.st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  if( err!=e_ClyApp_Ok)
    //if record can not be read - exist
    return err;
  ///////////////////////////////////////////////////////////////////////////////////////
  // Read event Record 1 - to be available for e_ClyApp_GetTransactionBinaryData command
  ///////////////////////////////////////////////////////////////////////////////////////
  err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
    1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    (void*) &union_EventRecord.st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  if( err!=e_ClyApp_Ok)
    //if record can not be read - exist
    return err;

  st_Static_StateMachine.c_ContractRecNumberForUse = st_CardCancelData->uc_ContractRecNumBefore;//this is for getting the correct binary data from state machine

  //read contract and counter just to get binary data for calypso record
  e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
    (clyCard_BYTE)(st_CardCancelData->uc_ContractRecNumBefore),//[IN] //not relevat for ticket - record number to read : 1 is always the first record
    e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    (void*) &Global_union_ContractRecord, &OutConverError,0);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts

  e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
    (clyCard_BYTE)(st_CardCancelData->uc_ContractRecNumBefore),//[IN] //not relevat for ticket - record number to read : 1 is always the first record
    e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    (void*) &Global_union_ContractRecord, &OutConverError,0) ;//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  ////////////////////////////////////////////////////////////////

  //==================================
  // update Next Event Priority List
  //==================================
  memcpy(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr,union_EventRecord.st_CardEventDataStruct.e_EventBestContractPriorityListArr,sizeof(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr));

  if(st_CardCancelData->Action == BIN_DATA_OF_LOAD)
  {
    return  e_ClyApp_CancelCardLoad(st_CardCancelData, stp_CardEventDataStruct, b_Result);
  }
  else if(st_CardCancelData->Action == BIN_DATA_OF_USE)
  {
    //allow cancel use only if the last action isn't cancel- fix bug that cancel of use after cancel of load restores the contract
    if(union_EventRecord.st_CardEventDataStruct.st_EventCode.e_CardEventCircumstances == e_CardEventCircumCancellation)
      return e_ClyApp_NotOk;
    else return e_ClyApp_CancelCardUse(st_CardCancelData, stp_CardEventDataStruct, b_Result, &union_EventRecord.st_CardEventDataStruct);
  }
  else return e_ClyApp_NotOk;

}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_CancelOperationByContractIndex
//
//DESCRITION:
//
//                Cancel the operation described by event index
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//            1 Send the contract bin data before use / Load
//             if it card :
//             {
//                2 Check if pBinDataForCancel  Correct  By converting
//
//                3 Save Last BCPL  by reading last event (1)
//                4 Preaper new Event at ram by taking the Input Event
//                 Modify the Priorty of the contract
//                5 Open session
//                6 read the curent counter Record from card
//                7 modify the counter of the Contract
//                8 Write the old Contract
//                9 Write Counter
//                10 write the new event
//                11 close session
//              }
//              else
//              {
//                 Write to ticket all the Data by e_Internal_Write with ticket paramer
//              }
//
//
//
//  Yoni 22.8.07 -
//  In case of SV we can't just change the priority, because sometimes we load
//  a contract on another- meaning incremeting the counter. Thus, the cancellation can't
//  erase the contract, and must restore the previous counter.
//  So, If this is a cancel of stored value issue- do a restore of contract+last event + counter
//  Because the previous priority of the contract was valid, it goes to cancel of use which does
//  exactly what we need.
//    ***
//
//
//
//  Cancel logic:
//Input:
//    Contract pointer
//    Data before cancel
//If card:
//�   check BCPL of the contract before the operation
//�   if  need to be change to not valid -  only update the contract BCPL - do not update the contract or the counter
//�   if cancel use operation restore the contract && the counter - only the counter related the contract
//�   add a new event - mark as cancel
//
//if ticket :
//    convert of ticket and restore the data.
//
//
//                Send the contract bin data before use - send by the USE command - if trying to cancel LOAD operation - send NULL in bin data since it is not needed.
//
//                Check if cancel possible :
//                Check all events until the requested record and check if there was not and loading of new contract in this contract record.
//
//                Load cancel
//                Only mark record as un exist in the priority list
//
//                Use cancel
//                The user request to cancel contract indicated by it's event record
//                Get contract record indicated by the event selected
//                Restore the counter of the contract
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
eCalypsoErr CLYAPP_STDCALL e_ClyApp_CancelOperationByContractIndex(const TR_St_CancelData *union_BinDataBeforeUseForCancel,//[IN]
  st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card. defines the parameters of the cancle operation.
  clyApp_BOOL *b_Result,//[OUT]1=Cancel OK ,0=Cancel fail
  union_ClyApp_TransactionBinData* p_TransactionBinData //[OUT]
  )
{
  eCalypsoErr err;
  //==============================
  //check card type  - if card
  //==============================
  if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
  {
    err =  e_ClyApp_CancelCardOperation(&union_BinDataBeforeUseForCancel->st_CardCancelData, stp_CardEventDataStruct, b_Result);

    if(err == e_ClyApp_Ok)
    {
      e_ClyApp_GetTransactionBinaryData(p_TransactionBinData);//[OUT] get the Transaction Binary Data
    }
  }
  else//ticket
  {
    err = e_ClyApp_CancelTicketOperation(union_BinDataBeforeUseForCancel->ucp_BinTktDataBeforUse, stp_CardEventDataStruct, b_Result);
    e_ClyApp_GetTransactionBinaryData(p_TransactionBinData);//[OUT] get the Transaction Binary Data

  }

  return err;

}

////////////////////////////////////////////////////////////////////////////
//  Yoni 04/2013
//  Mark contract as erasable -  write cancel event with new bcpl.
//                               delete associated special event
//  this function cant be used to cancel loading of stored value which 
//  had previously had credit
////////////////////////////////////////////////////////////////////////////
eCalypsoErr CLYAPP_STDCALL e_ClyApp_CardCancelContract(unsigned char uc_ContractRecNum,
                                                   st_clyApp_CardEventDataStruct *stp_CardEventDataStruct,//[IN] parameter relevant only for card. defines the parameters of the cancle operation.                                                   
												   union_ClyApp_TransactionBinData* p_TransactionBinData, //[OUT]
                                                   int* pSpecialEventIndexToDelete //[OUT] if 0-3 then this special event needs to be deleted
                                                   )
{
    eCalypsoErr err;
    RESPONSE_OBJ* Obj;
    eCalypsoErr OutConverError;
    union_ClyApp_EventRecord union_EventRecord;
    int iSpecialEventRecNum = -1;
    memset(p_TransactionBinData, 0, sizeof(*p_TransactionBinData));

    CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()
    CHECK_SESSION_OPEN()

    //verify uc_ContractRecNum
    if(uc_ContractRecNum > 8)
    {
        return e_ClyApp_WrongParamErr;
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // Read event Record 2 - to be available for e_ClyApp_GetTransactionBinaryData command
    // 2 before 1 because we pass event1 to canceluse
    ///////////////////////////////////////////////////////////////////////////////////////
    err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
        2,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        (void*) &union_EventRecord.st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
    {
    //if record can not be read - exist
        return err;
    }
    ///////////////////////////////////////////////////////////////////////////////////////
    // Read event Record 1 - to be available for e_ClyApp_GetTransactionBinaryData command
    ///////////////////////////////////////////////////////////////////////////////////////
    err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
                        1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
                        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                        (void*) &union_EventRecord.st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
    {
        //if record can not be read - exist
        return err;
    }  
    
    //read contract and counter just to get binary data for calypso record
    e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
                    (clyCard_BYTE)uc_ContractRecNum,//[IN] //not relevat for ticket - record number to read : 1 is always the first record
                    e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                    (void*) &Global_union_ContractRecord, &OutConverError,0);//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
            
    e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
                    (clyCard_BYTE)uc_ContractRecNum,//[IN] //not relevat for ticket - record number to read : 1 is always the first record
                    e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                    (void*) &Global_union_ContractRecord, &OutConverError,0) ;//[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    ////////////////////////////////////////////////////////////////
             
    
    //mark priority as erasable
    st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr[uc_ContractRecNum -1] = e_CardPriorityErasable;

    // fill event: VersionNumber,Service Provider,Event Contract Pointer,Event Date and Time,priority list
    v_Internal_FillEventData( stp_CardEventDataStruct,//[OUT] event to fill
                              uc_ContractRecNum);//[IN]Event Contract Pointer


    //Event Circumstances  = Contract  Cancellation
    stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances = e_CardEventCircumCancellation;

    //Write event
    err = e_Internal_Write(  e_ClyApp_Card,//[IN]type - card \ ticket
                            1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
                            e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                            stp_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
                            );
    if( err!=e_ClyApp_Ok)
        return err;


    /////////////////////////////////////////////////////////////////////////////////////////////////////
    //2. check if we need to erase special event
    iSpecialEventRecNum= i_Internal_FindSpecialEvent(uc_ContractRecNum); // recnum of old contract
    if(iSpecialEventRecNum>=1)
    {    
        //we have a special event associated with contract
        //delete it 
        memset(&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[iSpecialEventRecNum], 0, sizeof(st_clyApp_CardEventDataStruct));
        err = e_Internal_Write(e_ClyApp_Card,//[IN]
                            (char)(iSpecialEventRecNum),//[IN]
                            e_clyCard_SpecialEventFile, //[IN]
                            &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_SpecialEventRecArr[iSpecialEventRecNum]
                            ,(char)REC_SIZE
                            );
        if( err!=e_ClyApp_Ok)
            return err;

        *pSpecialEventIndexToDelete = iSpecialEventRecNum-1;
    }

    //==================
    // Close Session
    //==================
    Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
                                            st_Static_StateMachine.SamReaderId,
                                            clyCard_FALSE);//[IN] 
        
    if( !IS_RES_OBJ_OK(Obj) )
    {
        //if session close fail - make sure that the reader will not receive deselect by RF shutdown
        v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
        return e_ClyApp_CardWriteErr;
    }

    //========================
    // Update state machine
    //========================
    st_Static_StateMachine.e_TransactionState = e_clyApp_SessionCloseOk;
    st_Static_StateMachine.c_ContractRecNumberForUse = uc_ContractRecNum;
    e_ClyApp_GetTransactionBinaryData(p_TransactionBinData);//[OUT] get the Transaction Binary Data
    
    return e_ClyApp_Ok;
        
}    

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_Lock
//
//DESCRITION:
//
//                Lock card/Ticket
//                This function gives the ability to lock the issued card for future use in case of black list for example
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_LOCK 0
eCalypsoErr CLYAPP_STDCALL e_ClyApp_Lock(union_ClyApp_TransactionBinData* p_TransactionBinData) //[IN]reader ID
{
  eCalypsoErr err;
  RESPONSE_OBJ* obj;
#if !DEBUG_LOCK
  St_clySam_KIF_And_KVC St_KIF_And_KVC;
  e_clyCard_KeyType KeyType;
#endif

  CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()


    //==============================
    //check card type  - if card
    //==============================
    if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
    {
      err =  e_Internal_WriteEventCode(st_Static_StateMachine.CardReaderId, //[IN]reader ID
        e_CardEventCircumInvalidation);//[IN]Event Circums to write
      if( err!=e_ClyApp_Ok)
        return err;


#if DEBUG_LOCK
      return e_ClyApp_Ok;
#else


      //really lock- do invalidate   Yoni 4.6.07
      //open session, invalidate and close session
      //open session
      KeyType = (st_Static_StateMachine.e_SamType == e_ClyApp_SamCL)? e_clyCard_KeyCredit:e_clyCard_KeyDebit;
      v_Internal_GetKifVal(KeyType,&St_KIF_And_KVC);
      err = ClyApp_Virtual_OpenSecureSession(  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
        KeyType,//[IN] Key Type to use for the session
        NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
      if( err!=e_ClyApp_Ok)
        return err;//open session failed


      //invalidate
      obj= ClyApp_Virtual_Invalidate(st_Static_StateMachine.CardReaderId);// (IN)reader id
      if( IS_RES_OBJ_OK(obj) )
      {

        //close session
        obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
          st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
          (clyCard_BOOL)0);//[IN] //1= the session will be immediately Ratified

        if( IS_RES_OBJ_OK(obj) )
        {
          st_Static_StateMachine.e_TransactionState = e_clyApp_SessionCloseOk;//Yoni 8/2/2010

          e_ClyApp_GetTransactionBinaryData(p_TransactionBinData);//[OUT] get the Transaction Binary Data

          return e_ClyApp_Ok;
        }
      }

#endif
    }
    return e_ClyApp_CardLockedErr;

}


//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_IsLock
//
//DESCRITION:
//
//                Is Card/Ticket Locked
//                This function gives the ability to check if a card is locked for future use
//
//                IMPORTANT : befor any call to this function RESET MUST be done!!!
//
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//                check open session
//                dummy write to event file
//                check SW0 = 0x62 && SW1 = 0x83 -> card is loacd
//                Internal_ForgetCard - so that the write operation will have no efect
//////////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_IsLock(clyApp_BOOL *b_Result)//[OUT]1=Locked ,0=not locked
{


  eCalypsoErr err;
	eCalypsoErr OutConverError;
  st_clyApp_CardEventDataStruct st_CardEventDataStruct;
  CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()

    *b_Result=clyApp_FALSE;


  //==============================
  //check card type  - if card
  //==============================
  if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
  {
    memset(&st_CardEventDataStruct,0,sizeof(st_CardEventDataStruct));
    //read last event && copy last event data to new event_sem
    ////////////////////////
    // Read event Record 1
    ///////////////////////
    err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
      (clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &st_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
      //if record can not be read - exist
      return err;

    //chaeck if Event Circumstances  =  Invalidation

    if(st_CardEventDataStruct.st_EventCode.e_CardEventCircumstances == e_CardEventCircumInvalidation )
      *b_Result=clyApp_TRUE;
  }
  return e_ClyApp_Ok;
}



//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_GetSamType
//
//DESCRITION:
//
//                Get sam type CL/SL/CPP/CP/CV
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//                In calypso SAM OS read Parameters file and check the parameter Version:
//                �   SAM-SL: ParametersVersion = 0x306001
//                �   SAM-DV: ParametersVersion = 0x806001
//                �   SAM-CPP: ParametersVersion = 0x406001
//                �   SAM-CP: ParametersVersion = 0x506001
//                �   SAM-CL: ParametersVersion = 0x606001
//                �   SAM-CV: ParametersVersion = 0x706001
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ENABLE_COMM
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetSamType(e_7816_DEVICE i_SamReaderId, //[IN]SAM reader ID
  e_ClyApp_SamType *e_SamType)//[OUT] the SAM type
{
  RESPONSE_OBJ* obj;
  Union_clySam_ReadDataType  ReadDataIn;
  St_clySam_ReadDataResult ReadResult;

  ReadDataIn.DataFileType = e_clySam_ParamFile;

  obj =  pSt_ClySam_ReadData(i_SamReaderId,//[IN] SAM reader id
    e_clySam_File,//[IN] type of data - file or record
    &ReadDataIn,//[IN] type of file / record to read
    &ReadResult);//[out] read result
  if( IS_RES_OBJ_OK(obj) )
  {
    switch (ReadResult.DataOut.Params.ul_ParamsVer&0xffffff01)
    {
    case 0x306001: *e_SamType =  e_ClyApp_SamSL; break;
    case 0x806001: *e_SamType =  e_ClyApp_SamDV; break;
    case 0x406001: *e_SamType =  e_ClyApp_SamCPP; break;
    case 0x506001: *e_SamType =  e_ClyApp_SamCP; break;
    case 0x606001: *e_SamType =  e_ClyApp_SamCL; break;
    case 0x706001: *e_SamType =  e_ClyApp_SamCV; break;
    default : return e_ClyApp_WrongSamTypeErr;
    }
    return e_ClyApp_Ok;
  }
  return e_ClyApp_CardReadErr;
}
#endif

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
                                                )
{
		
    RESPONSE_OBJ* obj;
    Union_clySam_ReadDataType  ReadDataIn;

    memset(&ReadDataIn, 0, sizeof(ReadDataIn));

    if(!pReadResult)
        return e_ClyApp_WrongParamErr;

    memset(pReadResult, 0, sizeof(*pReadResult));
    
    if(e_clySam_Rec==eDataType)
    {
        ReadDataIn.DataRecType.RecType = eRecType;
        ReadDataIn.DataRecType.RecNum = RecNum; 
    }
    else
    {
        ReadDataIn.DataFileType = e_clySam_EPTransactionNum;//not used 
    }
    

    obj =  pSt_ClySam_ReadData(st_Static_StateMachine.SamReaderId,//[IN] SAM reader id 
							   eDataType,//[IN] type of data - file or record                               
                               &ReadDataIn,//[IN] type of file / record to read
                               pReadResult);//[out] read result

    if( IS_RES_OBJ_OK(obj) )
    {        
        return e_ClyApp_Ok;
    }

    return e_ClyApp_CardReadErr;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_ResetCard
//
//DESCRITION:
//
//                Reset Card
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//               call  calypso card OS
///////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_ResetCard(e_7816_DEVICE i_ReaderId,//[IN] the reader ID
  e_7816_CardType* e_CardType)//[OUT]type of card
{
  clyCard_BYTE p_Atr[CLY_CARD_MAX_ATR_LEN];
  int ip_AtrLen;
  unsigned long p_pSNum;
  unsigned char ucp_SnArr[4]={0};
  RESPONSE_OBJ* obj;
#if 0 //TBD:yoram
  const ContactlessInfo *sInfo ;//
#endif
  CHECK_INERFACE_INIT()

    //==================================================================================================================
    //check if the card is a memory card - if so work directly with TR1000 since the card does not support 7816 protocol
    //==================================================================================================================
#if 0 //TBD:yoram
    sInfo =  ContactlessGetCardInfo();
  *e_CardType = e_7816_UNKNOWN_CARD;
  if( sInfo )
    *e_CardType =e_Internal_ConvertTo7816TktType( sInfo->e_ClType);
  if(IS_CL_READER(i_ReaderId ) && IS_TICKET_CARD (*e_CardType) )
  {
    SN8 cp_SN8={0};
    //ticket SN = 8 out of 10 bytes
    memcpy(cp_SN8,sInfo->cp_ClUid,2);
    memcpy(cp_SN8+2,sInfo->cp_ClUid+4,6);

    //update state machine
    return e_ClyApp_StartWorkWithCard(i_ReaderId,//[IN] the reader ID in which to detect
      cp_SN8, /*[IN]*/ //The card SN
      8);//[IN] Serial Number Len

  }
  else // not a ticket
#endif
  {
    //Reset using card OS
    obj = pSt_ClyCard_Reset(i_ReaderId,//[IN] SAM reader id
      p_Atr,//[OUT] the card atr
      &ip_AtrLen,//[OUT] // the atr Length
      (eClyCardTypes*)e_CardType);//   type of card
    if( !(obj->sw1_sw2[0] == 0 && obj->sw1_sw2[1] == 0) )
    {
      st_Static_StateMachine.e_AppCardType = e_ClyApp_Card;
      if( ! e_ClyApp_GetCardSn(i_ReaderId,//[IN] the reader ID
        &p_pSNum ))/*[OUT]*/ //The card SN
      {

        memcpy(ucp_SnArr,&p_pSNum,sizeof(ucp_SnArr));
        //update state machine
        return e_ClyApp_StartWorkWithCard(i_ReaderId,//[IN] the reader ID in which to detect
          ucp_SnArr, /*[IN]*/ //The card SN
          4);//[IN] Serial Number Len
      }
    }
  }
  return e_ClyApp_ResetErr;
}
//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_GetCardSn
//
//DESCRITION:
//
//                Get Card Serial Number
//                get the card Serial Number - Function does not make an internal memory allocation.
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//               call  calypso card OS
///////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetCardSn(e_7816_DEVICE i_ReaderId,//[IN] the reader ID
  unsigned long* p_pSNum )/*[OUT]*/ //The card SN
{
  St_clyCard_SN St_SN;
  eClyCardTypes type;
  St_clySam_SN St_SamSN;

  CHECK_INERFACE_INIT()

    if( i_ReaderId != st_Static_StateMachine.SamReaderId)
    {
      if(st_Static_StateMachine.e_AppCardType == e_ClyApp_Ticket )
      {
        //   Slava :we need the second long
        memcpy(p_pSNum,st_Static_StateMachine.ucp_CardSn+4,4);
        v_FlipBytes((unsigned char*)p_pSNum,sizeof(short));
        v_FlipBytes(((unsigned char*)p_pSNum)+2,sizeof(short));

        return e_ClyApp_Ok;
      }
      else
      {
        //get sam serial number - 4 bytes long
        if( b_ClyCard_GetSerNum(i_ReaderId,//[IN] SAM reader id
          &St_SN,//[OUT] card serial numer
          &type))///(OUT) type of card
        {
          memcpy(p_pSNum,&St_SN.p_SerNum4,4);
          return e_ClyApp_Ok;
        }
      }
    }
    else
    {

      //get sam serial number - 4 bytes long
      if( b_ClySam_GetSerNum(i_ReaderId,//[IN] SAM reader id
        &St_SamSN) )//[OUT] card serial numer
      {
        memcpy(p_pSNum,&St_SamSN.p_SerNum4,4);
        return e_ClyApp_Ok;
      }

    }

    return e_ClyApp_NoCardErr;
}

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_GetCardFullSn
//
//DESCRITION:
//
//                Get Card Full Serial Number
//                get the card Full Serial Number - up to 8 bytes value - Function does not make an internal memory allocation.
//
//RETURN:
//
//                eCalypsoErr
//
//LOGIC :
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TIM_APP
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetCardFullSn( e_7816_DEVICE i_ReaderId,//[IN] the reader ID
  unsigned char* p_pSNum,//[OUT] //The card SN
  unsigned char *SnLen)//[OUT] [OUT] SN byte len
{
  St_clyCard_SN St_SN;
  eClyCardTypes type;
  St_clySam_SN St_SamSN;

  CHECK_INERFACE_INIT()

    if( i_ReaderId != st_Static_StateMachine.SamReaderId)
    {
      if(st_Static_StateMachine.e_AppCardType == e_ClyApp_Ticket )
      {
        memcpy(p_pSNum,st_Static_StateMachine.ucp_CardSn,st_Static_StateMachine.uc_CardSnLen);
        *SnLen = st_Static_StateMachine.uc_CardSnLen;
        return e_ClyApp_Ok;
      }
      else
      {
        //get sam serial number - 4 bytes long
        if( b_ClyCard_GetSerNum(i_ReaderId,//[IN] SAM reader id
          &St_SN,//[OUT] card serial numer
          &type))///(OUT) type of card
        {
          memcpy(p_pSNum,&St_SN.p_SerNum4,4);
          return e_ClyApp_Ok;
        }
      }
    }
    else
    {

      //get sam serial number - 4 bytes long
      if( b_ClySam_GetSerNum(i_ReaderId,//[IN] SAM reader id
        &St_SamSN) )//[OUT] card serial numer
      {
        memcpy(p_pSNum,&St_SamSN.p_SerNum4,4);
        return e_ClyApp_Ok;
      }

    }

    return e_ClyApp_NoCardErr;


}
#endif

//=======================================================================
//              MANDATORY CALLBACK
//=======================================================================
//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                v_ClyApp_SetUserCallBacks
//
//DESCRITION:
//
//                Set User Callbacks
//                Set all user callback - if callback is NULL it will not be used
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//               call  calypso card OS
//////////////////////////////////////////////////////////////////////////////////////////////////

////*//   1020 not in use!!!
void  CLYAPP_STDCALL v_ClyApp_SetUserCallBacks(st_ClyApp_Callback *stp_Callback)//[IN] user callbacks
{
  //store callbacks
  memcpy(&st_Static_StateMachine.st_UserCallbacks,stp_Callback,sizeof(st_ClyApp_Callback));
}


//Yoni 22/2/10
//if true this means that contract was read and converted and authentication ok
char CLYAPP_STDCALL isLegalContract(int recNum)
{
  if (st_Static_StateMachine.st_TransactionData.isContractConversionValid[recNum] == e_ClyApp_Ok
    && st_Static_StateMachine.st_TransactionData.isContractAuthOk[recNum] == e_ClyApp_Ok
    )
    return 1;
  else
    return 0;

}
#ifndef ENABLE_COMM
e_ClyTkt_ERR e_Internal_SamSig( clyTkt_BYTE * dataIN,
  clyTkt_WORD datalen,
  ClyTkt_SignType type,
  clyTkt_BYTE* outsign)
{
  //#define  DEBUG_DUMMY_SIG
#ifdef DEBUG_DUMMY_SIG
  unsigned char uch_SigLen;
  switch(type)
  {
  case ClyTkt_SignType_8: uch_SigLen=8; break;
  case ClyTkt_SignType_16: uch_SigLen=16; break;
  case ClyTkt_SignType_32: uch_SigLen=32; break;
  }

  memset(outsign,0xa5,uch_SigLen/8);

#else
  RESPONSE_OBJ *Obj;
  SN8 SerialNum={0};
  Union_clySam_WorKeyParamsAcess KeyAccess;

  if(st_Static_StateMachine.e_TransactionState == e_clyApp_NoCardExist )
    return (e_ClyTkt_ERR)e_ClyApp_NoCardErr;

  if( st_Static_StateMachine.uc_CardSnLen )
    memcpy(SerialNum,st_Static_StateMachine.ucp_CardSn,st_Static_StateMachine.uc_CardSnLen);


  if( type == ClyTkt_SignType_16 ) // 2 bytes sig
  {
    KeyAccess.KifAndKvc.KIF=0x2B ; //KIF
    KeyAccess.KifAndKvc.KVC=0x60;   // KVC
  }
  else // ClyTkt_SignType_32 = 4 bytes sig
  {
    KeyAccess.KifAndKvc.KIF=0x2B ; //KIF
    KeyAccess.KifAndKvc.KVC=0x61;   // KVC
  }

  Obj =   pSt_ClySam_Compute (st_Static_StateMachine.SamReaderId, //[IN] SAM reader id
    e_clySam_KeyKIFandKVC,//[IN] choose access type - e_clySam_KeyIndex /  e_clySam_KeyKIFandKVC
    KeyAccess, //[IN]
    SerialNum, //[IN] serial number
    dataIN, //[IN] data to certify
    datalen, //[IN] size of data
    (type == ClyTkt_SignType_16) ? 2:4, //[IN] certificate size out ( 2 or 4 bytes )
    outsign); //[OUT] certificate out

  if( !IS_RES_OBJ_OK(Obj) )
    return (e_ClyTkt_ERR)e_ClyApp_SamNotLoginErr;


#endif

  return (e_ClyTkt_ERR)e_ClyTkt_NO_ERROR;///e_ClyApp_Ok;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                v_Internal_FillDescriptors
//
//DESCRITION:
//
//                 Fill global Descriptors
//
//RETURN:
//
//                void
//LOGIC :
//
///////////////////////////////////////////////////////////////////////////////////////////////////

void v_Internal_FillDescriptors()
{

  unsigned short offset;//,ByteOffset,BitOffset;
  unsigned char i;

  memset(&stEnvDesc,0,sizeof(stEnvDesc));
  memset(&stEventMandatoryDataDesc,0,sizeof(stEventMandatoryDataDesc));
  ///1020       memset(&stContractMandatoryDataDesc,0,sizeof(stContractMandatoryDataDesc));
  //==========================
  // ENV file descriptor
  //==========================
  i=0;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.uc_EnvApplicationVersionNumber;
  stEnvDesc[i].uc_StreamBitCount = 3;
  stEnvDesc[i].us_StreamBitOffset = (offset=0);
  stEnvDesc[i].e_Fieltype = e_VariableType ;
  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.sh_EnvCountryld;
  stEnvDesc[i].uc_StreamBitCount = 12;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.uc_Envlssuerld;
  stEnvDesc[i].uc_StreamBitCount = 8;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.ul_EnvApplicationNumber;
  stEnvDesc[i].uc_StreamBitCount = 26;
  stEnvDesc[i].us_StreamBitOffset =(offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.sh_EnvlssuingDate;
  stEnvDesc[i].uc_StreamBitCount = 14;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.sh_EnvEndDate;
  stEnvDesc[i].uc_StreamBitCount = 14;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.uc_EnvPayMethod;
  stEnvDesc[i].uc_StreamBitCount = 3;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.ul_HolderBirthDate;
  stEnvDesc[i].uc_StreamBitCount =32 ;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.sh_HolderCompany;
  stEnvDesc[i].uc_StreamBitCount = 14;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.ul_HolderCompanylD;
  stEnvDesc[i].uc_StreamBitCount = 30;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.ul_HolderldNumber;
  stEnvDesc[i].uc_StreamBitCount = 30;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.uc_HoiderProf1Code;
  stEnvDesc[i].uc_StreamBitCount = 6;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.sh_HoiderProf1Date;
  stEnvDesc[i].uc_StreamBitCount = 14;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.uc_HoiderProf2Code;
  stEnvDesc[i].uc_StreamBitCount = 6;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.sh_HoiderProf2Date;
  stEnvDesc[i].uc_StreamBitCount = 14;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;
  ////////////////////////////////////////
  //Yoni 11.7.07 Add language field to ENV
  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.uc_HolderLanguage;
  stEnvDesc[i].uc_StreamBitCount =2 ;
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;//???????????
  ///////////////////////////////////////
  i++;
  stEnvDesc[i].vp_StFieldPtr = &st_Global_EnvAndHoldBinDataStruct.uc_HoiderRFU;
  //stEnvDesc[i].uc_StreamBitCount =6 ;
  stEnvDesc[i].uc_StreamBitCount =4 ;//Yoni 11.7.07
  stEnvDesc[i].us_StreamBitOffset = (offset+=stEnvDesc[i-1].uc_StreamBitCount);
  stEnvDesc[i].e_Fieltype = e_VariableType ;

  //======================================
  // Event Mandatory data file descriptor
  //======================================

  i=0;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventVersionNumber;
  stEventMandatoryDataDesc[i].uc_StreamBitCount = 3;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset=0);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventServiceProvider;
  stEventMandatoryDataDesc[i].uc_StreamBitCount = 8;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventContractPointer;
  stEventMandatoryDataDesc[i].uc_StreamBitCount =4 ;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventCode;
  stEventMandatoryDataDesc[i].uc_StreamBitCount =8 ;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;


  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.ul_EventDateTimeStamp;
  stEventMandatoryDataDesc[i].uc_StreamBitCount =30 ;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;


  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.b_EventIsJourneylnterchange;
  stEventMandatoryDataDesc[i].uc_StreamBitCount = 1;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;


  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.ul_EventDataTimeFirstStamp;
  stEventMandatoryDataDesc[i].uc_StreamBitCount = 30;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;


  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventBestContractPriority1;
  stEventMandatoryDataDesc[i].uc_StreamBitCount =4 ;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;


  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventBestContractPriority2;
  stEventMandatoryDataDesc[i].uc_StreamBitCount = 4;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;


  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventBestContractPriority3;
  stEventMandatoryDataDesc[i].uc_StreamBitCount = 4;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;


  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventBestContractPriority4;
  stEventMandatoryDataDesc[i].uc_StreamBitCount = 4;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventBestContractPriority5;
  stEventMandatoryDataDesc[i].uc_StreamBitCount =4 ;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventBestContractPriority6;
  stEventMandatoryDataDesc[i].uc_StreamBitCount =4 ;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventBestContractPriority7;
  stEventMandatoryDataDesc[i].uc_StreamBitCount =4 ;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventBestContractPriority8;
  stEventMandatoryDataDesc[i].uc_StreamBitCount =4 ;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;

  i++;
  stEventMandatoryDataDesc[i].vp_StFieldPtr = &st_Global_CardEventBinDataStruct.uc_EventLocation;
  stEventMandatoryDataDesc[i].uc_StreamBitCount =ENV_EVENT_LOCATIOM_FIELD_BIN_COUNT ;
  stEventMandatoryDataDesc[i].us_StreamBitOffset = (offset+=stEventMandatoryDataDesc[i-1].uc_StreamBitCount);
  stEventMandatoryDataDesc[i].e_Fieltype = e_VariableType ;


}


//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                pSt_Internal_Unlock
//
//DESCRITION:
//
//                Unlock Sam
//
//RETURN:
//
//                RESPONSE_OBJ
//LOGIC :
//                detect sam
//                reset sam
//                unlock sam
///////////////////////////////////////////////////////////////////////////////////////////////////
////T
#ifndef INSPECTOR_TERMINAL
static eCalypsoErr   e_Internal_ResetAndUnlock (e_7816_DEVICE ReaderId, UULOCK_PIN16 Pin)
{

  RESPONSE_OBJ* Obj;

  if(!_7816_CheckCardComm(ReaderId,&Obj))
    return e_ClyApp_WrongSamTypeErr;


  //Give SAM PIN
  Obj = pSt_ClySam_Unlock (ReaderId, Pin);
  if( IS_RES_OBJ_OK(Obj) || IS_SAM_NOT_LOCKED(Obj))
    return e_ClyApp_Ok;

  return e_ClyApp_WrongSamTypeErr;
}
#endif
///////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_Internal_UnlockSam
//
//DESCRITION:
//
//                Unlock Sam
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//                send password accurding to the SAM type - assume the SAM is a real SAM. If error type the
//                israeli PIN
//                if 69 85 - sam is not locked
//
///////////////////////////////////////////////////////////////////////////////////////////////////
////T
#ifndef INSPECTOR_TERMINAL
static eCalypsoErr e_ClyApp_Internal_UnlockSam(int index)
{
  UULOCK_PIN16 pin;
  eCalypsoErr err;
  //Israel - SAM-SP Lock - TEST 050203
  UULOCK_PIN16 ISRAEL_TEST_PIN ={0xA4,0x0B,0x01,0xC3,0x9C,0x99,0xCB,0x91,0x0F,0xE6,0x2A,0x23,0x19,0x2A,0x0C,0x5C};
  //Israel SAM-CP 050112
  ///1020    UULOCK_PIN16 ISRAEL_SAM_CP_PIN ={0xF3,0xA2,0xBA,0xA7,0x80,0x86,0x53,0x96,0x8D,0x69,0x70,0xED,0x81,0xCE,0x8F,0x72};
  //Israel SAM-CL 050112

  //SamSL - dummy
  UULOCK_PIN16 ISRAEL_SAM_SL_PIN ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x5D};
  //SamCPP - dummy
  UULOCK_PIN16 ISRAEL_SAM_CPP_PIN ={0xA1,0xC6,0xF1,0x95,0xE2,0xFB,0x6A,0x7F,0x62,0x62,0x15,0x65,0xD5,0x1E,0xEE,0x74};
  //SamCV - dummy
  UULOCK_PIN16 ISRAEL_SAM_CV_PIN ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x5D};

  //Israel SAM-CL 050112
  UULOCK_PIN16 ISRAEL_SAM_CP_PIN[4] ={
    {0x0B,0x1E,0xA2,0x37,0x66,0x7A,0x11,0xB1,0xA8,0x2F,0x63,0x8F,0xA2,0xB0,0x29,0xA9},
    {0xF3,0xCE,0xF5,0x1D,0x52,0xC5,0x03,0x08,0x40,0x57,0x09,0x42,0x37,0x27,0x17,0x5D},
    {0xF3,0xA2,0xBA,0xA7,0x80,0x86,0x53,0x96,0x8D,0x69,0x70,0xED,0x81,0xCE,0x8F,0x72},
    {0xA1,0xC6,0xF1,0x95,0xE2,0xFB,0x6A,0x7F,0x62,0x62,0x15,0x65,0xD5,0x1E,0xEE,0x74},

  };

  //choose PIN
  switch (st_Static_StateMachine.e_SamType)
  {

  case e_ClyApp_SamSL:
    memcpy(pin,ISRAEL_SAM_SL_PIN,sizeof(pin));
    break;
  case e_ClyApp_SamCPP:
    memcpy(pin,ISRAEL_SAM_CPP_PIN,sizeof(pin));
    break;
  case e_ClyApp_SamCP:
    memcpy(pin,ISRAEL_SAM_CP_PIN[index],sizeof(pin));
    break;
  case e_ClyApp_SamCL:
    memcpy(pin,ISRAEL_SAM_CP_PIN[index],sizeof(pin));
    //   1020            memcpy(pin,ISRAEL_SAM_CL_PIN,sizeof(pin));
    break;
  case e_ClyApp_SamCV:
    memcpy(pin,ISRAEL_SAM_CV_PIN,sizeof(pin));

    break;
  default: return e_ClyApp_WrongSamTypeErr;
  }


  //Give SAM PIN
  err = e_Internal_ResetAndUnlock (st_Static_StateMachine.SamReaderId, pin);
  if(err==e_ClyApp_Ok)
    return e_ClyApp_Ok;

  //if SAM still locked, try the Israeli TEST PIN
  err = e_Internal_ResetAndUnlock (st_Static_StateMachine.SamReaderId, ISRAEL_TEST_PIN);

  return err;

}
#endif


//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_Internal_InitSam
//
//DESCRITION:
//
//                init the SAM
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//                RESET + PIN
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef INSPECTOR_TERMINAL
int Mydelay = 100;

static eCalypsoErr  e_Internal_InitSam(UULOCK_PIN16 ucp_UnlockSampin)

{
  eCalypsoErr err;
  RESPONSE_OBJ* Obj;
  clySam_BYTE p_Atr[SAM_MAX_ATR_LEN];
  int ip_AtrLen;

  //Init SAM reader
  // Note st_Static_StateMachine.SamUartId must be known
    if(! b_ClySam_InitInterface(st_Static_StateMachine.SamUartId,st_Static_StateMachine.SamReaderId) )//[IN] SAM reader id
    return e_ClyApp_ReaderSamErr;

  //return clySam_TRUE if card in
  if( ! b_ClySam_DetectCard(st_Static_StateMachine.SamReaderId) )//[IN] SAM reader id
    return e_ClyApp_NoCardErr;

  Obj = pSt_ClySam_Reset(st_Static_StateMachine.SamReaderId,//[IN] SAM reader id
    p_Atr,//[OUT] the card atr
    &ip_AtrLen);//[OUT] // the atr Length

  if(  Obj == NULL || p_Atr==NULL || (Obj->sw1_sw2[0] == 0 && Obj->sw1_sw2[1] == 0) )
    return e_ClyApp_ResetErr;

  //get SAM type -  CL/SL/CPP/CP/CV
  err = e_ClyApp_GetSamType(st_Static_StateMachine.SamReaderId,&st_Static_StateMachine.e_SamType);//[IN]SAM reader ID
  if( err!=e_ClyApp_Ok)
    return err;

  //if PIN Exist - unlock sam
  if(ucp_UnlockSampin)///1020A    st_Static_StateMachine.ucp_UnlockSampin)
  {
    int j;
#ifdef INCLUDE_KEIL
    {
      CoreDelay(Mydelay);
    }
#endif
    //  Obj = pSt_ClySam_Unlock (st_Static_StateMachine.SamReaderId,ucp_UnlockSampin);///1020A    st_Static_StateMachine.ucp_UnlockSampin);
    //if( !( IS_RES_OBJ_OK(Obj)  || IS_SAM_NOT_LOCKED(Obj) ))
    // Guess the pin
    //{

    //unlock SAM
    err=(eCalypsoErr)(e_ClyApp_Ok+1);
    //unlock SAM
    for(j=0;j<4&&err!=e_ClyApp_Ok;j++)
      err = e_ClyApp_Internal_UnlockSam(j);
    if(err!=e_ClyApp_Ok)
      return err;

    // }

  }
  else
    return e_ClyApp_WrongParamErr;


  return e_ClyApp_Ok;

}
#endif //INSPECTOR_TERMINAL



/****************************/
// Get Card type   NEW FUNCTION FOR 1020
/****************************/
//get the card Serial Number - Function does not make an internal memory allocation.
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetCardType(e_7816_DEVICE i_ReaderId,//[IN] the reader ID
  unsigned char* p_CardType )/*[OUT]*/ //The card type
{
  (* p_CardType)=st_Static_StateMachine.e_AppCardType;
  return e_ClyApp_Ok;///???
}

/****************************/
// Get Environment   NEW FUNCTION FOR 1020
/****************************/
//Get All event record - for the cancellation operation -  helps to determine which of the last operation need to be canceled
eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetEnvironment(st_ClyApp_EnvAndHoldDataStruct *st_EnvAndHoldDataStruct)
{

  eCalypsoErr err;
	eCalypsoErr OutConverError;
  if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
  {
    // Read Environment Record
    err =e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
      (clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_EnvironmentFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      st_EnvAndHoldDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts

    if(err!=e_ClyApp_Ok)
      return err;
    //rise exist flag
    st_Static_StateMachine.e_TransactionState=e_clyApp_SessionOpenOk;


  }//   if card
  else   // else if card
  {

  }//   else if card

  return e_ClyApp_Ok;

}

int b_LoadTr1000Dll_InitInterface(int *ip_ReadersIdArr,int ArrLen);

//////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_InitInterface
//
//DESCRITION:
//
//                init interface
//                This function receive the ticket keys
//                the readers array describes the list all posible reader were a card/ticket/SAM SL  can be handled.
//
//
//RETURN:
//
//                eCalypsoErr
//LOGIC :
//                Init the state machine global data
//                Store SAM id + type
//                Init SAM reader
//                unlock sam PIN
//                Init all reader
//                Store keys - see St_KeyInfo in gemclub
//                init clyTktOs with the keys
//                Update state machine
//
//////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_InitInterface(
													UULOCK_PIN16 ucp_UnlockSampin,					//[IN]  Unlock SAM pin
													st_ReaderComInfo *op_CardReaderIdArr,			//[IN]	Card reader ID array
													unsigned char uc_ArrLen,						//[IN]	Card reader ID array len
													st_ClyTkt_KeyInfo st_KeyInfoArr[MAX_TIKET_KEYS])//[IN]	file path to the encrypted Card<->Sam key schema and the Ticket encrypt\decrypt key
{
	int i = 0;

#ifndef INSPECTOR_TERMINAL
  eCalypsoErr err;
#endif
//#ifndef INCLUDE_KEIL
//    int i;
//    #ifndef WIN32
//        p_st_Static_StateMachine= ( st_clyApp_StateMachine *)farcalloc(sizeof(st_clyApp_StateMachine),1);
//        //check memory problem
//        if( !p_st_Static_StateMachine)
//            return e_ClyApp_MemErr;
//    #else
//        #ifdef INCLUDE_KEIL
//            int i_ReadersIdArr[_READER_NON];
//        #else
//            int i_ReadersIdArr[_READER_NON - _READER_CONTACTLESS_1 ];
//        #endif
//    #endif
//#endif
//    //====================
  // Init Global Data
  //====================
  //Init the state machine
  memset(&st_Static_StateMachine,0,sizeof(st_Static_StateMachine));
    st_Static_StateMachine.CardWriteMode = e_NormalMode;
  //fill descriptors for bin -> st conversions
  v_Internal_FillDescriptors();

  //====================
  // Init SAM
  //====================
  //store SAM reader ID

  for (i = 0; i < uc_ArrLen; i++)
  {
	  if (op_CardReaderIdArr[i].mIsSAMReader)
	  {
		st_Static_StateMachine.SamUartId     = op_CardReaderIdArr[i].mUART;
		st_Static_StateMachine.SamReaderId   = (e_7816_DEVICE)i;
		st_Static_StateMachine.CardReaderId  = (e_7816_DEVICE)op_CardReaderIdArr[i].mPairedReaderId;

		// exit from loop
		break;
	  }
  }

//inspector works without sam
#ifndef INSPECTOR_TERMINAL
if(st_Static_StateMachine.SamUartId != eCoreUARTNone)
{
    //init the SAM - Reset + PIN
  err = e_Internal_InitSam(ucp_UnlockSampin);
  if( err!=e_ClyApp_Ok)
    return err;
}
#endif
  //=============================
  // Init Card / Ticket READERS
  //=============================
  if(!b_ClyCard_InitInterface(op_CardReaderIdArr, //[IN]Card reader ID array
    uc_ArrLen))//[IN]Card reader ID array len
    return e_ClyApp_ReaderErr;



  //=============================
  // init clyTktOs with the keys
  //=============================
#ifndef ENABLE_COMM
  // Init Interface - Get interface keys
  v_ClyTkt_InitInterface(st_KeyInfoArr,e_Internal_SamSig,e_Tkt_TimeAndDateCallback,e_Tkt_Add2DateCallback);
#endif
  //=============================
  // Update state machine - Interface init OK
  //=============================
  st_Static_StateMachine.i_IsInteraceInit=OK_VAL;
  return e_ClyApp_Ok;

}

//=======================================================================================================
//              HELPER FUNCTIONS - NOT MANDATORY FOR THIS LAYER( CAN BE DONE OUTSIDE THIS LAYER )
//========================================================================================================
///////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_DetectCard
//
//DESCRITION:
//
//                detect card
//                Check if card found in one of the readers - this command is not mandatory to this layer
//
//RETURN:
//
//                e_TR_ReaderError
//
//LOGIC :
//               call  calypso card OS
////////////////////////////////////////////////////////////////////////////////////////////////////
int ii;
eCalypsoErr CLYAPP_STDCALL e_ClyApp_DetectCard(e_7816_DEVICE i_ReaderId)//[IN] the reader ID in which to detect
{

#ifndef WIN32
    const ContactlessInfo *sInfo ;//=  e_Mif_GetCardInfo();
#endif

    //test
    if(st_Static_StateMachine.i_IsInteraceInit == OK_VAL)
    {
        ii = 2;
    }
    CHECK_INERFACE_INIT()


#ifndef win_or_linux

    if( IS_CL_READER( i_ReaderId )  )
    {
        if ( ContactlessDetect() )
        {
            sInfo =  ContactlessGetCardInfo();
            if( IS_TICKET_CARD (e_Internal_ConvertTo7816TktType( sInfo->e_ClType) ) )
                return e_ClyApp_Ok;
        }

    }
#endif

    //return clyCard_TRUE if card in
    if(b_ClyCard_DetectCard(i_ReaderId))//[IN] SAM reader id
        return e_ClyApp_Ok;

    return e_ClyApp_NoCardErr;
}




///////////////////////////////////////////////////////////////////////////////
//  Yoni 10/2011
//  read 3 events - no logic
//  card only
//  return ok if all 3 regular events were read
//  return readerr if existing record failed to read (io)
//  return e_ClyApp_InterfaceNotInitErr if interface not init
///////////////////////////////////////////////////////////////////////////////
eCalypsoErr CLYAPP_STDCALL e_ClyApp_SimpleGetAllEvent( union_ClyApp_EventRecord union_EventRecordArr[3]//[IN]Array memory allocation of events to fill
                                                       ,clyApp_BOOL bIsEventOkArr[3] //[OUT] indicate for each event if ok //Yoni 14/6/10
                                                       )
{
	eCalypsoErr OutConverError;
    if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
    {
      eCalypsoErr err;
    int i;
        CHECK_SESSION_OPEN()

        //=================
        //read Last event
        //=================
        //if Event File was updated - read the data from the card and not from the memory in case the update has fail
        if( st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[0])
            memset( st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr,0,sizeof(st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr));

        i=0;
        do
        {
            //===================
            // Read event Record
            //===================
              err = e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
                                        (clyCard_BYTE)(i+1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
                                        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                        (void*) &union_EventRecordArr[i], &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
      if ((e_ClyApp_CardReadErr == err) || (e_ClyApp_InterfaceNotInitErr == err))
          {
              return err;
          }
      else if(err == e_ClyApp_Ok && (union_EventRecordArr[i].st_CardEventDataStruct.uc_EventVersionNumber==EVENT_VERSION_NUM)) //Yoni 02/2012  Added version check
      {
              //data structure is valid
        bIsEventOkArr[i]=clyApp_TRUE;
      }
            i++;
        } while(i<3 );


    //copy to  e_NextEventPriorityListArr in state machine
    memcpy( st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr,
          st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[1].e_EventBestContractPriorityListArr,
          sizeof(st_Static_StateMachine.st_TransactionData.e_NextEventPriorityListArr));


    }
    else // ticket
    {
    return e_ClyApp_NotOk;
    }

    return e_ClyApp_Ok;

}
///////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_WaitCard
//
//DESCRITION:
//
//             try to reconnect card in TimeOut time               
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :


///////////////////////////////////////////////////////////////////////////////////////////////////
static eCalypsoErr e_ClyApp_WaitCard(int retries,int *p_IsCardReseleted)
{
	 eCalypsoErr err=e_ClyApp_NotOk;
	// if callback exist
	if(st_Static_StateMachine.st_UserCallbacks.e_WaitCard)
			err=st_Static_StateMachine.st_UserCallbacks.e_WaitCard(retries,p_IsCardReseleted);
	
	return err;

}

// shift the index one position down  
static void e_ClyApp_ShiftTransactionEvents(void)
{
	int i=0;
	for(i=0;i<3;i++)
	{ 
		memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[i],
			st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[i+1],29);


	}
}
///////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_VerifyLoading
//
//DESCRITION:
//
//             test if contract loadeded by comparing the last event befor and after               
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :


///////////////////////////////////////////////////////////////////////////////////////////////////

static eCalypsoErr e_ClyApp_VerifyLoading(st_clyApp_CardEventDataStruct *p_SaveFirstEvent)
{

	st_clyApp_CardEventDataStruct *p_union_EventRecordArr;
	p_union_EventRecordArr=&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[1];

	if((memcmp(p_union_EventRecordArr,p_SaveFirstEvent,
	   sizeof(st_clyApp_CardEventDataStruct))!=0))
	{

     
    // e_ClyApp_ShiftTransactionEvents();
	  return 	e_ClyApp_Ok;
	}
	else
		return e_ClyApp_NotOk;

}
///////////////////////////////////////////////////////////////////////////////
//FUNCTION:
//                e_ClyApp_GetAllEvent
//
//DESCRITION:
//
//                Get All event record - for the cancellation operation -  helps to determine which of the last operation need to be canceled
//
//RETURN:
//
//                eCalypsoErr
//
//
//LOGIC :
//            �   check (interface init, card exist, type= card )
//            �   read all events until error - record not found  - to support cards with more then 4 events
//            �   check event version number - return error if unknown version
//
///////////////////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_GetAllEvent( union_ClyApp_EventRecord union_EventRecordArr[MAX_EVENT_COUT]//[IN]Array memory allocation of events to fill
                                                       ,clyApp_BOOL bIsEventOkArr[MAX_EVENT_COUT] //[OUT] indicate for each event if ok //Yoni 14/6/10
                                                       )
{
    eCalypsoErr err;
		eCalypsoErr OutConverError;
    RESPONSE_OBJ* obj;
    clyApp_BYTE i;

    CHECK_INERFACE_INIT()
    CHECK_CARD_EXIST()

    //==============================
    //check card type  - if card
    //==============================
    if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
    {
        CHECK_SESSION_OPEN()

        //=================
        //read Last event
        //=================
        //if Event File was updated - read the data from the card and not from the memory in case the update has fail
        if( st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr[0])
            memset( st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr,0,sizeof(st_Static_StateMachine.st_TransactionData.b_IsEventRecExistArr));

        i=0;
        do
        {
            //===================
            // Read event Record
            //===================
            err = e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
                                        (clyCard_BYTE)(i+1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
                                        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                        (void*) &union_EventRecordArr[i], &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
            if (err!=e_ClyApp_Ok)
            {
                if (i==0)
                {
                    return err;
                }
                else
                {
                    i++;
                    continue;
                }
            }
                                        //if record can not be read - exist

            if ((i == 0) &&
                (union_EventRecordArr[i].st_CardEventDataStruct.uc_EventVersionNumber != EVENT_VERSION_NUM))
                return e_ClyApp_CardEventErr;

            bIsEventOkArr[i]=clyApp_TRUE;//Yoni 14/6/10

            i++;
        //=================================================
        //while not found - read all records
        //==================================================
        } while(i<MAX_EVENT_COUT );

        //check error type
        if(err!=e_ClyApp_Ok)
        {
            //check if error resulted from tring to read unexisting record
            obj = pSt_ClyCard_GetLastCardResponseObj();
            if(obj->sw1_sw2[0] == 0x6a && obj->sw1_sw2[1] == 0x83) // record not found
                return e_ClyApp_Ok;
        }
        return err;
    }
    else // ticket
    {
    }

    return e_ClyApp_Ok;
}

unsigned char uc_clyapp_GetCurrentCardType()
{
  return st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card;
}



static clyApp_BOOL b_IsValidAccordingToRestrictTimeCode(int RestrictCode)
{
  unsigned short hm_now;//10*hour+minutes
  st_Cly_DateAndTime st_CurrentDateAndTime = st_GetCurrentDateAndTime();
  hm_now = st_CurrentDateAndTime.st_Time.hour*60 + st_CurrentDateAndTime.st_Time.min;
  switch(RestrictCode)
  {
    case 1://morning hour
      //current hour must be >= morning hour
      if(g_Params.us_MorningEndHour > hm_now)
        return clyApp_FALSE;

      break;

    case 2://evening hour ??????????????????????? todo is this correct
      //if evening hour exists and passed evening start hour then return false
      if(g_Params.us_EveningStartHour && g_Params.us_EveningStartHour < hm_now)
          return clyApp_FALSE;

      break;
    default:
      return clyApp_TRUE;
  }

  return clyApp_TRUE;
}

//pre: read contracts and events
static eValidityStatus e_GetContractValidityStatusForUse(e_ClyApp_CardType e_AppCardType, const union_ClyApp_ContractRecord *pContractRecord, int cntrct_inx, st_ProcessedContractValidityInfo* pProcessedValidityInfo/*out*/)
{

  int iCurrPriority;
    e_clyApp_ValidityType e_validity;

  if(st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
  {

    //so if priority is invalid - set it in ov_ProcessedValidityInfo.status
    //pre: last event was already read
    iCurrPriority = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[1].e_EventBestContractPriorityListArr[cntrct_inx];
    //

  if(iCurrPriority==e_CardPriorityErasable ||
        iCurrPriority==e_CardPriorityInvalid)
    {
      pProcessedValidityInfo->status = e_Invalid;
            return e_Invalid;
    }
  }

  if (clyApp_FALSE == b_IsContractValidBasic(pContractRecord, (char)(cntrct_inx + 1)))
    {
        return e_Invalid;
    }

  e_validity =  e_ClyApp_CheckContractValidity( (char)(cntrct_inx + 1) , pProcessedValidityInfo);
  pProcessedValidityInfo->BCPL_Priority = (unsigned char)iCurrPriority;//

  switch(e_validity)
  {
    case e_ClyApp_ContractNoLongerValid:
      return e_Invalid;

    case e_ClyApp_ContractValidButNotInThisPeriod://future
      return e_Future;
    default:
      break;
  }

  //so the contract is valid but there are still more checks to be done
  //1.if card check restrict time
  if(e_AppCardType == e_ClyApp_Card)
  {
    if(pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_IsContractRestrictTimeCodeExist)
    {

      if(clyApp_FALSE == b_IsValidAccordingToRestrictTimeCode(pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictTimeCode))
      {
        return e_ValidButCantBeUsed;
      }
    }
  }


  return e_ValidForUse;

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//  Yoni 11/2011
//  e_clyapp_GetContractsForUseapi
//  Return value=number of contracts for use, by ref the contracts
//  Pre: CardIn
//
//////////////////////////////////////////////////////////////////////////////////////////////////
eCalypsoErr e_clyapp_GetContractsForUse(union_ClyApp_ContractRecord ContractsArr[MAX_CONTRACT_COUT],//[OUT]
                                        st_ProcessedContractValidityInfo ProcessedInfoArr[MAX_CONTRACT_COUT], //[OUT]
                                        int* pNumOfValidContracts //[OUT]
                                        )
{
  int i_valid_contracts=0, i;
  eValidityStatus l_status;

  int max_contracts=MAX_CONTRACT_COUT;

  if(st_Static_StateMachine.e_TransactionState == e_clyApp_NoCardExist)
      return e_ClyApp_NoCardErr;

  if(st_Static_StateMachine.e_AppCardType !=  e_ClyApp_Card)
  {
    max_contracts=1;
  }


	//Yoni 05/2013 - mark the invalid contracts
	v_UpdateNextBCPL();


  for(i=0;i<max_contracts;i++)
  {
    const union_ClyApp_ContractRecord *pContractRecord;
    st_ProcessedContractValidityInfo ov_ProcessedValidityInfo={0};


    //check if contract was read correctly (into clyapp state machine) and auth is ok
    // If e_VirtualReadMode ...

        if(!isLegalContract(i+1)) //iCurrContractIndex+1=rec num
      continue;


    //todo add support for ticket
    pContractRecord = (union_ClyApp_ContractRecord*)&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[i+1];


    l_status = e_GetContractValidityStatusForUse(st_Static_StateMachine.e_AppCardType, pContractRecord, i, &ov_ProcessedValidityInfo);
    ov_ProcessedValidityInfo.status = l_status;

    if(l_status==e_ValidForUse)
    {
      //add ro array
      ContractsArr[i_valid_contracts]=*pContractRecord;
      ProcessedInfoArr[i_valid_contracts]=ov_ProcessedValidityInfo;
      i_valid_contracts++;

    }

  }


  *pNumOfValidContracts = i_valid_contracts;
  return e_ClyApp_Ok;
}

/////////////////////////////////////////////////////////////////////////
//  Yoni 11/2011
//  e_clyapp_GetContractsForLoadOrReport
//  get all contracts on card : invalid/valid/future
//  Pre: read all regular events and contracts
/////////////////////////////////////////////////////////////////////////
eCalypsoErr e_clyapp_GetContractsForLoadOrReport(union_ClyApp_ContractRecord ContractsArr[MAX_CONTRACT_COUT]/*out*/, st_ProcessedContractValidityInfo ProcessedInfoArr[MAX_CONTRACT_COUT], int* pNumOfContracts)
{
  int i_num_contracts=0, i;
  //eValidityStatus l_status;
    e_clyApp_ValidityType l_status;

  int max_contracts=MAX_CONTRACT_COUT;

  if(st_Static_StateMachine.e_TransactionState == e_clyApp_NoCardExist)
    return e_ClyApp_NoCardErr;

  if(st_Static_StateMachine.e_AppCardType !=  e_ClyApp_Card)
  {
    max_contracts=1;
  }

  for(i=0;i<max_contracts;i++)
  {
    const union_ClyApp_ContractRecord *pContractRecord;
    st_ProcessedContractValidityInfo ov_ProcessedValidityInfo={0};


    //check if contract was read correctly (into clyapp state machine) and auth is ok
    if(!isLegalContract(i+1)) //iCurrContractIndex+1=rec num
    {
      continue;
    }



    //todo add support for ticket
    pContractRecord = (union_ClyApp_ContractRecord*)&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[i+1];


    if (clyApp_FALSE == b_IsContractValidBasic(pContractRecord, (char)(i + 1)))
    {
      continue;
    }

    l_status =  e_ClyApp_CheckContractValidity( (char)(i + 1) , &ov_ProcessedValidityInfo);

    //translate eValidityStatus
    switch(l_status)
    {
    case e_ClyApp_ContractNoLongerValid:
      ov_ProcessedValidityInfo.status = e_Invalid;
      break;
    case e_ClyApp_ContractValidButNotInThisPeriod:
      ov_ProcessedValidityInfo.status = e_Future;
      break;
    case e_ClyApp_ContractValid:
      ov_ProcessedValidityInfo.status = e_ValidForUse;
      break;
    default:
      ov_ProcessedValidityInfo.status = e_Undefined;

    }

    //up to here we didnt check bcpl because we wanted to get the contract processed data
    //if invalid set status
    if(st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card)
    {
      int iCurrPriority;
      //so if priority is invalid - set it in ov_ProcessedValidityInfo.status
      //pre: last event was already read
      iCurrPriority = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[1].e_EventBestContractPriorityListArr[i];

      ov_ProcessedValidityInfo.BCPL_Priority = (unsigned char)iCurrPriority;

      if(iCurrPriority==e_CardPriorityErasable ||
        iCurrPriority==e_CardPriorityInvalid)
      {
        ov_ProcessedValidityInfo.status = e_Invalid;

      }
    }


    //add ro array
    ContractsArr[i_num_contracts]=*pContractRecord;
    ProcessedInfoArr[i_num_contracts]=ov_ProcessedValidityInfo;
    i_num_contracts++;

  }

  *pNumOfContracts = i_num_contracts;
  return e_ClyApp_Ok;
}

/////////////////////////////////////////////////////////////////////////
//  Yoni 11/2011
//  clyApp_GetWhiteContractListOnCard
//
//  Pre:
/////////////////////////////////////////////////////////////////////////
eCalypsoErr clyApp_GetWhiteContractListOnCard(unsigned short* Bitmap/*out*/, unsigned short Codes[8]/*out*/)
{
  st_clyApp_ContractListStruct ov_Contractlist;
	eCalypsoErr OutConverError;
  //read existing
  eCalypsoErr err =e_Internal_Read(  e_ClyApp_Card,//[IN]type - card \ ticket
    1,  //[IN] //not relevat for ticket - record number to read - 1 is always the first record
    e_clyCard_ContractListFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
    (void*) &ov_Contractlist, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  if( err!=e_ClyApp_Ok)
    return err;

  //todo what if auth failed??


  *Bitmap = ov_Contractlist.us_Bitmap;
  memcpy(Codes, ov_Contractlist.ContractListAuthorizationCodeArr, 8*sizeof(short));

  return err;

}


// Check comm with client card
eCalypsoErr e_clyapp_CheckCardComm(void)
{

     RESPONSE_OBJ* Obj;

    if(st_Static_StateMachine.CardWriteMode != e_VirtualReadMode)  // When in virtual read mode there is no card ..
    {
        if(!_7816_CheckCardComm(st_Static_StateMachine.CardReaderId,&Obj))
        return e_ClyApp_NotOk;
    }

    return e_ClyApp_Ok;

}

// Check sam comm
eCalypsoErr e_clyapp_CheckSamComm(void)
{
  RESPONSE_OBJ* Obj;
  if(!_7816_CheckCardComm(st_Static_StateMachine.SamReaderId,&Obj))
    return e_ClyApp_NotOk;
  return e_ClyApp_Ok;

}



/////////////////////////////////////////////////////////////////////////
//  Yoni 11/2011
//  e_clyapp_IsValidEnv
//  Check env country,card end date, version (0)
//
/////////////////////////////////////////////////////////////////////////
eCalypsoErr e_clyapp_IsValidEnv(const st_ClyApp_EnvAndHoldDataStruct *stEnv)
{

  long l_DateCurrent,l_DateEnvEnd;
  st_Cly_Date st_Now;
  st_Cly_DateAndTime st_DateTimeNow={0};

  if(stEnv->sh_EnvCountryld != ISRAEL_COUNTRY_ISO_IDENTIFICATION)
    return e_ClyApp_CardEnvErr;
  if(stEnv->uc_EnvApplicationVersionNumber != 0 /*ENV_VERSION_NUM*/)
    return e_ClyApp_CardEnvErr;


  // Now:
  st_DateTimeNow = st_GetCurrentDateAndTime();
  //get current long date
  st_Now.Year = st_DateTimeNow.st_Date.Year;
  st_Now.Month = st_DateTimeNow.st_Date.Month ;
  st_Now.Day = st_DateTimeNow.st_Date.Day;
  l_DateCurrent = ush_GetDateCompact(&st_Now);
  l_DateEnvEnd = ush_GetDateCompact(&stEnv->st_EnvEndDate);

  //check if the card end date has not yet expired
  if( l_DateCurrent > l_DateEnvEnd )
    return e_ClyApp_CardEnvEndDateErr;

  return e_ClyApp_Ok;
}



#ifdef WIN32
/////////////////////////////////////////////////////////////////////////
//  Yoni 11/2011
//  e_WinReadSpecialEvents
//  For tester only. read special events
/////////////////////////////////////////////////////////////////////////
eCalypsoErr e_WinReadSpecialEvents(union_ClyApp_EventRecord SpecialEventsArr[SPECIAL_EVENTS_COUNT] /*OUT*/)
{
  RESPONSE_OBJ* Obj;
  int i;
  clyCard_BYTE ucp_RecDataOut[REC_SIZE];
  eCalypsoErr err;


  for (i=1; i <= SPECIAL_EVENTS_COUNT; i++)
  {
    Obj =  ClyApp_Virtual_ReadRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
      (char)i,//[IN] //record number to read - 1 is always the first record
      e_clyCard_SpecialEventFile, //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
      REC_SIZE,//[IN] len to read - 1 to record size
      ucp_RecDataOut,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts

    if( !IS_RES_OBJ_OK(Obj) )
      return e_ClyApp_CardReadErr;//return error only if ClyApp_Virtual_ReadRecord failed
    //=========================================
    //translate the bin data to API struct
    //=========================================
    err = e_Internal_Bit2Byte_Convert( e_BitStream2St,//[IN] convert direction - bit stream to struct OR struct to bit stream
      e_ClyApp_Card,//[IN]type - card \ ticket
      e_clyCard_SpecialEventFile,//[IN] if not a ticket  - which record in the card
      ucp_RecDataOut,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
      (void*)&SpecialEventsArr[i-1]);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
  }

  return e_ClyApp_Ok;//no matter the result of e_Internal_Bit2Byte_Convert

}

/////////////////////////////////////////////////////////////////////////////////
//  pre: b_clyapp_IsCardIn returned true


/////////////////////////////////////////////////////////////////////////////////

TR_BOOL b_WinReadCustomerCard(st_ClyApp_EnvAndHoldDataStruct* pEnvAndHoldDataStruct, //out
                              union_ClyApp_EventRecord      s_EventRecordArr[7],
                              union_ClyApp_ContractRecord   s_ContractRecordArr[MAX_CONTRACT_COUT], //out
							  st_clyApp_ContractListStruct*	pContractList //out
                           )
{
  clyApp_BOOL bIsEventOkArr[3];
  clyApp_BOOL bIsLock;

  if(st_Static_StateMachine.e_AppCardType != e_ClyApp_Card)
  {
    return TR_FALSE;//todo support ticket
  }


  if(e_ClyApp_GetEnvironment(pEnvAndHoldDataStruct)!=e_ClyApp_Ok)
  {
    printf("Failed to read env");
    return TR_FALSE;//failed to read env file
  }

  //check if locked
   e_ClyApp_IsLock(&bIsLock);//[OUT]1=Locked ,0=not locked
   if(bIsLock)
   {
      printf("Card locked");
   }


  if(e_clyapp_IsValidEnv(pEnvAndHoldDataStruct) == e_ClyApp_Ok)
  {
     //this is a client card with good env
      printf("Environment is ok!");
  }


   // read events
   if(e_ClyApp_Ok !=  e_ClyApp_SimpleGetAllEvent(s_EventRecordArr//
                        ,bIsEventOkArr)) //[OUT] indicate for each event if ok

   {
     printf("Failed to read events");
     return TR_FALSE;
   }

   // read contracts
   if(e_ClyApp_Ok !=  e_ClyApp_SimpleReadAllContracts(s_ContractRecordArr))
   {
     printf("Failed to read contracts");
     return TR_FALSE;
   }

   //special events, (4 last places in s_EventRecordArr)
   if(e_WinReadSpecialEvents(&s_EventRecordArr[3]) != e_ClyApp_Ok)
   {
     printf("Failed to read special events");
   }

   //contract list
   if(clyApp_GetWhiteContractListOnCard(&pContractList->us_Bitmap, pContractList->ContractListAuthorizationCodeArr) != e_ClyApp_Ok)
   {
	   printf("Failed to read contractlist");
   }

  return TR_TRUE;
}

#endif //win32

////////////////////////////////////////////////////////////////////////////
// bIsCardEmptyEnv
// 
// Return true if card has empty env
////////////////////////////////////////////////////////////////////////////

TR_BOOL b_clyapp_IsCardEmptyEnv(void)
{
    if(st_Static_StateMachine.b_IsCardEmptyEnv)
        return TR_TRUE;

    return TR_FALSE;
}

////////////////////////////////////////////////////////////////////////////
// bIsCardIn
// Yoni Hartmann 09/2011
// Detect card and check env and fill TR_st_CardInfo
// Return true if card detected
////////////////////////////////////////////////////////////////////////////

TR_BOOL b_clyapp_IsCardIn(TR_st_CardInfo* pInfo/*info*/)
{

    TR_BOOL RetVal = TR_FALSE;
    unsigned long lSerialNum;
    int res;
    char *pEnvByte = 0;
    int i,EnvSize = 0;
#ifndef ENABLE_COMM
    st_7816_CardResetInfo stp_CardResetInfo;
#endif
    // large static  structs to read temp data, instead of local which might cause stack overflow
    static st_ClyApp_EnvAndHoldDataStruct   s_TempEnvAndHoldDataStruct;
    static union_ClyApp_EventRecord         s_TempEventRecordArr[3];
    static union_ClyApp_ContractRecord      s_ContractRecordArr[MAX_CONTRACT_COUT];

#ifndef ENABLE_COMM
    if(!ContactlessDetect())
        return RetVal;
#endif

#ifdef ENABLE_COMM
	St_ClyApp_SmartCardData CardData;
	//Detect and read card data
	if(DetectCardAndReadData(&CardData) ==  TR_TRUE)//Send MSG e_CmdK10_ClyApp_IsCardIn
#else
    if(e_ClyApp_DetectCard(st_Static_StateMachine.CardReaderId)==e_ClyApp_Ok)
#endif
    {
        do
        {
#ifdef ENABLE_COMM

			//Upadte state machine  -  e_Internal_UpdateCardSateMachin
			res = UpdateStateMachine(&CardData);
	

            st_Static_StateMachine.b_IsCardEmptyEnv =  0; // Card empty flag reset   [eitan...]
#else
            memset(&stp_CardResetInfo,0,sizeof(st_7816_CardResetInfo));

            _7816_GetCardResetInfo((e_7816_DEVICE)st_Static_StateMachine.CardReaderId,//[IN] the reader id
                &stp_CardResetInfo);//[OUT] the card info

            res = e_ClyApp_StartWorkWithCard(st_Static_StateMachine.CardReaderId,//[IN] the reader ID in which to detect
                stp_CardResetInfo.cp_ClUid, /*[IN]*/ //The card SN
                stp_CardResetInfo.c_UidLen);//[IN] Serial Number Len
#endif
            if(res==e_ClyApp_Ok)
            {
            	clyApp_BOOL bIsLock;
#ifdef ENABLE_COMM
				st_Static_StateMachine.e_AppCardType = e_ClyApp_Card;
#endif
                // set  m_cardType
                if(st_Static_StateMachine.e_AppCardType == e_ClyApp_Card)
                    pInfo->m_cardType = enmSCard;
                else
                    pInfo->m_cardType = enmTicket;
#ifdef ENABLE_COMM
				lSerialNum = CardData.SerialNumber;
#else
                // set m_serialNnumber
                res = e_ClyApp_GetCardSn(st_Static_StateMachine.CardReaderId,&lSerialNum);
#endif
				if(res!=e_ClyApp_Ok)
					break; //failed to read Serial Number
				else
					sprintf((char*)pInfo->m_serialNumber, "%lu", lSerialNum);

                if(e_ClyApp_GetEnvironment(&s_TempEnvAndHoldDataStruct)!=e_ClyApp_Ok)
                    break;//failed to read env file

                // Check if empty
                pEnvByte    = (char*)&s_TempEnvAndHoldDataStruct;
                EnvSize     = sizeof(s_TempEnvAndHoldDataStruct);
                for( i = 0;i < EnvSize;i++)
                {
                   if(*pEnvByte != 0)
                    break;
                    pEnvByte++;
                }
                if(i == EnvSize)
                    st_Static_StateMachine.b_IsCardEmptyEnv =  1; // Card empty flag reset   

                //check if locked
                e_ClyApp_IsLock((clyApp_BOOL *)&bIsLock);//[OUT]1=Locked ,0=not locked
                pInfo->IsCardLock = bIsLock;

                if(e_clyapp_IsValidEnv(&s_TempEnvAndHoldDataStruct) == e_ClyApp_Ok)
                {
                    //this is a client card with good env

                    clyApp_BOOL bIsEventOkArr[3];
                    //pInfo->IsEnvOk = 1; determine later after checking that event1 is ok 
                    pInfo->PermissionType=enmClient;

                    // for client card we also read all regular events and all contract in order to
                    // prevent read errors
                    // read events
                    if(e_ClyApp_Ok != e_ClyApp_SimpleGetAllEvent(s_TempEventRecordArr,bIsEventOkArr)) //[OUT] indicate for each event if ok
                        break;

										if(bIsEventOkArr[0] == clyApp_TRUE)
										{
												pInfo->IsEnvOk = 1; //Yoni 02/2012
										}
										else
										{
												pInfo->IsEnvOk  = 0;
										}
#ifndef ENABLE_COMM
                    // read contracts
                    if(e_ClyApp_Ok != e_ClyApp_SimpleReadAllContracts(s_ContractRecordArr))
                        break;
#endif
                }
                else
                {
                	pInfo->IsEnvOk  = 0;
                    pInfo->PermissionType=enmUserUnknown;
                    //to find out PermissionType u need to call e_ReadMetronitUserData
                }

                RetVal = TR_TRUE;

            }      

        } while(0);
#ifdef ENABLE_COMM
    if(RetVal == TR_FALSE)
        v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
#endif
    }
#ifndef ENABLE_COMM
    if(RetVal == TR_FALSE)
        v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
#endif
    return RetVal;
}


void clyapp_ForgetCard()
{
  e_ClyApp_ReleaseCard(st_Static_StateMachine.CardReaderId);
}

//static st_Cly_DateAndTime st_GetCurrentDateAndTime(void)

//10/2011
clyApp_BOOL clyapp_bIsProfileValid(int ProfileCode)
{
    st_Time ProfTime,Now;
  int ProfCode1,ProfCode2;
  st_ClyApp_HoiderProf *Hoider = 0;
  st_Cly_DateAndTime DtNow;
  long l;


  if( !st_Static_StateMachine.st_TransactionData.b_IsEnvRecExist)
    return clyApp_FALSE;


    memset(&ProfTime,0,sizeof(st_Time));
    memset(&Now,0,sizeof(st_Time));

  ProfCode1 = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec.st_HoiderProf1.uc_HoiderProfCode;
  ProfCode2 = st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec.st_HoiderProf2.uc_HoiderProfCode;


  if(ProfileCode == ProfCode1)
    Hoider = &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec.st_HoiderProf1;
  if(ProfileCode == ProfCode2)
    Hoider = &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EnvRec.st_HoiderProf2;
  if(ProfileCode == 0)
    return clyApp_TRUE; // Profile 0 will not be checked for validity
    if(!Hoider)
    return clyApp_FALSE; // Requested profile was not found on card

  DtNow = st_GetCurrentDateAndTime();

  l = l_Internal_DateCmp(&Hoider->st_HoiderProfDate, &DtNow.st_Date);

  if(l < 0)
    return clyApp_FALSE;

//  trace("Error: profile %d is invalid!",ProfileCode);
    return clyApp_TRUE;
}



//////////////////////////////////////////////////////////////////////////////////////////////
// Yoni 
// e_GetInterchangeType
//////////////////////////////////////////////////////////////////////////////////////////////
e_InterchangeType e_GetInterchangeType(const st_clyApp_CardContractIssuingData* pContractIssuingData)
{
    /*
	e_NoInterchange=0,
	e_OneHemshech=1,
	e_TwoHemshech=2,
	e_Maavar=9,  
    */   
    if(pContractIssuingData->b_ContractIsJourneylnterchangesAllowed)
    {
        //maavar or hemshech?
        //decide according to Contract Period Journeys
        if(pContractIssuingData->st_OptionalContractData.b_ContractPeriodJourneysExist)
        {
           if(e_PeriodKartisiaHemshech == pContractIssuingData->st_OptionalContractData.st_ContractPeriodJourneys.e_PeriodType)
           {
             if(2 == pContractIssuingData->st_OptionalContractData.st_ContractPeriodJourneys.uc_MaxNumOfTripsInPeriod)   
								return e_OneHemshech;     
						 else 
								return e_TwoHemshech;
           }  
        }
        return e_Maavar;

    }
    return e_NoInterchange;
}



///// user card
//the following functions handle issue and read of user card


#include <Core.h>

//defined in object files nor in code
//defined in object files nor in code
extern const unsigned char user_key1[16];
extern const unsigned char user_key2[16];

void  Xor8Bytes(unsigned char In1[8], unsigned char In2[8],unsigned char Out[8])
{
 int i;
 for(i=0;i<8;i++)
  Out[i]=In1[i]^In2[i];
}

void  Xor16Bytes(const unsigned char In1[16], const unsigned char In2[16],unsigned char Out[16])
{
 int i;
 for(i=0;i<16;i++)
  Out[i]=In1[i]^In2[i];
}

static void My_Des3CBC(unsigned char Key[16],int DataSize,unsigned char *PlanTxt,unsigned char *KeyDiverSifyResult)
{
  int i;
  unsigned char InitBlock[8];
  unsigned char tmp[8];
  memset(InitBlock,0,sizeof(InitBlock));

  for(i=0;i<DataSize/8;i++)
  {
    Xor8Bytes(InitBlock,PlanTxt+i*8,tmp);

    des3_encipher(tmp,DataSize,Key,InitBlock);

    memcpy(KeyDiverSifyResult+i*8,InitBlock,8);
  }

}

static void my_des_sign(
  unsigned char *plaintext, // [IN] ciphertext
  int lenDataIn,           // [IN] the data length
  unsigned char *key,        //[IN]  key
  unsigned char *Signiture //  [OUT]  Signiture
)

{
  int i;
  unsigned char InitValue[8],tmp[8];

  // chek lentgh validity
   if((lenDataIn%8)!=0)
    return;
  // init initvalue to ZERO
  memset(InitValue,0,8);

  // for all  8 bytes  blocks
  for(i=0;i<lenDataIn/*/8*/;i+=8)
  {
  // XOR the plan text with the previus result or at first with the InitValue
    Xor8Bytes(&plaintext[i],InitValue,tmp);

    // encrypt the xor result with the key and put it in the InitValue
    des_encipher(tmp,InitValue,key);

  }
   // modify Signiture
   memcpy(Signiture,InitValue,8);
}

void my_des3_sign(
  unsigned char *plaintext, // [IN] ciphertext
  int lenDataIn,           // [IN] the data length
  unsigned char *key,        //[IN]  key
  unsigned char *Signature //  [OUT]  Signiture
  )
 {
 unsigned char Tmp[8];
 // chek lentgh validity
 if((lenDataIn%8)!=0)
    return;
 // do the mac  into Signiture
 my_des_sign(plaintext,lenDataIn,key,Signature);


 // do des -1  with the right key
 des_decipher(Signature,Tmp,key+8);
 // do des   with the left key
 des_encipher(Tmp,Signature,key);

 }
//#ifndef TIM7020
//lrc calculation
char cGetLRC(const char* src,unsigned int len)
{
    unsigned char lrc=0xff;
    unsigned int i;

    for(i=0;i<len;i++)
        lrc^=src[i];
    return lrc;
}
//#else
//char IsLrcIOk(void *Data,int len)//,unsigned char Lrc)
//{
//    unsigned char Lrc=0;
//    unsigned char *ptr=(unsigned char *)Data;
//    int i;
//
//    for(i=0;i<len;i++)
//        Lrc^=ptr[i];
//
//    return Lrc==0;
//
//
//}
//#endif
//#ifndef TIM7020
//return des3 signature on user data
static void v_CreateUserDataSignature(unsigned char Key[16], unsigned char cpSerialNumber[8], const stUserData* pUserData, unsigned char signature[8])
{
  unsigned char buff[16]={0};
  int data_len;

  //create diversify
  unsigned char keyResult[16]={0};

  //create user data key
  unsigned char user_data_key[16]={0};
  Xor16Bytes(user_key1,user_key2,user_data_key);

  My_Des3CBC(user_data_key,
        8,
        cpSerialNumber,
        keyResult);


   //copy stUserData to buffer without cp_Signature
    data_len = (long)&pUserData->cp_Signature[0]-(long)pUserData;
  if(data_len>16)
    data_len=16;
  memcpy(buff, pUserData, data_len);
  //now buff contains pUserData without cp_Signature field,  and padded with 0's, and size%8==0

  //now calc signature
  my_des3_sign(buff, sizeof(buff), keyResult, signature);
}
//#endif



static void v_EncryptPassword(long password, unsigned char cpResult[8]/*out*/)
{

  const unsigned char base[8]={0xa5};
  //DIVERSIFY [user_data_key] with [password] and encrypt
  //1. create diversify
  unsigned char keyResult[16]={0};
  unsigned char padded_pwd[8]={0};

  //create user data key
  unsigned char user_data_key[16]={0};
  Xor16Bytes(user_key1,user_key2,user_data_key);

  memcpy(padded_pwd, &password, sizeof(password));
  My_Des3CBC(user_data_key,
        sizeof(padded_pwd),
        padded_pwd,
        keyResult);

  //2. encrpypt
  my_des3_sign((unsigned char*)base, sizeof(base), keyResult, cpResult);

}



eCalypsoErr e_ReadUserData(stUserData* pUserData/*out*/)
{
//   RESPONSE_OBJ* Obj;
   clyCard_BYTE ucp_RecDataOut[REC_SIZE];
   char lrc;
   st_ClyApp_EnvAndHoldDataStruct st_EnvAndHoldDataStruct;
   unsigned char signature[8];

   eCalypsoErr err;
   eCalypsoErr OutConverError;

   //create user data key
   unsigned char user_data_key[16]={0};
   Xor16Bytes(user_key1,user_key2,user_data_key);

  err =e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
	(clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
	e_clyCard_EnvironmentFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
	&st_EnvAndHoldDataStruct, &OutConverError, 0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  if( err!=e_ClyApp_Ok)
	return err;
  //copy state machine to ucp_RecDataOut
  memcpy(ucp_RecDataOut, st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EnvRec, sizeof(ucp_RecDataOut));

  memcpy(pUserData, ucp_RecDataOut, sizeof(*pUserData));

  //check lrc
  lrc = cGetLRC((const char*)pUserData,(sizeof(*pUserData)-1));
  if(lrc != pUserData->lrc){
	  return e_ClyApp_NotOk;//not user card or data error
  }
  //CHECK SIGNATURE
  v_CreateUserDataSignature(user_data_key, st_Static_StateMachine.ucp_CardSn, pUserData, signature);
  if(memcmp(pUserData->cp_Signature, signature, sizeof(pUserData->cp_Signature)))
  {
	//bad signature or not a user card
    return e_ClyApp_NotOk;
  }
//#endif
    return e_ClyApp_Ok;
}


//return e_ClyApp_Ok if password is ok
eCalypsoErr e_VerifyPassword(long Pswrd)
{
#ifndef TIM7020
   stUserData stUserData;
   //read user data
   if(e_ReadMetronitUserData(&stUserData)==e_ClyApp_Ok)
   {
    //now check the password
    unsigned char enc_pwd[8];
    v_EncryptPassword(Pswrd, enc_pwd/*out*/);

    if(memcmp(enc_pwd, stUserData.Password, sizeof(stUserData.Password)) == 0)
    {
      return e_ClyApp_Ok;
    }

    
   }
   return e_ClyApp_NotOk;//todo
#else
   return e_ClyApp_Ok;//todo
#endif 
}

#ifndef TIM7020
eCalypsoErr e_ReadInspectionData(TR_st_InspectionData* pInspectionData)
{
    //RESPONSE_OBJ* Obj;
		eCalypsoErr OutConverError;
    eCalypsoErr err;
    //clyCard_BYTE ucp_RecDataOut[REC_SIZE];
		unsigned  char lrc;
    st_clyApp_CardEventDataStruct st_CardEventDataStruct;
    //if data already exists, dont read again 

//    Obj =  ClyApp_Virtual_ReadRecord(st_Static_StateMachine.CardReaderId,
//                                      1,
//                                      e_clyCard_EventLogFile,
//                                      REC_SIZE,
//                                      ucp_RecDataOut);
//
//    if( !IS_RES_OBJ_OK(Obj) )
//      return e_ClyApp_CardReadErr;//return error only if ClyApp_Virtual_ReadRecord failed

    //Yoni 07/2012
   err =e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
            1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
            e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
            &st_CardEventDataStruct, &OutConverError); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
   if( err!=e_ClyApp_Ok)
    return err;

  memcpy(pInspectionData, &st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[1], sizeof(*pInspectionData));
  //check lrc
  lrc = cGetLRC((const char*)pInspectionData,(sizeof(*pInspectionData)-1));
  {
    if(lrc != pInspectionData->lrc)
      return e_ClyApp_NotOk;//data error
  }

  return e_ClyApp_Ok;
}

eCalypsoErr e_WriteInspectionData(TR_st_InspectionData* pInspectionData)
{
    RESPONSE_OBJ* Obj;
    eCalypsoErr err;
    e_clyCard_KeyType e_KeyType;
    St_clySam_KIF_And_KVC St_KIF_And_KVC;


    pInspectionData->lrc = cGetLRC((const char*)pInspectionData,(sizeof(*pInspectionData)-1));

    e_KeyType = e_clyCard_KeyDebit;

    v_Internal_GetKifVal(e_KeyType,&St_KIF_And_KVC);

    err = ClyApp_Virtual_OpenSecureSession( St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
                                        e_KeyType,//[IN] Key Type to use for the session
                                        NULL); //[IN]read record not requested send NULL
    if( err!=e_ClyApp_Ok)
        return err;
  //now write the record
    Obj =  ClyApp_Virtual_UpdateRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
                                        1,//[IN]
                                        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                        REC_SIZE,//[IN] len to read - 1 to record size
                                        (unsigned char*)pInspectionData); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( !IS_RES_OBJ_OK(Obj) )
        return e_ClyApp_CardWriteErr;


    Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
                                              st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
                                              clyCard_FALSE);//[IN] //1= the session will be immediately Ratified
    if( !IS_RES_OBJ_OK(Obj) )
        return e_ClyApp_CardWriteErr;

    return e_ClyApp_Ok;

}
#endif
//////////////////////////////////////////////////////////////////////////////
#ifndef TIM7020
//#ifdef ISSUING_STATION
extern const unsigned char chlng_key_for_issue1[16];
extern const unsigned char chlng_key_for_issue2[16];
static struct
{
  unsigned char cpSeed[8];
  long g_Tick;
}stIssueStationGLobals={{'9','7','3','5','a','B','c','D'},0};//give initial value to seed

///todo comments
static void build_plain_chlng(unsigned char buff[16])
{
  memcpy(buff, &stIssueStationGLobals.g_Tick ,4);
  memcpy(buff+4, stIssueStationGLobals.cpSeed,8);
  memcpy(buff+12, st_Static_StateMachine.ucp_CardSn, 4);
}

//return random 8 bytes
//logic: do des3_sign(seed+sys_tick+card_sn)
eCalypsoErr e_clyapp_GetChallenge(unsigned char cp_random[8]/*out*/)
{

  unsigned char buff[16]={0};
  //create chlng key by xor
  unsigned char chlng_key_for_issue[16];
  Xor16Bytes(chlng_key_for_issue1,chlng_key_for_issue2,chlng_key_for_issue);

  stIssueStationGLobals.g_Tick = CoreGetTickCount();
  build_plain_chlng(buff);
  my_des3_sign(buff, sizeof(buff), chlng_key_for_issue, cp_random);
  //save random in seed
  memcpy(stIssueStationGLobals.cpSeed, cp_random, sizeof(stIssueStationGLobals.cpSeed));
  return e_ClyApp_Ok;
}

//compare current plain chlng with the one decrypted by host
eCalypsoErr e_clyapp_CheckChallenge(const TR_CHLNG encrypted_chlng)
{
  unsigned char buff[8]={0};
  //create chlng key by xor
  unsigned char chlng_key_for_issue[16];
  Xor16Bytes(chlng_key_for_issue1,chlng_key_for_issue2,chlng_key_for_issue);  my_des3_sign(stIssueStationGLobals.cpSeed, sizeof(stIssueStationGLobals.cpSeed), chlng_key_for_issue, buff);

  //check that host gave exact chlng
  if(memcmp(encrypted_chlng, buff, sizeof(encrypted_chlng)) == 0)
  {
    return e_ClyApp_Ok;
  }
  return e_ClyApp_NotOk;
}
#endif
////////////////////////////////////////////////////////////////////////////
//  Yoni 10/2011
//write user card data on issue
////////////////////////////////////////////////////////////////////////////
eCalypsoErr e_WriteUserData(long lv_password, long id, short type, const TR_CHLNG Chlng)
{
#ifndef TIM7020
    RESPONSE_OBJ* Obj;
    eCalypsoErr err;

    e_clyCard_KeyType e_KeyType;
    St_clySam_KIF_And_KVC St_KIF_And_KVC;
    unsigned char signature[8];
  stUserData ov_UserData={0};
  unsigned char cp_env[REC_SIZE]={0};

  //create user data key
  unsigned char user_data_key[16]={0};
  Xor16Bytes(user_key1,user_key2,user_data_key);


  //check challenge
#ifndef CALYPSO_BYPASS_ENCRYPTION

    if(e_clyapp_CheckChallenge(Chlng) != e_ClyApp_Ok)
  {
    return e_ClyApp_NotOk;
  }
#endif

  //1. set id and type
  ov_UserData.lv_UserId = id;
  ov_UserData.iv_UserType = type;


  //2. set encrypted password
  v_EncryptPassword(lv_password, (unsigned char*)ov_UserData.Password/*out*/);

  //3. calc signature. without correct signature the card isn't valid for use
  v_CreateUserDataSignature(user_data_key, st_Static_StateMachine.ucp_CardSn, &ov_UserData, signature);
  memcpy(ov_UserData.cp_Signature, signature, sizeof(ov_UserData.cp_Signature));

  //4. now put lrc at last byte
  ov_UserData.lrc = cGetLRC((const char*)&ov_UserData,(sizeof(ov_UserData)-1));

  //now we need to write it to the card env file
  memcpy(cp_env, &ov_UserData, sizeof(ov_UserData));

    e_KeyType = e_clyCard_KeyIssuer;


    v_Internal_GetKifVal(e_KeyType,&St_KIF_And_KVC);


    err = ClyApp_Virtual_OpenSecureSession( St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
                                        e_KeyType,//[IN] Key Type to use for the session
                                        NULL); //[IN]read record not requested send NULL
    if( err!=e_ClyApp_Ok)
        return err;
  //now write the record
    Obj =  ClyApp_Virtual_UpdateRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
                                        1,//[IN]
                                        e_clyCard_EnvironmentFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                        REC_SIZE,//[IN] len to read - 1 to record size
                                        cp_env); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( !IS_RES_OBJ_OK(Obj) )
        return e_ClyApp_CardWriteErr;


    Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
                                              st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
                                              clyCard_FALSE);//[IN] //1= the session will be immediately Ratified
    if( !IS_RES_OBJ_OK(Obj) )
        return e_ClyApp_CardWriteErr;

#endif
    return e_ClyApp_Ok;


}



////////////////////////////////////////////////////////////////////////////

//Remote Sam support

////////////////////////////////////////////////////////////////////////////
#ifndef TIM7020  //TBD:yoram
eCalypsoErr e_clyappCardInOut(PACKET_7816* p_7816_Packet/*in*/,
                              RESPONSE_OBJ* p_7816_ResponseBuff/*out*/,
                              unsigned short timeout
                              )
{

  RESPONSE_OBJ* ans=_7816_CardInOut(p_7816_Packet,(e_7816_DEVICE)st_Static_StateMachine.SamReaderId, timeout);

  memcpy(p_7816_ResponseBuff, ans, sizeof(*p_7816_ResponseBuff));

  return e_ClyApp_Ok;
}
#endif
////////////////////////////////////////////////////////////////////////////
//
//FUNCTION: ClyApp_Virtual_CloseSecureSession
//DESCRITION:
//
////////////////////////////////////////////////////////////////////////////

RESPONSE_OBJ* CLY_CARD_STDCALL  ClyApp_Virtual_CloseSecureSession(e_7816_DEVICE CardReaderId, // [IN]  card reader id
                                  e_7816_DEVICE SamReaderId,  // [IN]  sam reader id
                                  clyCard_BOOL b_IsRatifyImmediatly // [IN] //1= the session will be immediately Ratified
                                  )
{

    static RESPONSE_OBJ VirtualResponse;

    if(st_Static_StateMachine.CardWriteMode == e_NormalMode)
        return pSt_ClySession_CloseSecureSession(CardReaderId,SamReaderId,b_IsRatifyImmediatly);


    VirtualResponse.sw1_sw2[0] = 0x90;
    VirtualResponse.sw1_sw2[1] = 0;
    st_Static_StateMachine.e_TransactionState=e_clyApp_SessionCloseOk;
    return &VirtualResponse;
}

////////////////////////////////////////////////////////////////////////////
//
//FUNCTION: ClyApp_Virtual_OpenSecureSession
//DESCRITION:
//
////////////////////////////////////////////////////////////////////////////

static eCalypsoErr ClyApp_Virtual_OpenSecureSession(St_clySam_KIF_And_KVC  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
                          e_clyCard_KeyType KeyType, //[IN] Key Type to use for the session
                          st_ClyApp_EnvAndHoldDataStruct *stp_EnvAndHoldDataStruct)
{


    if(st_Static_StateMachine.CardWriteMode == e_NormalMode)
        return e_Internal_OpenSecureSession(St_KIF_And_KVC,KeyType, stp_EnvAndHoldDataStruct);

    st_Static_StateMachine.e_TransactionState=e_clyApp_SessionOpenOk;
    return e_ClyApp_Ok;
}

////////////////////////////////////////////////////////////////////////////
//
//FUNCTION: ClyApp_Virtual_Invalidate
//DESCRITION:
//
////////////////////////////////////////////////////////////////////////////

static RESPONSE_OBJ* CLY_CARD_STDCALL ClyApp_Virtual_Invalidate(e_7816_DEVICE ReaderId)
{

    static RESPONSE_OBJ VirtualResponse;

    if(st_Static_StateMachine.CardWriteMode == e_NormalMode)
        return pSt_ClyCard_Invalidate(ReaderId);


    VirtualResponse.sw1_sw2[0] = 0x90;
    VirtualResponse.sw1_sw2[1] = 0;
    return &VirtualResponse;

}

////////////////////////////////////////////////////////////////////////////
//
//FUNCTION:       ClyApp_Virtual_GetSet
//DESCRITION:
//
////////////////////////////////////////////////////////////////////////////

static void ClyApp_Virtual_GetSet(clyCard_BYTE RecNum,
                     e_clyCard_FileId FileToSelect,
                     clyCard_BYTE Len,
                     clyCard_BYTE *RecData,
                                         clyCard_BYTE IsSet)
{

    unsigned char *pTarget = 0,*pCountersTarget = 0;

  switch(FileToSelect)
  {

  case e_clyCard_EnvironmentFile:
    {
      if(IsSet)
                st_Static_StateMachine.CardVirtualImage.Flag_EnvironmentData = VIRTUAL_CARD_VALID_FLAG;
       pTarget = (unsigned char*)&st_Static_StateMachine.CardVirtualImage.ucp_EnvironmentData;
    }break;

  case e_clyCard_ContractsFile:
    {

            if(RecNum < MAX_CONTRACT_COUT)
            {

        if(IsSet)
          st_Static_StateMachine.CardVirtualImage.Flag_ContractData[RecNum] = VIRTUAL_CARD_VALID_FLAG;
            pTarget = (unsigned char*)&st_Static_StateMachine.CardVirtualImage.ucp_ContractData[RecNum];

            }
    }break;

  case e_clyCard_CountersFile:
    {
      if(IsSet)
            {
                st_Static_StateMachine.CardVirtualImage.Flag_CounterData = VIRTUAL_CARD_VALID_FLAG;

                // Read current counter (all 29 bytes)
                pCountersTarget = (unsigned char*)&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec;
                pTarget         = (unsigned char*)&st_Static_StateMachine.CardVirtualImage.ucp_Counter;

                memcpy(pTarget,pCountersTarget,29);
                memcpy(pTarget,RecData,Len);   // Overwrite and update the counters
                memcpy(pCountersTarget,pTarget,29); // Update internal calypso state
                return; // Special case for counters
            }
            else
                pTarget = (unsigned char*)&st_Static_StateMachine.CardVirtualImage.ucp_Counter;
    }break;

  case e_clyCard_EventLogFile:
    {
      if(RecNum < MAX_EVENT_COUT)
      {
        if(IsSet)
                    st_Static_StateMachine.CardVirtualImage.Flag_EventData[RecNum] = VIRTUAL_CARD_VALID_FLAG;
            pTarget = (unsigned char*)&st_Static_StateMachine.CardVirtualImage.ucp_Event[RecNum];
      }
    }break;
  }

    if(IsSet) // Write te virtual image to the state machine
    {
        if(pTarget)
            memcpy(pTarget,RecData,Len);
  }
    else  // Read the virtual image data
    {

        if(pTarget)
          memcpy(RecData,pTarget,Len);
    }


}

////////////////////////////////////////////////////////////////////////////
//
//FUNCTION:       ClyApp_Virtual_UpdateRecord
//DESCRITION:
//
////////////////////////////////////////////////////////////////////////////

static RESPONSE_OBJ* CLY_CARD_STDCALL ClyApp_Virtual_UpdateRecord  (e_7816_DEVICE ReaderId,     // [IN] reader id
                                  clyCard_BYTE RecNum,                // [IN] //record number to Write - set to 1 for cyclic or counter EF
                                  e_clyCard_FileId FileToSelect,      // [IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                  clyCard_BYTE Len2Update,            // [IN] len to Write - 1 to record size
                                  clyCard_BYTE *RecData2Update)       // [IN] data to Update - in case len<Full recode size, the record will be padded with zeroes
{

    static RESPONSE_OBJ VirtualResponse;



    switch(st_Static_StateMachine.CardWriteMode)
    {

  case  e_NormalMode:
    {
      // Call the real ClyCard API when we are not in virtual mode
      return pSt_ClyCard_UpdateRecord(ReaderId,RecNum,FileToSelect,Len2Update,RecData2Update);







    }

        ////////////////////////////////////////////////////////////////

  case e_VirtualWriteMode:
        {

            // Store the info
            ClyApp_Virtual_GetSet(RecNum, FileToSelect,Len2Update, RecData2Update,1);
            VirtualResponse.sw1_sw2[0] = 0x90;
            VirtualResponse.sw1_sw2[1] = 0;

            return &VirtualResponse;
    }

        ////////////////////////////////////////////////////////////////

  case e_VirtualReadMode:
        {
            // Error
      VirtualResponse.sw1_sw2[0] = 0x6a;
      VirtualResponse.sw1_sw2[1] = 0x82; // File not found error
      return &VirtualResponse;
        }

    }

    return &VirtualResponse;
}

////////////////////////////////////////////////////////////////////////////
//
//FUNCTION:       ClyApp_Virtual_ReadRecord
//DESCRITION:
//
////////////////////////////////////////////////////////////////////////////

static RESPONSE_OBJ* CLY_CARD_STDCALL  ClyApp_Virtual_ReadRecord(e_7816_DEVICE ReaderId, // [IN]  reader id
  clyCard_BYTE RecNum,                 // [IN] //record number to read - 1 is always the first record
  e_clyCard_FileId FileToSelect,       // [IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
  clyCard_BYTE Len2Read,               // [IN] len to read - 1 to record size
  clyCard_BYTE RecDataOut[REC_SIZE],   // [OUT] data read - this data need casting in the application layer accurding to the application data strucuts
  clyCard_BYTE ForceRead)
{

    static RESPONSE_OBJ VirtualResponse;
  RESPONSE_OBJ *Obj;

    // Default to ok response
    VirtualResponse.sw1_sw2[0] = 0x90;
    VirtualResponse.sw1_sw2[1] = 0;

    switch(st_Static_StateMachine.CardWriteMode)
    {

  case e_NormalMode:
        {
            // Call the real ClyCard API when we are not in virtual mode
            return pSt_ClyCard_ReadRecord(ReaderId,RecNum,FileToSelect,Len2Read,RecDataOut,ForceRead);









        }
    case e_VirtualWriteMode:
    {
      // Call the real ClyCard API when we are not in virtual mode
      Obj = pSt_ClyCard_ReadRecord(ReaderId,RecNum,FileToSelect,Len2Read,RecDataOut,ForceRead);
            if(IS_RES_OBJ_OK(Obj))
            {
               if(FileToSelect == e_clyCard_EventLogFile)
                    ClyApp_Virtual_GetSet(RecNum, FileToSelect,Len2Read, RecDataOut,1);

            }
            return Obj;

    }
  case e_VirtualReadMode:
        {
               // Read from the virtual data (according to the validity flag)
               ClyApp_Virtual_GetSet(RecNum,FileToSelect,Len2Read,RecDataOut,0);

        }break;

    }

    return &VirtualResponse;


}




////////////////////////////////////////////////////////////////////////////
//
//FUNCTION: e_ClyApp_Virtual_GetCardImage
//DESCRITION:
//
////////////////////////////////////////////////////////////////////////////

void e_ClyApp_Virtual_GetCardImage(st_ClyApp_TransactionVirtualData *TransactionBinData)
{

    memcpy(TransactionBinData,&st_Static_StateMachine.CardVirtualImage,sizeof(st_ClyApp_TransactionVirtualData));

}

////////////////////////////////////////////////////////////////////////////
//
//FUNCTION: e_ClyApp_Virtual_SetCardImage
//DESCRITION:
//
////////////////////////////////////////////////////////////////////////////

void e_ClyApp_Virtual_SetCardImage(st_ClyApp_TransactionVirtualData *TransactionBinData)
{

    memcpy(&st_Static_StateMachine.CardVirtualImage,TransactionBinData,sizeof(st_ClyApp_TransactionVirtualData));


}
////////////////////////////////////////////////////////////////////////////
//
//FUNCTION:       e_ClyApp_Virtual_GetRecord
//DESCRITION:
//
////////////////////////////////////////////////////////////////////////////

eCalypsoErr e_ClyApp_Virtual_GetRecord(clyCard_BYTE RecNum,
                     e_clyCard_FileId FileToSelect,
                     clyCard_BYTE *RecData2Update,
                                         clyCard_BYTE BytesToCopy)
{

     CalypsoFileType *pTarget = 0;

   switch(FileToSelect)
   {

   case e_clyCard_EnvironmentFile:
    {
      pTarget = (CalypsoFileType*)&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EnvRec;
    }break;

  case e_clyCard_ContractsFile:
    {
        pTarget = (CalypsoFileType*)&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_ContractRecArr[RecNum];
    }break;

  case e_clyCard_CountersFile:
    {
        pTarget = (CalypsoFileType*)&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec;
    }break;

  case e_clyCard_EventLogFile:
    {
        pTarget = (CalypsoFileType*)&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_EventRecArr[RecNum];
    }break;
  }

    if(pTarget)

    {
        memcpy(RecData2Update,pTarget,BytesToCopy);
        return e_ClyApp_Ok;
    }

    return e_ClyApp_NotOk;
}

////////////////////////////////////////////////////////////////////////////
//
// e_ClyApp_Virtual_IsCardIn
//
////////////////////////////////////////////////////////////////////////////

static TR_BOOL e_ClyApp_Virtual_SetCardIn(TR_st_CardInfo* pInfo)
{


    //large static  structs to read temp data, instead of local which might cause stack overflow
  static st_ClyApp_EnvAndHoldDataStruct       s_TempEnvAndHoldDataStruct;
    static union_ClyApp_EventRecord             s_TempEventRecordArr[3];
  static union_ClyApp_ContractRecord          s_ContractRecordArr[MAX_CONTRACT_COUT];

  st_Static_StateMachine.CardReaderId           = e_7816_CONTACTLESS;
  st_Static_StateMachine.e_AppCardType        = e_ClyApp_Card;
  st_Static_StateMachine.e_TransactionState   = e_clyApp_CardExist;
  st_Static_StateMachine.e_7816CardType       = e_7816_Cly_CD21;
    st_Static_StateMachine.uc_CardSnLen         = sizeof(st_Static_StateMachine.ucp_CardSn);
  memcpy(st_Static_StateMachine.ucp_CardSn,pInfo->m_serialNumber,st_Static_StateMachine.uc_CardSnLen);


  if(pInfo) // If a valid pointer
  {
    clyApp_BOOL bIsLock;

    if(e_ClyApp_GetEnvironment(&s_TempEnvAndHoldDataStruct)!=e_ClyApp_Ok)
      return TR_FALSE;//failed to read env file

    //check if locked
    e_ClyApp_IsLock((clyApp_BOOL *)&bIsLock);//[OUT]1=Locked ,0=not locked
    pInfo->IsCardLock = bIsLock;

    if(e_clyapp_IsValidEnv(&s_TempEnvAndHoldDataStruct) == e_ClyApp_Ok)
    {
      //this is a client card with good env

      clyApp_BOOL bIsEventOkArr[3];
      pInfo->IsEnvOk = 1;
      pInfo->PermissionType=enmClient;

      // for client card we also read all regular events and all contract in order to
      // prevent read errors
      // read events
      if(e_ClyApp_Ok != e_ClyApp_SimpleGetAllEvent(s_TempEventRecordArr//[IN]Array memory allocation of events to fill
        ,bIsEventOkArr)) //[OUT] indicate for each event if ok

      {
        return TR_FALSE;
      }
      // read contracts
      if(e_ClyApp_Ok != e_ClyApp_SimpleReadAllContracts(s_ContractRecordArr))
      {
        return TR_FALSE;
      }
    }
    else
    {
      pInfo->IsEnvOk  = 0;
      pInfo->PermissionType=enmUserUnknown;
      //to find out PermissionType u need to call e_ReadMetronitUserData
    }

    return TR_TRUE;

  }
    else
    {
    // Could not read card, forget it
    v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);

    }

  return TR_FALSE;

}


////////////////////////////////////////////////////////////////////////////
//
//FUNCTION: Virtual card write suuport
//DESCRITION:
//
////////////////////////////////////////////////////////////////////////////

eCalypsoErr e_ClyApp_Virtual_SetCalypsoMode(TR_st_CardInfo* pInfo,e_CardWriteMode eWriteMode)
{

    // Note: calypso will change mode only when the current mdde is normal, this is to prevent a mode switch
    // while a function has allready selected a virtual read \ write mode
    if(st_Static_StateMachine.CardWriteMode == e_NormalMode)
    {

        st_Static_StateMachine.CardWriteMode = eWriteMode;

        if(eWriteMode ==  e_VirtualReadMode)
        {
            // Update clyapp anf make it think that it has a card
            if(e_ClyApp_Virtual_SetCardIn(pInfo) != TR_TRUE)
            {
                st_Static_StateMachine.CardWriteMode = e_NormalMode;
                return e_ClyApp_NotOk;
            }

        }
        else // Reset buffers in virtual write mode
            memset(&st_Static_StateMachine.CardVirtualImage,0,sizeof(st_ClyApp_TransactionVirtualData));  // Virtual data card cleanup

        return e_ClyApp_Ok;
    }

    return e_ClyApp_NotOk;

}

////////////////////////////////////////////////////////////////////////////
//
//FUNCTION:       e_ClyApp_UpdateCardEnvAndHoldDataRec
//DESCRITION:     Perform Personalization = write personal data
//
////////////////////////////////////////////////////////////////////////////

eCalypsoErr CLYAPP_STDCALL e_ClyApp_Virtual_UpdateCardEnvAndHoldDataRec (clyApp_BOOL BCPLReset, st_ClyApp_EnvAndHoldDataStruct* stp_EnvAndHoldData)//[IN]Environment data to write
{

    RESPONSE_OBJ*                   Obj;
    eCalypsoErr                     err;
    e_clyCard_KeyType               e_KeyType;
    St_clySam_KIF_And_KVC           St_KIF_And_KVC;
    st_clyApp_CardEventDataStruct   stp_CardEventDataStruct;
    st_Cly_DateAndTime              stp_TimeReal;
    st_ClyApp_EnvAndHoldDataStruct  st_CurrentEnvAndHoldData;
    long                            l;

    stp_TimeReal = st_GetCurrentDateAndTime();

    CHECK_INERFACE_INIT();
  CHECK_CARD_EXIST();


  ////////////////////////////////////////////////////////////////////////////
  //
  // Check card type  - if card
  //
  ////////////////////////////////////////////////////////////////////////////

  if( st_Static_StateMachine.e_AppCardType ==  e_ClyApp_Card )
  {
		eCalypsoErr OutConverError;

    ////////////////////////////////////////////////////////////////////////////
    //
    // read two last history files ( since they need to be send
    // back to the user in the end of the transaction -
    // third record will be written by the use operation )
    //
    ////////////////////////////////////////////////////////////////////////////


    // Read event Record 1

    err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
      (clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &stp_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
      //if record can not be read - exist
      return err;

    // Read event Record 2

    err =e_Internal_Read( e_ClyApp_Card,//[IN]type - card \ ticket
      (clyCard_BYTE)2,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      (void*) &stp_CardEventDataStruct, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
      //if record can not be read - exist
      return err;

    // Read Current Environmen Record

    err =e_Internal_Read(e_ClyApp_Card,//[IN]type - card \ ticket
      (clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_EnvironmentFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      &st_CurrentEnvAndHoldData, &OutConverError,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
      return err;

    // Check ENV input data
    l = l_Internal_DateCmp(&stp_EnvAndHoldData->st_EnvEndDate,&stp_TimeReal.st_Date);

    e_KeyType = e_clyCard_KeyIssuer;

    v_Internal_GetKifVal(e_KeyType,&St_KIF_And_KVC);

    // Open session and write default ENV data

    err = ClyApp_Virtual_OpenSecureSession(  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
      e_KeyType,//[IN] Key Type to use for the session
      NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
    if( err!=e_ClyApp_Ok)
      return err;

    // Write  ENV data

    // Make sure that the original Application Number remain unchanged

    err = e_Internal_Write( e_ClyApp_Card,//[IN]type - card \ ticket
      1,//[IN] //record number to read - 1 is always the first record
      e_clyCard_EnvironmentFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      stp_EnvAndHoldData, //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
      REC_SIZE); // write only 3 byte version(3 bits ) + start date ( 14 bit ) -> totaly 17 but = 3 BYTES
    if( err!=e_ClyApp_Ok)
      return err;

    // Event update

    memset(&stp_CardEventDataStruct,0,sizeof(st_clyApp_CardEventDataStruct));

    // fill event: VersionNumber,Service Provider,Event Contract Pointer,Event Date and Time,priority list
    v_Internal_FillEventData( &stp_CardEventDataStruct,//[OUT] event to fill
      (unsigned char)0);//[IN]Event Contract Pointer

    stp_CardEventDataStruct.st_EventCode.e_CardEventTransportMean = e_CardEventTransOther;//Change to 'other' on 20/05/13 e_CardEventTransUnspecified;
    stp_CardEventDataStruct.st_EventCode.e_CardEventCircumstances = e_CardEventCircumApplicationIssuing;
    stp_CardEventDataStruct.b_EventIsJourneylnterchange = clyApp_FALSE;
    stp_CardEventDataStruct.st_EventDataTimeFirstStamp = stp_CardEventDataStruct.st_EventDateTimeStamp;
    if(g_Params.lv_DeviceNumber)
    {
   	    stp_CardEventDataStruct.st_OptionalEventData.b_IsEventDevice4Exist=(clyApp_BOOL)1;
	    stp_CardEventDataStruct.st_OptionalEventData.ush_EventDevice4=us_GetSaleDeviceNumber(g_Params.lv_DeviceNumber);//05/2013
    }
#ifndef TIM7020
    if(g_Params.us_PlaceUniqueId) //05/2013
    {
       stp_CardEventDataStruct.st_OptionalEventData.b_IsEventPlaceExist = (clyApp_BOOL)1;
       stp_CardEventDataStruct.st_OptionalEventData.ush_EventPlace = g_Params.us_PlaceUniqueId;
    }
#endif

    // BestPriorityList
    l = MAX_CONTRACT_COUT * sizeof(stp_CardEventDataStruct.e_EventBestContractPriorityListArr[0]);

    // Reset bspl if requested
    if(BCPLReset == clyApp_TRUE)
    {
      stp_CardEventDataStruct.e_EventBestContractPriorityListArr[0] = e_CardPriorityErasable;
      stp_CardEventDataStruct.e_EventBestContractPriorityListArr[1] = e_CardPriorityErasable;
      stp_CardEventDataStruct.e_EventBestContractPriorityListArr[2] = e_CardPriorityErasable;
      stp_CardEventDataStruct.e_EventBestContractPriorityListArr[3] = e_CardPriorityErasable;
      stp_CardEventDataStruct.e_EventBestContractPriorityListArr[4] = e_CardPriorityErasable;
      stp_CardEventDataStruct.e_EventBestContractPriorityListArr[5] = e_CardPriorityErasable;
      stp_CardEventDataStruct.e_EventBestContractPriorityListArr[6] = e_CardPriorityErasable;
      stp_CardEventDataStruct.e_EventBestContractPriorityListArr[7] = e_CardPriorityErasable;

    }
    else
    {
        // Copy last priority list the the envent
        memcpy(stp_CardEventDataStruct.e_EventBestContractPriorityListArr,&st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_EventRecArr[1].e_EventBestContractPriorityListArr,l);
    }

    // Write!
    err = e_Internal_Write(  e_ClyApp_Card,//[IN]type - card \ ticket
      1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
      e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
      &stp_CardEventDataStruct,REC_SIZE); //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
    if( err!=e_ClyApp_Ok)
      return err;

    // Close Session to store ENV
    Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
      st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
      clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified
    if( !IS_RES_OBJ_OK(Obj) )
      return e_ClyApp_CardWriteErr;

    // Update state machine
    st_Static_StateMachine.e_TransactionState = e_clyApp_SessionCloseOk;

    }
    else // Ticket
    {
    return e_ClyApp_CardWriteErr;
    // Not supported
    }

    return e_ClyApp_Ok;
}

////////////////////////////////////////////////////////////////////////////
//
//  End virtual card support
//
////////////////////////////////////////////////////////////////////////////


#ifdef INSPECTOR_TERMINAL
  #ifdef ISSUING_STATION
    #error //can't compile inspector and issue together
  #endif
#endif


#define ZABAD_UTILITY

#ifdef ZABAD_UTILITY
#ifdef WIN32
eCalypsoErr e_ClyApp_DeleteCardContracts(clyApp_BOOL bBCPL_Only)
{

    RESPONSE_OBJ*    Obj;
    st_clyApp_CardEventDataStruct st_CardEventDataStruct;
    e_clyCard_KeyType e_KeyType;
    St_clySam_KIF_And_KVC St_KIF_And_KVC;
    eCalypsoErr err;
    union_ClyApp_ContractRecord union_ContractRecord;
    int i;


    CHECK_INERFACE_INIT();
    CHECK_CARD_EXIST();
    CHECK_SESSION_OPEN();

    UISetProgress(0);

    memset(&union_ContractRecord, 0, sizeof(union_ContractRecord));
    memset(&st_CardEventDataStruct, 0, sizeof(st_CardEventDataStruct));

    // Set empty BCPL's
    for(i=0;i<8;i++)
        st_CardEventDataStruct.e_EventBestContractPriorityListArr[i]=(e_ClyApp_CardPriorityType)14;

    if(st_Static_StateMachine.e_SamType == e_ClyApp_SamCL || st_Static_StateMachine.e_SamType == e_ClyApp_SamCP)
        e_KeyType = e_clyCard_KeyCredit;
    else
        return e_ClyApp_NotOk;

    v_Internal_GetKifVal(e_KeyType,&St_KIF_And_KVC);


    ///////////////////////////////////////////////////////
    //
    // DELETE EVENT 1
    //
    ///////////////////////////////////////////////////////

    err = ClyApp_Virtual_OpenSecureSession(  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
        e_KeyType,//[IN] Key Type to use for the session
        NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
    if( err!=e_ClyApp_Ok)
        return err;

    printf("Deleting Event 1..");
    err = e_Internal_Write( e_ClyApp_Card,//[IN]type - card \ ticket
        1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        &st_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
        );
    if( err!=e_ClyApp_Ok)
        return err;

    if(bBCPL_Only)
    {
        Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
            st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
            clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified

        if( !IS_RES_OBJ_OK(Obj) )
        {
            //if session close fail - make sure that the reader will not receive deselect by RF shutdown
            v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
            return e_ClyApp_CardWriteErr;

        }

        return e_ClyApp_Ok;
    }
    UISetProgress(8);
    ///////////////////////////////////////////////////////
    //
    // DELETE EVENT 2
    //
     ///////////////////////////////////////////////////////


    printf("Deleting Event 2..");
    err = e_Internal_Write( e_ClyApp_Card,//[IN]type - card \ ticket
        1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        &st_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
        );
    if( err!=e_ClyApp_Ok)
        return err;

    UISetProgress(16);
    ///////////////////////////////////////////////////////
    //
    // DELETE EVENT 3
    //
     ///////////////////////////////////////////////////////

    printf("Deleting Event 3..");
    err = e_Internal_Write( e_ClyApp_Card,//[IN]type - card \ ticket
        1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
        e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
        &st_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
        );
    if( err!=e_ClyApp_Ok)
        return err;

    UISetProgress(24);
    ///////////////////////////////////////////////////////
    //
    // DELETE CONTRACTS AND COUNTERS
    //
    ///////////////////////////////////////////////////////

    for(i=0;i<8;i++)
    {


        // DELETE CONTRACT

        printf("Deleting Contract %d..",i);
        err = e_Internal_Write( e_ClyApp_Card,//[IN]type - card \ ticket
            (unsigned char)(i+1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
            e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
            &union_ContractRecord,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
            );
        if( err!=e_ClyApp_Ok)
            return err;

        // DELETE COUNTER

        err = e_Internal_Write( e_ClyApp_Card,//[IN]type - card \ ticket
            (unsigned char)(i+1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
            e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
            &union_ContractRecord,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
            );
        if( err!=e_ClyApp_Ok)
            return err;

        UISetProgress(24 + ((i+1) * 7));
        if(i%3 == 0)
        {


            Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
                st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
                clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified

            if( !IS_RES_OBJ_OK(Obj) )
            {
                // if session close fail - make sure that the reader will not receive deselect by RF shutdown
                v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
                return e_ClyApp_CardWriteErr;
            }

            err = ClyApp_Virtual_OpenSecureSession(  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
                e_KeyType,//[IN] Key Type to use for the session
                NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
            if( err!=e_ClyApp_Ok)
                return err;

        }

    }

    UISetProgress(80);
    // Close session..
    ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
        st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
        clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified
    if( !IS_RES_OBJ_OK(Obj) )
    {
        // if session close fail - make sure that the reader will not receive deselect by RF shutdown
        v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
        return e_ClyApp_CardWriteErr;
    }

    // Reopen the session ...
    err = ClyApp_Virtual_OpenSecureSession(  St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
        e_KeyType,//[IN] Key Type to use for the session
        NULL); //[IN]Rec Num 2 Return: if read not requested send NULL
    if( err!=e_ClyApp_Ok)
        return err;

    ///////////////////////////////////////////////////////
    //
    // DELETE SPECIAL EVENT
    //
    ///////////////////////////////////////////////////////

    for(i=0;i<4;i++)
    {

        UISetProgress(80 + ((i+1) * 5));
        printf("Deleting Special Event..",i);
        err = e_Internal_Write( e_ClyApp_Card,//[IN]type - card \ ticket
            (unsigned char)(i+1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
            e_clyCard_SpecialEventFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
            &st_CardEventDataStruct,REC_SIZE //[IN] data to write - this data need casting in the application layer accurding to the application data strucuts
            );
        if( err!=e_ClyApp_Ok)
            return err;

    }


    // Close session ..
    // Close session..
    ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
        st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
        clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified
    if( !IS_RES_OBJ_OK(Obj) )
    {
        // if session close fail - make sure that the reader will not receive deselect by RF shutdown
        v_Internal_ForgetCard(st_Static_StateMachine.CardReaderId);
        return e_ClyApp_CardWriteErr;
    }

    return e_ClyApp_Ok;
}


#endif // Win32
#endif // ZABAD_UTILITY

//////////////////////////////////////////////////////////////////////////
// Yoni 07/2012
// e_clyapp_WriteStreamToCardEventAndRead
// Wrute buff to event1, read it and compare
// works with any sam (cv,cl,cp)
//////////////////////////////////////////////////////////////////////////
eCalypsoErr e_clyapp_WriteStreamToCardEventAndRead(const char buff[29])
{
    eCalypsoErr err=e_ClyApp_NotOk;
    St_clySam_KIF_And_KVC St_KIF_And_KVC;
    e_clyCard_KeyType KeyType;
    RESPONSE_OBJ* Obj;
	char ReadData[29];
                                     
    //=========================================
    //open session
    //=========================================

    KeyType = e_clyCard_KeyDebit;
    v_Internal_GetKifVal(KeyType,&St_KIF_And_KVC);

    err = ClyApp_Virtual_OpenSecureSession(St_KIF_And_KVC,//[IN] the key KIF And KVC type to open session with
      KeyType,//[IN] Key Type to use for the session
      NULL); //[IN]Rec Num 2 Return: if read not requested send NULL

    if( err!=e_ClyApp_Ok)
      return err;

    //write buffer to event1

    Obj =  ClyApp_Virtual_UpdateRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
                                        1,//[IN]
                                        e_clyCard_SpecialEventFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                        REC_SIZE,//[IN] len to read - 1 to record size
                                        (unsigned char*)buff); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts

    if( !IS_RES_OBJ_OK(Obj) )
    {
        return e_ClyApp_CardWriteErr;
    }

    Obj =   ClyApp_Virtual_CloseSecureSession(st_Static_StateMachine.CardReaderId,//[IN]  card reader id
        st_Static_StateMachine.SamReaderId,//[IN]  sam reader id
        clyCard_FALSE/*b_IsRatifyImmediatly*/);//[IN] //1= the session will be immediately Ratified
    
    if (!IS_RES_OBJ_OK(Obj))
        return e_ClyApp_CardWriteErr;

	//now read
	memset(ReadData, 0, sizeof(ReadData)); 
    Obj =  ClyApp_Virtual_ReadRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
      1,//[IN] //record number to read - 1 is always the first record
      e_clyCard_SpecialEventFile, //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
      REC_SIZE,//[IN] len to read - 1 to record size
      (clyCard_BYTE*)ReadData,0); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts


	if(!IS_RES_OBJ_OK(Obj)) 
		return e_ClyApp_CardReadErr;
	
    if(memcmp(ReadData, buff, sizeof(ReadData))!=0)
		return e_ClyApp_NotOk;

    return e_ClyApp_Ok;

}
#if ENABLE_DEBUG_LOAD

static void ApllaySetting(void)
{
	i_CountDebugOpAfterX=St_CurrSetting.i_CountDebugOpAfterX;
	i_DebugValue=St_CurrSetting.i_DebugValue;

}

void e_ClyApp_SettDebugInfo(St_ClyDebugSettin *p_Setting)
{
  St_CurrSetting=*p_Setting;
  ApllaySetting();
  

}

void e_ClyApp_ResetDebugState(void)
{
	i_CurrCount=0;
	ApllaySetting();

}


#endif
eCalypsoErr e_ClyApp_ReadRecordData(clyCard_BYTE Record[29], // [IN/OUT]
										clyCard_BYTE RecNum,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
										e_clyCard_FileId FileToSelect, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
										void* StOut //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
										)
{
	RESPONSE_OBJ* pRes;
	pRes = ClyApp_Virtual_ReadRecord(st_Static_StateMachine.CardReaderId, // [IN]  reader id
							  RecNum,                 // [IN] //record number to read - 1 is always the first record
							  FileToSelect,       // [IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
							  REC_SIZE,               // [IN] len to read - 1 to record size
							  Record,0);   // [OUT] data read - this data need casting in the application layer accurding to the application data strucuts
    if( !IS_RES_OBJ_OK(pRes) )
      return e_ClyApp_CardReadErr;


	return e_ClyApp_Ok;
}

clyApp_BOOL b_ClyApp_TestReadWrite(void)
{
   RESPONSE_OBJ     *p_Ans;

   //Test
   p_Ans = pSt_ClyCard_TestReadWrite(st_Static_StateMachine.CardReaderId, st_Static_StateMachine.SamReaderId);
	//check return status
    if(!IS_RES_OBJ_OK(p_Ans))
		return clyApp_FALSE; // error
	else
		return clyApp_TRUE; 

}


#if 1
//extern int IsBuzzerBusy;
//Yoni 07/2013
//this function reads the card's records: env, contract, counter, event1-3
//use this for debugging transaction binary data
eCalypsoErr e_Debug_GetCardBinaryForCompare(unsigned char RecNum,
                                            st_ClyApp_CardTransactionBinData* pOutBinaryCardData)
{
        //e_ClyApp_ERR err = e_ClyApp_CardReadErr;
        //SN8 cp_SN8={0};
        RESPONSE_OBJ* Obj;
#if 0
        e_7816_CardType e_CardType; /// ticket or card
        unsigned long SNum;

        static st_7816_CardResetInfo ResetInfo;

        int i=0;
        char b= 0;
        IsBuzzerBusy = 0;
        //detect
        while(!b  && i++ < 100)
        {
            b = b_IsCardIn();
        }
        if(!b)
            return e_ClyApp_CardReadErr;                                 
 
       //_v_GetCardResetInfo(_READER_CONTACTLESS_1, &ResetInfo);
       e_CardType = ResetInfo.e_CardType;
       //start work with card
       if(
       (e_ClyApp_StartWorkWithCard(READER_PC_READER_ID, ResetInfo.cp_ClUid, ResetInfo.c_UidLen) != e_ClyApp_Ok)||
            (e_ClyApp_ResetCard(READER_PC_READER_ID, &e_CardType) != e_ClyApp_Ok) || 
            //serial number
            (e_ClyApp_GetCardSn(READER_PC_READER_ID, &SNum) != e_ClyApp_Ok)
            )
            //if startwork failed give invalid card error
             return e_ClyApp_CardReadErr;
#else
		TR_st_CardInfo Info;
		if(b_clyapp_IsCardIn(&Info) == TR_FALSE)
			return e_ClyApp_CardReadErr;


#endif
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        Obj =  pSt_ClyCard_ReadRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
                                    1,//[IN] //record number to read - 1 is always the first record 
                                    e_clyCard_EnvironmentFile, //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                    REC_SIZE,//[IN] len to read - 1 to record size 
                                    pOutBinaryCardData->ucp_EnvironmentData, //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts 
									0);

        
        if( !IS_RES_OBJ_OK(Obj) )
            return e_ClyApp_CardReadErr;
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        Obj =  pSt_ClyCard_ReadRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
                                    1,//[IN] //record number to read - 1 is always the first record 
                                    e_clyCard_EventLogFile, //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                    REC_SIZE,//[IN] len to read - 1 to record size 
                                    pOutBinaryCardData->ucp_Event1, //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts 
									0);

        
        if( !IS_RES_OBJ_OK(Obj) )
            return e_ClyApp_CardReadErr;
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        Obj =  pSt_ClyCard_ReadRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
                                    2,//[IN] //record number to read - 1 is always the first record 
                                    e_clyCard_EventLogFile, //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                    REC_SIZE,//[IN] len to read - 1 to record size 
                                    pOutBinaryCardData->ucp_Event2, //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts 
									0);

        
        if( !IS_RES_OBJ_OK(Obj) )
            return e_ClyApp_CardReadErr;
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        Obj =  pSt_ClyCard_ReadRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
                                    3,//[IN] //record number to read - 1 is always the first record 
                                    e_clyCard_EventLogFile, //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                    REC_SIZE,//[IN] len to read - 1 to record size 
                                    pOutBinaryCardData->ucp_Event3, //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts 
									0);

        
        if( !IS_RES_OBJ_OK(Obj) )
            return e_ClyApp_CardReadErr;
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        Obj =  pSt_ClyCard_ReadRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
                                    RecNum,//[IN] //record number to read - 1 is always the first record 
                                    e_clyCard_ContractsFile, //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                    REC_SIZE,//[IN] len to read - 1 to record size 
                                    pOutBinaryCardData->ucp_ContractData, //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts 
									0);

        
        if( !IS_RES_OBJ_OK(Obj) )
            return e_ClyApp_CardReadErr;
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        Obj =  pSt_ClyCard_ReadRecord(st_Static_StateMachine.CardReaderId,//[IN]  reader id
                                    1,//[IN] //record number to read - 1 is always the first record 
                                    e_clyCard_CountersFile, //[IN] e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                    REC_SIZE,//[IN] len to read - 1 to record size 
                                    pOutBinaryCardData->ucp_Counter, //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts 
									0);

        
        if( !IS_RES_OBJ_OK(Obj) )
            return e_ClyApp_CardReadErr;

      return e_ClyApp_Ok; 
}
#endif
#endif //  defined(CORE_SUPPORT_SMARTCARD) && defined(CORE_SUPPORT_CALYPSO)


#ifdef ENABLE_COMM
eCalypsoErr UpdateStateMachine(St_ClyApp_SmartCardData *pScData)
{

	eCalypsoErr ret;
	int i;
	union
	{
	  st_ClyApp_EnvAndHoldDataStruct st_EnvRec;
	  st_ClyApp_CardContractRecord  st_ContractRecArr;
	  st_clyApp_CardEventDataStruct st_EventRecArr;
	  st_clyApp_ContractListStruct  st_ContractList;
	}union_StOutData; 

	if(pScData == 0 || pScData->CARD_IN == 0)
        return e_ClyApp_RecordNotFoundErr;

	//serial number  //added by Yoni 4/8/14
	memset(st_Static_StateMachine.ucp_CardSn, 0, sizeof(st_Static_StateMachine.ucp_CardSn));
	memcpy(st_Static_StateMachine.ucp_CardSn, &pScData->SerialNumber , sizeof(pScData->SerialNumber));
	st_Static_StateMachine.uc_CardSnLen = 4;
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Environment
    if(pScData->Env.isRead == 0)
		return e_ClyApp_RecordNotFoundErr;
	memset(&union_StOutData.st_EnvRec,0,sizeof(union_StOutData.st_EnvRec));
	ret = e_Internal_Bit2Byte_Convert(e_BitStream2St,	//[IN] convert direction - bit stream to struct OR struct to bit stream
								      e_ClyApp_Card,	//[IN]type - card \ ticket
									  e_clyCard_EnvironmentFile,//[IN] if not a ticket  - which record in the card
									  pScData->Env.data,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
									  &union_StOutData.st_EnvRec);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
	if(ret != e_ClyApp_Ok)
		return ret;
	ret = e_Internal_UpdateCardSateMachin(e_clyCard_EnvironmentFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                          1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
										  pScData->Env.data,//[IN]bit stream
                                          &union_StOutData.st_EnvRec, //[IN] St data
                                          e_ClyApp_Ok); // if we convert the record from bits to a data structure - is covertion OK - for contract and counter only

	if(ret != e_ClyApp_Ok)
		return ret;

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Events
    for(i=0; i < K10_EVENTS_COUNT; i++)
    {
        if(pScData->Event[i].isRead == 0)
			return e_ClyApp_RecordNotFoundErr;
		memset(&union_StOutData.st_EventRecArr,0,sizeof(union_StOutData.st_EventRecArr));
		ret = e_Internal_Bit2Byte_Convert(e_BitStream2St,	//[IN] convert direction - bit stream to struct OR struct to bit stream
									e_ClyApp_Card,	//[IN]type - card \ ticket
									e_clyCard_EventLogFile,//[IN] if not a ticket  - which record in the card
									pScData->Event[i].data,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
									&union_StOutData.st_EventRecArr);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
		if(ret != e_ClyApp_Ok)
			return ret;
		ret = e_Internal_UpdateCardSateMachin(e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
											  i+1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
											  pScData->Event[i].data,//[IN]bit stream
											  &union_StOutData.st_EventRecArr, //[IN] St data
											  e_ClyApp_Ok); // if we convert the record from bits to a data structure - is covertion OK - for contract and counter only
		if(ret != e_ClyApp_Ok)
			return ret;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
    // Special Events
    for(i=0; i < K10_SPECIAL_EVENTS_COUNT; i++)
    {
        if(pScData->SpecialEvent[i].isRead == 0)
			continue;  // there isn't any contract with (Priority[i] == 4 || Priority[i] == 5)
		memset(&union_StOutData.st_EventRecArr,0,sizeof(union_StOutData.st_EventRecArr));
		ret = e_Internal_Bit2Byte_Convert(e_BitStream2St,	//[IN] convert direction - bit stream to struct OR struct to bit stream
									e_ClyApp_Card,	//[IN]type - card \ ticket
									e_clyCard_SpecialEventFile,//[IN] if not a ticket  - which record in the card
									pScData->SpecialEvent[i].data,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
									&union_StOutData.st_EventRecArr);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
		if(ret != e_ClyApp_Ok)
			return ret;
		ret = e_Internal_UpdateCardSateMachin(e_clyCard_SpecialEventFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
											  i+1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
											  pScData->SpecialEvent[i].data,//[IN]bit stream
											  &union_StOutData.st_EventRecArr, //[IN] St data
											  e_ClyApp_Ok); // if we convert the record from bits to a data structure - is covertion OK - for contract and counter only
		if(ret != e_ClyApp_Ok)
			return ret;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////
    // Contract
    for(i=0; i < K10_CONTRACTS_COUNT; i++)
    {
		if(pScData->Contract[i].isRead == 0)
			continue;  // priorty >= 13 
		memset(&union_StOutData.st_ContractRecArr,0,sizeof(union_StOutData.st_ContractRecArr));
		ret = e_Internal_Bit2Byte_Convert(e_BitStream2St,	//[IN] convert direction - bit stream to struct OR struct to bit stream
									e_ClyApp_Card,	//[IN]type - card \ ticket
									e_clyCard_ContractsFile,//[IN] if not a ticket  - which record in the card
									pScData->Contract[i].data,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
									&union_StOutData.st_ContractRecArr);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
		if(ret != e_ClyApp_Ok)
			return ret;
		ret = e_Internal_UpdateCardSateMachin(e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
											  i+1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
											  pScData->Contract[i].data,//[IN]bit stream
											  &union_StOutData.st_ContractRecArr, //[IN] St data
											  e_ClyApp_Ok); // if we convert the record from bits to a data structure - is covertion OK - for contract and counter only
		if(ret != e_ClyApp_Ok)
			return ret;
	

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Counter
		if(pScData->Counter.isRead == 0)
			return e_ClyApp_RecordNotFoundErr;

		ret = e_Internal_Bit2Byte_Convert(e_BitStream2St,	//[IN] convert direction - bit stream to struct OR struct to bit stream
											e_ClyApp_Card,	//[IN]type - card \ ticket
											e_clyCard_CountersFile,//[IN] if not a ticket  - which record in the card
											pScData->Counter.data+i*3,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
											&union_StOutData.st_ContractRecArr);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
		if(ret != e_ClyApp_Ok)
			return ret;
	
		ret = e_Internal_UpdateCardSateMachin(e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
												i+1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
												0,//[IN]bit stream
												&union_StOutData.st_ContractRecArr, //[IN] St data
												e_ClyApp_Ok); // if we convert the record from bits to a data structure - is covertion OK - for contract and counter only

		if(ret != e_ClyApp_Ok)
			return ret;
	}
	//Counter
	memcpy(st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionBinData.st_CardBinData.ucp_CounterRec,pScData->Counter.data,REC_SIZE);




    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Contract List
	if(pScData->ContractList.isRead == 0)
		return e_ClyApp_RecordNotFoundErr;
	memset(&union_StOutData.st_ContractList,0,sizeof(union_StOutData.st_ContractList));
	ret = e_Internal_Bit2Byte_Convert(e_BitStream2St,	//[IN] convert direction - bit stream to struct OR struct to bit stream
								      e_ClyApp_Card,	//[IN]type - card \ ticket
									  e_clyCard_ContractListFile,//[IN] if not a ticket  - which record in the card
									  pScData->ContractList.data,//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream
									  &union_StOutData.st_ContractList);//[OUT if e_BitStream2St IN if e_St2BitStream]bit stream
	if(ret != e_ClyApp_Ok)
		return ret;
	ret = e_Internal_UpdateCardSateMachin(e_clyCard_ContractListFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
                                          1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
										  pScData->ContractList.data,//[IN]bit stream
                                          &union_StOutData.st_ContractList, //[IN] St data
                                          e_ClyApp_Ok); // if we convert the record from bits to a data structure - is covertion OK - for contract and counter only

	if(ret != e_ClyApp_Ok)
		return ret;


	return e_ClyApp_Ok;

}
#endif

clyApp_BOOL b_GetContractFromStateMachine(int RecNum, st_ClyApp_CardContractRecord *p_Contract_Out)
{

	if(st_Static_StateMachine.st_TransactionData.isContractAuthOk[RecNum]==e_ClyApp_Ok)
	{
		*p_Contract_Out=
			st_Static_StateMachine.st_TransactionData.st_CardAndTktData.union_TransactionStData.st_Card.st_ContractRecArr[RecNum];//.st_CardContractIssuingData;
		return (clyApp_BOOL)1;
	}

//*Union_Contract_Out;//=

return (clyApp_BOOL)0;
}
