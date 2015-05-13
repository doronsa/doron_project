#ifndef TW_K10232_P_MSSG
 #define TW_K10232_P_MSSG
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    TW_K10232_P.h   
//    protocol k10 mx6 defenition
//    protocol version is 1 and run on TW_MTR_AS_V1.23 protocol 
//    the data field an commands is fit to TW_K10232_P version 1.5 see document rtos_slave_cpu.docx
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif
#ifdef win_or_linux
#define TR_PACK pck
#define TR_PACK_PREFIX
#pragma pack(push, W_K10232_P_MSSG_SET, 1) // correct way to align
#endif

#ifdef INCLUDE_KEIL
// #define TR_PACK __attribute__ ((aligned (1),packed))
#define TR_PACK_PREFIX __packed
#endif

#define MAX_BUFFER_SIZE_FOR_DOWNLOAD 960//128//4096

///////////// status bit //////////////////////////////////////////////////////////////
#define STATUS_BIT //see TW_MTR_AS_V1.2 header.state

#define STATE_IS_IN_LOADER            1
#define STATE_IS_GOOD_APP_EXIST       2
#define STATE_IS_COUSTOMER_CARD_EXIST 4
#define STATE_LOADER_IN_LOAD_PROGRESS 8
#define STATE_IS_RFID_READER_OK       0x10  // RFID SAM POWER
#define STATE_IS_PARAM_SET            0x20 // if reset occurred than param is not set 
#define STATE_IS_IN_LOW_POWER_MODE    0x40
#define STATE_IS_SAM1_OK              0x80
#define STATE_IS_DRIVER_MODULE_LOCK   0x100





////////////////////////////////////////////////////////////////////////////////////
//
//     commands  definition:  see TW_MTR_AS_V1.2 header.Command
//       
////////////////////////////////////////////////////////////////////////////////////

typedef enum
{   //                                       In                                   Out
    // ----------------------------------------------------------------------------------------
	
    // query k10   MX6 to K10  -----------------------------------------------------------------------------

	// check comunication (loader and application )
    e_CmdK10_CheckComm=0,     //               St_K10_TestComm                     St_K10_TestComm
	// self test
	e_CmdK10_SelfTest=1,     //                void                                St_K10_BITResult

	e_CmdK10_SetParam=2,     //                St_K10Param                         void
	// lcd commands for 2 displays
	e_CmdK10_DisplayCommandGet=3,//            void                                St_K10_DisplayGet                   
	e_CmdK10_DisplayCommandSet=4,//            St_K10_DisplaySet                   void                   
    
	// set k10 keyboard commands support if SupprtKeyboardFunction true
	e_CmdK10_KeyboardCommandConfig=5,//         St_K10_KeyboardCommandConfig           void         
	// 4 commands support if SupprtKeyboardFunction true
	e_CmdK10_KeyboardCommand=6,//              St_K10_KeyboardCommand              St_K10_KeyboardCommandResponse         
	// get keyboard commands get one key from Q (FIFO) support if SupprtKeyboardFunction true
	e_CmdK10_KeyboardGetKey=7,//               void                                St_CmdK10_KeyboardGetKey

	// power get state 
	e_CmdK10_PowerCommandGet=8,//              void                                St_K10_PowerCommandGet
	// connect/disconnect chanel
	e_CmdK10_PowerCommandSet=9,//              St_K10_PowerCommandSet              void

	// sensor get state 
	e_CmdK10_SensorCommandGet=10,//             void                                St_K10_SensorCommandGet
	e_CmdK10_SolenoidCommandSet=11,//           St_K10_SolenoidCommandSet           void         

	e_CmdK10_PrinterCommandSet=12,//            St_PrinterCommandSet                void
	e_CmdK10_LedCommandSet=13,//                St_K10_LedCommandSet                void
	e_CmdK10_PeriodicMonitorPoll=14,//          void                                St_K10_PeriodicMonitorPoll
	
	// loader functions (loader & application)
	e_CmdK10_GetAppVersion=15,//                void                                 St_K10_GetAppVersion
    // loader only 
	e_CmdK10_DownLoad=16,//                     St_K10_DownLoad                      void                 


	// application  only   if mx6 want to load new application it must before to send "e_CmdK10_Jump2Loader" command
 	e_CmdK10_Jump2Loader=17,//                     void                                 void 



	// loader only   After the MX6 loads the software its  sending application e_CmdK10_Jump2App message

	e_CmdK10_Jump2App=18,//                        void                                 void 
	e_CmdK10_Set2LowPower=19,// in case that  AutoLowPowerManage =0
	/*e_CmdK10_EndLowPower=20,// its Not applicable , The only way to change the status 
	of k10 from low power is to raise the RTS to 300 ms and a second later it will be available */
	
	e_CmdK10_ResetApp=21,// jump to start application (not to loader )
	e_CmdK10_GetRTCTime=22,//                     void                                  St_K10_RTCTime 
	e_CmdK10_SetRTCTime=23,//                     St_K10_RTCTime                        void 

	


	// events k10  K10 send event to MX6  -----------------------------------------------------------------------------
    //                                      in	                               --

	e_CmdK10_EventReset=30,//                  void                                   
	e_CmdK10_KeyBoardEvent=31,//               St_CmdK10_KeyboardGetKey
	e_CmdK10_PeriodicMonitorEvent=32,//        St_K10_PeriodicMonitorEvent
	e_CmdK10_EventSensorChange=33,//           St_K10_EventSensorChange
	



	// k10  calypso commands  to be define  start from 60 -----------------------------------------------------------------------------
    //                                                  In                                       Out
	e_CmdK10_ClySam_InitInterface=60,  	    // [IN] St_ClySam_InitInterface,          [OUT] void      
	e_CmdK10_ClySam_DetectCard,     	    // [IN] St_ClySam_DetectCard,             [OUT] void
	e_CmdK10_ClySam_Reset,                  // [IN] St_ClySam_ResetReq,               [OUT] St_ClySam_ResetResp
	e_CmdK10_ClySam_GetType,                // [IN] St_ClySam_GetTypeReq,             [OUT] St_ClySam_GetTypeResp
	e_CmdK10_ClySam_GetSerNum,              // [IN] St_ClySam_SerNumReq,              [OUT] St_ClySam_SerNumResp
	e_CmdK10_ClySam_Unlock,                 // [IN] St_ClySam_UnlockReq,              [OUT] St_ClySam_UnlockResp       
	e_CmdK10_ClySam_ReadData,               // [IN] St_ClySam_ReadDataReq,            [OUT] St_ClySam_ReadDataResp       
              
	e_CmdK10_ClyCard_Init,                  // [IN] St_ClyCard_initReq,               [OUT] St_ClyCard_SerNumReq 
	e_CmdK10_ClyCard_GetSerNum,             // [IN] St_ClyCard_SerNumReq,             [OUT] St_ClyCard_SerNumResp 
	e_CmdK10_ClyCard_DetectCard,            // [IN] St_ClyCard_DetectCard,            [OUT] void 
	e_CmdK10_ClyCard_EjectCard,             // [IN] St_ClyCard_EjectCard,             [OUT] void 
	e_CmdK10_ClyCard_Reset,                 // [IN] St_ClyCard_ResetReq,              [OUT] St_ClyCard_ResetResp 
	e_CmdK10_ClyCard_StartWorkWithCard,	    // [IN] St_ClyCard_StartWorkWithCardReq,  [OUT] St_ClyCard_StartWorkWithCardResp 
	e_CmdK10_ClyCard_ReadRecord,		    // [IN] St_ClyCard_ReadRecordReq,         [OUT] St_ClyCard_ReadRecordResp 
	e_CmdK10_ClyCard_WriteRecord,		    // [IN] St_ClyCard_WriteRecordReq,        [OUT] St_ClyCard_WriteRecordResp 
	e_CmdK10_ClyCard_UpdateRecord,		    // [IN] St_ClyCard_UpdateRecordReq,       [OUT] St_ClyCard_UpdateRecordResp 
	e_CmdK10_ClyCard_IncreaseDecrease,	    // [IN] St_ClyCard_IncDecCntRecordReq,    [OUT] St_ClyCard_IncDecCntRecordResp 
	e_CmdK10_ClyCard_Invalidate,		    // [IN] St_ClyCard_InvalidateReq,         [OUT] St_ClyCard_InvalidateResp
      
	e_CmdK10_ClyCard_TestReadWrite,		    // [IN] St_ClyCard_TestReadWriteReq,      [OUT] St_ClyCard_TestReadWriteResp 
      
	e_CmdK10_7816_ContactlessDetect,        // [IN] void,                             [OUT] void
	e_CmdK10_7816_GetCardResetInfo,         // [IN] St_7816_GetCardResetInfoReq,      [OUT] St_7816_GetCardResetInfoResp
	e_CmdK10_7816_ContactlessForgetCard,    // [IN] void,                             [OUT] void
	e_CmdK10_7816_CheckCardComm,            // [IN] St_7816_CheckCardCommReq,         [OUT] void

	e_CmdK10_ClySession_OpenSecureSession,  // [IN] St_ClyCard_OpenSecureSessionReq , [OUT] St_ClyCard_OpenSecureSessionResp
	e_CmdK10_ClySession_CloseSecureSession, // [IN] St_ClyCard_CloseSecureSessionReq, [OUT] St_ClyCard_CloseSecureSessionResp
	
	e_CmdK10_ClyApp_PingTest,	            // [IN] void,                             [OUT] void
    e_CmdK10_ClyApp_TxDataTest,             // [IN] St_ClyApp_TxDataReq,              [OUT] St_ClyApp_TxDataResp
    e_CmdK10_ClyApp_IsCardIn,	            // [IN] void,                             [OUT] St_ClyApp_SmartCardData

    e_CmdK10_ClyLast,                       // Not command

    // End of calypso commands                  
    // ----------------------------------------------------------------------------------------
    e_CmdK10Last,                  //  Mast be last
}e_CmdK10MssgType;

////////////////////////////////////////////////////////////////////////////////////
//
//     Application Errors description (see TW_MTR_AS_V1.2 header.ResultApp) 
//       
////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
    e_K10_Ok,
	e_K10_WrongCommand, 
	e_K10_WrongDataSizeForCmd,
	e_K10_WrongDataInfoForCmd,
	e_K10_RequestImposible ,
	e_K10_OperationFail,
	e_K10_LDR_Prev_Package_Miss ,
	e_K10_LDR_Wrong_Crc_MX6 ,

}e_K10AppResult;

////////////////////////////////////////////////////////////////////////////////////
//
//     K10 type
//       
////////////////////////////////////////////////////////////////////////////////////

typedef unsigned char  K_BYTE;
typedef unsigned short K_WORD;
typedef unsigned long  K_DWORD;

#define K_TRUE 1
#define K_FALSE 0

typedef K_BYTE K_BOOL ; // K_TRUE/FALSE

////////////////////////////////////////////////////////////////////////////////////
//
//     structures 
//       
////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------
// for e_CmdK10_CheckComm request and response 
#define ECHO_SIZE_TEST 200

TR_PACK_PREFIX struct TAG_St_K10_TestComm
{
    K_BYTE EchoTest[ECHO_SIZE_TEST];// The application should return the same information
};
typedef TR_PACK_PREFIX struct TAG_St_K10_TestComm St_K10_TestComm;



//-------------------------------------------------------------------
// for e_CmdK10_SelfTest  response 


TR_PACK_PREFIX struct TAG_St_K10_BITResult
{
  K_BOOL RfIdOk;
  K_BOOL SamOk;
  K_BOOL ExternalCurrentOk;
  K_BOOL ExternalVoltageOk;
  K_BOOL Volt5ageOk;
  K_BOOL Volt9ageOk;
  K_BOOL Volt24ageOk;
  K_BOOL FanOk;
  K_BOOL RTCOk;
 };
typedef TR_PACK_PREFIX struct TAG_St_K10_BITResult St_K10_BITResult;



//-------------------------------------------------------------------
//// e_CmdK10_DisplayCommandSet
#define DSP_ROW_SIZE 16
#define DSP_COLUMN_SIZE 2

typedef enum 
{
 e_K10_Dsp_ID_Driver,
 e_K10_Dsp_ID_Passanger,
 e_K10_Dsp_ID_Both
}e_K10_DisplayId;


typedef enum 
{
 e_K10_Dsp_ReInit,// init dsiplay again
 e_K10_Dsp_Cls,
 e_K10_Dsp_Set_CursorMode,
 e_K10_Dsp_Set_WriteStringXY,
 e_K10_Dsp_Set_BackGround,
 e_K10_Dsp_Clear_BackGround,
 e_K10_Dsp_SelfTest,
 e_K10_Dsp_SetArrowLed,
 e_K10_Dsp_ClrArrowLed,
 e_K10_Dsp_SetAmbient,
 e_K10_Dsp_ClrAmbient,
  }e_K10_DisplayCommand;

typedef enum 
{
 e_CursorHide,
 e_CursorBlink,
}e_DspCursorMode;

TR_PACK_PREFIX struct TAG_St_K10_DisplaySet
{
 K_BYTE K10_DisplayId;// driver lcd /passenger lcd/both
 K_BYTE DisplayCommand;//e_K10_DisplayCommand
 K_BYTE CursorMode;//e_DspCursorMode relevant for e_K10_Dsp_Set_CursorMode only
 char StringXY[DSP_ROW_SIZE];   // relevant for e_K10_Dsp_Set_WriteStringXY only
 K_BYTE x;// from 0 to DSP_ROW_SIZE-1 relevant for e_K10_Dsp_Set_WriteStringXY  and  e_K10_Dsp_Set_CursorMode  only
 K_BYTE y;// from 0 to DSP_COLUMN_SIZE relevant for e_K10_Dsp_Set_WriteStringXY and  e_K10_Dsp_Set_CursorMode  only
 };
typedef TR_PACK_PREFIX struct TAG_St_K10_DisplaySet St_K10_DisplaySet;


//-------------------------------------------------------------------
//// e_CmdK10_DisplayCommandGet


typedef TR_PACK_PREFIX struct 
{
 K_BYTE CursorMode;
 char Image[DSP_COLUMN_SIZE][DSP_ROW_SIZE];   
 K_BYTE Where_CUrsor_x;
 K_BYTE Where_CUrsor_y;
 }St_DisplayState;

TR_PACK_PREFIX struct TAG_St_K10_DisplayGet
{
  St_DisplayState dsp_Driver;
  St_DisplayState dsp_Psngr;
};
typedef TR_PACK_PREFIX struct TAG_St_K10_DisplayGet St_K10_DisplayGet;

//-------------------------------------------------------------------
// e_CmdK10_KeyboardCommandConfig
typedef enum 
{
 e_KeyBoardPolling,
 e_KeyBoardEvent,
}e_KeyBoardGetMethod;

typedef enum 
{
 e_KeyOneKeyPerPrerss,
 e_KeyBerst,
}e_KeyBoardDebounc; 

TR_PACK_PREFIX struct TAG_St_K10_KeyboardCommandConfig
{
 K_BYTE KeyBoardMethodGet;//e_KeyBoardGetMethod
 K_BYTE KeyBoardDebounc;//e_KeyBoardDebounc
 K_BYTE KeyBufferSize;
 K_WORD BytesPerSecondInBertsMode;
 K_WORD DelayBitweenPress;
 };



typedef TR_PACK_PREFIX struct TAG_St_K10_KeyboardCommandConfig St_K10_KeyboardCommandConfig;

//-------------------------------------------------------------------
//e_CmdK10_KeyboardCommand,//                 St_K10_KeyboardCommand 

typedef enum 
{
 e_KeyboardClearBuffer,
 e_KeyboardDisable,
 e_KeyboardEnable,
 e_KeyboardGetQsize,
}e_KeyBoardCmd;

// request 
TR_PACK_PREFIX struct TAG_St_K10_KeyboardCommand
{
 K_BYTE KeyCmd;  //e_KeyBoardCmd
};
typedef TR_PACK_PREFIX struct TAG_St_K10_KeyboardCommand St_K10_KeyboardCommand;


// response  

TR_PACK_PREFIX struct TAG_St_K10_KeyboardCommandResponse
{
 K_BYTE Qsize;// the keys count in buffer  after read the key 
};

typedef TR_PACK_PREFIX struct TAG_St_K10_KeyboardCommandResponse St_K10_KeyboardCommandResponse;


//-------------------------------------------------------------------
//e_CmdK10_KeyboardGetKey     St_CmdK10_KeyboardGetKey
typedef enum 
{
 e_Key1,
 e_Key2,
 e_Key3,
 e_Key4,
 e_NoKey  // in case that 
}e_KeyId;

TR_PACK_PREFIX struct TAG_St_CmdK10_KeyboardGetKey
{
 K_BYTE LastKey;//e_KeyId
 K_BYTE Qsize;// the keys count in buffer  after read the key 
};
typedef TR_PACK_PREFIX struct TAG_St_CmdK10_KeyboardGetKey St_CmdK10_KeyboardGetKey;

//-------------------------------------------------------------------

typedef enum 
{
 e_ChargingOn,
 e_ChargingOff,
 e_ChargingFail,
}e_ChargerState;

//e_CmdK10_PowerCommandGet	  St_K10_PowerCommandSet              
TR_PACK_PREFIX struct TAG_St_K10_PowerCommandGet
{
 K_DWORD Current24;
 K_BOOL Good24;
 K_BYTE ChargeState;//e_ChargerState
 K_BOOL SomPowerGood;// 0 fail 1 ok
 K_DWORD BataryVotage;// in case of SupprtBataryAndChargerMonitoring is true
 K_DWORD BataryCurrent;// in case of SupprtBataryAndChargerMonitoring is true
 K_DWORD Voltage5;
 K_DWORD ExendedlCurrent;// batary curreny when it use
 K_DWORD ExendedlVoltage;// batary voltage when it use
 K_DWORD ExternalVoltage;
 K_DWORD Voltage9;
 K_BOOL RfidOn;// 1 RFID on 0 RFID off
 K_DWORD RfIDAntenaSensor;
 K_BOOL DisplayBackLightFault;
 };
typedef TR_PACK_PREFIX struct TAG_St_K10_PowerCommandGet St_K10_PowerCommandGet;


//-------------------------------------------------------------------
//e_CmdK10_PowerCommandSet	  St_K10_PowerCommandSet              

typedef enum 
{
 e_RfidPower,
 e_24Power,
 e_BataryChannel,// in case of SupprtBataryAndChargerCommand is true 
}e_PowerChannel;

typedef enum 
{
 e_ChannelOn,
 e_ChannelOff,
}e_PowerCommand;



TR_PACK_PREFIX struct TAG_St_K10_PowerCommandSet
{
	K_BYTE Channel;//e_PowerChannel
	K_BYTE Command;//e_PowerCommand
};
typedef TR_PACK_PREFIX struct TAG_St_K10_PowerCommandSet St_K10_PowerCommandSet;


//-------------------------------------------------------------------
//e_CmdK10_SensorCommandGet, St_K10_SensorCommandGet

TR_PACK_PREFIX struct TAG_St_K10_SensorCommandGet
{
 K_BOOL SensorACC;// switch 0 close 1 open
 K_BOOL SensorDriverModuleIn;// 0 out 1 in ,  in case of SupprtDriveModuleFunction is true
 K_BOOL SensorFanFault;// 1 fault 0 ok , in case of SupprtFanFunctions is true
 K_BOOL SensorPrinterDoor;// 1 open 0 close
 K_DWORD LightSensor;// anlog data	
};
typedef TR_PACK_PREFIX struct TAG_St_K10_SensorCommandGet St_K10_SensorCommandGet;

//-------------------------------------------------------------------
//e_CmdK10_SolenoidCommandSet,  St_K10_SolenoidCommandSet        

typedef enum 
{
 e_SolenoidOpen,
 e_SolenoidClose,
}e_SolenoidOperation;

typedef enum 
{
 
 e_DriverSelonoid,// in case of SupprtDriveModuleFunction is true
}e_SolenoidId;


TR_PACK_PREFIX struct TAG_St_K10_SolenoidCommandSet
{
	K_BYTE SolenoidId;//e_SolenoidId
	K_BYTE SolenoidOperation;//e_SolenoidId
};
typedef TR_PACK_PREFIX struct TAG_St_K10_SolenoidCommandSet St_K10_SolenoidCommandSet;


//-------------------------------------------------------------------
//e_CmdK10_PrinterCommandSet, St_PrinterCommandSet                

typedef enum 
{
 e_SetPrinterInterface,// usb/232 interface
 e_FanCmd,// fan on / off  in case of SupprtFanFunctions is true 
 }e_K10PrinterCommand;

TR_PACK_PREFIX struct TAG_St_PrinterCommandSet
{
 	K_BYTE PrinterCommand;//e_K10PrinterCommand fan/set printer interface 
	K_BYTE CommandValue;// 0 Fan off / set as usb  ,1 Fan on / set as 232 (232 for k10 USB for MX6) 
};
typedef TR_PACK_PREFIX struct TAG_St_PrinterCommandSet St_PrinterCommandSet;


//-------------------------------------------------------------------
//e_CmdK10_LedCommandSet    St_K10_LedCommandSet                

typedef enum 
{
 e_LedOn,
 e_LedOff
}e_K10LedCmd;

typedef enum 
{
 e_DriverLed,
 e_CoustomerLed,
 e_BothLed,// driver and coustomer
}e_LedBlockId;


TR_PACK_PREFIX struct TAG_St_K10_LedCommandSet
{
 	K_BYTE LedBlockId;//e_LedBlockId;
	K_BYTE Red;//e_K10LedCmd
	K_BYTE Green;//e_K10LedCmd
	K_BYTE Yellow;//e_K10LedCmd
};

typedef TR_PACK_PREFIX struct TAG_St_K10_LedCommandSet St_K10_LedCommandSet;

//-------------------------------------------------------------------
//e_CmdK10_SetParam,               St_K10Param                         

TR_PACK_PREFIX struct TAG_St_K10Param
{
	K_BOOL  MonitorPolling;// 1 polling 0 event
	K_DWORD MonitorPeriodInSec;// Time setting for  monitoring routine 
	K_DWORD WatchDogTimeInSec;// watchdog time
	K_BOOL  AutoLowPowerManage;// 0 disable 1 enable

	/* IdleTimeForLowPowerInSec when AutoLowPowerManage is 1 
	If no action was not made between the MX6 to 10K more than X time 
	Intention in action is any action that makes something active ( get.. is not active operation) 
	*/
	K_DWORD IdleTimeForLowPowerInSec;
	K_DWORD Time2ResetIfNoComunicationInSec;// default 20 minites

	// Automat event per sensor 
	K_BOOL SendAutomatEvent_DriverModule;// if Equal to 1 the event will be sent automatically 
	K_BOOL SendAutomatEvent_Switch;// if Equal to 1 the event will be sent automatically 
	K_BOOL SendAutomatEvent_PaperLevel;// if Equal to 1 the event will be sent automatically 
	K_BOOL SendAutomatEvent_PrinterDoor;// if Equal to 1 the event will be sent automatically 

	// anable disable feathure 
	K_BOOL  SupprtFanFunctions;
	K_BOOL  SupprtDriveModuleFunction;//  MX6 has been such this function  it is backup
	K_BOOL  SupprtKeyboardFunction;
	K_BOOL  SupprtBataryAndChargerMonitoring;// MX6 has been such this function  it is backup
	K_BOOL  SupprtBataryAndChargerCommand;  // MX6 has been such this function it is backup 


	
	

};

typedef TR_PACK_PREFIX struct TAG_St_K10Param St_K10Param;

//-------------------------------------------------------------------
//e_CmdK10_PeriodicMonitorPoll,   St_K10_PeriodicMonitorPoll

typedef enum 
{
 e_ComplexSam1=0,                 //e_CpmlexStatus
 e_ComplexSam2=1,
 e_ComplexSam3=2,
 e_ComplexSam4=3,
 e_ComplexCurrent24=4,            //e_CpmlexValue
 e_ComplexGood24=5,               //e_CpmlexStatus
 e_ComplexChargeState=6,          //e_CpmlexStatus
 e_ComplexSomPowerGood=7,         //e_CpmlexStatus
 e_ComplexBataryVotage=8,         //e_CpmlexValue      
 e_ComplexBataryCurrent=9,        //e_CpmlexValue
 e_ComplexVoltage5=10,             //e_CpmlexValue
 e_ComplexExternalCurrent=11,      //e_CpmlexValue
 e_ComplexExternalVoltage=12,      //e_CpmlexValue
 e_ComplexVoltage9=13,             //e_CpmlexValue
 e_ComplexRfidOn=14,
 e_ComplexRfIDAntenaSensor=15,     //e_CpmlexValue
 e_ComplexSensorACC=16,
 e_ComplexSensorDriverModuleIn=17,
 e_ComplexSensorPaperLow=18,
 e_ComplexSensorFanFault=19,        //e_CpmlexStatus 
 e_ComplexSensorPrinterDoor=20,
 e_ComplexLightSensor=21,          //e_CpmlexValue
 e_CompleAppVer=22,                //e_CpmlexValue
 e_ComplexLoaderVer=23,            //e_CpmlexValue  
 e_ComplexBoardVer=24,             //e_CpmlexValue
 e_ComplexRTC=     25,             //e_CpmlexStatus
 e_ComplexCardIn = 26,						 //
  e_ComplexLast,
}e_ComplexItem;

#define COMPLEX_COUNT e_ComplexLast
typedef enum 
{
 e_ComplexStatusOk=0,
 e_ComplexStatusFail=1,
 e_ComplexDisable=2,
 e_ComplexStatusUnknown=3,
 }e_ComplexStatus;

typedef enum 
{
 e_CpmlexValue=0,// any integer value
 e_CpmlexStatus=1,//e_ComplexStatus
 e_CpmlexSensor=2,//e_ComplxSensorValue
 e_CpmlexOnOff=3,//
}e_ComplexType;

typedef enum 
{
	e_SensorNoActive=0,
	e_Sensorigh=1,
}e_ComplxSensorValue;

typedef enum 
{
	e_CmlxStateOff=0,
	e_CmlxStateON=1,
}e_ComplxOnOffValue;

typedef  TR_PACK_PREFIX struct 
{
 K_WORD ComplexId; //e_ComplexItem
 K_BYTE ComlexType;//e_ComplexType

 /*
  in case of e_ComplexType==e_CpmlexStatus data is  status (e_ComplexStatus)
  in case of e_ComplexType== e_CpmlexOnOff data is e_ComplxOnOffValue 
  in case of e_ComplexType== e_CpmlexSensor data is  e_ComplxSensorValue
  in case of e_ComplexType== e_CpmlexValue data is value

 */
 K_DWORD ComplexData;// in case of e_ComplexType== e_CpmlexValue 
 K_BOOL  IsChangedFromLastSample; 
 }St_ComplexSate;

TR_PACK_PREFIX struct TAG_St_K10_PeriodicMonitorPoll
{
	K_WORD ComplexCount;// Number of items can be smaller than the maximum (COMPLEX_COUNT)
	St_ComplexSate Array[COMPLEX_COUNT];
};

typedef TR_PACK_PREFIX struct TAG_St_K10_PeriodicMonitorPoll St_K10_PeriodicMonitorPoll,St_K10_PeriodicMonitorEvent;

//-------------------------------------------------------------------
//e_CmdK10_EventSensorChange=33,//           St_K10_EventSensorChange
//-------------------------------------------------------------------

TR_PACK_PREFIX struct TAG_St_K10_EventSensorChange
{
 K_BYTE SensorThatChane;// e_ComplexItem e_ComplexSensorPrinterDoor/e_ComplexSensorACC/e_ComplexSensorDriverModuleIn/e_ComplexSensorPaperLow
 K_BYTE SensorState;//e_ComplxSensorValue





};
typedef TR_PACK_PREFIX struct TAG_St_K10_EventSensorChange St_K10_EventSensorChange;

// Flash bin file header (on file system)
typedef TR_PACK_PREFIX struct                      
{
    K_DWORD CPUType;		// CPU type 
    K_DWORD Signature;		// File signature 
    K_DWORD CRC32;			// Data CRC
    K_DWORD Length;			// Data length
    K_DWORD Version;		// File version
    K_DWORD Offset;			// Destination Addrss 
    K_DWORD BitMap;			// TBD
    K_DWORD LRC;			// Header  Lrc
} BinFileHeader;

//	e_CmdK10_LdrGetAppVersion   St_K10_LdrGetAppVersion

typedef TR_PACK_PREFIX struct
{
    K_DWORD  CRC32;		
    K_DWORD  Length;
    K_DWORD  Offset;
    K_DWORD  Version;
    K_DWORD  Reserved[3];
    K_DWORD  LRC;
}St_LoaderAppHeader; // the header that loader write to check software integrity

TR_PACK_PREFIX struct TAG_St_K10_GetAppVersion
{
	K_BYTE AppOrLoader;// 0xaa=APP 0xbb=loader
	K_WORD LoaderVersion; // not relevant in App case (0xaa)
	K_WORD AppVersion; // if appliction not exist the version is 0
	K_DWORD AppActualCrc32;//  The actual calculated crc32 of application
	St_LoaderAppHeader LoaderAppHeader; //(not relevant in App case (0xaa))
	K_BYTE ProrotocolDataVer; // 

};
typedef TR_PACK_PREFIX struct TAG_St_K10_GetAppVersion St_K10_GetAppVersion;
//-------------------------------------------------------------------
//	e_CmdK10_DownLoad   St_K10_DownLoad
// buffer is long (2000 ms ) 
TR_PACK_PREFIX struct TAG_St_K10_DownLoad  // each buffer the loader write the buffer and return ack the timeout for the first buffer and last 
{
	K_WORD AppVersion;  
	K_DWORD AppCRC32;  // after write all data the loader check if the crc32 that sent is equal  to for actual  dtat written 
	K_DWORD PacketNum;// run form 0 to n 
	K_DWORD Offset;// the offset in application area to write PacketSize bytes 
	K_WORD PacketSize;// MX6 does not have to use all the allocated size (MAX_BUFFER_SIZE_FOR_DOWNLOAD)
	K_BYTE IsLast;// 0 not last 1 last (when it last the the loader update header and return ack 
	K_BYTE Buffer[MAX_BUFFER_SIZE_FOR_DOWNLOAD];

};
typedef TR_PACK_PREFIX struct TAG_St_K10_DownLoad St_K10_DownLoad;

//-------------------------------------------------------------------
//e_CmdK10_SetRTCTime=23,//                     St_K10_RTCTime                        void 
TR_PACK_PREFIX struct TAG_St_K10_RTCTime  

{
	K_DWORD Year;
	K_BYTE Month;
	K_BYTE Day;
	K_BYTE Hour;
	K_BYTE Minutes;
    K_BYTE Secondes;
};
typedef TR_PACK_PREFIX struct TAG_St_K10_RTCTime St_K10_RTCTime;








   







#ifdef win_or_linux
#pragma pack(pop, W_K10232_P_MSSG_SET)
#endif

#endif  //PROTO_FT_MSGS

