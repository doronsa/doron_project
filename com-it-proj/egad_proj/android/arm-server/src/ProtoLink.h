
#if !defined(PROTO_LINK_H_)
#define PROTO_LINK_H_ // Symbol preventing repeated inclusion

#include "P_TwMtrBase.h"    // Protocol basae include file
//#include <Core.h>

////////////////////////////////////////////////////////////////////////////////////
//
// Abstract:
//
//
////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
//
// Protocol Hooks
// [IN] RetVal: Uart return value after the RX or TX operation
// [IN] pBuffer: Bytes sent or read from the port
// [IN] BufferLength: Bytes sent or read from the port
//
///////////////////////////////////////////////////////////////////////////////////////

typedef void (*Protocol_TXHook)(unsigned char RetVal, unsigned char *pBuffer,int BufferLength);
typedef void (*Protocol_RXHook)(unsigned char RetVal, unsigned char *pBuffer,int BufferLength);

///////////////////////////////////////////////////////////////////////////////////////
//
// Protocol parameters
//
///////////////////////////////////////////////////////////////////////////////////////
#ifndef NULL
#define NULL 0
#endif

#define     PROTO_DEFAULT_TIMEOUT                       4096
#define     PROTO_IS_WORKING_WITH_BYTE_EVENT            1       // if true the client 232 driver have to call to  v_OnByteReceive every byte that recived
#define     PROTO_TIME_WAIT_TO_ENTERCRITICALSECITON     4096
#define     PROTO_BUFFER_SIZE                           1024
#define     PROTO_USING_EXTERNAL_TIMER                  1
#define __core_cb       void
typedef __core_cb       (*CORE_UART_CB)    (char Byte);

typedef enum
{
	e_Embedded_Unknown = -1,
    e_Embedded_TR1020m=0,					//  Without SAM
    e_Embedded_TR1020m_s=1,				//  With SAM
    e_Embedded_PMU=2,
    e_Embedded_Nurit=3,
    e_PC,                               //	It's not an embedded but we need it here in some rare cases
    eEmbedded_Last,                     //	Must be the kast one
}Proto_EmbeddedTypeEnum;

typedef     void* PROTO_HANDLE;

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ProtocolSetHook 
// Description: Set a hook api for RX and TX logging
// Return:  void  
//
////////////////////////////////////////////////////////////////////////////////////

void ProtocolSetHook(Protocol_TXHook TxHook, Protocol_RXHook Rxhook);

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ProtocolStart 
// Description: Start a serial protocol for a specific device
// Return:  a valid handle, on error - null    
//
////////////////////////////////////////////////////////////////////////////////////

PROTO_HANDLE ProtocolStart( int  UARTId,                      // [IN] The serial comm port id
                            unsigned char *pUARTBufferPtr,          // [IN] Buffer for the internal UART Setup
                            int UARTBufferLen,                      // [IN] Length of the UART internal buffer
                            PROC_ON_MSG_RECEIVED MsgRecvFunc,       // [IN] a handler for protocol messeges
                            PROC_ON_MSG_ERR MsgErrFunc);            // [IN] a handler for protocol errors


////////////////////////////////////////////////////////////////////////////////////
//
// Function: ProtocolRelease
// Description: Free the protocol session
// Parameters:PROTO_HANDLE
// Return: void
//
////////////////////////////////////////////////////////////////////////////////////

void ProtocolRelease(void);

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ProtoGetChannele
// Description:
// Params: 
// Return: 
//
////////////////////////////////////////////////////////////////////////////////////

CHANNEL_HANDLER ProtoGetChannele(void);


void ProtoUARTPolling(void);

// UART Config structure
typedef enum                                                    // UART Id
{
    eCoreUART0,
    eCoreUART1,
    eCoreUART2,
    eCoreUART3,
    eCoreUART4,
    eCoreUART5,
    eCoreUARTLast,
    eCoreUARTNone,                                              // Invalid Uart
}eCoreUARTId;

typedef enum                                                    // TX or RX enum
{
    eCoreUARTRx,
    eCoreUARTTx,
    eCoreUARTRxAndTx,
}eCoreUARTRxTx;

typedef enum                                                    // UART Parity
{
    eCoreUARTParityNone,
    eCoreUARTParityEven,
    eCoreUARTParityOdd,
}eCoreUARTParity;

typedef enum                                                    // UART Baudrates
{
    eCoreUARTBaud9600   = 9600,
    eCoreUARTBaud19200  = 19200,
    eCoreUARTBaud38400  = 38400,
    eCoreUARTBaud57600  = 57600,
    eCoreUARTBaud115200 = 115200,
    eCoreUARTBaud230400 = 230400,
}eCoreUARTBaud;


typedef struct
{
    eCoreUARTId         UARTId;
    eCoreUARTBaud       Baud;
    eCoreUARTParity     Parity;
    char             RxRTS;                                  // Receiver request-to-send enable
    char             TxRTS;                                  // Transmitter request-to-send enable
    char             TxCTS;                                  // Transmitter clear-to-send enable
    char             RxEchoEnabled;                          // Set to 1 if you need RX echo
    char             TxEchoKill;                             // Set to 1 if you need to kill the tx echo
    char             Uart7816Invert;                         // Set to 1 to invert the TX (7816 )
    char             Uart7816Use9Bit;                        // Set 1 to use start + 9 data bits - as this is 7816 setup
    char             *RXBuffer;                              // Caller provided RX buffer
    int            RXBufferLength;                         // RX Buffer length
    CORE_UART_CB        UARTExternalHandler;                    // External handler for incomming bytes
    CORE_UART_CB        UARTExternalFifoFull;                   // External handler for full fifo alert
}CoreUARTConfig;

//typedef enum
//{
//    eCoreError = 0,
//    eCoreOK,
//
//}CoreReturnVal;

#endif // PROTO_LINK_H_
