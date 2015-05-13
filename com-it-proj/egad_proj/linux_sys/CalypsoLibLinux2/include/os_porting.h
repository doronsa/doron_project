#pragma once

#ifndef OS_PORTING_H
	#define OS_PORTING_H


#define LINUX
#ifdef WIN32
    #include <ProtoFT.h>
#endif

//#include <TW_DCU_CDC_P.h>
//#include <TW_D_S_A.h>
#include "P_TwMtrBase.h"


#ifdef LINUX //declare types from ProtoLink in windoes

#define     PROTO_DEFAULT_TIMEOUT       (1000)

#define     e_CmdGetAppVer               (237)

typedef     void* PROTO_HANDLE;

//typedef enum e_ComResult
//{
//	 e_ComOk,
//	 e_ComTimeOut,
//	 e_ComInvalid,
//	 e_ComNotInit,
//	 e_ComCallbackMissing,
//	 e_ComInvalidHandler,
//	 e_ComTheFunctionNotFitForThisMode,
//	 e_ComLinkNack,
//	 eCom_Q_ReqFull,
//	 e_ComHandShakeTimeOut,
//	 e_ComDataLengthToBig,
//	 e_ComDataLengthMoreThenAllocatedSize,
//	 e_ComUnKnwon,
//	 e_ComInvalidArgument,
//	 e_ComNotImpl,
//	 e_ComInvalidArgument_Use_e_SendResponse_Instead
//} e_ComResult;

//typedef enum
//{
//	e_ReqTypeRequest=1,// the outher side must respond
//	e_ReqTypeRespond=0, // respond to the request
//	e_ReqTypeEvent=3,// the outher side does not must respond ,this request is not relavant for master/slave mode
//}e_RequestType;

#endif

typedef int (*DEV_RemoteCallBack)(int Cmd,int Timeout,int OutDataSize, void *pOutData,int InExpectedSize,void *pInData);


typedef struct
{
	DEV_RemoteCallBack	ProtocolCB;				// pointer ReaderExchangeDataCallBack function
	//Time and Date
//	GetTimeAndDateCallBack		TimeAndDateCB;			//	pointer GetTimeAndDateCallBack CB function
}
st_InitDCU;

typedef struct tag_DEVICE_INFO
{
	int  Unit; //defined in ProtoLink.h
//	St_dcu_GetAppVersion           Ver;//defined in TW_DCU_CDC_P.h
	int					Port;
	unsigned char		InfoValid;

}DEVICE_INFO;

int GetOSRandom(unsigned long *os_random);




void ProtocolRelease(PROTO_HANDLE ProtoHandler);

#endif


