#define ENABLE_COMM
#include <Core.h>


#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif
#ifndef win_or_linux

extern unsigned long CoreOSTicks;


////////////////////////////////////////////////////////////////////////////////////
//
// Function: CoreInit
// Description: BSP Init
// Parameters: void
// Return: void
//
////////////////////////////////////////////////////////////////////////////////////

void CoreInit(void)
{

    stSpiConfig         CfgSPI;
    stUrtConfig         CfgUART;
    stConfigComm        CfgCOM;
	
    InitCpuBaseDrvr();          // Base hardware init
    
    // Leds off
    CoreLedOff(eCoreCPURedLed);  
    CoreLedOff(eCoreCPUGreenLed);  
	
    InitIsrDrvr();              // Interrupts init 
	
    // UART config
    CfgUART.UId        =   eUart0;
    CfgUART.ebaudrate  =   eUBaudRate57600; // New speed for TIM
    CfgUART.eparity    =   eParityEven;     // eParityNone;
    CfgUART.eSbit      =   eStopBit_2;
    CfgUART.eDBit      =   eDataBit_8;
	
    // USB Com config
    CfgCOM.commid      =  eCommIdU0;
    CfgCOM.emux        =  eUMux0SamCal;
    CfgCOM.uconfg      =  &CfgUART;
    CfgCOM.spiconfg    =  0;
    CfgCOM.URX_buff    =  NULL;	
    CfgCOM.UTX_buff    =  NULL;	
    CfgCOM.URX_len     =  0;		
    CfgCOM.UTX_len     =  0;
	
    b_Comm_InitInterface(&CfgCOM, 0);
   	
    // SPI config    
    CfgSPI.espi          =   eSpiTypeSPI1;
    CfgSPI.phase         =   eClckPhase_lowBtwnFrames;
    CfgSPI.eDsize        =   eSpiDataSize_8bit;
    CfgSPI.eFRF          =   eFRF_SPI;
    CfgSPI.polarity      =   eSpiPolarity_firstclock;
    CfgSPI.lbckmode      =   eLoopBckModeNormal;
    CfgSPI.sspe          =   eSSP_EnableDisable;
    CfgSPI.e_master      =   eSpiMasterSlave_master;
    CfgSPI.e_slavedis    =   eSlaveOutDisable_enable;
	
    InitSpiDrvr(&CfgSPI);
    CfgSPI.sspe          =   eSSP_EnableEnable;
    InitSpiDrvr(&CfgSPI);
      
    // Init MFRC    
    MfrcInit();
    
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: CoreGetTickCount
// Description: Get current ticks count
// Parameters: void
// Return: Sys ticks
//
////////////////////////////////////////////////////////////////////////////////////

uint32_t        CoreGetTickCount                (void)
{
    return CoreOSTicks; 	
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: CoreMicroDelay
// Description: short wait - do nops while waitig
// Parameters:number if cpu nops to do
// Return: void
//
////////////////////////////////////////////////////////////////////////////////////

void CoreMicroDelay(uint32_t  NopsCount)
{
	
	uint32_t Count = 0;
	
	while(Count++ <  NopsCount)
		__nop();
}


////////////////////////////////////////////////////////////////////////////////////
//
// Function: CoreCPUIOStateSet
// Description: Sets the state (active or not) for an IO Peripheral
// Parameters:
// Return: void
//
////////////////////////////////////////////////////////////////////////////////////

void CoreCPUIOStateSet(eCoreCPULineName LineName,eCoreCPUIOState State)
{

    SetCpuLinesDrvr((eCpuLinesNames)LineName,State);	
}

////////////////////////////////////////////////////////////////////////////////////

CoreReturnVal   CoreUARTInit                    (eCoreUARTId UARTId,CoreUARTConfig *UARTConfig)
{
	stUrtConfig         CfgUART;
	stConfigComm        CfgCOM;

	if(UARTId >= eCoreUARTLast ||  UARTId == eCoreUARTNone)
        return eCoreError;
         
    // Uart config
    CfgUART.UId        =   (eUartId)UARTId;
    CfgUART.ebaudrate  =   (eUBaudRate)UARTConfig->Baud;
    CfgUART.eparity    =   (eParity)UARTConfig->Parity;
    CfgUART.eSbit      =   eStopBit_2;
    CfgUART.eDBit      =   eDataBit_8;
    CfgUART.cropechobytes = 1;

    // Com config
    CfgCOM.commid      =   (CommId)UARTId;
    CfgCOM.emux        =   eUMux0SamUnknown;
    CfgCOM.uconfg      =   &CfgUART;
    CfgCOM.spiconfg    =   0;
    CfgCOM.URX_buff    =   UARTConfig->RXBuffer;
    CfgCOM.UTX_buff    =   NULL;;
    CfgCOM.URX_len     =   UARTConfig->RXBufferLength;;
    CfgCOM.UTX_len     =   0;// sizeof(USB_TxBuffer);
    
    if(b_Comm_InitInterface(&CfgCOM, 0) == 0)
		return eCoreOK;
    
    return eCoreError;
	
	
}

////////////////////////////////////////////////////////////////////////////////////

void            CoreUARTClose                   (eCoreUARTId UARTId)
{
    b_Comm_Close((CommId)UARTId);
}

////////////////////////////////////////////////////////////////////////////////////

void            CoreUARTPurge                   (eCoreUARTId UARTId, eCoreUARTRxTx eUARTRxTx)
{
    b_PurgeComm((CommId)UARTId);
}

////////////////////////////////////////////////////////////////////////////////////

CoreReturnVal   CoreUARTRead                    (eCoreUARTId UARTId,void *Bytes,uint32_t  *Length,uint32_t Timeout)
{
	if(b_Comm_Read((CommId)UARTId, Bytes, (short*)Length, Timeout, eUMux0SamUnknown) == 0)
		return eCoreOK;
    return eCoreError;
	
}

////////////////////////////////////////////////////////////////////////////////////

CoreReturnVal   CoreUARTWrite                   (eCoreUARTId UARTId,void *Bytes, uint32_t Length, uint32_t  Timeout)
{
	
	if(b_Comm_Write((CommId)UARTId, Bytes, Length, Timeout, eUMux0SamUnknown, CORE_UART_IS_POLLING) == 0)
        return eCoreOK;
    return eCoreError;
	
}

////////////////////////////////////////////////////////////////////////////////////

CoreReturnVal   CoreUARTSetBaud                 (eCoreUARTId UARTId,eCoreUARTBaud UARTBaud)
{
    return eCoreError; // Not supported
}

////////////////////////////////////////////////////////////////////////////////////

uint32_t CoreUARTBaudToLong(eCoreUARTBaud Baud)
{
    switch(Baud)
    {
        case eCoreUARTBaud9600:        return 9600;
        case eCoreUARTBaud19200:       return 19200;
        case eCoreUARTBaud38400:       return 38400;
        case eCoreUARTBaud57600:       return 57600;
        case eCoreUARTBaud115200:      return 115200;
        case eCoreUARTBaud230400:      return 230400;
        default:
            return 0;
    }
}

eCoreUARTBaud CoreUARTLongToBaud (uint32_t Baud)
{
    switch(Baud)
    {
        case 9600:        return eCoreUARTBaud9600;
        case 19200:       return eCoreUARTBaud19200;
        case 38400:       return eCoreUARTBaud38400;
        case 57600:       return eCoreUARTBaud57600;
        case 115200:      return eCoreUARTBaud115200;
        case 230400:      return eCoreUARTBaud230400;
        default:
            return (eCoreUARTBaud)0;
    }

}


#endif // WIN32 or linux
