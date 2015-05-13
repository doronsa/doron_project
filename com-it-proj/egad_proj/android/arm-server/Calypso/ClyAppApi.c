//#define ENABLE_COMM
#include <os_def.h>
#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif

#ifdef win_or_linux
#define CORE_SUPPORT_SMARTCARD
#define CORE_SUPPORT_CALYPSO
#else

#include <Core.h>
#endif
#include <ClyCrdOs.h>
#if defined(CORE_SUPPORT_SMARTCARD) && defined(CORE_SUPPORT_CALYPSO)


//#include <ClyAppTypes.h>  //TBD:yoram
#include <ClyApp.h>
#include <AppProtocol.h>
#include <ClyAppApi.h>

//#include <stdio.h>
#if ANDROID//doron
#define LOG_TAG "ClyAppApi"
#include "jniutils.h"
#endif
#ifdef VALIDATOR_DEF
#include <Tw_Time.h>
#endif

#define ANDROID 0
#if ANDROID
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#else
#define  LOGI(...)
#endif

///////////////////////////////////////////////////////////////////

struct
{
	char bIsParamsSet;
	char bIsTimeSet;
	char bSamInit;
	char bWorkWithoutSam;
	char bContractsFileOpened;
	char bIsProtocolInit;
}stInitFlags;

///////////////////////////////////////////////////////////
// Event data
///////////////////////////////////////////////////////////
TR_PACK_PREFIX struct TAG_TR_St_EventForValidityCheck
{
	unsigned char	isValidEvent;
	unsigned char   EventCircumstances;
	unsigned char   EventJourneyInterchange;
	unsigned char   EventContractPointer;
	unsigned short  TicketType;//ticket type of contract that was
	unsigned char   ContractProfile;
	unsigned char	ContractSpatial;
	unsigned char   ContractCluster;
	unsigned short  ContractZoneBitmap;
    unsigned char   ContractFareCode;

	unsigned short  ContractPredefine;
	unsigned char   NumberOfPassengers;
	
	TR_St_DateTime  DateTime;
	TR_St_DateTime  DateTimeFirstUse;
	unsigned short  EventPlace;
	
	unsigned short  EventDevice;
	unsigned		char InterchangeRights;

	unsigned char	 EventTicketFareCode;


};
typedef TR_PACK_PREFIX struct TAG_TR_St_EventForValidityCheck TR_St_EventForValidityCheck;

//char g_bIsRemoteSam=TR_FALSE;
#ifdef EBABLE_COMM
#define IS_CALYPSO_LOCKED (!(stInitFlags.bIsParamsSet & stInitFlags.bIsTimeSet & stInitFlags.bIsProtocolInit))
#else
#define IS_CALYPSO_LOCKED (!(stInitFlags.bIsParamsSet & stInitFlags.bIsTimeSet))
#endif

#define ETT_FOR_QUERY 90

#ifndef TIM7020
#define BIT_PARAMS_STATUS    0
#define BIT_TIME_STATUS      1
#define BIT_SAM_STATUS       2
#define BIT_CONTRACTS_STATUS 3



//MAKE A BITMAP OUT OF stInitFlags
unsigned short sInitFlagsToBitmap()
{
	unsigned short s=0;
	if(stInitFlags.bIsParamsSet)
	{
		s |= 1<<BIT_PARAMS_STATUS;
	}
	if(stInitFlags.bIsTimeSet)
	{
		s |= 1<<BIT_TIME_STATUS;
	}
	if(stInitFlags.bSamInit)
	{
		s |= 1<<BIT_SAM_STATUS;
	}
	if(stInitFlags.bContractsFileOpened)
	{
		s |= 1<<BIT_CONTRACTS_STATUS;
	}

	return s;
}

#endif 


///////////////////////////////////////////////////////////////////
///////// GLOBALS


static TR_st_CardInfo    g_CardInfo;
static e_ClyApp_SamType  g_sam_type;
static unsigned long			g_sam_serial;
TR_st_Parameters  g_Params;
St_ChngTripcmd g_TripInfo;

static GetTimeAndDateCallBack TimeAndDate_fp; // function pointer GetTimeAndDateCallBack 

st_InitResource InitRes;



static int i_GetSamEggedSerNum(unsigned long *p_SerNum)
{
int ApplicationError;
	
	//St_ClySam_InitInterface req;
	St_ClySam_SerNumReq  req;
	St_ClySam_SerNumResp resp;
	
	int RecvSize;

	//req.ComPort	   = ComPort;   
	//req.ReaderId = i_ReaderId;
//	retval = clySam_FALSE;
// [IN] St_ClySam_ResetReq,               [OUT] St_ClySam_ResetResp

	ApplicationError = 0; //TBD:yoram debug
	*p_SerNum=0;
if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_ClySam_GetSerNum_Sam_Egged_Sign, 	//int i_cmd,//[IN] the command
							 PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
	{
		//OK
		if(ApplicationError=e_ClyApp_Ok && RecvSize== sizeof(resp))
		{
			*p_SerNum=resp.SerNum.p_SerNum4;
			return 1;
		}
		
	}
return 0;
}
///////////////////////////////////////////////////////////
// NAME: TR_GetSamData
// DESCRIPTION: get sam load counter (EventCounter0)
// PARAMETERS: counter OUT
// PRE REQUIREMENT:  to TR_InitReader,
// RETURNS: eCalypsoErr result
///////////////////////////////////////////////////////////
eCalypsoErr TR_GetSamData(TR_St_SamData* pSamCounter/*out*/)
{
	eCalypsoErr err;
	St_clySam_ReadDataResult dataRes;  														  
	memset(&dataRes, 0, sizeof(dataRes));
	err = e_ClyApp_ReadSamData( e_clySam_Rec,
		e_clySam_EventCounterRec,
		1,
		&dataRes);

	pSamCounter->SamEggedSerialNumber=0;
	// get egged serial number 
	i_GetSamEggedSerNum(&pSamCounter->SamEggedSerialNumber);

	if (e_ClyApp_Ok == err)
	{
		pSamCounter->SamCounter = dataRes.DataOut.EventCeillingArr[0].ul_EventCounter;
		pSamCounter->SamNumber = g_sam_serial;
		pSamCounter->SamType = g_sam_type;
#if ANDROID//linux
		LOGD("ClyAppApi.c : TR_GetSamData eCalypsoErr = e_ClyApp_Ok(1) , SamType = %d",g_sam_type);
#endif
		return e_ClyApp_Ok;
	} 
	else
	{
#if ANDROID//linux
		LOGD("ClyAppApi.c : TR_GetSamData eCalypsoErr =  %d",err);
#endif
		return err;
	}
}



///////// globals common to clyappapi and clyapp
long g_time_zone_bias_minutes; //  The bias is the difference, in minutes, between (UTC) and local time.  UTC = local time + time_zone_bias



///////////////////////////////////////////////////////////////
//  us_GetSaleDeviceNumber
//  Return the sale device number of in calypso format 1-4095
///////////////////////////////////////////////////////////////
unsigned short 
us_GetSaleDeviceNumber(unsigned long lv_DeviceNumber)
{
	return (unsigned short)(lv_DeviceNumber&DEVICE_NUMBER_MASK); //get 12LSBs of device number 1-4095
}

//st_AllAzmashRecords* pAzmashRecords;//03/2013

static USER_PROC_INTEROPPERBILITY GlobalUser_INTEROPPERBILITY_Proc=NULL;

#ifdef NOT_RE_SELECT_AT_CARDIN	
static eCalypsoErr WaitCard(int Count,int *p_IsCardReseleted/*can be null*/);
#endif
//////////////////////////////////////////    STATIC FUNCTIONS  /////////////////////////////////////////////////////////////////
static unsigned char e_GetCardSnapshotOfStoredValueCancel(TR_St_TransactionData* pTransactionData/*out*/);
//check that serial number exists
static TR_BOOL bCardWasRead()
{
	return (TR_BOOL)(g_CardInfo.m_serialNumber[0]);
}

//todo ticket
static unsigned char uc_GetContractProvider(const union_ClyApp_ContractRecord *p_union_ContractRecord)
{
	return p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractProvider;
}

//get spatial of first validity location
static unsigned char uc_CardGetContractSpatialType(const union_ClyApp_ContractRecord *p_union_ContractRecord)
{
	if(p_union_ContractRecord)
	{
		return  p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].e_CardSpatialType;
	}
	else return 0;
}

//GET CLUSTER OF FIRST VALIDITY LOCATION (MUST BE ZONE OR FARE)
static unsigned short us_CardGetContractCluster(const union_ClyApp_ContractRecord *p_union_ContractRecord)
{
	if(p_union_ContractRecord)
	{
		int spatial = p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].e_CardSpatialType;
		if(spatial==e_CardSpatialTypeZones)
		{
			return  p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].union_ContractValidityLocation.st_Zones.ush_SpatialRoutesSystem;
		}
		else if(spatial==e_CardSpatialTypeFareCode)
		{
			return  p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].union_ContractValidityLocation.st_FareCode.ush_SpatialRoutesSystem;
		}

	}
	return 0;
}
///////////////////////////////////////////////////////////////
//  us_CardGetContractZone
//  GET ZONE OF FIRST VALIDITY LOCATION (MUST BE ZONE)
///////////////////////////////////////////////////////////////
static unsigned short us_CardGetContractZone(const union_ClyApp_ContractRecord *p_union_ContractRecord)
{
	if(p_union_ContractRecord)
	{
		return  p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].union_ContractValidityLocation.st_Zones.ush_SpatialZones;
	}
	return 0;
}
static unsigned short us_GetPredefineCodeFromCardContractRecord(const st_ClyApp_CardContractRecord *p_st_ClyApp_CardContractRecord)
{
	int LocationArrLen;//number of validity locations
	if(p_st_ClyApp_CardContractRecord)
	{
		LocationArrLen= p_st_ClyApp_CardContractRecord->st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen;
		return  p_st_ClyApp_CardContractRecord->st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[LocationArrLen-1].union_ContractValidityLocation.st_PredefinedContract.ush_SpetailCode;
	}
	return 0;
}
///////////////////////////////////////////////////////////////
//  us_GetPredefineCode
//  get predefine code in last validity location of contract
///////////////////////////////////////////////////////////////
unsigned short us_GetPredefineCode(const union_ClyApp_ContractRecord *p_union_ContractRecord)
{

	int LocationArrLen;//number of validity locations
	if(p_union_ContractRecord)
	{
		LocationArrLen= p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen;
		return  p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[LocationArrLen-1].union_ContractValidityLocation.st_PredefinedContract.ush_SpetailCode;
	}
	return 0;
}

///////////////////////////////////////////////////////////////
//  us_CardGetContractFareCode
//  GET FARE CODE OF FIRST VALIDITY LOCATION (MUST BE FARE)
///////////////////////////////////////////////////////////////
static unsigned char uc_CardGetContractFareCode(const union_ClyApp_ContractRecord *p_union_ContractRecord)
{
	if(p_union_ContractRecord)
	{
		return  p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].union_ContractValidityLocation.st_FareCode.uc_SpatialFareCode;
	}
	return 0;
}


///////////////////////////////////////////////////////////////
//  ClyDateAndTimetoTrDateTime
//  convert st_Cly_DateAndTime to TR_St_DateTime
///////////////////////////////////////////////////////////////
void ClyDateAndTimetoTrDateTime(const st_Cly_DateAndTime* clyDt/*in*/,  TR_St_DateTime* trDt/*out*/)
{
	trDt->Year = clyDt->st_Date.Year;
	trDt->Month = clyDt->st_Date.Month;
	trDt->Day = clyDt->st_Date.Day;
	trDt->Hour = clyDt->st_Time.hour;
	trDt->Minute = clyDt->st_Time.min;
	trDt->Second = clyDt->st_Time.sec;
}

///////////////////////////////////////////////////////////////
//  ClyDatetoTrDateTime
//  convert st_Cly_Date to TR_St_DateTime
///////////////////////////////////////////////////////////////

void ClyDatetoTrDateTime(const st_Cly_Date* clyDt/*in*/,  TR_St_DateTime* trDt/*out*/)
{
	trDt->Year = clyDt->Year;
	trDt->Month = clyDt->Month;
	trDt->Day = clyDt->Day;
	trDt->Hour = 0;
	trDt->Minute = 0;
	trDt->Second = 0;
}


///////////////////////////////////////////////////////////////
//  TrDateTimeToClyDateAndTimet
//  convert TR_St_DateTime to st_Cly_DateAndTime to
///////////////////////////////////////////////////////////////

static void v_TrDateTimeToClyDateAndTimet(const TR_St_DateTime* trDt/*in*/, st_Cly_DateAndTime* clyDt/*out*/)
{
	clyDt->st_Date.Year =  trDt->Year;
	clyDt->st_Date.Month = (unsigned char)trDt->Month;
	clyDt->st_Date.Day = (unsigned char)trDt->Day;
	clyDt->st_Time.hour =  (unsigned char)trDt->Hour;
	clyDt->st_Time.min = (unsigned char)trDt->Minute;
	clyDt->st_Time.sec = (unsigned char)trDt->Second;
}


///////////////////////////////////////////////////////////////
//  compare TR_St_DateTime
///////////////////////////////////////////////////////////////

long l_DateTimeToLong(const TR_St_DateTime* ptrDt)
{

	st_Cly_DateAndTime clyDt;
	//convert to st_Cly_DateAndTime and then to long
	v_TrDateTimeToClyDateAndTimet(ptrDt, &clyDt);
	return l_Internal_ConvertStTime2SecFrom2000(&clyDt);// [IN] time struct needs to be converted

}


static unsigned short us_CardGetTicketType(const union_ClyApp_ContractRecord *p_union_ContractRecord)
{
	//return ett?
	int i_LocationCount = p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen;
	int Tarif,TarifLSB=0;
	const st_ClyApp_ContractValidityLocation *p_LocationEnd=&p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[i_LocationCount-1];
	TarifLSB=p_LocationEnd->union_ContractValidityLocation.st_PredefinedContract.uc_Tariff_Lsb;
	Tarif=p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType;
	return 10*Tarif+TarifLSB%10;
}


//todo support ticket
static void v_GetValidityDuration(const union_ClyApp_ContractRecord *p_union_ContractRecord,
	unsigned short*   pStartType, //[OUT]
	unsigned short*   pDurationUnitsType, //[OUT]
	unsigned short*   pDurationUnitsCount //[OUT]
	)
{
	//init
	*pStartType=e_PeriodUndefined;
	*pDurationUnitsType=e_DurationUndefined;
	*pDurationUnitsCount=0;
	// check if ContractValidityDuration exist
	if(p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityDurationExist)
	{
		*pStartType=e_Sliding;
		// set ContractValidityDuration ( units and count)
		*pDurationUnitsType = p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.e_DurationType;
		*pDurationUnitsCount= p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.uc_DurationUnitCount;
	}
	else if(p_union_ContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityEndDateExist)// check end date
	{
		*pStartType=e_Calendar;// set ContractValidityEndDate 
	}
}


///////////////////////////////////////////////////////////////////////////
//	b_DoesContractNeedTblTranslation
//	return TR_TRUE 
//		if contract of another provider and which isnt stored value
///////////////////////////////////////////////////////////////////////////
static TR_BOOL b_DoesContractNeedTblTranslation(unsigned char ucEtt, unsigned char ucContractProvider)
{

	if(ucContractProvider != g_Params.uc_ProviderId)
		if(!IsStoredValue(ucEtt))
			return TR_TRUE;
	return TR_FALSE;

}

///////////////////////////////////////////////////////////////////////////
//
// return TR_TRUE if found and by ref the cluster and zones
///////////////////////////////////////////////////////////////////////////
//static TR_BOOL b_TranslateInteropContract(unsigned short usPredefineCode, unsigned short* pcluster/*out*/, unsigned short* pzones/*out bitmap*/)
static TR_BOOL b_TranslateInteropContract(
	unsigned short   ContractPreDefine,//[IN]
	unsigned short  ContraxctEtt,//[IN]
	unsigned short  ContractProvider,//[IN]
	//unsigned short  *p_TableCluster,//[OUT]
	//unsigned short  *p_TableZone//[OUT]		
	St_TblValidityLocation* pTblValidityLocation //[OUT]
	)

{
	//*p_TableCluster=*p_TableZone=0;

	if(!GlobalUser_INTEROPPERBILITY_Proc)
		return TR_FALSE;
	else
		//return GlobalUser_INTEROPPERBILITY_Proc(ContractPreDefine,ContraxctEtt,ContractProvider,p_TableCluster
		//,p_TableZone);
		return GlobalUser_INTEROPPERBILITY_Proc(ContractPreDefine,ContraxctEtt,ContractProvider,pTblValidityLocation);



}

static TR_BOOL b_OpenContractsFile()
{
	//todo
	//set bContractsFileOpened
	stInitFlags.bContractsFileOpened = 0;
	return TR_FALSE;
}


//todo support ticket
static void v_GetContractReportData(const union_ClyApp_ContractRecord* pContractRecord,
	const st_ProcessedContractValidityInfo* pProcessedInfo,
	TR_St_ContractReportData* p_ContractData/*out*/)
{

	//fill p_ContractData according to pContractRecord

	p_ContractData->ucIndexOnCard = pProcessedInfo->ContractIndex;

	p_ContractData->us_SaleNumberDaily =  pContractRecord->st_CardContractRecord.st_CardContractIssuingData.sh_ContractSaleNumberDaily;

	p_ContractData->uc_ContractProvider = uc_GetContractProvider(pContractRecord);

	p_ContractData->ucEtt = us_CardGetTicketType(pContractRecord);

	p_ContractData->ucInterchangeType = (e_InterchangeType)e_GetInterchangeType(&pContractRecord->st_CardContractRecord.st_CardContractIssuingData);//pContractRecord->st_CardContractRecord.st_CardContractIssuingData.b_ContractIsJourneylnterchangesAllowed;

	if(pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractCustomerProfileExist)
	{
		p_ContractData->uc_ContractCustomerProfile = pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractCustomerProfile;
	}


	p_ContractData->usValidityStatus=pProcessedInfo->status;
	ClyDateAndTimetoTrDateTime(&pProcessedInfo->ClyDtm_StartDate/*in*/,  &p_ContractData->st_ContractValidityStartDate/*out*/);
	//ClyDateAndTimetoTrDateTime(&pProcessedInfo->ClyDtm_EndDate/*in*/,  &p_ContractData->st_ContractSaleDate/*out*/);
	ClyDateAndTimetoTrDateTime(&pProcessedInfo->ClyDtm_EndDate/*in*/,  &p_ContractData->st_ContractValidityEndDate/*out*/);

	// ***
	ClyDatetoTrDateTime(&pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractSaleDate/*in*/,  &p_ContractData->st_ContractSaleDate/*out*/);
	p_ContractData->usDeviceNumber = pContractRecord->st_CardContractRecord.st_CardContractIssuingData.sh_ContractSaleDevice;


	v_GetValidityDuration(pContractRecord,
		(unsigned short*)&p_ContractData->usPeriodStartType, //[OUT]
		(unsigned short*)&p_ContractData->usDurationUnitsType, //[OUT]
		(unsigned short*)&p_ContractData->usDurationCount //[OUT]
		);


	p_ContractData->ulCounter=pProcessedInfo->Counter;

	p_ContractData->ucSpatialType=uc_CardGetContractSpatialType(pContractRecord);
	p_ContractData->sPredefinedCode=us_GetPredefineCode(pContractRecord);


	//b_TranslateInteropContract
	//if interop contract loaded by different provider we need to translate, but only if not stored value
	if(b_DoesContractNeedTblTranslation((unsigned char)p_ContractData->ucEtt, p_ContractData->uc_ContractProvider))
	{
		//unsigned short cluster=0,zones=0;
		St_TblValidityLocation ov_TranslatedValidity;
		memset(&ov_TranslatedValidity, 0, sizeof(ov_TranslatedValidity));
		b_TranslateInteropContract(p_ContractData->sPredefinedCode,
			p_ContractData->ucEtt,
			(unsigned short)p_ContractData->uc_ContractProvider,
			&ov_TranslatedValidity);

		p_ContractData->ucSpatialType = ov_TranslatedValidity.ucSpatialType;
		switch(p_ContractData->ucSpatialType)
		{
		case e_CardSpatialTypeZones:
			p_ContractData->usCluster = ov_TranslatedValidity.st_Location.stZones.ucCluster;
			p_ContractData->sZoneBitmap= ov_TranslatedValidity.st_Location.stZones.usZoneBitmap;
			break;
		case e_CardSpatialTypeFareCode:
			p_ContractData->usCluster = ov_TranslatedValidity.st_Location.stFare.ucCluster;
			p_ContractData->sFareCode = ov_TranslatedValidity.st_Location.stFare.ucFareCode;
			break;
		default:
			//shouldnt get here because other types aren't supported
			break;
		}

	}
	else if(p_ContractData->ucSpatialType == e_CardSpatialTypeZones)
	{
		p_ContractData->sZoneBitmap= us_CardGetContractZone(pContractRecord);
		p_ContractData->usCluster= us_CardGetContractCluster(pContractRecord);
	}
	else if(p_ContractData->ucSpatialType == e_CardSpatialTypeFareCode)
	{
		p_ContractData->sFareCode=  uc_CardGetContractFareCode(pContractRecord);
		p_ContractData->usCluster= us_CardGetContractCluster(pContractRecord);
	}

}

//todo support ticket
static void v_GetContractDataForUse(const union_ClyApp_ContractRecord* pContractRecord,
	const st_ProcessedContractValidityInfo* pProcessedInfo,
	TR_St_ContractForUse* p_ContractDataForUse)
{
	//fill p_ContractData according to pContractRecord

	p_ContractDataForUse->ucIndexOnCard = pProcessedInfo->ContractIndex;

	p_ContractDataForUse->ucBCPL_Val = pProcessedInfo->BCPL_Priority;

	p_ContractDataForUse->ucEtt = us_CardGetTicketType(pContractRecord);

	p_ContractDataForUse->ucInterchangeType = (e_InterchangeType)e_GetInterchangeType(&pContractRecord->st_CardContractRecord.st_CardContractIssuingData);//pContractRecord->st_CardContractRecord.st_CardContractIssuingData.b_ContractIsJourneylnterchangesAllowed;

	p_ContractDataForUse->uc_ContractProvider = uc_GetContractProvider(pContractRecord);
	p_ContractDataForUse->m_MaavarPrevLine=pProcessedInfo->m_MaavarPrevLine;
	p_ContractDataForUse->m_MaavarPrevProvider=pProcessedInfo->m_MaavarPrevProvider;


	if(pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractCustomerProfileExist)
	{
		p_ContractDataForUse->uc_ContractCustomerProfile = pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractCustomerProfile;
	}

	ClyDateAndTimetoTrDateTime(&pProcessedInfo->ClyDtm_StartDate/*in*/,  &p_ContractDataForUse->st_ContractValidityStartDate/*out*/);
	ClyDateAndTimetoTrDateTime(&pProcessedInfo->ClyDtm_EndDate/*in*/,  &p_ContractDataForUse->st_ContractValidityEndDate/*out*/);


	v_GetValidityDuration(pContractRecord,
		(unsigned short*)&p_ContractDataForUse->usPeriodStartType, //[OUT]
		(unsigned short*)&p_ContractDataForUse->usDurationUnitsType, //[OUT]
		(unsigned short*)&p_ContractDataForUse->usDurationCount //[OUT]
		);


	p_ContractDataForUse->ulCounter=pProcessedInfo->Counter;

	p_ContractDataForUse->ucSpatialType=uc_CardGetContractSpatialType(pContractRecord);
	p_ContractDataForUse->sPredefinedCode=us_GetPredefineCode(pContractRecord);


	//03/2013 if stored value  - set the fare code of first trip
	if(IsStoredValue(p_ContractDataForUse->ucEtt)) 
	{

		if(pProcessedInfo->IsValidInterchange)
		{
			p_ContractDataForUse->FareCode=pProcessedInfo->ucEventTicketFareCode;
		}
		else
		{
			p_ContractDataForUse->FareCode=0;
		}
	}
	//b_TranslateInteropContract
	//if interop contract (which is not SV) loaded by different provider we need to translate	
	else if(b_DoesContractNeedTblTranslation((unsigned char)p_ContractDataForUse->ucEtt, p_ContractDataForUse->uc_ContractProvider))
	{
		//unsigned short cluster=0,zones=0;
		St_TblValidityLocation ov_TranslatedValidity;
		memset(&ov_TranslatedValidity, 0, sizeof(ov_TranslatedValidity));
		b_TranslateInteropContract(p_ContractDataForUse->sPredefinedCode,
			p_ContractDataForUse->ucEtt,
			(unsigned short)p_ContractDataForUse->uc_ContractProvider,
			&ov_TranslatedValidity);

		p_ContractDataForUse->ucSpatialType = ov_TranslatedValidity.ucSpatialType;
		switch(p_ContractDataForUse->ucSpatialType)
		{
		case e_CardSpatialTypeZones:
			p_ContractDataForUse->usCluster = ov_TranslatedValidity.st_Location.stZones.ucCluster;
			p_ContractDataForUse->sZoneBitmap= ov_TranslatedValidity.st_Location.stZones.usZoneBitmap;
			break;
		case e_CardSpatialTypeFareCode:
			p_ContractDataForUse->usCluster = ov_TranslatedValidity.st_Location.stFare.ucCluster;
			p_ContractDataForUse->FareCode = ov_TranslatedValidity.st_Location.stFare.ucFareCode;
			break;
		default:
			//shouldnt get here because other types aren't supported
			break;
		}

	}
	else if(p_ContractDataForUse->ucSpatialType == e_CardSpatialTypeZones)
	{
		p_ContractDataForUse->sZoneBitmap= us_CardGetContractZone(pContractRecord);
		p_ContractDataForUse->usCluster= us_CardGetContractCluster(pContractRecord);
	}
	else if(p_ContractDataForUse->ucSpatialType == e_CardSpatialTypeFareCode)
	{
		p_ContractDataForUse->FareCode= uc_CardGetContractFareCode(pContractRecord);
		p_ContractDataForUse->usCluster= us_CardGetContractCluster(pContractRecord);
	}



	p_ContractDataForUse->bIsInterchangeValid = pProcessedInfo->IsValidInterchange;
	p_ContractDataForUse->usInterchangeValidityMinutes = pProcessedInfo->usRemainingInterchangeMinutes;


	p_ContractDataForUse->ucInterchangeRights = pProcessedInfo->ucInterchangeRIghts;
	p_ContractDataForUse->ucPsngrCount = pProcessedInfo->ucPsngrCount;


	// Added by Eitan 13/2/12
	p_ContractDataForUse->usDeviceNumber    = pContractRecord->st_CardContractRecord.st_CardContractIssuingData.sh_ContractSaleDevice;
	p_ContractDataForUse->usSaleNumberDaily = pContractRecord->st_CardContractRecord.st_CardContractIssuingData.sh_ContractSaleNumberDaily;
	ClyDatetoTrDateTime(&pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractSaleDate, &p_ContractDataForUse->st_ContractSaleDate);



}




void v_FillCommonFieldsOfEvent(st_clyApp_CardEventDataStruct *stp_CardEventDataStruct/*IN/OUT*/);

static void v_FillLoadingEvent(const TR_St_LoadContract* pLoadData, st_clyApp_CardEventDataStruct *stp_CardEventDataStruct/*out*/)
{
	//todo
	stp_CardEventDataStruct->st_OptionalEventData.st_EventTicket.uc_EventTicketFareCode=0;
	stp_CardEventDataStruct->st_OptionalEventData.st_EventTicket.ush_EventTicketDebitAmount=0;

	v_FillCommonFieldsOfEvent(stp_CardEventDataStruct);
	switch(pLoadData->ucLoadOperationType)
	{
	case 0:
		    stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances=e_CardEventCircumContractLoading;
			break;
	case 1:
		    stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances=e_CardEventCircumContractLoading;
			break;
	case  2:
		    stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances=e_CardEventCircumContractLodingWithImmediateFirstUse;
			break;

	default:
		    stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances=e_CardEventCircumContractLoading;

	}
	
}

static void v_FillUseEvent(unsigned char NumberOfPassengers, //1 or more
	unsigned short Cluster,
	unsigned short PrcCode,   //0 if not fare code
	unsigned long  SVDebitSum, //stored value sum. 0 if not SV
	unsigned char	InterchangeRights,							 
	TR_BOOL  IsFirstTrip, //if 0 then this is interchange
	st_clyApp_CardEventDataStruct *stp_CardEventDataStruct/*IN/OUT*/)
{

	v_FillCommonFieldsOfEvent(stp_CardEventDataStruct);


	if(IsFirstTrip)
	{
		stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances =  e_CardEventCircumEntry;
	}
	else
	{
		stp_CardEventDataStruct->st_EventCode.e_CardEventCircumstances = e_CardEventCircumInterchangeEntry;
	}


	stp_CardEventDataStruct->st_OptionalEventData.b_IsEventPassengersNumberExist=(clyApp_BOOL)1;
	stp_CardEventDataStruct->st_OptionalEventData.uc_EventPassengersNumber= NumberOfPassengers;

	if(PrcCode || SVDebitSum || Cluster)
	{
		stp_CardEventDataStruct->st_OptionalEventData.st_EventTicket.uc_EventTicketFareCode=(unsigned char)PrcCode;
		stp_CardEventDataStruct->st_OptionalEventData.st_EventTicket.ush_EventTicketDebitAmount=(unsigned short)SVDebitSum;
		stp_CardEventDataStruct->st_OptionalEventData.st_EventTicket.ush_EventTicketRoutesSystem=(unsigned short)Cluster;
	}


	//InterchangeRights
	if(InterchangeRights)
	{
		stp_CardEventDataStruct->st_OptionalEventData.b_IsEventInterchangeRightsExist = (clyApp_BOOL)1;
		stp_CardEventDataStruct->st_OptionalEventData.uc_EventInterchangeRights = InterchangeRights;
	}
	else
	{
		stp_CardEventDataStruct->st_OptionalEventData.b_IsEventInterchangeRightsExist = (clyApp_BOOL)0;
	}


	if(IsFirstTrip==0)
	{
		//set interchange flag
		stp_CardEventDataStruct->b_EventIsJourneylnterchange=(clyApp_BOOL)1;
	}
}


static void v_FillCancelEvent(st_clyApp_CardEventDataStruct *stp_CardEventDataStruct/*out*/)
{
	v_FillCommonFieldsOfEvent(stp_CardEventDataStruct);
}


e_ClyApp_CardTariffAppType GetTariffAppTypeByETT(e_EttType ett)
{
	switch(ett)
	{
	case e_EttHalosh:
		return e_ClyApp_FreeCertificate;
	case e_EttSingle:
	case e_EttMlt2:
	case e_EttMlt5:
	case e_EttMlt10:
	case e_EttMlt11:
	case e_EttMlt15:
	case e_EttMlt20:
		return e_ClyApp_OneTimeOrMultiRideTicket;

	case e_EttMlt4:
	case e_EttMlt6:
		return e_ClyApp_OneTimeOrMultiRideTicket46;

	case e_EttMonthly:
	case e_EttWeekly:
	case e_Ettdaily:
	case e_EttSemesterB:
	case e_EttSemesterA:
	case e_EttYearly:
	case e_EttHour:
		return e_ClyApp_SeasonPass;

	case e_EttPass:
		return e_ClyApp_TransferTick;

		/*???       case  e_EttSoldjerVoutcher:
		case e_EttPectialTrip:
		return ;
		*/

	case e_EttStoreValue1_30:
	case e_EttStoreValue2_50:
	case e_EttStoreValue3_100:
	case e_EttStoreValue4_150:
	case e_EttStoreValue5_200:
	case e_EttStoreValue6_Special:
		return e_ClyApp_StoredValue;

		/*???       case e_EttHemshechDiscount:
		case e_EttHemshechSpectial:
		return ;
		*/
	default:
		break;
	}
	return e_ClyApp_OneTimeOrMultiRideTicket;
}

//number of nikuvim for given kartisia ett
static int iKartisiaEttToTokens(unsigned char Ett)
{

	switch(Ett)
	{
	case e_EttSingle://10
		return 1;
	case e_EttMlt2://12
		return 2;
	case e_EttMlt5://13
		return 5;
	case e_EttMlt10://14
		return 10;
	case e_EttMlt11://15
		return 11;
	case e_EttMlt15:
		return 15;
	case e_EttMlt20:
		return 20;
	case e_EttMlt4:
		return 4;
	case e_EttMlt6:
		return 6;
	case e_EttPass:
		return 1;//ben-irony

	}

	return 0;
}

static void v_FillCardContractForLoad(const TR_St_LoadContract* pLoadData, union_ClyApp_ContractRecord* pContractRecord/*out*/)
{

	e_ClyApp_CardTariffAppType l_TariffAppType;
	st_ClyApp_CardCounterRecord  *p_Counter=0;


	memset(pContractRecord,0,sizeof(*pContractRecord));

	//first we fill fields that aren't application dependant

	pContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractVersionNumber=CONTRACT_VERSION_NUM;///stGlobalDataObject.ExtrnParData.iv_CONTRACT_VERSION; //Contract structure version number

	//set start date (if 0 than sliding)
	if(pLoadData->st_ContractValidityStartDate.Year == 0)
	{
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractValidityStartDate = stSlidingZeroDate;
	}
	else
	{
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractValidityStartDate.Year=pLoadData->st_ContractValidityStartDate.Year;
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractValidityStartDate.Month=(unsigned char)pLoadData->st_ContractValidityStartDate.Month;
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractValidityStartDate.Day=(unsigned char)pLoadData->st_ContractValidityStartDate.Day;
	}

	//sale date is filled in clyapp.c

	//pContractRecord->st_CardContractRecord.st_CardContractIssuingData.uc_ContractProvider=g_Params.uc_ProviderId; filled in clyapp

	pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffTransportType=e_ClyApp_TransportAccessOnly;

	l_TariffAppType = pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffAppType = GetTariffAppTypeByETT((e_EttType)pLoadData->ucEtt);

	if(pLoadData->uc_ContractCustomerProfile)
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractCustomerProfileExist=(clyApp_BOOL)1;
	else
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractCustomerProfileExist=(clyApp_BOOL)0;
	pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractCustomerProfile=pLoadData->uc_ContractCustomerProfile; //Social profile giving predefined transportation rights

	pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPassengersNumberExist=(clyApp_BOOL)0;
	pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractPassengersNumber=0;
	pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRFUExist=(clyApp_BOOL)0;
	pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.ul_ContractRFUval=0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//validity location
	pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].e_CardSpatialType= (e_ClyApp_CardSpatialType)pLoadData->ucSpatialType;
	if(pLoadData->ucSpatialType == e_CardSpatialTypePredefinedContract)
	{
		//one validity location
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen=1;
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].union_ContractValidityLocation.st_PredefinedContract.ush_SpetailCode=pLoadData->sPredefinedCode;
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].union_ContractValidityLocation.st_PredefinedContract.uc_Tariff_Lsb = pLoadData->ucEtt%10;
	}
	else
	{

		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_LocationArrLen=2;


		//first validity location (zone or fare)
		if(pLoadData->ucSpatialType == e_CardSpatialTypeZones)
		{
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].union_ContractValidityLocation.st_Zones.ush_SpatialZones =pLoadData->sZoneBitmap;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].union_ContractValidityLocation.st_Zones.ush_SpatialRoutesSystem = pLoadData->usCluster;
		}
		else if(pLoadData->ucSpatialType == e_CardSpatialTypeFareCode)
		{
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].union_ContractValidityLocation.st_FareCode.uc_SpatialFareCode=(unsigned char)pLoadData->sFareCode;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[0].union_ContractValidityLocation.st_FareCode.ush_SpatialRoutesSystem=pLoadData->usCluster;
		}
		//second validity location (predefine)
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[1].e_CardSpatialType=(e_ClyApp_CardSpatialType)e_CardSpatialTypePredefinedContract;
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[1].union_ContractValidityLocation.st_PredefinedContract.uc_Tariff_Lsb = pLoadData->ucEtt%10;
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityLocationArr[1].union_ContractValidityLocation.st_PredefinedContract.ush_SpetailCode=pLoadData->sPredefinedCode;

	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	pContractRecord->st_CardContractRecord.st_CardContractIssuingData.sh_ContractSaleDevice=us_GetSaleDeviceNumber(g_Params.lv_DeviceNumber); //set 12LSBs of device number 1-4095

	pContractRecord->st_CardContractRecord.st_CardContractIssuingData.sh_ContractSaleNumberDaily = pLoadData->usSaleNumberDaily;


	//set restrict time code
	if(pLoadData->ucRestrictTimeCode)
	{
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_IsContractRestrictTimeCodeExist = (clyApp_BOOL)1;
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictTimeCode = pLoadData->ucRestrictTimeCode;
	}


	//different fields for different ticket types (double check on host params)
	switch(l_TariffAppType) // pLoadData->ucEtt/10
	{
		const PeriodLoadData*     pPeriodSpecificLoadData;
		const StoredValueLoadData*    pStoredValueSpecificLoadData;
		const MaavarOrHemshechKartisiaLoadData* pKartisiaSpecificLoadData;
		//////////  PERIOD  //////////
	case e_ClyApp_SeasonPass:
	case e_ClyApp_FreeCertificate:
	case e_ClyApp_ParkAndRideSeasonPass:
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType=e_ClyApp_CounterNotUsed;
		pPeriodSpecificLoadData = &pLoadData->ov_SpecificLoadData.ov_PeriodLoadData;
		//set end date if exists
		if(pPeriodSpecificLoadData->st_ContractValidityEndDate.Year>0)
		{
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityEndDateExist=(clyApp_BOOL)1;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityEndDate.Year=pPeriodSpecificLoadData->st_ContractValidityEndDate.Year;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityEndDate.Month=(unsigned char)pPeriodSpecificLoadData->st_ContractValidityEndDate.Month;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityEndDate.Day=(unsigned char)pPeriodSpecificLoadData->st_ContractValidityEndDate.Day;
		}
		//if sliding season pass set validity duration (sliding not allowed for ett's other than 2X)
		if(pPeriodSpecificLoadData->usPeriodStartType == e_Sliding)
		{
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractValidityDurationExist=(clyApp_BOOL)1;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.e_DurationType= (e_ClyApp_DurationType)pPeriodSpecificLoadData->usDurationUnitsType;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractValidityDuration.uc_DurationUnitCount= (unsigned char)pPeriodSpecificLoadData->usDurationCount;  ;

		}
		break;
		//////////  STORED VALUE  //////////
	case e_ClyApp_StoredValue:
		pStoredValueSpecificLoadData = &pLoadData->ov_SpecificLoadData.ov_StoredValueLoadData;
		//for stored value set counter
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType=e_ClyApp_CounterAsNumOfToken;

		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.b_ContractIsJourneylnterchangesAllowed=(clyApp_BOOL)(e_Maavar==pLoadData->ucInterchangeType);
		//set restrict duration for maavar/hemshech
		if(pLoadData->ucInterchangeType != e_NoInterchange)
		{
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictDurationExist=(clyApp_BOOL)1;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictDuration= pStoredValueSpecificLoadData->uc_RestrictDuration; //Duration restriction of a journey
			if(pStoredValueSpecificLoadData->bIsFiveMinutesResolution)
			{

				pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictCodeExist=(clyApp_BOOL)1;
				pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictCode=0x10;
			}
		}		

		p_Counter=&pContractRecord->st_CardContractRecord.st_CardCounterRecord;///&union_ContractRecord->st_CardContractRecord.st_CardCounterRecord;
		p_Counter->union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue=  pStoredValueSpecificLoadData->ul_StoredValueSumToLoadAgorot;
		p_Counter->e_CardCounterRecordType=e_ClyApp_CardCounter_NumberOfTokensOrAmount;

		break;

		//////////  KARTISIA  //////////
	case e_ClyApp_OneTimeOrMultiRideTicket:
	case e_ClyApp_TransferTick:
	case e_ClyApp_OneTimeOrMultiRideTicket46:
		//for kartisia regular/maavar set counter and interchange
		//counter according to ett
		pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType=e_ClyApp_CounterAsNumOfToken;
		p_Counter=&pContractRecord->st_CardContractRecord.st_CardCounterRecord;
		p_Counter->e_CardCounterRecordType=e_ClyApp_CardCounter_NumberOfTokensOrAmount;

		// Eitan 1/2012 Special case, restoring a contract   - Personalization -
		if(pLoadData->ucLoadOperationType == 1)
		{
			p_Counter->union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue = pLoadData->RetoreKartisiyaUnits;
		}
		else
		{
			p_Counter->union_CardCounterRecord.st_CardCounter_NumberOfTokensOrAmount.CounterValue=  iKartisiaEttToTokens(pLoadData->ucEtt);
		}

		//interchange
		if(e_Maavar == pLoadData->ucInterchangeType)
		{
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.b_ContractIsJourneylnterchangesAllowed=(clyApp_BOOL)1;
		}
		else if(e_OneHemshech == pLoadData->ucInterchangeType ||
			e_TwoHemshech == pLoadData->ucInterchangeType)
		{
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.b_ContractIsJourneylnterchangesAllowed=(clyApp_BOOL)1;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist = (clyApp_BOOL)1;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.e_PeriodType = e_PeriodKartisiaHemshech;          

			if(e_OneHemshech == pLoadData->ucInterchangeType)
			{
				pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.uc_MaxNumOfTripsInPeriod = 2;
			}
			else //e_TwoHemshech
			{
				pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.uc_MaxNumOfTripsInPeriod = 3;
			}
		}
		else //default
		{
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist = (clyApp_BOOL)0;
		}




		//set restrict duration for maavar/hemshech
		if(pLoadData->ucInterchangeType != e_NoInterchange)
		{
			pKartisiaSpecificLoadData = &pLoadData->ov_SpecificLoadData.ov_MaavarKartisiaLoadData;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictDurationExist=(clyApp_BOOL)1;
			pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictDuration= pKartisiaSpecificLoadData->uc_RestrictDuration; //Duration restriction of a journey
			if(pKartisiaSpecificLoadData->bIsFiveMinutesResolution)
			{

				pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictCodeExist=(clyApp_BOOL)1;
				pContractRecord->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictCode=0x10;
			}
		}
		break;
		//////////      //////////

	default:
		break;
	}


}

static void v_FillContractForLoad(const TR_St_LoadContract* pLoadData, union_ClyApp_ContractRecord* pContractRecord/*out*/)
{
	if(uc_clyapp_GetCurrentCardType()==e_ClyApp_Card)
		v_FillCardContractForLoad(pLoadData, pContractRecord);
	//ticket currently not supported


#if 0
	//this function translates TR_St_LoadContractData to union_ClyApp_ContractRecord
	//for card and ticket
	//all the logic of implementing the various application types is done here
	//the following application types are supported:

	//case card:
	set start date if not 0,  end date if not 0 ,
		set restrict duration according to
		set uc_ContractCustomerProfile
		set 2 validity locations: first is according to ucSpatialType and sFareCode/sZone
		the second is predefine according to sPredefinedCode (and app lsb)

		fill restrict code according to

		set RestrictTime according to ucRestrictTimeCode if > 0


		switch(usTicketType)
  case  calendar period : //(daily/weekly/monthly/yearly/semester
	  set ett according to ticket type (period length)


		  case stored value
		  set ett and counter according to ulCounter


		  case  kartisia/kartisia shatit
		  set ett and counter according to ulCounter
		  set restrict code for shatit, journey interchange, restrict duration according to usDurationUnitsType,usDurationCount




		  //case ticket
		  suppports regular kartisia and daily (yomi)
		  case regular kartisia: write point to point (2 zones) according to sZone
								 write ValidityJourneys according to ulCounter

		  case daily:  where is the data structure defined in srs ??????


		else///if(stGlobalDataObject.CardType==e_ClyApp_Card)
	  {

		  st_ClyTkt_TicketSeasonPassTicket *pSeasonTkt=0;
		  st_ClyTkt_MultiRideTicket *pMulTkt=0;
		  pSeasonTkt=&union_ContractRecord->struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket;
		  pMulTkt=&union_ContractRecord->struct_Ticket.union_TicketAppType.st_MultiRideTicket;

		  /////////////   season
		  ///   contract - all types
		  union_ContractRecord->struct_Ticket.st_TicketContractCommonData.uc_TC_KeyIndex=0;///const value of key index
		  //union_ContractRecord->struct_Ticket.st_TicketContractCommonData.st_Tariff=e_ClyTkt_TariffPredefinedSeasonPass; //Multi-ride ticket (0, 1 or 2 ) Season pass (4, 5 or 6 )
		  union_ContractRecord->struct_Ticket.st_TicketContractCommonData.uc_TC_Profile=0;///(e_PssngrType)GetTWPssngrType(cp_Trn->iv_PssngrType,0);///   passenger type
		  ///   season ticket
		  pSeasonTkt->st_TicketSeasonPassContractRec.e_TSC_Sliding=e_ClyTkt_ValidityStartsAtFirstUse;/// 1-period 0-calendar

		  pSeasonTkt->st_TicketSeasonPassContractRec.st_TSC_Date.Year=0;
		  pSeasonTkt->st_TicketSeasonPassContractRec.st_TSC_Date.Month=0;
		  pSeasonTkt->st_TicketSeasonPassContractRec.st_TSC_Date.Day=0;

		  pSeasonTkt->st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType=e_ClyTkt_DurationInDays; //Duration of validity
		  pSeasonTkt->st_TicketSeasonPassContractRec.st_TicketDuration.uc_DurationValue=atoi(contractRec->StartDate);
		  ///   11 bits
		  ///   validation record
		  pSeasonTkt->st_TicketSeasonPassValidationRec.ush_TSLL_Locationld=0;//CalcLocationId(); //Location of validation (within TSLL_ServiceProvider).

		  //set cluster tkt
		  pSeasonTkt->st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialAreaSP.uc_TSC_RoutesSystem =  contractRec->Cluster;
		  /////////////////   multi stored transfer

		  ///   contract - all types
		  union_ContractRecord->struct_Ticket.st_TicketContractCommonData.uc_TC_KeyIndex=0;///const value of key index
		  //union_ContractRecord->struct_Ticket.st_TicketContractCommonData.st_Tariff=e_ClyTkt_TariffPredefinedMultiRideTicket; //Multi-ride ticket (0, 1 or 2 ) Season pass (4, 5 or 6 )
		  union_ContractRecord->struct_Ticket.st_TicketContractCommonData.uc_TC_Profile=0;///(e_PssngrType)GetTWPssngrType(cp_Trn->iv_PssngrType,0);///   passenger type
		  ///   multi ticket
		  pMulTkt->st_TicketMultiRideContractRec.uc_TMC_ValidityJourneys=atoi(contractRec->StartDate);//???????GSD->StartCounter;                   ///   11 bits
		  ///   first validation
		  pMulTkt->st_TicketMultiRideFirstValidationRec.e_TMF_Direction=(e_ClyTkt_Direction)0;///0 or 1
		  pMulTkt->union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRec.ush_TML_LocationId=0;//CalcLocationId(); //Location of last validation (within TMLL_Service Provider) .

		  /////////////////   additional data
		  pMulTkt->st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialFareCode.uc_TMC_RoutesSystem= contractRec->Cluster;
		  case PeriodIssue:/// hofshi tkufati          struct_Ticket.st_TicketContractCommonData.st_Tarif
			  union_ContractRecord->struct_Ticket.st_TicketContractCommonData.st_Tariff = e_ClyTkt_TariffAreAeasonPass;
			  pSeasonTkt=&union_ContractRecord->struct_Ticket.union_TicketAppType.st_TicketSeasonPassTicket;
			  pSeasonTkt->st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialAreaSP.ush_TSC_ValidityZones = 0x0FFF&code;
			  break;
		  case MultiIssue:
			  union_ContractRecord->struct_Ticket.st_TicketContractCommonData.st_Tariff = e_ClyTkt_TariffMultiRideFareCodeTicket;
			  pMulTkt=&union_ContractRecord->struct_Ticket.union_TicketAppType.st_MultiRideTicket;
			  pMulTkt->st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialFareCode.uc_TCM_FareCode= 0x07ff&code;
			  break;


	}///   else if card
#endif //if 0

}

///////////////////////////////////////////////////////////////
//  b_IsAnonymousEnv
//  return 1 if anonymous card
///////////////////////////////////////////////////////////////
static unsigned char b_IsAnonymousEnv(const st_ClyApp_EnvAndHoldDataStruct* pEnvAndHoldDataStruct)
{

	if(pEnvAndHoldDataStruct->st_HoiderProf1.uc_HoiderProfCode==0 && pEnvAndHoldDataStruct->st_HoiderProf2.uc_HoiderProfCode==0
		&& pEnvAndHoldDataStruct->ul_HolderldNumber==0)
		return 1;
	return 0;
}


static clyApp_BOOL DateAndTimeCallBack(st_Cly_DateAndTime *stp_DateAndTime)//[OUT]current date and time
{
#ifdef TIM7020
	TR_St_DateTime trDt;
	if(InitRes.TimeAndDateCB && InitRes.TimeAndDateCB(&trDt) == TR_TRUE)
	{
		//convert to struct st_Cly_DateAndTime 
		v_TrDateTimeToClyDateAndTimet(&trDt, stp_DateAndTime);
		return (clyApp_BOOL)1;
	}
#elif defined(CORE_SUPPORT_RTC)

	st_Time newtime;

#ifndef VALIDATOR_DEF
	if(i_TimeHGetNow(&newtime))
#else	
	if(i_TimeGMtOffsetGetNow(&newtime))
#endif			
	{

		//convert to struct st_Time
		stp_DateAndTime->st_Date.Year = newtime.ui_Year;          /* Year : for example 20000  */
		stp_DateAndTime->st_Date.Month  = newtime.uc_Month;       /* Month. [1-12] */
		stp_DateAndTime->st_Date.Day    = newtime.uc_Day;     /* Day.   [1-31] */
		stp_DateAndTime->st_Time.hour   = newtime.uc_Hour;      /* Hours. [0-23] */
		stp_DateAndTime->st_Time.min    = newtime.uc_Minute;    /* Minutes. [0-59] */
		stp_DateAndTime->st_Time.sec    = newtime.uc_Second;    /* Seconds. [0-59] */
		return (clyApp_BOOL)1;
	}
#else
	struct tm  *t;
	t=RTCGetTime();
	if(t)
	{
		stp_DateAndTime->st_Date.Year = t->tm_year+1900;          /* Year : for example 20000  */
		stp_DateAndTime->st_Date.Month  = t->tm_mon+1;       /* Month. [0-11] */
		stp_DateAndTime->st_Date.Day = t->tm_mday;     /* Day.   [1-31] */
		stp_DateAndTime->st_Time.hour   = t->tm_hour;      /* Hours. [0-23] */
		stp_DateAndTime->st_Time.min    = t->tm_min;    /* Minutes. [0-59] */
		stp_DateAndTime->st_Time.sec    = t->tm_sec;    /* Seconds. [0-59] */
		return (clyApp_BOOL)1;

	}


#endif

	return (clyApp_BOOL)0;

}




//return positive if contract1 is higher priority than contract2
static int CompareContractPriority(const TR_St_ContractForUse* contract1, const TR_St_ContractForUse* contract2)
{
	//the one with valid interchange is higher priority
	if(contract1->bIsInterchangeValid != contract2->bIsInterchangeValid)
	{
		return contract1->bIsInterchangeValid - contract2->bIsInterchangeValid;
	}
	//both have or dont have valid interchange. so lower bcpl is higher priority
	return contract2->ucBCPL_Val - contract1->ucBCPL_Val;
}


//bubble sort
static void v_SortContracts(TR_St_ContractForUse Array[], int num_contracts //[IN/OUT]
	)
{
	int i,j;
	for(i=0;i<num_contracts-1;i++)
	{
		for(j=0;j<(num_contracts-(i+1));j++)
		{
			if(CompareContractPriority(&Array[j], &Array[j+1]) < 0)
			{
				//swap
				TR_St_ContractForUse ov_temp = Array[j];
				Array[j] = Array[j+1];
				Array[j+1] = ov_temp;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define SAM_CL 4
#define WORK_WITH_PIN_SAM_ISRAEL 1


static eCalypsoErr ClyInitInterface(e_7816_DEVICE SamReaderId, e_7816_DEVICE CardReaderId,int SamUARTId)
{

	//    st_ClyApp_Callback stp_Callback;
	int i;
#ifndef INSPECTOR_TERMINAL
	eCalypsoErr err;

#endif
	st_ClyApp_Callback stp_Callback;
	UULOCK_PIN16 ucp_UnlockSampin={0};//valuer isn't important


	st_ReaderComInfo op_CardReaderIdArr[e_7816_LAST];

	unsigned char uc_ArrLen = e_7816_LAST;
	st_ClyTkt_KeyInfo st_KeyInfoArr[MAX_TIKET_KEYS]={
		{(ClyTkt_BOOL)1, {0x27,0xE9,0xC9,0xA4,0xF6,0xF3,0x10,0x3F,0x87,0xA3,0x1B,0x45,0x84,0x62,0x2B,0x6C}}
	};

	//memset(&stInitFlags, 0, sizeof(stInitFlags));//init flags
	memset(&stp_Callback,0,sizeof(stp_Callback));

	// Store the readers info
	memset(op_CardReaderIdArr,0,sizeof(op_CardReaderIdArr));

	//////////////////////////////////////
	// Set SAM Reader info
	//////////////////////////////////////
	op_CardReaderIdArr[SamReaderId].mComPort = 0;
	op_CardReaderIdArr[SamReaderId].mIsExist = 1;     // ? See below
	op_CardReaderIdArr[SamReaderId].mIsSAMReader = 1; 
	op_CardReaderIdArr[SamReaderId].mPairedReaderId = CardReaderId;
	op_CardReaderIdArr[SamReaderId].mUART = SamUARTId;

	//////////////////////////////////////
	// Set User Card Reader info
	//////////////////////////////////////
	op_CardReaderIdArr[CardReaderId].mComPort = 0;
	op_CardReaderIdArr[CardReaderId].mIsExist = 1;
	op_CardReaderIdArr[CardReaderId].mIsSAMReader = 0;
	op_CardReaderIdArr[CardReaderId].mPairedReaderId = SamReaderId;
	op_CardReaderIdArr[CardReaderId].mUART = 0;

	if(SamUARTId != eCoreUARTNone)
		op_CardReaderIdArr[SamReaderId].mIsExist     = 1; // ?

	for(i=0;i<MAX_TIKET_KEYS;i++)
		st_KeyInfoArr[i].b_IsKeyExist=(ClyTkt_BOOL)0;

	st_KeyInfoArr[0].b_IsKeyExist=(ClyTkt_BOOL)1;

	if(( e_ClyApp_InitInterface(
		ucp_UnlockSampin, //[IN]  Unlock SAM pin
		op_CardReaderIdArr, //[IN]Card reader ID array
		uc_ArrLen,    //[IN]Card reader ID array len
		st_KeyInfoArr))!=e_ClyApp_Ok)//[IN] file path to the encrypted Card<->Sam key schema and the Ticket encrypt\decrypt key
	{
		return e_ClyApp_NotOk;
	}
	else
	{
#ifndef INSPECTOR_TERMINAL
		if(SamUARTId != eCoreUARTNone)
		{
			//get sam type CL/SL/CPP/CP/CV
			err=e_ClyApp_GetSamType(SamReaderId,(e_ClyApp_SamType *)&g_sam_type);
			if( err!=e_ClyApp_Ok || (g_sam_type ==e_ClyApp_SamSL) || (g_sam_type ==e_ClyApp_SamDV) ||
				(g_sam_type > e_ClyApp_SamCV))//[OUT] the SAM type
			{
				///                sGlobalBBRamData->i_CalypsoBlockCode = e_ClySAMInvalidType;   //Yoni 28.8.07
				return e_ClyApp_NotOk;///           e_CheckErrorStatus (e_ClySAMInvalidType,m_Parent, true);
			}

			//and get sam serial num
			g_sam_serial = 0;
			e_ClyApp_GetCardSn(SamReaderId,//[IN] the reader ID
											  &g_sam_serial);

			stInitFlags. bSamInit=1;//sam init done
		}
#endif
		//stGlobalDataObject.sams_data.m_SamCalypsoType=sam_type;
	}


	//    stp_Callback.fp_IsValidCardEnvironmentCallBack=isValidEnv_clbck;
	//    stp_Callback.fp_IsValidContractCallBack=isValidContrct_clbck;
	//    stp_Callback.fp_AreContaractsIdenticalCallBack=areContrctIndentical_clbck;
	//    stp_Callback.fp_IsRetificationValidCallBack=isRatifValid_clbck;
	stp_Callback.fp_DateAndTimeCallBack=DateAndTimeCallBack;
	//    stp_Callback.fp_addMsg2LogCallBack=addMssgToLog_clbck;
	//    stp_Callback.fp_IsChangeContractStatus2InvalidAllowed=isChangeContrctToInvalidStatus_clbck;
	//    stp_Callback.fp_uch_GetProviderId=getPrvdrId_clbck;
	//    stp_Callback.fp_b_StoreTktRecoveryDate=storeTcktRecovery_clbck;
	//    stp_Callback.fp_b_GetStorageTktRecoveryDate=getStorageTcktRecovery_clbck;
	//    stp_Callback.fp_b_GetParams=getParams_clbck;
#ifdef NOT_RE_SELECT_AT_CARDIN	
	stp_Callback.e_WaitCard=WaitCard;
#endif

	v_ClyApp_SetUserCallBacks(&stp_Callback);//[IN] user callbacks



	return e_ClyApp_Ok;
}


////////////////////////////////////////////////////////////////////////////////////
//
//  TR_AutoInitReader : this will init the reader and use defaults.
//  Debug only in most cases
//
////////////////////////////////////////////////////////////////////////////////////

eCalypsoErr TR_InitReaderAuto(e_7816_DEVICE SamReaderId,e_7816_DEVICE CardReaderId,int SamUARTId)
{

	TR_st_Parameters Params;
	TR_st_SetTime    NewDate = {2012,2,1,12,0,0,0};
	eCalypsoErr err;

	err = TR_InitReader(NULL);
	if(err != e_ClyApp_Ok)
		return err;

	memset(&Params,0,sizeof(Params));
	Params.lv_DeviceNumber          = 115;
	Params.uc_ProviderId            = 0x14;
	Params.lv_StoredValueCeiling    = 100000;
	TR_SetParam(&Params);
#ifndef TIM7020 
	TR_SetTime(&NewDate);
#endif
	return e_ClyApp_Ok;


}
#ifndef ENABLE_COMM
TR_BOOL TIM7020TimeAndDateCallBack(TR_St_DateTime* trDt);
#endif

eCalypsoErr TR_InitReader(const st_InitResource* pData)
{
#if ANDROID//linux
	LOGI( "TR_InitReader");
#endif	
	volatile unsigned short m_Tw_Sam_Sn =  0;
	eCalypsoErr err;


	err = e_ClyApp_WrongParamErr;
	if(pData)
	{
		memset(&stInitFlags, 0, sizeof(stInitFlags));//init flags
#ifdef ENABLE_COMM
		if(pData->TimeAndDateCB)
		{
			InitRes.TimeAndDateCB = pData->TimeAndDateCB; // override current CB
			stInitFlags.bIsTimeSet = 1;
		}
#else
		
		InitRes.TimeAndDateCB = TIM7020TimeAndDateCallBack; // override current CB
		stInitFlags.bIsTimeSet = 1;
#endif

		if(pData->pHandler && pData->ProtocolCB)
		{
			InitRes.pHandler = pData->pHandler;
			InitRes.ProtocolCB	= pData->ProtocolCB;
			stInitFlags.bIsProtocolInit = 1;
		}

#ifndef ENABLE_COMM
		if((stInitFlags.bIsTimeSet ))
			err = ClyInitInterface(e_7816_CONTACTLESS, e_7816_SAM_TW, 0);
#else
		if(stInitFlags.bIsTimeSet & stInitFlags.bIsProtocolInit)
		{
#if ANDROID//linux
			LOGI( "calling ClyInitInterface");
#endif
			err = ClyInitInterface( e_7816_SAM_CAL, e_7816_CONTACTLESS, 0);
		}
		else
		{
#if ANDROID//linux
			LOGI("not calling ClyInitInterface");
#endif

		}
#endif
	}
	if(err != e_ClyApp_Ok)
	{
#if ANDROID//linux
		LOGI( "TR_InitReader end err=%d", err);
#endif
		return err;
	}

	//LOGE("TR_InitReader memset(&g_CardInfo, 0, sizeof(g_CardInfo));");
	memset(&g_CardInfo, 0, sizeof(g_CardInfo));


	//open contracts file
	b_OpenContractsFile();

#ifdef TR1020_PERSONALIZATION

	stInitFlags.bSamInit = 1;//Fake SAM init
	stInitFlags.bWorkWithoutSam = 1;

#endif
#if ANDROID//linux
	LOGI( "TR_InitReader end err=%d", err);
#endif

	return  err;

}
#ifndef TIM7020
// This eill set a specific parameter, make sure not to overflow!
eCalypsoErr TR_SetParamByType(e_ClyApp_ParamType ParamType, long Value)
{

	switch(ParamType)
	{
	case e_PlaceUniqueId:               g_Params.us_PlaceUniqueId       = (unsigned short)Value; break;
	case e_DeviceNumber:                g_Params.lv_DeviceNumber        = Value; break;
	case e_ProviderId:                  g_Params.uc_ProviderId          = (unsigned char)Value; break;
	case e_StoredValueCeiling:          g_Params.lv_StoredValueCeiling  = Value; break;
	case e_MorningEndHour:              g_Params.us_MorningEndHour      = (unsigned short)Value; break;
	case e_EveningStartHour:            g_Params.us_EveningStartHour    = (unsigned short)Value; break;
	default:
		return e_ClyApp_NotOk;

	}

	return  e_ClyApp_Ok;
}
#endif
//this function sets the paramters that are a must for working with smart card
eCalypsoErr TR_SetParam(const TR_st_Parameters* pParams)
{

	//  check that parametrs are valid
	//  uc_ProviderId > 0 (and that's it)
	if(pParams->uc_ProviderId <=0)
	{
		return e_ClyApp_NotOk;
	}

	g_Params = *pParams;

	stInitFlags.bIsParamsSet=1;
	//Set GMT offset in Minuts
	g_time_zone_bias_minutes = pParams->lv_time_zone_bias_minutes;

	return  e_ClyApp_Ok;
}

///////////////////////////////////////////////////////////
// NAME: TR_SetNewBusLineParam,
// DESCRIPTION: sets parameters of new bus line 
// PARAMETERS:  Line and and Cluster
// PRE REQUIREMENT:  call to TR_InitReader
// RETURNS: eCalypsoErr result
///////////////////////////////////////////////////////////
eCalypsoErr TR_SetNewBusLineParam(const St_ChngTripcmd *p_TripData)      // MAKAT of MOT
{
	g_TripInfo=*p_TripData;
	return  e_ClyApp_Ok;
	
	
}

///////////////////////////////////////////////////////////
// NAME: TR_SetNewStationParam
// DESCRIPTION: sets parameters of new station
// PARAMETERS:  Station number
// PRE REQUIREMENT:  call to TR_InitReader
// RETURNS: eCalypsoErr result
///////////////////////////////////////////////////////////
eCalypsoErr TR_SetNewStationParam(const unsigned short  us_StationNumber)      // MAKAT of MOT
{
	if(us_StationNumber) // Non zero
	{
		g_Params.us_StationNumber = us_StationNumber;    // MAKAT of MOT
		return  e_ClyApp_Ok;
	}
	return e_ClyApp_NotOk; // error
}

#ifndef TIM7020
////////////////////////////////////////////////////////////////////////////
//	TR_AddAzmashRecords
//	Yoni 03/2013
//	This function adds array of azmash records to user allocated memory
//	Each call adds records from given offset in array
////////////////////////////////////////////////////////////////////////////
void TR_AddAzmashRecords(void* pMemoryAllocationForArray	//pointer to user allocated memory for the whole data structure 
	,const TR_St_AzmashRecord* pRecordsToAdd //records to add
	,int iStartFrom									//the next record from which to start adding (0 if first call)
	,int iCount											//how many records to add												 
	)
{
	int i;
	if(!pMemoryAllocationForArray)
		return;
	pAzmashRecords	=		pMemoryAllocationForArray;
  


	if(0==iStartFrom)
		pAzmashRecords->RecCount=0;

	for(i=0;i<iCount && (iStartFrom+i) < MAX_RECORDS_AZMASH_ARRAY;i++)
	{    
    pAzmashRecords->Records[iStartFrom+i] = pRecordsToAdd[i];
		pAzmashRecords->RecCount++;
	}

}
#endif

////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
static TR_BOOL bCheckCorrectDateTime(const TR_St_DateTime * dt)
{
	if(dt->Year<2011 || dt->Year>2031)
		return TR_FALSE;
	if(dt->Month>12 || dt->Month < 0)
		return TR_FALSE;
	if(dt->Day>31 || dt->Day < 0)
		return TR_FALSE;
	if(dt->Hour>23 || dt->Hour<0)
		return TR_FALSE;
	if(dt->Minute>59 || dt->Minute < 0)
		return TR_FALSE;
	if(dt->Second>59 || dt->Second<0)
		return TR_FALSE;

	return TR_TRUE;
}

////////////////////////////////////////////////////////////////////////////
//  TR_SetTime
//
//
////////////////////////////////////////////////////////////////////////////
#ifndef TIM7020 
eCalypsoErr TR_SetTime(const TR_st_SetTime * stDateTime)
{

	st_Time Now;
	g_time_zone_bias_minutes = stDateTime->lv_time_zone_bias_minutes;

#ifndef WIN32  && #ifndef linux
#ifndef TR1020_PERSONALIZATION    
	// No need to set if the RTC was allready set - Hardware only
	if(stDateTime->st_CurrDateTime.Year == 0) // If the year is null the caller did not want to set the clock so ..
	{
		if(RTCGetTime())  
		{
			stInitFlags.bIsTimeSet = 1;
			return e_ClyApp_Ok;
		}
	}
#endif
#endif

	if(bCheckCorrectDateTime(&stDateTime->st_CurrDateTime))
	{
		stInitFlags.bIsTimeSet = 1;

		// Set rtc:
		Now.ui_Year    = stDateTime->st_CurrDateTime.Year;
		Now.uc_Month   = (unsigned char)stDateTime->st_CurrDateTime.Month;
		Now.uc_Day     = (unsigned char)stDateTime->st_CurrDateTime.Day;
		Now.uc_Hour    = (unsigned char)stDateTime->st_CurrDateTime.Hour;
		Now.uc_Minute  = (unsigned char)stDateTime->st_CurrDateTime.Minute;
		Now.uc_Second  = (unsigned char)stDateTime->st_CurrDateTime.Second;

		i_TimeSet(&Now);
		// for debug only 
		i_TimeHGetNow(&Now);

		return e_ClyApp_Ok;
	}

	return  e_ClyApp_NotOk;
}
#endif
////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
int cnt = 1000;
static const unsigned char  empty_buff[20] = {0};
eCalypsoErr TR_IsCardIn(TR_BOOL* cardIn,TR_st_CardInfo* pInfo)
{
	//LOGE("called TR_IsCardIn");
	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;

	//Yoni 7/8/14
	if(memcmp(g_CardInfo.m_serialNumber, empty_buff, sizeof(g_CardInfo.m_serialNumber))!= 0)
	{
		*cardIn = TR_TRUE;
		*pInfo = g_CardInfo;
		return  e_ClyApp_Ok;
	}

	*cardIn = b_clyapp_IsCardIn(pInfo);
	if(*cardIn)
	{
		//save current card info in global
		g_CardInfo=*pInfo;
	
	}
	else
	{
		//LOGE("TR_IsCardIn memset(&g_CardInfo, 0, sizeof(g_CardInfo));");
		memset(&g_CardInfo, 0, sizeof(g_CardInfo));
	}
	return  e_ClyApp_Ok;
}
#ifndef TIM7020
TR_BOOL TR_wait_card(int TimeOut)
{
	eCalypsoErr e;
	TR_BOOL cardIn;
	TR_st_CardInfo  pInfo;
	long start=CoreGetTickCount();
	while((long)(CoreGetTickCount()-start)<TimeOut)
	{
		e=TR_IsCardIn(&cardIn,&pInfo);
		if((e== e_ClyApp_Ok) &&  cardIn)
			return 1;

		CoreDelay(10000);

	}
	return 0;

}
#endif
////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_ForgetCard(void)
{
	//LOGE("TR_ForgetCard");
	clyapp_ForgetCard();
	memset(&g_CardInfo, 0, sizeof(g_CardInfo));
	return   e_ClyApp_Ok;
}

#ifndef TIM7020 //TBD:yoram
static TR_st_BIT_Info stat_LastBITResult = {0};

#ifndef WIN32  && #ifdef linux
////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_GetLastBITResult(TR_st_BIT_Info* pInfo)
{
	if(pInfo)
	{   
		*pInfo= stat_LastBITResult;
		return   e_ClyApp_Ok;
	}
	return  e_ClyApp_NotOk;
}


////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
uint8_t    CoreInitFlagResult=0xff;               // Core init bitmap flags
void v_SetFlagResult(uint8_t flagresult)
{
	CoreInitFlagResult=flagresult;
}

eCalypsoErr TR_RunBIT(TR_st_BIT_Info* pInfo)
{
	if (!pInfo)
		return e_ClyApp_InvalidArg;

	memset (&stat_LastBITResult, 0, sizeof(TR_st_BIT_Info));

	////////////////////////
	// ToDO
	////////////////////////
	//
	if((CoreInitFlagResult&CORE_ERROR_INIT_SDHC)==0)
		stat_LastBITResult.FileSystem_Ok = 0;

	stat_LastBITResult.F_RAM_Ok = 0;
	stat_LastBITResult.Power_Ok = 0;
	if((CORE_ERROR_INIT_MFRC& CoreInitFlagResult)==0)
		stat_LastBITResult.RFID_Ok = 1;
	//
	////////////////////////

	// BIT done
	stat_LastBITResult.IsBITDone = 1;

	// Return result
	memcpy(pInfo, &stat_LastBITResult, sizeof(TR_st_BIT_Info));

	// All OK
	return  e_ClyApp_Ok;

}
#else


////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_GetLastBITResult(TR_st_BIT_Info* pInfo)
{
	if (!pInfo)
		return e_ClyApp_InvalidArg;

	// Return result
	memcpy(pInfo, &stat_LastBITResult, sizeof(TR_st_BIT_Info));

	// All OK
	return  e_ClyApp_Ok;
}


////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_RunBIT(TR_st_BIT_Info* pInfo)
{
	if (!pInfo)
		return e_ClyApp_InvalidArg;

	memset (&stat_LastBITResult, 0, sizeof(TR_st_BIT_Info));

	////////////////////////
	// ToDO
	////////////////////////
	//
	stat_LastBITResult.FileSystem_Ok = 1;
	stat_LastBITResult.F_RAM_Ok = 1;
	stat_LastBITResult.Power_Ok = 1;
	stat_LastBITResult.RFID_Ok = 1;
	//
	////////////////////////

	// BIT done
	stat_LastBITResult.IsBITDone = 1;

	// Return result
	memcpy(pInfo, &stat_LastBITResult, sizeof(TR_st_BIT_Info));

	// All OK
	return  e_ClyApp_Ok;
}
#endif
#endif
////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////


TR_BOOL TR_IsEnvironmentEmpty(void)
{
	return b_clyapp_IsCardEmptyEnv();

}

////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_GetEnvironmentData(TR_st_EnvironmentData* pEnv)
{

	st_Cly_DateAndTime ov_DtIssuingDate={0};
	st_Cly_DateAndTime ov_DtEndDate={0};
	st_Cly_DateAndTime ov_DtBirthDate={0};
	st_Cly_DateAndTime ov_DtProf1Date={0};
	//  st_Cly_DateAndTime ov_DtProf2Date={0};
	st_ClyApp_EnvAndHoldDataStruct ov_Env={0};

	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;

	if(!g_CardInfo.IsEnvOk){
		//LOGE("!g_CardInfo.IsEnvOk");
		return e_ClyApp_CardEnvErr;
	}

	if(e_ClyApp_GetEnvironment(&ov_Env) == e_ClyApp_Ok)
	{

		pEnv->AppVer      = ov_Env.uc_EnvApplicationVersionNumber;
		pEnv->sh_EnvCountryld = ov_Env.sh_EnvCountryld;
		pEnv->uc_Envlssuerld  = ov_Env.uc_Envlssuerld;
		pEnv->ul_EnvApplicationNumber = ov_Env.ul_EnvApplicationNumber;

		//translate issuing date
		ov_DtIssuingDate.st_Date = ov_Env.st_EnvlssuingDate;
		ClyDateAndTimetoTrDateTime(&ov_DtIssuingDate, &pEnv->st_EnvlssuingDate);

		//translate end date
		ov_DtEndDate.st_Date = ov_Env.st_EnvEndDate;
		ClyDateAndTimetoTrDateTime(&ov_DtEndDate, &pEnv->st_EnvEndDate);

		//translate birth date
		ov_DtBirthDate.st_Date = ov_Env.st_HolderBirthDate;
		ClyDateAndTimetoTrDateTime(&ov_DtBirthDate, &pEnv->st_HolderBirthDate);

		pEnv->uc_EnvPayMethod = ov_Env.uc_EnvPayMethod;
		pEnv->sh_HolderCompany  = ov_Env.sh_HolderCompany;
		pEnv->ul_HolderCompanylD= ov_Env.ul_HolderCompanylD;
		pEnv->CustomerLangUage = ov_Env.uc_HolderLanguage;
		pEnv->ul_HolderldNumber = ov_Env.ul_HolderldNumber;

		//prof1
		ov_DtProf1Date.st_Date = ov_Env.st_HoiderProf1.st_HoiderProfDate;
		ClyDateAndTimetoTrDateTime(&ov_DtProf1Date, &pEnv->st_HoiderProf1.st_HolderProfDate);
		pEnv->st_HoiderProf1.uc_HolderProfCode  = ov_Env.st_HoiderProf1.uc_HoiderProfCode;
		pEnv->st_HoiderProf1.IsValid  =  clyapp_bIsProfileValid(pEnv->st_HoiderProf1.uc_HolderProfCode);

		//prof2
		ov_DtProf1Date.st_Date = ov_Env.st_HoiderProf2.st_HoiderProfDate;
		ClyDateAndTimetoTrDateTime(&ov_DtProf1Date, &pEnv->st_HoiderProf2.st_HolderProfDate);
		pEnv->st_HoiderProf2.uc_HolderProfCode  = ov_Env.st_HoiderProf2.uc_HoiderProfCode;
		pEnv->st_HoiderProf2.IsValid= clyapp_bIsProfileValid(pEnv->st_HoiderProf2.uc_HolderProfCode);



		pEnv->bIsAnonymous = b_IsAnonymousEnv(&ov_Env);

		return e_ClyApp_Ok;

	}
	return  e_ClyApp_NotOk;
}



////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////




eCalypsoErr TR_GetListForReportAndReload(TR_st_AllContracts* array/*out*/)
{
	//  //this function fills array with all contracts on card/tickets
	int num_contracts,i;
	eCalypsoErr err;
	int StoreValueCount=0;

	union_ClyApp_ContractRecord ContractsArr[MAX_CONTRACT_COUT]={0};
	st_ProcessedContractValidityInfo ProcessedInfoArr[MAX_CONTRACT_COUT]={0};


	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;

#ifdef NOT_RE_SELECT_AT_CARDIN
	err=WaitCard(5,NULL);
	if(err!=e_ClyApp_Ok)
		return err;
#endif

	if(!bCardWasRead())
		return e_ClyApp_NotOk;

	memset(array, 0, sizeof(*array));


	array->m_FlagIsLock = g_CardInfo.IsCardLock;
	array->m_FlagIsEnvaromentOk = g_CardInfo.IsEnvOk;


	err = e_clyapp_GetContractsForLoadOrReport(ContractsArr, ProcessedInfoArr, &num_contracts);

	if(err != e_ClyApp_Ok)
		return err;

	/*
	// test if it is store value 
	if(ov_ContractData.ucEtt/10==6)
	StoreValueCount++;

	*/
	for(i=0;i<num_contracts;i++)
	{
		int ett;
		ett=i_GetEttType(&ContractsArr[i]);
		if(ett/10==6)
			StoreValueCount++;

	}


	for(i=0;i<num_contracts;i++)
	{
		//convert to TR_St_ContractData
		TR_St_ContractReportData ov_ContractData={0};
		v_GetContractReportData(&ContractsArr[i], &ProcessedInfoArr[i], &ov_ContractData);


		if(ov_ContractData.ucEtt/10==6)
		{
			// there is more than one store value so just the valid is relevant 
			if(StoreValueCount>1)
			{
				if(ov_ContractData.usValidityStatus==e_Invalid)
					ov_ContractData.usValidityStatus=e_Undefined;
			}
		}


		//add to array
		array->Contracts[array->ContractsCount]=ov_ContractData;
		array->ContractsCount++;

	}

	return  e_ClyApp_Ok;
}


////////////////////////////////////////////////////////////////////////////
// b_IsContractValidInCurrentZone
// check that contract is valid for given cluster and zone
// Pre: contract is zonal (if it was originaly interop contract, already translated)
//
////////////////////////////////////////////////////////////////////////////
static TR_BOOL  b_IsContractValidInCurrentZone(const TR_St_ContractForUse* pContract,
	unsigned short usCurrCluster, unsigned short usCurrZone)
{
	if(pContract->usCluster == usCurrCluster)
	{
		if((1<<(usCurrZone-1)) & pContract->sZoneBitmap)
			return TR_TRUE;
	}

	return TR_FALSE;
}

#ifndef TIM7020
////////////////////////////////////////////////////////////////////////////
// Yoni 03/2013
// b_CheckContractInAzmashTable
// 
// return true if azmash table (global) contains a record with given Azmash1,Azmash2 and FareCode
// Azmash 1/2 are interchangable
//	logic: linear search
////////////////////////////////////////////////////////////////////////////
static TR_BOOL b_CheckContractInAzmashTable(unsigned char ucAzmash1, unsigned char ucAzmash2, unsigned char ucFareCode)
{
	int i;

	if(!pAzmashRecords)
		return TR_FALSE;

	for(i=0;i<pAzmashRecords->RecCount;i++)	
	{

		if(pAzmashRecords->Records[i].usPriceCode == ucFareCode)
		{
			if(pAzmashRecords->Records[i].ucSrc == ucAzmash1 &&
				pAzmashRecords->Records[i].ucDst == ucAzmash2)			 
			{				
				return TR_TRUE;
			}
			//or the opposite direction
			if(pAzmashRecords->Records[i].ucSrc == ucAzmash2 &&
				pAzmashRecords->Records[i].ucDst == ucAzmash1)			 
			{				
				return TR_TRUE;
			}

		}

	}

	return TR_FALSE;
}
#endif 
////////////////////////////////////////////////////////////////////////////
// Yoni 03/2013
// b_IsContractOkForUseHere
// 
// return true if contract can be used in current location
// different logic for each contract type 
////////////////////////////////////////////////////////////////////////////
#ifndef TIM7020
static TR_BOOL  b_IsContractOkForUseHere(const TR_St_ContractForUse* pContractData, unsigned short usCurrCluster, unsigned short usCurrZone, TR_BOOL* pHashlamaNeeded /*out*/)
{

	*pHashlamaNeeded = TR_FALSE;
	//if stored value
	if(IsStoredValue(pContractData->ucEtt))
	{
		//only predefine 200
		if(pContractData->sPredefinedCode != g_Params.us_StoredValuePredefineCode)
			return TR_FALSE;

		//in maavar check azmash
		if(pContractData->bIsInterchangeValid)
		{
			//check that fare code and azmash of first trip, and current azmash exist in azmash table. if not hashlama is needed
			*pHashlamaNeeded = !b_CheckContractInAzmashTable(g_Params.uc_CurrAzmash, pContractData->ucInterchangeRights, pContractData->FareCode/*pContractData->FareCode*/);
		}

		return TR_TRUE;

	}
	else if(pContractData->ucInterchangeType == e_Maavar)
	{
		//if kartisia maavar check azmash
		//in maavar check azmash in first trip (ucInterchangeRights) and curr azmash 
		if(pContractData->bIsInterchangeValid)
		{
			return b_CheckContractInAzmashTable(g_Params.uc_CurrAzmash, pContractData->ucInterchangeRights, pContractData->FareCode);
		}
		else
		{
			//when first trip check curr azmash only (curr azmash -> curr azmash)
			return b_CheckContractInAzmashTable(g_Params.uc_CurrAzmash, g_Params.uc_CurrAzmash, pContractData->FareCode);
		}

	}
	else if(pContractData->ucInterchangeType == e_OneHemshech || pContractData->ucInterchangeType == e_TwoHemshech)
	{
		//if kartisia hemshech
		//check zone
		return b_IsContractValidInCurrentZone(pContractData, usCurrCluster, usCurrZone);
	}

	//default check  zone
	return b_IsContractValidInCurrentZone(pContractData, usCurrCluster, usCurrZone);

}
#endif
////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_GetListForUse(TR_st_ContractsForUseResponse* pContractsForUse)
{
	//  //this funcrion returns an array of all contracts that can be used now (no check for validity location)
	eCalypsoErr err;
	int num_valid,i;
	union_ClyApp_ContractRecord ContractsArr[MAX_CONTRACT_COUT]={0};
	st_ProcessedContractValidityInfo ProcessedInfoArr[MAX_CONTRACT_COUT]={0};
	//unsigned char Pos;
	TR_St_EventForValidityCheck EventsArr[3];
	memset(EventsArr, 0, sizeof(EventsArr));


	memset(pContractsForUse, 0, sizeof(*pContractsForUse));

	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;


	if(!bCardWasRead())
		return e_ClyApp_NotOk;


	pContractsForUse->m_FlagIsLock = g_CardInfo.IsCardLock;
	pContractsForUse->m_FlagIsEnvironmentOk = g_CardInfo.IsEnvOk;

#ifndef TIM7020
	//Yoni 04/2013
	//check card reuse
	// test if the card used in the same station before the limit time 
	if(TR_GetRegularEvents(EventsArr) != e_ClyApp_Ok)
			return e_ClyApp_CardEventErr;
	
  for(Pos = 0;Pos < 3;Pos++)
  {
		if((EventsArr[Pos].EventPlace == g_Params.us_PlaceUniqueId) &&  ( (EventsArr[Pos].EventCircumstances == e_CardEventCircumEntry) || (EventsArr[Pos].EventCircumstances == e_CardEventCircumInterchangeEntry)))
    {

			st_Cly_DateAndTime clyDt;
			unsigned long EventTime=0,CurrTime=0;
			st_Cly_DateAndTime ov_DateAndTime;

			//calc now local
			DateAndTimeCallBack(&ov_DateAndTime);
			CurrTime = ul_GetTimeReal(&ov_DateAndTime, 0);	
			
			//calc event time local
			v_TrDateTimeToClyDateAndTimet(&EventsArr[Pos].DateTime/*in*/, &clyDt/*out*/);
			EventTime = ul_GetTimeReal(&clyDt, 0);


      if((EventTime + (g_Params.us_ReuseLimitInMinutes * 60)) >  CurrTime)
			{						
				return e_ClyApp_IllegalReuseInStation;
			}
     }
	 }
		
#endif


	err = e_clyapp_GetContractsForUse(ContractsArr, ProcessedInfoArr, &num_valid);

	if(err != e_ClyApp_Ok)
		return err;

	for(i=0;i<num_valid;i++)
	{
		//TR_BOOL bHashlamaNeeded = TR_FALSE;

		TR_St_ContractForUse ov_ContractData={0};


		// convert to TR_St_ContractData
		v_GetContractDataForUse(&ContractsArr[i], &ProcessedInfoArr[i], &ov_ContractData);

#ifndef TIM7020
		//filter out contracts that aren't valid for current cluster/zone/azmash
		//if(IsStoredValue(ov_ContractData.ucEtt) || b_IsContractValidInCurrentZone(&ov_ContractData, p_Request->usCurrentCluster, p_Request->usCurrentZone))
		if(b_IsContractOkForUseHere(&ov_ContractData, p_Request->usCurrentCluster, p_Request->usCurrentZone, &bHashlamaNeeded))
#endif
		{
			//add to array
			//bcpl[pContractsForUse->ContractsCount]=ProcessedInfoArr[i].BCPL_Priority;
			pContractsForUse->Contracts[pContractsForUse->ContractsCount]=ov_ContractData;
			//TBD:yoram remove :pContractsForUse->Contracts[pContractsForUse->ContractsCount].bIsHashlamaForStoredValueNeeded = bHashlamaNeeded;
			pContractsForUse->ContractsCount++;

		}


	}

	//Now sort contracts
	//No sorting 7020
	//v_SortContracts(pContractsForUse->Contracts, pContractsForUse->ContractsCount);

	return  e_ClyApp_Ok;
}

/*
//! Byte swap unsigned int
unsigned long  swap_uint32( unsigned long  val )
{
val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
return (val << 16) | (val >> 16);
}


//Yoni 02/2013
//convert buffer of 3 bytes which represents a value in big endian, to unsigned long in little endian
unsigned long ul_Convert3ByteBigEndianToUnsignedLongLittle(const char in[3])
{
char tmp[4];
memset(tmp, 0, sizeof(tmp));
memcpy(&tmp[1], in, 3);
return swap_uint32(*(unsigned long*)tmp);
}
*/

unsigned long ul_Convert3ByteBigEndianToUnsignedLongLittle(const char in[3]);
////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
	static void v_CopyTransactionBinaryData(int RecNum,
		const union_ClyApp_TransactionBinData* clyappBinData,
		TR_St_TransactionData* pTrBinData/*out*/,int CounterFlag)
	{
		memcpy(pTrBinData->Env,     clyappBinData->st_CardTransactionBinData.ucp_EnvironmentData, sizeof(pTrBinData->Env));
		memcpy(pTrBinData->Contract,  clyappBinData->st_CardTransactionBinData.ucp_ContractData,  sizeof(pTrBinData->Contract));
		memcpy(pTrBinData->Event1,    clyappBinData->st_CardTransactionBinData.ucp_Event1,      sizeof(pTrBinData->Event1));
		memcpy(pTrBinData->Event2,    clyappBinData->st_CardTransactionBinData.ucp_Event2,      sizeof(pTrBinData->Event2));
		memcpy(pTrBinData->Event3,    clyappBinData->st_CardTransactionBinData.ucp_Event3,      sizeof(pTrBinData->Event3));
		//copy only the relevant counter
		if(RecNum>=1 && RecNum<=8)
		{	
		 unsigned short usAuthorizationCode=0;
		 char TmpBuffer[15]; 
		 if(CounterFlag)
		 {
		
			//memcpy(pTrBinData->cCounter,    clyappBinData->st_CardTransactionBinData.ucp_Counter+3*(RecNum-1),      sizeof(pTrBinData->cCounter));
			//03/2013
			
			unsigned long ulCounterAsLong = ul_Convert3ByteBigEndianToUnsignedLongLittle((const char*)clyappBinData->st_CardTransactionBinData.ucp_Counter+3*(RecNum-1));
#ifdef WIN32
 #pragma warning(push)
 #pragma warning(disable : 4996) // CRT Secure - off
#endif
			
			//get string from number
			sprintf(TmpBuffer, "%08lu", ulCounterAsLong);	
#ifdef WIN32
#pragma warning(pop)
#endif

			//copy 8 bytes
			memcpy(pTrBinData->cCounterStr, TmpBuffer,	sizeof(pTrBinData->cCounterStr));

		 }
		 else
		{
			memset(pTrBinData->cCounterStr, '0', sizeof(pTrBinData->cCounterStr));//03/2013 all '0' in ascii
			
		}
			//04/2013
			//write authorizaion code in ascii
			usAuthorizationCode = us_GetAuthorizationCodeAsciiFromContractList(clyappBinData->st_CardTransactionBinData.ucp_ContractList, (unsigned char)RecNum);
#ifdef WIN32
 #pragma warning(push)
 #pragma warning(disable : 4996) // CRT Secure - off
#endif

			sprintf(TmpBuffer, "%05d", usAuthorizationCode);
#ifdef WIN32
#pragma warning(pop)
#endif

			memcpy(pTrBinData->cAuthorizationCodeStr, TmpBuffer, 5);
		
		
	}
	}



//function to check if contract to load is identical to existing contract (or 'contained')
static TR_BOOL bComparePeriodContracts(const TR_St_ContractReportData* pExistingContract, //contract on card
	const TR_St_LoadContract* pContractToLoad    //contract to load
	)
{


	long lExistingStart=0, lLoadStart=0, lExistingEnd=0, lLoadEnd=0;
	//check the same spatial type: 
	if(pExistingContract->ucSpatialType !=  pContractToLoad->ucSpatialType)
	{
		return TR_FALSE;
	}

	//check same cluster for spatial type: zone, code 
	if((pExistingContract->ucSpatialType == e_CardSpatialTypeZones ||
        pExistingContract->ucSpatialType == e_CardSpatialTypeFareCode) &&
		pExistingContract->usCluster != pContractToLoad->usCluster) 
	{
		return TR_FALSE;
	}
	//check overlapping zones  (check that bits that are on in new contract are on in existing)
	if((pExistingContract->sZoneBitmap & pContractToLoad->sZoneBitmap) != pContractToLoad->sZoneBitmap)
	{
		return TR_FALSE;
	}

	//check if different fare code
	if((pExistingContract->sFareCode != pContractToLoad->sFareCode))
	{
		return TR_FALSE;
	}

	if((((int)pExistingContract->ucEtt)/10)!=2)
		return TR_FALSE;



	lExistingStart = l_DateTimeToLong(&pExistingContract->st_ContractValidityStartDate);
	lExistingEnd  = l_DateTimeToLong(&pExistingContract->st_ContractValidityEndDate);


	if((pExistingContract->st_ContractValidityStartDate.Year==0)|| (pExistingContract->st_ContractValidityEndDate.Year==0))
		return TR_FALSE;



	lLoadStart = l_DateTimeToLong(&pContractToLoad->st_ContractValidityStartDate);
	lLoadEnd = l_DateTimeToLong(&pContractToLoad->ov_SpecificLoadData.ov_PeriodLoadData.st_ContractValidityEndDate);

	if(lExistingStart <= lLoadStart)
	{
		if(lExistingEnd >= lLoadEnd)
			return TR_TRUE;
	}


	return TR_FALSE;
}

////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
#ifdef NOT_RE_SELECT_AT_CARDIN	
TR_st_CardInfo ov_LastCardInfo;
static eCalypsoErr WaitCard(int retries,int *p_IsCardReseleted)
{
	int  Count=0;
	TR_st_CardInfo_Out *CardInfo,ov;

	CardInfo=&ov;

	if(p_IsCardReseleted)
		*p_IsCardReseleted=0;


	while(1)
	{

		if(e_clyapp_CheckCardComm()== e_ClyApp_Ok)
			break;

		if(Count++>retries)
			return 	e_ClyApp_CardReadErr;
		else
		{
			if((Count>0 && ((Count%2) ==0)))
			{
				TR_ForgetCard();
				if(p_IsCardReseleted)
					*p_IsCardReseleted=1;

				CoreDelay(50);
				TR_IsCardIn(&(CardInfo->IsCardIn), &(CardInfo->CardInfo));	

				if(CardInfo->IsCardIn)
				{

					//save cardinfo
					ov_LastCardInfo = CardInfo->CardInfo;			
					CardInfo->IsCardIn = TR_TRUE;
				}
				else
					CardInfo->IsCardIn = TR_FALSE;

			}

		}
	}

	return e_ClyApp_Ok;
}
#endif



///////////////////////////////////////////////////////////
// NAME: TR_IsFreeRecExistForLoad
// DESCRIPTION: 
// PARAMETERS: none
// PRE REQUIREMENT: cardin
// RETURNS: e_ClyApp_Ok if exists, else e_ClyApp_CardIsFull 
// Alg: call e_ClyApp_IsFreeRecExist for monthly 
/////////////////////////////////////////////////////////
eCalypsoErr TR_IsFreeRecExistForLoad(void)
{
	clyApp_BOOL b_Result;
	unsigned char first_free_index;
	if(e_ClyApp_IsFreeRecExist(&b_Result,&first_free_index)!=e_ClyApp_Ok || b_Result!=clyApp_TRUE)
	{
		return e_ClyApp_CardIsFull;
	}
	else return e_ClyApp_Ok;
}

eCalypsoErr TR_IsPossibleLoad(const TR_St_LoadContract* pLoadData)
{
	eCalypsoErr err;
	clyApp_BOOL b_Result;//[OUT]1=free record exist ok,0=no free record
	unsigned char first_free_index;///[out] first free index or ff if not found
	//int Count=0;

	//make some validation on pLoadData
	if(pLoadData->ucEtt == ETT_FOR_QUERY)
		return e_ClyApp_Ok;


	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;

#ifdef NOT_RE_SELECT_AT_CARDIN
	err=WaitCard(6,NULL);
	if(err!=e_ClyApp_Ok)
		return err;
#endif

	//card was read
	if(!bCardWasRead())
		return e_ClyApp_CardReadErr;


	//check card comm
	//if(e_clyapp_CheckCardComm() != e_ClyApp_Ok)
	//return e_ClyApp_CardReadErr;
	/*	
	Count=0;
	while(1)
	{

	if(e_clyapp_CheckCardComm()== e_ClyApp_Ok)
	break;
	CoreDelay(10000);
	if(Count++>5)
	return 	e_ClyApp_CardReadErr;
	}
	*/  





	//check connection with sam (local/remote), wnen there is a sam
	if(!stInitFlags.bWorkWithoutSam && e_clyapp_CheckSamComm() != e_ClyApp_Ok)
		return e_ClyApp_ReaderSamErr;


	//e_ClyApp_IsFreeRecExist
	if(e_ClyApp_IsFreeRecExist(&b_Result,&first_free_index/*,(e_EttType)pLoadData->ucEtt*/)!=e_ClyApp_Ok || b_Result!=clyApp_TRUE)
	{
		return e_ClyApp_CardIsFull;
	}

	//check conditions for stored value
	if(IsStoredValue(pLoadData->ucEtt))
	{
		unsigned long SVTotalAmount;
		long SVIndex;
		long lLoadAmount = pLoadData->ov_SpecificLoadData.ov_StoredValueLoadData.ul_StoredValueSumToLoadAgorot;
		err = e_ClyApp_CheckConditionsForStoredValue(pLoadData->sPredefinedCode, lLoadAmount, &SVTotalAmount/*out*/, &SVIndex/*out*/);
		if(err != e_ClyApp_Ok)
			return err;

	}
	else
	{
		static TR_st_AllContracts ov_AllContracts;
		//if we are loading a period calendar contract , check if identical contract already exists
		//   if(pLoadData->ucEtt / 10 == 2 && pLoadData->sPredefinedCode==e_Calendar) //period contract
		if(pLoadData->ucEtt / 10 == 2 && pLoadData->ov_SpecificLoadData.ov_PeriodLoadData.usPeriodStartType==e_Calendar) //period contract

		{
			if(e_ClyApp_Ok == TR_GetListForReportAndReload(&ov_AllContracts/*out*/))
			{
				int i;
				for(i=0;i<ov_AllContracts.ContractsCount;i++)
				{
					if(ov_AllContracts.Contracts[i].usValidityStatus != e_Invalid)
					{
						if(bComparePeriodContracts(&ov_AllContracts.Contracts[i], pLoadData) == TR_TRUE)
						{
							return e_ClyApp_CanNotLoadOverWriteSameContract;//identical
						}
					}
				}
			}
			else return e_ClyApp_NotOk;//shouldn't happen
		}

	}


	//todo


	return  e_ClyApp_Ok;
}




////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_Load(const TR_St_LoadContract* pLoadData, TR_St_LoadContractResponse* pResponse)
{
	eCalypsoErr err;
	int index;
#if linux
	if(pLoadData != NULL)
		LOGI("calling TR_Load with loadData->ov_SpecificLoadData.ov_PeriodLoadData.usDurationCount=%d", pLoadData->ov_SpecificLoadData.ov_PeriodLoadData.usDurationCount );
#endif

#ifdef NOT_RE_SELECT_AT_CARDIN
	int IsCardReSelected;
#endif
	clyApp_BOOL b_Result;
	int CounterFlag=1;
	union_ClyApp_ContractRecord   ov_ContractToLoad={0};
	st_clyApp_CardEventDataStruct ov_CardEventDataStruct={0};
	TR_St_CancelData union_BinDataForCancel={0};
	union_ClyApp_TransactionBinData union_TransactionBinData={0};
	unsigned long SV_SumToLoad=0;




	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;



	//24/7/13 workaround 
	//use the load command to get cancel transaction of stored value, when loading stored value on existing
//	if(pLoadData->ucEtt == ETT_FOR_QUERY)
//	{
//		return e_GetCardSnapshotOfStoredValueCancel(&pResponse->TransactionData/*out*/);
//	}
//	else
//		v_ClyApp_EmptyStoreValueTransactionData();		

#ifdef NOT_RE_SELECT_AT_CARDIN
	err=WaitCard(6,&IsCardReSelected);
	if(err!=e_ClyApp_Ok)
		return err;
	else
	{
		if(IsCardReSelected)
			TR_IsPossibleLoad(pLoadData);
	}
#endif

	if(!bCardWasRead())
		return e_ClyApp_NotOk;

	if(e_clyapp_CheckCardComm() != e_ClyApp_Ok)
		return e_ClyApp_NotOk;


	v_FillContractForLoad(pLoadData, &ov_ContractToLoad);

	v_FillLoadingEvent(pLoadData, &ov_CardEventDataStruct);

	if(IsStoredValue(pLoadData->ucEtt))
	{
		v_ClyApp_EmptyStoreValueTransactionData();
		SV_SumToLoad = pLoadData->ov_SpecificLoadData.ov_StoredValueLoadData.ul_StoredValueSumToLoadAgorot;
	}

	err =  e_ClyApp_LoadContract(&ov_ContractToLoad,//[IN]the contract record to load
		&ov_CardEventDataStruct,//[IN] parameter relevant only for card
		SV_SumToLoad,
		pLoadData->usWhiteListId,
		&union_BinDataForCancel,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
		&union_TransactionBinData,//[OUT]
		&b_Result,  //[OUT]1=Load OK ,0=Load Fail
		&index
		);



	if(err != e_ClyApp_Ok  )
	{

#ifdef NOT_RE_SELECT_AT_CARDIN	

		// to avoid case of nesting open session 
		TR_ForgetCard();
		//TR_wait_card(100);
#endif
		return err;
	}

	// counter not use 	
	if(ov_ContractToLoad.st_CardContractRecord.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType==e_ClyApp_CounterNotUsed)
		CounterFlag=0;

	//set TR_St_LoadContractResponse
	memcpy(&pResponse->CancelData, &union_BinDataForCancel, sizeof(pResponse->CancelData));
	v_CopyTransactionBinaryData(index+1, &union_TransactionBinData, &pResponse->TransactionData,CounterFlag);

	//In case of stored value copy transaction data 
	if(IsStoredValue(pLoadData->ucEtt))
	{
		pResponse->uc_IsStoredValueCacelDataExist = e_GetCardSnapshotOfStoredValueCancel(&pResponse->StoredValueCancelTransactionData); // [out]
	}

#ifdef NOT_RE_SELECT_AT_CARDIN	

	// to avoid case of nesting open session 
	TR_ForgetCard();
	//TR_wait_card(100);
#endif

	return  e_ClyApp_Ok;

}

////////////////////////////////////////////////////////////////////////////
//	Yoni 23/7/13
//	e_GetCardSnapshotOfStoredValueCancel
//	call this after loading of stored value, to get the saved snapshot of card after cancel
//	return e_ClyApp_Ok if snapshot exists 
////////////////////////////////////////////////////////////////////////////
static unsigned char e_GetCardSnapshotOfStoredValueCancel(TR_St_TransactionData* pTransactionData/*out*/)
{
	union_ClyApp_TransactionBinData ov_TransactionBinData;
	unsigned char CancelledRecNum;
	int IsAllEmpty;
	e_ClyApp_GetSnapshotAfterStoredValueCancel(&CancelledRecNum, &ov_TransactionBinData,&IsAllEmpty);
	if(!IsAllEmpty)
	{
		v_CopyTransactionBinaryData(CancelledRecNum, &ov_TransactionBinData, pTransactionData, TR_TRUE);
	}
	

	return CancelledRecNum; //CancelledRecNum = 0 means the data doesn't exist
	
}


////////////////////////////////////////////////////////////////////////////
//	v_MarkSegment
//	Set bit.for kartisia hemshech  (benironi)
//	If first trip clr and set bit
//	else set bit on current 
////////////////////////////////////////////////////////////////////////////
static void v_MarkSegment(unsigned char* pucInterchangeRights/*in/out*/
	,unsigned char  ucSegmentNumber //1-3
	,TR_BOOL bIsFirstTrip)
{

	if(bIsFirstTrip)
	{
		*pucInterchangeRights=0;
		if(ucSegmentNumber>=1)
			*pucInterchangeRights |= 1<<(ucSegmentNumber-1);
	}
	else
	{
		if(ucSegmentNumber>=1)
			*pucInterchangeRights |= 1<<(ucSegmentNumber-1);
	}

}

////////////////////////////////////////////////////////////////////////////
//	TR_Use
//	
//
////////////////////////////////////////////////////////////////////////////


eCalypsoErr TR_Use(const TR_St_UseContractData* pUseData, TR_St_UseContractResponse* pResponse)
{
#if ANDROID//linux
	//__android_log_print(ANDROID_LOG_INFO,"CalypsoTiming","TR_Use start");
	LOGI(ANDROID_LOG_INFO,"CalypsoTiming","TR_Use start")
#endif
	eCalypsoErr err;
	int CounterFlag=1;
	st_clyApp_CardEventDataStruct st_CardEventDataStruct={0};
	struct_ClyTkt_Ticket ov_TicketEvent={0};
	union_ClyApp_TransactionBinData union_TransactionBinData={0};
	unsigned char uc_InterchangeRights=0;
	e_InterchangeType interchangeType=e_NoInterchange;
	union_ClyApp_ContractRecord Contract;

	clyApp_BOOL b_Result=clyApp_FALSE;

	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;


	if(e_clyapp_CheckCardComm() != e_ClyApp_Ok)
		return e_ClyApp_NotOk;

	//todo check that card wasn't replaced


	//determine the value of interchange rights
	err=e_ClyApp_GetContract(&Contract, (unsigned char)(pUseData->m_Contract_index_on_the_card+1));
	if(err != e_ClyApp_Ok)
	{
		return err;
	}

	interchangeType = e_GetInterchangeType(&Contract.st_CardContractRecord.st_CardContractIssuingData);

	if(e_Maavar == interchangeType)
	{
		if(pUseData->m_IsFirstTrip)
		{
			//in case of kartisia maavar or stored value, if this is first trip (nikuv) write the current azmash
			uc_InterchangeRights = pUseData->m_CurrAzmash;
		}
		else
		{
			//in maavar write the otiginal azmash
			st_clyApp_CardEventDataStruct ov_SpecialEvent;
			if(e_ClyApp_Ok == e_ClyApp_GetSpecialEvent((unsigned char)(pUseData->m_Contract_index_on_the_card+1) //contract rec num
				,&ov_SpecialEvent /*out*/))
			{
				if(ov_SpecialEvent.st_OptionalEventData.b_IsEventInterchangeRightsExist)
				{
					//should always get here, otherwise something is wrong
					uc_InterchangeRights = ov_SpecialEvent.st_OptionalEventData.uc_EventInterchangeRights;
				}
			}
		}
	}
	else if(e_OneHemshech == interchangeType || e_TwoHemshech == interchangeType)
	{		
		//in case of kartisia hemshech (benironi) write the updated bitmap of segments
		if(pUseData->m_IsFirstTrip)
		{				
			//set only the bit for segment
			v_MarkSegment(&uc_InterchangeRights, pUseData->m_SegmentForHemshech, TR_TRUE);			
		}
		else
		{
			st_clyApp_CardEventDataStruct ov_SpecialEvent;
			//set the bit for the segment and keep the previous bits
			//get the curr bitmap from releated special event
			if(e_ClyApp_Ok == e_ClyApp_GetSpecialEvent((unsigned char)(pUseData->m_Contract_index_on_the_card+1) //contract rec num
				,&ov_SpecialEvent /*out*/))
			{
				if(ov_SpecialEvent.st_OptionalEventData.b_IsEventInterchangeRightsExist)
				{
					//should always get here, otherwise something is wrong
					uc_InterchangeRights = ov_SpecialEvent.st_OptionalEventData.uc_EventInterchangeRights;
				}
			}
			v_MarkSegment(&uc_InterchangeRights, pUseData->m_SegmentForHemshech, TR_FALSE);			
		}

	}


	v_FillUseEvent((unsigned char)pUseData->m_PassengerCount, //1 or more
		//pUseData->m_ClusterNumber,
		g_TripInfo.m_Clustrer,
		pUseData->m_Code,
		pUseData->m_StoredValueSum,
		uc_InterchangeRights,//03/2013
		pUseData->m_IsFirstTrip,
		&st_CardEventDataStruct/*IN/OUT*/);

	err =  e_ClyApp_UseContract((unsigned char)pUseData->m_PassengerCount,//[IN] to be recored in the event record
		pUseData->m_StoredValueSum, //[IN] Amount (0 to 65535) of the contract counter - for stored value. Not relevalt for Season Pass
		0, //[IN] Add Amount (0 to 65535) to the contract counter - for stored value only!!!
		&st_CardEventDataStruct,//[IN] user event data - for card
		&ov_TicketEvent,//[IN] Ticket event data
		&pResponse->CancelData,//TR_St_CancelData *union_BinDataForCancle,//[OUT] copy of the binary data of the operation before the use operation - for cancellation purpose only
		&b_Result,
		(unsigned char)(pUseData->m_Contract_index_on_the_card+1),//[OUT]1=use ok,0=use fail
		(clyApp_BOOL)pUseData->m_IsFirstTrip,//[IN] false if nikuv is needed
		&union_TransactionBinData
		);
#ifdef NOT_RE_SELECT_AT_CARDIN	

	// to avoid case of nesting open session 
	TR_ForgetCard();
	//TR_wait_card(100);
#endif	

	if(err == e_ClyApp_Ok)
	{
		// counter not use 	
		if(pUseData->m_TokensCounter==0 && pUseData->m_StoredValueSum==0)    //fixed "&&" by Yoni 01/2013
			CounterFlag=0;

		v_CopyTransactionBinaryData((pUseData->m_Contract_index_on_the_card+1), &union_TransactionBinData, &pResponse->TransactionData,CounterFlag);

	}
#if ANDROID//linux
	//__android_log_print(ANDROID_LOG_INFO,"CalypsoTiming","TR_Use end");
	LOGI(ANDROID_LOG_INFO,"CalypsoTiming","TR_Use end")
#endif

	return err;

}
#if 0
static int GetLastOperationData(
	TR_CAncelParam *p_Result,int *p_DeviceId,int *p_Operator,int *p_TimePassInSecondes)
{
	eCalypsoErr e;
	union_ClyApp_EventRecord union_EventRecordArr[MAX_EVENT_COUT];
	clyApp_BOOL bIsEventOkArr[MAX_EVENT_COUT];
	e_clyApp_CardEventCircumstances eventtpe;
	union_ClyApp_ContractRecord Contract;
	int ContractIndex=0;
	st_ClyApp_CardContractValidLocZones *p_Zones;
	unsigned long EventTimeGmt,NowGmt;
	int def	;
	st_clyApp_CardContractIssuingData *p_Conract;
	int DecreaseSize=0;


	NowGmt=GetCurrGmtTime(1);

	///////// read last  event ////////////////

	e=e_ClyApp_GetAllEvent(union_EventRecordArr,bIsEventOkArr );

	if(e!=e_ClyApp_Ok)
		return 0;


	//// test if last event is loding or using 
	eventtpe=union_EventRecordArr[0].st_CardEventDataStruct.st_EventCode.e_CardEventCircumstances;

	if(!IsUseEvent(eventtpe) && !IsLoadEvent(eventtpe))
		return 0;

	EventTimeGmt=ul_GetTimeReal(&union_EventRecordArr[0].st_CardEventDataStruct.st_EventDateTimeStamp,1);


	ContractIndex=union_EventRecordArr[0].st_CardEventDataStruct.uc_EventContractPointer;

	if(union_EventRecordArr[0].st_CardEventDataStruct.st_OptionalEventData.b_IsEventPassengersNumberExist)
		DecreaseSize=union_EventRecordArr[0].st_CardEventDataStruct.st_OptionalEventData.b_IsEventPassengersNumberExist;
	else
		DecreaseSize=1;





	/////// read the contract that belong to last event

	e=e_ClyApp_GetContract(&Contract,ContractIndex);
	if(e!=e_ClyApp_Ok)
		return 0;

	p_Conract=&Contract.st_CardContractRecord.st_CardContractIssuingData;

	*p_DeviceId=p_Conract->sh_ContractSaleDevice;
	*p_Operator=p_Conract->uc_ContractProvider;

	p_Result->Ett=i_GetEttType(&Contract);

	// the first validity location must be zone
	if(p_Conract->st_OptionalContractData.st_ContractValidityLocationArr[0].
		e_CardSpatialType!=e_CardSpatialTypeZones)
		return 0;

	//p_Conract->st_OptionalContractData.st_ContractValidityLocationArr[0].e_CardSpatialType;




	p_Zones=&p_Conract->st_OptionalContractData.st_ContractValidityLocationArr[0].
		union_ContractValidityLocation.st_Zones;


	p_Result->Zones=p_Zones->ush_SpatialZones;
	p_Result->Cluster=p_Zones->ush_SpatialRoutesSystem;
	p_Result->Operation=p_Conract->uc_ContractProvider;



	if(p_Conract->st_OptionalContractData.b_ContractCustomerProfileExist)
		p_Result->Profile=p_Conract->st_OptionalContractData.uc_ContractCustomerProfile;
	else
		p_Result->Profile=0;

	if(p_Conract->st_ContractTariff.e_TariffCounterType==e_ClyApp_CounterAsNumOfToken)
		p_Result->l_incrementAmount=DecreaseSize;


	p_Result->ContractIndex=ContractIndex;


	//////////  calculate the time now in gmt - the event time
	def=NowGmt-EventTimeGmt;
	*p_TimePassInSecondes=def;

	return 1;


}
#endif


#ifndef TR1020_PERSONALIZATION
static int IsLegalEventForCancel(e_clyApp_CardEventCircumstances eventtpe)
{

	switch(eventtpe)
	{
	case e_CardEventCircumContractLoading:
	case e_CardEventCircumEntry:
		return 1;

	default:
		return 0;

	}

}


static int ContractNotBelongToThisDevice(const IsPossibleCancel* pIsPossible,int CurrProvider,int CurrDevice)
{
	/*
	1) test if the contract index point to a valid contract to be cancel (load or use )

	common test 
	the last event point to the correct index  and the event operation 

	the device id of    the event is the  same  
	the operator id of the event is the same

	*/
	eCalypsoErr e;
	union_ClyApp_EventRecord union_EventRecordArr[MAX_EVENT_COUT];
	clyApp_BOOL bIsEventOkArr[MAX_EVENT_COUT];
	int index;
	int Provider;
	int Device;

	union_ClyApp_ContractRecord Contract;
	int ContractIndex;
	st_clyApp_CardContractIssuingData *p_Conract;
	e_clyApp_CardEventCircumstances eventtpe;

	ContractIndex=pIsPossible->ContractNum;
	///////// read last  event ////////////////

	e=e_ClyApp_GetAllEvent(union_EventRecordArr,bIsEventOkArr );

	if(e!=e_ClyApp_Ok)
		return 0;


	//// test if last event is loding or using 
	index=union_EventRecordArr[0].st_CardEventDataStruct.uc_EventContractPointer;
	Provider=union_EventRecordArr[0].st_CardEventDataStruct.uc_EventServiceProvider;

	eventtpe=union_EventRecordArr[0].st_CardEventDataStruct.st_EventCode.e_CardEventCircumstances;

	if(!IsLegalEventForCancel(eventtpe))
		return 0;

	e=e_ClyApp_GetContract(&Contract,(unsigned char)ContractIndex);
	if(e!=e_ClyApp_Ok)
		return 0;

	p_Conract=&Contract.st_CardContractRecord.st_CardContractIssuingData;

	Device=p_Conract->sh_ContractSaleDevice;

	if(index!=pIsPossible->ContractNum)
		return 0;

//	if(Device != us_GetSaleDeviceNumber(CurrDevice))
//		return 0;

	if(Provider!=CurrProvider)// && IsLoadEvent(eventtpe))
		return 0;

	return 1;  
}
#endif //#ifndef TR1020_PERSONALIZATION
////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
static char *GetStringAfterZeror(char *m_serialNumber,int *Size,int max)
{
	char *p=m_serialNumber;

	int i;

	for(i=0;i<max && p[i]=='0' ;i++);
	*Size=max-i;
	return &p[i];


}
eCalypsoErr TR_IsPossibleCancel(const IsPossibleCancel* pIsPossible)
{

	char *p;
	int Size;
	if(IS_CALYPSO_LOCKED){
		//LOGE("IS_CALYPSO_LOCKED failed");

		return e_ClyApp_InterfaceNotInitErr;
	}

	if(g_sam_type == e_ClyApp_SamCV){
		//LOGE("e_ClyApp_SamCV failed");

		return e_ClyApp_WrongSamTypeErr;
	}

	if(!bCardWasRead()){
		//LOGE("bCardWasRead failed");

		return e_ClyApp_NotOk;
	}

	if(e_clyapp_CheckCardComm() != e_ClyApp_Ok){
		//LOGE(" e_clyapp_CheckCardComm failed");

		return e_ClyApp_NotOk;
	}

	p=GetStringAfterZeror((char*)pIsPossible->m_serialNumber,&Size,strlen((const char *)pIsPossible->m_serialNumber));
	//check serial number
	if(memcmp(p, g_CardInfo.m_serialNumber, Size))
	{
		//LOGE("memcmp");

		return e_ClyApp_NotOk;
	}
	#ifdef TR1020_PERSONALIZATION  //04/2013
		return e_ClyApp_NotOk;//we donr check ContractNotBelongToThisDevice in personalozation 
	#else
	if(ContractNotBelongToThisDevice(pIsPossible,g_Params.uc_ProviderId,g_Params.lv_DeviceNumber))
	{
		//LOGE("ContractNotBelongToThisDevice is false");

		return e_ClyApp_Ok;
	}
	else{
		//LOGE("ContractNotBelongToThisDevice is true");

		return e_ClyApp_NotOk;
	}
	#endif
}



////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_CancelOp(const TR_St_CancelData* pCancelData, //[IN]
	TR_St_TransactionData* pTransactionDataResponse //[OUT]
	)
{

	eCalypsoErr err;

	clyApp_BOOL b_Result=clyApp_FALSE;
	st_clyApp_CardEventDataStruct st_CardEventDataStructForCancel={0};
	union_ClyApp_TransactionBinData union_TransactionBinData={0};

	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;

	//fill event
	v_FillCancelEvent(&st_CardEventDataStructForCancel);

	err =  e_ClyApp_CancelOperationByContractIndex(pCancelData,//[IN]
		&st_CardEventDataStructForCancel,//[IN] parameter relevant only for card. defines the parameters of the cancle operation.
		&b_Result,
		&union_TransactionBinData
		);
#ifdef NOT_RE_SELECT_AT_CARDIN	

	// to avoid case of nesting open session 
	TR_ForgetCard();
	//TR_wait_card(100);
#endif	


	if(err == e_ClyApp_Ok)
	{

		v_CopyTransactionBinaryData(pCancelData->st_CardCancelData.uc_ContractRecNumBefore,
			&union_TransactionBinData, pTransactionDataResponse,1);
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
eCalypsoErr TR_CancelContract(const TR_St_CancelContract* pCancel, //[IN]
	TR_St_TransactionData* pTransactionDataResponse, //[OUT]
    int* pSpecialEventIndexToDelete //[OUT] if 0-3 then this special event needs to be deleted
	)
{
	eCalypsoErr err;
	st_clyApp_CardEventDataStruct st_CardEventDataStructForCancel={0};
	union_ClyApp_TransactionBinData union_TransactionBinData={0};

	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;

	//fill event
	v_FillCancelEvent(&st_CardEventDataStructForCancel);

	err =  e_ClyApp_CardCancelContract(pCancel->uc_RecNum,//[IN]
		&st_CardEventDataStructForCancel,//[IN] parameter relevant only for card. defines the parameters of the cancle operation.
		&union_TransactionBinData,
        pSpecialEventIndexToDelete
		);


	if(err == e_ClyApp_Ok)
	{ 
		v_CopyTransactionBinaryData(pCancel->uc_RecNum,
			&union_TransactionBinData, pTransactionDataResponse,1);
	}

	return err;

}
////////////////////////////////////////////////////////////////////////////
//
// Sets the calypso mode to virtual write\read or normal
// Virtual mode will make all write operation virtual (only in memory)
// Eitan  - 2013
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_SetCalypsoMode(TR_st_CardInfo *CardInfo, e_CardWriteMode eWriteMode)
{
	eCalypsoErr err;

	err = e_ClyApp_Virtual_SetCalypsoMode(CardInfo,eWriteMode);
	if(err == e_ClyApp_Ok)
	{
		if(eWriteMode == e_VirtualReadMode)
			g_CardInfo=*CardInfo;
	}

	return err;
}

////////////////////////////////////////////////////////////////////////////
//
// Makes calypso think it hasd a real card
// Eitan  - 2013
//
////////////////////////////////////////////////////////////////////////////

eCalypsoErr TR_SetVirtualCard(unsigned char *CardSerialNumber)
{

	if(bCardWasRead())
		return e_ClyApp_NotOk;

	memcpy(g_CardInfo.m_serialNumber,CardSerialNumber,sizeof(g_CardInfo.m_serialNumber));
	g_CardInfo.m_cardType = enmSCard;

	return e_ClyApp_Ok;
}

////////////////////////////////////////////////////////////////////////////
//
// Read the current virtual data
// Eitan  - 2013
//
////////////////////////////////////////////////////////////////////////////

void TR_GetVirtualCardImage(TR_St_TransactionVirtualData *pVirtualData)
{


	e_ClyApp_Virtual_GetCardImage(pVirtualData);

}

////////////////////////////////////////////////////////////////////////////
//
// Write the current virtual data to the ClyApp
// Eitan  - 2013
//
////////////////////////////////////////////////////////////////////////////

void TR_SetVirtualCardImage(TR_St_TransactionVirtualData *pVirtualData)
{

	e_ClyApp_Virtual_SetCardImage(pVirtualData) ;

}

////////////////////////////////////////////////////////////////////////////
//
// Read a record from the virtual image
// Eitan  - 2013
//
////////////////////////////////////////////////////////////////////////////

eCalypsoErr TR_GetVirtualRecord(unsigned char RecNum,e_clyCard_FileId FileToSelect,unsigned char *pBindata,int BinLen)
{
	return e_ClyApp_Virtual_GetRecord(RecNum,FileToSelect,pBindata,(unsigned char)BinLen);

}

void TR_SetContratAgreementUserFunction(USER_PROC_INTEROPPERBILITY UserProc)
{
	GlobalUser_INTEROPPERBILITY_Proc= UserProc;
}
////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_LockCard(TR_St_LockResponse* pLockResponse)//what is the input ?
{

	union_ClyApp_TransactionBinData ov_TransactionBinData;

	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;

	if(!bCardWasRead())
		return e_ClyApp_Ok;


	if(e_ClyApp_Lock(&ov_TransactionBinData) == e_ClyApp_Ok)
	{
		v_CopyTransactionBinaryData(0,&ov_TransactionBinData, &pLockResponse->TransactionData,1);
		return e_ClyApp_Ok;
	}

	return  e_ClyApp_NotOk;
}

////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_GetWhiteContractList(TR_St_WhiteContractList* pStContracts)
{
	//give count+numbers that exist in contract list
	eCalypsoErr err;
	unsigned short Bitmap;
	unsigned short Codes[8];
	memset(pStContracts, 0, sizeof(*pStContracts));

	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;

#ifdef NOT_RE_SELECT_AT_CARDIN
	err=WaitCard(6,NULL);
	if(err!=e_ClyApp_Ok)
		return err;
#endif

	if(!bCardWasRead())
		return e_ClyApp_NotOk;

	err  = clyApp_GetWhiteContractListOnCard(&Bitmap/*out*/, Codes/*out*/);
	if(err == e_ClyApp_Ok)
	{
		int i;
		for(i=0;i<8;i++)
		{
			if(Bitmap & 1<<i)
			{
				pStContracts->Codes[pStContracts->Count] = Codes[i];
				pStContracts->Count++;
			}
		}

		return e_ClyApp_Ok;
	}


	return  e_ClyApp_NotOk;
}


////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_GetInspectorCardData(TR_st_UserData* pUserData)
{
	stUserData UserData;
	eCalypsoErr res;

	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;

	res =  e_ReadUserData(&UserData);
	if(res==e_ClyApp_Ok)
	{
		pUserData->lv_UserId = UserData.lv_UserId;
		pUserData->iv_UserType = UserData.iv_UserType;
	}

	return res;

}




////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////
eCalypsoErr TR_VerifyPassword(long Pswrd)
{
	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;
	return  e_VerifyPassword(Pswrd);

}

////////////////////////////////////////////////////////////////////////////
//
//  TR_GetFreeIndexForContract
//
////////////////////////////////////////////////////////////////////////////

//eCalypsoErr TR_GetFreeIndexForContract(const TR_St_LoadContract* pLoadData ,long *Indx)
//{
//
//	clyApp_BOOL b_Result;//[OUT]1=free record exist ok,0=no free record
//	unsigned char FreeIndex;///[out] first free index or ff if not found
//
//	*Indx = 0;
//
//
//	if(e_ClyApp_IsFreeRecExist(&b_Result,&FreeIndex,(e_EttType)pLoadData->ucEtt)!=e_ClyApp_Ok || b_Result!=clyApp_TRUE)
//		return e_ClyApp_CardIsFull;
//
//	*Indx = (unsigned char)FreeIndex;
//
//	return e_ClyApp_Ok;
//}

//#ifdef INSPECTOR_TERMINAL

///////////////////////////////////////////////////////////////
//  TR_GetRegularEvents
//  get from card data to check use
//  currently supports only card
//  return e_ClyApp_ReaderCardErr if read error (io)
///////////////////////////////////////////////////////////////
eCalypsoErr TR_GetRegularEvents(TR_St_EventForValidityCheck EventsArrOut[3] //[OUT]
)

{
	union_ClyApp_EventRecord EvntArr[3];
	clyApp_BOOL IsEventOkArr[3]={clyApp_FALSE};
	union_ClyApp_ContractRecord union_ContractRecordArr[MAX_CONTRACT_COUT]={0};
	if (e_ClyApp_SimpleGetAllEvent(EvntArr, IsEventOkArr)==e_ClyApp_Ok)
	{
		int i;
		//read all contracts
		if(e_ClyApp_SimpleReadAllContracts(union_ContractRecordArr)!= e_ClyApp_Ok)
		{
			return e_ClyApp_ReaderCardErr;
		}

		//if card
		if(uc_clyapp_GetCurrentCardType()==e_ClyApp_Ticket)
		{
			return e_ClyApp_NotOk;//ticket currently not supported
		}

		for(i=0;i<3;i++)
		{
			union_ClyApp_ContractRecord* pCurrContract;
			memset(&EventsArrOut[i], 0, sizeof(EventsArrOut[i]));
			EventsArrOut[i].isValidEvent = IsEventOkArr[i];
			if(EventsArrOut[i].isValidEvent)
			{
				EventsArrOut[i].EventCircumstances = EvntArr[i].st_CardEventDataStruct.st_EventCode.e_CardEventCircumstances;
				EventsArrOut[i].EventJourneyInterchange = EvntArr[i].st_CardEventDataStruct.b_EventIsJourneylnterchange;
				EventsArrOut[i].EventContractPointer = EvntArr[i].st_CardEventDataStruct.uc_EventContractPointer;
				//datetimestamp
				ClyDateAndTimetoTrDateTime(&EvntArr[i].st_CardEventDataStruct.st_EventDateTimeStamp,  &EventsArrOut[i].DateTime);
				//datetimefirststamp
				ClyDateAndTimetoTrDateTime(&EvntArr[i].st_CardEventDataStruct.st_EventDataTimeFirstStamp, &EventsArrOut[i].DateTimeFirstUse);

				if(EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.b_IsEventPassengersNumberExist)
				{
					EventsArrOut[i].NumberOfPassengers = EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.uc_EventPassengersNumber;
				}
				else
				{
					EventsArrOut[i].NumberOfPassengers = 1;
				}
				//eventplace
				if(EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.b_IsEventPlaceExist)
					EventsArrOut[i].EventPlace = EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.ush_EventPlace;

				//eventdevice
				if(EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.b_IsEventDevice4Exist)
					EventsArrOut[i].EventDevice = EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.ush_EventDevice4;

				////runid
				//if(EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.b_IsEventRunlDExist)
				//  EventsArrOut[i].EventRunID = EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.ush_EventRunlD;

				//03/2013
				if(EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.b_IsEventInterchangeRightsExist)
					EventsArrOut[i].InterchangeRights = EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.uc_EventInterchangeRights;


				//03/2013
				//get farecode from EventTicket
				if(EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.b_IsEventTicketExist)
					EventsArrOut[i].EventTicketFareCode = EvntArr[i].st_CardEventDataStruct.st_OptionalEventData.st_EventTicket.uc_EventTicketFareCode;


				if(EventsArrOut[i].EventContractPointer<=0)
					continue;

				//get the associated contract
				pCurrContract =  &union_ContractRecordArr[EventsArrOut[i].EventContractPointer-1];
				//set ticket type
				EventsArrOut[i].TicketType = us_CardGetTicketType(pCurrContract);
				//set profile
				if(pCurrContract->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.b_ContractCustomerProfileExist)
				{
					EventsArrOut[i].ContractProfile=pCurrContract->st_CardContractRecord.st_CardContractIssuingData.st_OptionalContractData.uc_ContractCustomerProfile;
				}

				//get spatial type, cluster and code/zone. predefine code
				EventsArrOut[i].ContractSpatial=uc_CardGetContractSpatialType(pCurrContract);
				EventsArrOut[i].ContractCluster=(unsigned char)us_CardGetContractCluster(pCurrContract);
				if(EventsArrOut[i].ContractSpatial==e_CardSpatialTypeZones)
				{
					EventsArrOut[i].ContractZoneBitmap=us_CardGetContractZone(pCurrContract);
				}
        else if(EventsArrOut[i].ContractSpatial==e_CardSpatialTypeFareCode)
        {
          EventsArrOut[i].ContractFareCode = uc_CardGetContractFareCode(pCurrContract);
        }

				EventsArrOut[i].ContractPredefine=us_GetPredefineCode(pCurrContract);
			}
		}

		return  e_ClyApp_Ok;

	}
	else return   e_ClyApp_ReaderCardErr;
}


// #endif //#ifdef INSPECTOR_TERMINAL
#ifndef TIM7020
//get from inspector card the data that was written by validator
eCalypsoErr TR_GetInspectionData(TR_st_InspectionData* pInspectionData)
{
	return e_ReadInspectionData(pInspectionData);
}

//write to event on card the given inspection data
eCalypsoErr TR_SetInspectionData(TR_st_InspectionData* pInspectionData)
{
	return e_WriteInspectionData(pInspectionData);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#ifdef ISSUING_STATION
//#ifndef TIM7020
eCalypsoErr TR_WriteUserData(long password, long id, short type, const TR_CHLNG Chlng)
{
	return e_WriteUserData(password, id, type, Chlng);

}

#ifndef TIM7020
//reader returns random 16 bytes
eCalypsoErr TR_GetChallenge(TR_CHLNG Chlng/*out*/)
{
	return  e_clyapp_GetChallenge(Chlng/*out*/);
}
//#endif

//REQUEST FROM PC TO SAM
// call cardinout (with the sam on board)
eCalypsoErr TR_RemoteSamCardInOut(TR_St_RemoteSamTransmit* in, TR_St_RemoteSamRcv* out)
{
	//if(!g_bIsRemoteSam)
	//  return e_ClyApp_NotOk;//shouldn't happen, wrong logic

	//todo mutex?

	//call cardinout
	return e_clyappCardInOut(&in->ov_7816_Packet/*in*/,
		&out->ov_7816_Response/*out*/,
		in->us_timeout
		);
}
#endif

///////////////////////////////////////////////////////////
// NAME: TR_GetAllEventsForReport
// DESCRIPTION: gets all events on card. 
// PARAMETERS:  [IN/OUT] EventsArr - Array buffer for writing the output events
//				[IN/OUT] EventNum  - Number of the event that ware wirtten to the EventsArr array
// PRE REQUIREMENT:  card in
// RETURNS: eCalypsoErr result
///////////////////////////////////////////////////////////
//eCalypsoErr TR_GetAllEventsForReport(TR_St_EventForReport EventsArr[MAX_EVENT_FOR_REPORT])
eCalypsoErr TR_GetAllEventsForReport(St_GetActionReportRespond *Respond)
	
{
	eCalypsoErr  eRet;
	union_ClyApp_EventRecord union_EventRecordArr[MAX_EVENT_COUT];
	st_ClyApp_CardContractRecord  contract;
	clyApp_BOOL bIsEventOkArr[MAX_EVENT_COUT];
	int i_ContractPointer=0;

	
	unsigned int i;
	int IsEventPoint2Contract=1;
	TR_st_CardInfo  pInfo;
	TR_BOOL b;
	memset(Respond,0,sizeof(*Respond));


	// modify m_FlagIsEnvaromentOk & m_FlagIsLock
	//b=b_clyapp_IsCardIn(&pInfo/*info*/);
	eRet=TR_IsCardIn(&b,&pInfo);

	 if(b!=TR_TRUE)
		 return e_ClyApp_NoCardErr;

	 Respond->m_FlagIsEnvaromentOk=pInfo.IsEnvOk;
	 Respond->m_FlagIsLock=pInfo.IsCardLock;



	//Get all Regular events
	if((eRet = e_ClyApp_GetAllEvent(union_EventRecordArr,//[IN]Array memory allocation of events to fill
                                    bIsEventOkArr))  == e_ClyApp_Ok)  //[OUT] indicate for each event if ok //Yoni 14/6/10) != e_ClyApp_Ok)
	{
		// convert the data 
		for(i=0; i < MAX_EVENT_FOR_REPORT; i++)
		{
				
			Respond->m_Actions[i].isValidEvent = (unsigned char)bIsEventOkArr[i]; 
			if(!Respond->m_Actions[i].isValidEvent)
				continue;
			else
				Respond->m_ActionCount++;
			//  redundant if(bIsEventOkArr[i]) 
			
			// Event type
			switch(union_EventRecordArr[i].st_CardEventDataStruct.st_EventCode.e_CardEventCircumstances)
			{
			case e_CardEventCircumCancellation:
				Respond->m_Actions[i].EventType = e_ClyApp_EventTypeCancel;
				
				break;
			case e_CardEventCircumEntry:
			case e_CardEventCircumInterchangeEntry:
				Respond->m_Actions[i].EventType = e_ClyApp_EventTypeUse;

				break;
			case e_CardEventCircumContractLodingWithImmediateFirstUse:
			case e_CardEventCircumContractLoading:
				Respond->m_Actions[i].EventType = e_ClyApp_EventTypeLoading;
				break;
			default:
				Respond->m_Actions[i].EventType = e_ClyApp_EventTypeUnknown;
				IsEventPoint2Contract=0;
				break;
			}
			// Service provider
			Respond->m_Actions[i].EventServiceProvider = union_EventRecordArr[i].st_CardEventDataStruct.uc_EventServiceProvider;
			// Num of passangers  
			if(union_EventRecordArr[i].st_CardEventDataStruct.st_OptionalEventData.b_IsEventPassengersNumberExist)
				Respond->m_Actions[i].EventNumOfPassanges = union_EventRecordArr[i].st_CardEventDataStruct.st_OptionalEventData.uc_EventPassengersNumber;
			else 
				Respond->m_Actions[i].EventNumOfPassanges = 1; //default
			// Date and Time 
			ClyDateAndTimetoTrDateTime(&union_EventRecordArr[i].st_CardEventDataStruct.st_EventDateTimeStamp,  &Respond->m_Actions[i].DateTime);

			// get shilut 
			Respond->m_Actions[i].Shilut=usGetShilutFromEventAccordingToVersion(&union_EventRecordArr[i].st_CardEventDataStruct);

			   //get EventTicketFareCode
              if(union_EventRecordArr[i].st_CardEventDataStruct.st_OptionalEventData.b_IsEventTicketExist)
                 Respond->m_Actions[i].EventTicketFareCode =(TR_BYTE) union_EventRecordArr[i].st_CardEventDataStruct.st_OptionalEventData.st_EventTicket.uc_EventTicketFareCode;

			  if(union_EventRecordArr[i].st_CardEventDataStruct.b_EventIsJourneylnterchange)
				  Respond->m_Actions[i].IsInterchange=1;
			  else
				  Respond->m_Actions[i].IsInterchange=0;


			  

			// read contract from stater machine
			if(IsEventPoint2Contract)
			{
				clyApp_BOOL b2;
				//st_ClyApp_CardContractRecord contract;
				i_ContractPointer=union_EventRecordArr[i].st_CardEventDataStruct.uc_EventContractPointer;
				// read contract from stater machine
				b2=b_GetContractFromStateMachine(i_ContractPointer,&contract);

				// get data from contract
				if(b2==clyApp_TRUE)
				{
					int ett;
					// get ett
					ett=Respond->m_Actions[i].ETT=i_GetEttTypeFromCardContractRecord(&contract);
					// get predefine 
					Respond->m_Actions[i].PredefinedCode=us_GetPredefineCodeFromCardContractRecord(&contract);
					// get restrict code
					if(contract.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictCodeExist)
						Respond->m_Actions[i].RestrictCode = contract.st_CardContractIssuingData.st_OptionalContractData.uc_ContractRestrictCode;

					// start type 
			        if(contract.st_CardContractIssuingData.st_ContractTariff.e_TariffCounterType ==e_ClyApp_CounterNotUsed)
                         Respond->m_Actions[i].StartType=e_Calendar;//todo in the future distinguish between calendar and sliding but note that validity duration is not the field for it!!
					else
						 Respond->m_Actions[i].StartType=e_PeriodUndefined;

					// get continue flag
					Respond->m_Actions[i].ContinueFlag= e_GetInterchangeType(&contract.st_CardContractIssuingData);
				   
					// if interchange allowed flag 
					if(contract.st_CardContractIssuingData.b_ContractIsJourneylnterchangesAllowed)
					{
						
                      // if it continue contract
					  if(contract.st_CardContractIssuingData.st_OptionalContractData.b_ContractPeriodJourneysExist)
					  {
						switch((contract.st_CardContractIssuingData.st_OptionalContractData.st_ContractPeriodJourneys.uc_MaxNumOfTripsInPeriod)-1)
						{
						case 1:
							Respond->m_Actions[i].InterchangeType=e_OneHemshech;
							Respond->m_Actions[i].ContinueFlag=1;
							break;

						case 2:
							Respond->m_Actions[i].InterchangeType=e_TwoHemshech;
							Respond->m_Actions[i].ContinueFlag=1;
							break;
						default:
							Respond->m_Actions[i].InterchangeType=e_NoInterchange;
						}
					 }// it is not continue 
					 else
					 {
						 // if it "Mavar"
						 if(contract.st_CardContractIssuingData.st_OptionalContractData.b_ContractRestrictDurationExist)   
						    Respond->m_Actions[i].InterchangeType=e_Maavar;
						 else
							 Respond->m_Actions[i].InterchangeType=e_NoInterchange;

					 }

					
				}// end (contract.st_CardContractIssuingData.b_ContractIsJourneylnterchangesAllowed)
				else // interchange flag not exist
                   Respond->m_Actions[i].InterchangeType=e_NoInterchange;
				}
			}
			//contr = &stGlobalDataObject.CntrctsArr[eventdata->st_CardEventDataStruct.uc_EventContractPointer-1];

		}
	}
	return eRet; 
}


///////////////////////////////////////////////////////////
// NAME: TR_CheckCardReader
// DESCRIPTION: Check the card reader by writing data and read it back 
// PARAMETERS: 
// PRE REQUIREMENT:  Call to TR_InitReader,
// RETURNS: eCalypsoErr result
/////////////////////////////////////////////////////////
eCalypsoErr TR_CheckCardReader(void)
{
   TR_st_CardInfo	stInfo;
   eCalypsoErr		eRet;


   	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;

	eRet = e_ClyApp_NotOk;
	// Detect
	if(b_clyapp_IsCardIn(&stInfo))
	{
		//Test
		if(b_ClyApp_TestReadWrite() == clyApp_TRUE)
			eRet = e_ClyApp_Ok;
		else
        	eRet = e_ClyApp_ReaderErr;// error
		
			 

		// Eject
		TR_ForgetCard();
	}
	
	return eRet;
}

///////////////////////////////////////////////////////////
// NAME: TR_ReadAllCardBinData
// DESCRIPTION: Read all the card binary data 
// PARAMETERS: [IN] pBuf - pointer to user buffer
// PRE REQUIREMENT:  Call to TR_InitReader,
// RETURNS: eCalypsoErr result
/////////////////////////////////////////////////////////
eCalypsoErr TR_ReadAllCardBinData(TR_CardBinData *pBuf)
{
	unsigned int i;
	eCalypsoErr err;
	st_ClyApp_EnvAndHoldDataStruct st_EnvAndHoldDataStruct;
	st_ClyApp_CardContractRecord st_CardContractRecord;
	

	if(IS_CALYPSO_LOCKED)
		return e_ClyApp_InterfaceNotInitErr;

	// Environment 
	err = e_ClyApp_ReadRecordData(pBuf->Env,//[IN]type - card \ ticket
									(clyCard_BYTE)1,//[IN] //not relevat for ticket - record number to read - 1 is always the first record
									e_clyCard_EnvironmentFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
									&st_EnvAndHoldDataStruct); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
      if( err!=e_ClyApp_Ok)
        return err;

	// Contract 
	for(i=0; i < BIN_CONTRACTS_COUNT; i++)
	{
		err = e_ClyApp_ReadRecordData(pBuf->Contracts[i],//[IN]type - card \ ticket
										(clyCard_BYTE)(i+1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
										e_clyCard_ContractsFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
										&st_CardContractRecord); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
		if( err!=e_ClyApp_Ok)
			return err;
	}

	// Counter of the contract
	err = e_ClyApp_ReadRecordData(pBuf->Counters,//[IN]type - card \ ticket
										(clyCard_BYTE)(1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
										e_clyCard_CountersFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
										&st_CardContractRecord); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
		if( err!=e_ClyApp_Ok)
			return err;
	

	// Events 
	for(i=0; i <BIN_EVENTS_COUNT  ; i++)
	{
		err = e_ClyApp_ReadRecordData(pBuf->Events[i],//[IN]type - card \ ticket
										(clyCard_BYTE)(i+1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
										e_clyCard_EventLogFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
										&st_CardContractRecord); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
		if( err!=e_ClyApp_Ok)
			return err;
	}
	// Special Events 
	for(i=0; i < BIN_SPECIAL_EVENTS_COUNT; i++)
	{
		err = e_ClyApp_ReadRecordData(pBuf->SpecialEvents[i],//[IN]type - card \ ticket
										(clyCard_BYTE)(i+1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
										e_clyCard_SpecialEventFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
										&st_CardContractRecord); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts
		if( err!=e_ClyApp_Ok)
			return err;
	}
	// Contract List
	err = e_ClyApp_ReadRecordData(pBuf->ContractList,//[IN]type - card \ ticket
									(clyCard_BYTE)(1),//[IN] //not relevat for ticket - record number to read - 1 is always the first record
									e_clyCard_ContractListFile, //[IN]not relevat for ticket - e_clyCard_NoFile2Select = not requested - current file remain unchanged
									&st_CardContractRecord); //[OUT] data read - this data need casting in the application layer accurding to the application data strucuts

	return err;

}

///////////////////////////////////////////////////////////////
// Name: GetTimeAndDateCallBack
// DESCRIPTION: Register Time and Data CB function, should be called at initialization
// PARAMETERS:  [IN] CallBack - pointer GetTimeAndDateCallBack CB function
///////////////////////////////////////////////////////////////
void TR_RegisterTimeAndDataCallback(GetTimeAndDateCallBack CallBack)
{
	if(CallBack)
	{
		TimeAndDate_fp = CallBack; // override current CB
		stInitFlags.bIsTimeSet = 1;
	}
}
#ifndef ENABLE_COMM
//Time and date callback
TR_BOOL TIM7020TimeAndDateCallBack(TR_St_DateTime* trDt)
{
	st_Time newtime;

#ifndef VALIDATOR_DEF
	if(i_TimeHGetNow(&newtime))
#else	
	if(i_TimeGMtOffsetGetNow(&newtime))
#endif			
	{

		//convert to struct st_Time
		trDt->Year = newtime.ui_Year;          /* Year : for example 20000  */
		trDt->Month  = newtime.uc_Month;       /* Month. [1-12] */
		trDt->Day    = newtime.uc_Day;     /* Day.   [1-31] */
		trDt->Hour   = newtime.uc_Hour;      /* Hours. [0-23] */
		trDt->Minute = newtime.uc_Minute;    /* Minutes. [0-59] */
		trDt->Second = newtime.uc_Second;    /* Seconds. [0-59] */
		return (clyApp_BOOL)1;
	}
	return (clyApp_BOOL)0;
}
#endif



#endif // defined(CORE_SUPPORT_SMARTCARD) && defined(CORE_SUPPORT_CALYPSO)
eCalypsoErr TR_VerifyTransaction(TR_TrnsactionQuery *p_In,TR_St_LoadContractResponse *p_Respond)
{
	return e_ClyApp_WrongParamErr;
}

///////////////////////////////////////////////////////////
// NAME: TestReaderRequest
// DESCRIPTION: test reader antena & sam 
// PARAMETERS: [IN] test parameter 
// PRE REQUIREMENT:  Call to Call to TR_InitReader
// RETURNS: eCalyps
/////////////////////////////////////////////////////////
eCalypsoErr  TestReaderRequest(St_TestReaderRequest * p_TestReaderRequest, St_TestReaderResults *p_Result)
	{
int ApplicationError;
	
#define req (*p_TestReaderRequest)
#define resp (*p_Result)
  
	int RecvSize;
	 memset(p_Result,0,sizeof(*p_Result));	
	ApplicationError = 0; //TBD:yoram debug

	// call to k10 
if(InitRes.ProtocolCB && InitRes.ProtocolCB(InitRes.pHandler,//  CHANNEL_HANDLER p_handler,//[IN]
							 e_CmdK10_Test_Reader, 	//int i_cmd,//[IN] the command
							 p_TestReaderRequest->TimeOutWaitCardMS+PROTO_DEFAULT_TIMEOUT,	//int i_TimeOutMs,//[IN] // the time out
							 sizeof(req),	//int i_ObjectInSize,//[IN]// the data size to send
							 &req, //void *p_ObjectIn,//[IN] the data 
							 sizeof(resp),	//int i_ObjectOutSizeReq,//[IN] // the data respond size except to 
							 &resp,	//void *p_ObjectOut,//[OUT]// the data return 
							 &RecvSize,	//int *p_OutSizeArive,//[IN]// the size of data respond
							 1, //unsigned short  ApplicationStatusBits,//[IN]  in case of i_IsRequest=1 the function  e_SendMessage ignore this value
							 &ApplicationError ) == e_ComOk ) //int *p_ApplicationError [OUT]// the application respond			  
{
	
	    // test k10 result
		if(ApplicationError=e_ClyApp_Ok && RecvSize== sizeof(resp))
		{
			
			return e_ClyApp_Ok;
		}
}


		return e_ClyApp_WrongParamErr;
}
	
