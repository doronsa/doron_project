


#ifdef WIN32
#include <windows.h>
#include <Windowsx.h>
#include <setupapi.h>
    #include <Rpc.h>
#endif
#include <stdio.h>
#include "os_porting.h"
#include "P_TwMtrBase.h"
st_InitDCU Initdcu;

//#pragma comment(lib, "Rpcrt4.lib")

///*extern*/DCU_ReaderExchangeDataCallBack pDeviceExchange;
/*extern*/PROTO_HANDLE             ProtoHandler                    = 0;


#ifdef WIN32
/*extern*/CRITICAL_SECTION    CriticalSection;
    static  Proto_EmbeddedTypeEnum  TargetUnit                      = e_Embedded_DCU6020;
#endif

#define MAX_DEV_STRING          1024
#define MAX_SERIAL_ATTEMPTS     3

static  char                    ProtoWorking                    = 0;
static  int                     ProtoErrors                     = 0;


static unsigned char            SerialTestMode                  = 0;


int DEV_SerialPortTest(int Port/*in*/, DEVICE_INFO* di/*in_out*/)
{


#ifdef LINUX
//	#error //remove #error after implementation
    return 0; //todo implement
#else
    St_ModuleIdentity   Identity;
    //St_dcu_GetAppVersion           AppVer;
	St_AppVer		AppVer;
    st_PrimaryProtocolResource p;
    int                 RetVal;

    if(ProtoHandler)
    {
        //***UISetLed(eLedTypeComm,e_UI_COLOR_RED,NULL);
        ProtocolRelease(ProtoHandler);
        ProtoHandler = 0;
        //***TargetUnit = e_PC;
    }

    ProtoHandler = ProtocolStart(Port,TargetUnit,DEV_MsgReceived,0);
    if(!ProtoHandler)
        return 0;

    // Send check comm message
    SerialTestMode = 1;
    RetVal = DEV_RemoteCall(e_CmdModuleIdentity,250,0,NULL,sizeof(Identity),&Identity);
    SerialTestMode = 0;
    if(RetVal >= 0)
    {

        if((Proto_EmbeddedTypeEnum)Identity.l_mdlIdentity == TargetUnit)
        {
            RetVal = DEV_RemoteCall(e_CmdGetAppVer,PROTO_DEFAULT_TIMEOUT,0,NULL,sizeof(AppVer),&AppVer);
            if(RetVal >= 0)
            {	
				
                //***UIprintf(&pUI->CtlEditResponse,e_UI_COLOR_OFF,"> Found remote device type: %s version %d\r\n",PMU_GetIDType((Proto_EmbeddedTypeEnum)Identity.l_mdlIdentity),AppVer.AppVer);
                //if(AppVer.AppVer != atol(APPLICATION_VERSION))
					di->Unit=TargetUnit;
                    //di->Ver=AppVer;//***MessageBox(pUI->CtlDialog.Hwnd,"���� ��� ������� �� ����� ����� ������","����� ������",MB_OK | MB_ICONEXCLAMATION );
					di->Port=Port;
					di->InfoValid=1;
                //***UISetLed(eLedTypeComm,e_UI_COLOR_GREEN,NULL);
                p.Proc=pDeviceExchange;
                p.p_handle=ProtoGetChannelByDeviceType(TargetUnit);
                //FT_SetLinkWithPrimeryDll(&p);
                return 1;
            }
        }
    }

    ProtocolRelease(ProtoHandler);
    Sleep(50);
    ProtoHandler = 0;
    return 0;


#endif
}

int DEV_SerialPortSearch(DEVICE_INFO* di/*in_out*/)
{
#ifdef LINUX
//	#error //remove #error after implementation
//    return 0;//todo linux
#else
    HDEVINFO        hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD           i;
    int             CommPort;
    char            *Ptr,DevStringBuffer[MAX_DEV_STRING] ={0};
    DWORD           DataT;
    DWORD           buffersize  = 0;
    int             RetVal      = 0;

    // Create a HDEVINFO with all present devices.
    hDevInfo = SetupDiGetClassDevs(NULL,
        0, // Enumerator
        0,
        DIGCF_PRESENT | DIGCF_ALLCLASSES );

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        // Insert error handling here.
        return 1;
    }

    // Enumerate through all devices in Set.
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i=0;SetupDiEnumDeviceInfo(hDevInfo,i, &DeviceInfoData);i++)
    {

        //
        // Call function with null to begin with,
        // then use the returned buffer size (doubled)
        // to Alloc the buffer. Keep calling until
        // success or an unknown failure.
        //
        //  Double the returned buffersize to correct
        //  for underlying legacy CM functions that
        //  return an incorrect buffersize value on
        //  DBCS/MBCS systems.
        //

        if(SetupDiGetDeviceRegistryProperty(
            hDevInfo,
            &DeviceInfoData,
            SPDRP_FRIENDLYNAME,
            &DataT,
            (PBYTE)DevStringBuffer,
            MAX_DEV_STRING,
            &buffersize))
        {

            // Filter somme unwanted ports
            if(strstr(DevStringBuffer,"Bluetooth") || strstr(DevStringBuffer,"PCI"))
                DevStringBuffer[0] = 0;

            Ptr = strstr(DevStringBuffer,"(COM");
            if(Ptr)
            {
                // Found comm port
                CommPort = atoi(Ptr + 4);
                if(CommPort)
                {
                    // Test it!
                    //***UIprintf(&pUI->CtlEditResponse,e_UI_COLOR_OFF,"Checking.. %s\r\n",DevStringBuffer);
                    if(DEV_SerialPortTest(CommPort,di))
                    {
                        RetVal = CommPort;
                        break;
                    }
                }
                DevStringBuffer[0] = 0;
            }
        }
    }

    //  Cleanup
    SetupDiDestroyDeviceInfoList(hDevInfo);
    return RetVal;
#endif
}


int DEV_RemoteCall(int Cmd,int Timeout,int OutDataSize, void *pOutData,int InExpectedSize,void *pInData)
{
#ifdef LINUX
//	#error //remove #error after implementation
//    return 0;//todo linux
#else
    e_ComResult         StatusComm;
    unsigned short      StatusBits = 0;
    int                 StatusApp;
    int                 ActualIncommingBytes = 0;
    int                 Attempts = 0;
    char                ErrorsString[64];

    // Basic parameters check
    if((OutDataSize > 0) && !pOutData)
        return 0;

    if((InExpectedSize > 0) && !pInData)
        return 0;

	if(pDeviceExchange == 0)
		return 0;

    // Request ownership of the critical section.
    EnterCriticalSection(&CriticalSection);

    // Transive
    while(Attempts++ < MAX_SERIAL_ATTEMPTS)
    {
        StatusComm = pDeviceExchange(ProtoGetChannelByDeviceType(TargetUnit),(int)Cmd,Timeout,
            OutDataSize,pOutData, // Send (Outgoing data)
            InExpectedSize,pInData,&ActualIncommingBytes, // Receive (Incomming Data)
            StatusBits,&StatusApp);
        if(StatusComm == e_ComOk)
            break;
        if((StatusComm != e_ComOk) || (InExpectedSize != ActualIncommingBytes))
        {
            ProtoErrors++;
            sprintf_s(ErrorsString,"Link Errors: %d",ProtoErrors);
            //***SetWindowText(pUI->CtlStaticErrors.Hwnd,ErrorsString);
        }

    }

    // Release ownership of the critical section.
    LeaveCriticalSection(&CriticalSection);


    ProtoWorking = 0;

    // Check results and incomming data
    if((StatusComm != e_ComOk) || (InExpectedSize != ActualIncommingBytes))
    {
        //if(!SerialTestMode)
        //    ;//***UIprintf(&pUI->CtlEditResponse,e_UI_COLOR_OFF,"> Command terminated with link error!\r\n");
        return -1;
    }
    else
        return StatusApp;
#endif
    return 0;
}
int GetOSRandom(unsigned long *os_random)
{
#ifdef LINUX
//	#error //remove #error after implementation
//    return DCU_FALSE;//todo linux
#else
	unsigned long dev_id;
	UUID uuid;
	DCU_BOOL res = DCU_TRUE;

	RPC_STATUS ret_val = ::UuidCreate(&uuid);

	if (ret_val == RPC_S_OK)
	{
		dev_id ^= uuid.Data1^(((DWORD)uuid.Data3<<16) | uuid.Data2)^
				(((DWORD)uuid.Data4[7]<<24) | ((DWORD)uuid.Data4[6]<<16) | ((WORD)uuid.Data4[5]<<8) | uuid.Data4[4])^
				(((DWORD)uuid.Data4[0]<<24) | ((DWORD)uuid.Data4[1]<<16) | ((WORD)uuid.Data4[2]<<8) | uuid.Data4[3]);

		*os_random =dev_id;

	}
	else
	{
		res=DCU_FALSE;
		goto exit;
	}
exit:
	return res;
#endif
}

st_InitDCU Initdcu;

int DEV_InitDCU(const st_InitDCU* pdata )
{
	//for debug
	int Cmd = 100;
	int Timeout = 1000;
	int OutDataSize = 8;
	void *pOutData = NULL;
	int InExpectedSize;
	void *pInData = (char*)malloc(8);
	sprintf(pInData,"12345678");

	if (pdata)
	{

		Initdcu.ProtocolCB = pdata->ProtocolCB;
	}
	//DEV_RemoteCallBack(100,300,0,NULL,0,NULL)
	Initdcu.ProtocolCB(Cmd, Timeout, OutDataSize, pOutData, InExpectedSize, pInData );
	return 1;
}

//static void DEV_MsgReceived(void *pData , // [IN]
//    unsigned short Size,        // [IN]
//    int  RequestId,             // [IN]
//    int  Command,               // [IN]
//    int  AppStatus,             // [IN]
//    e_RequestType RequestType)  // [IN]
//{
//
//}


#ifdef LINUX
//void ProtocolRelease(PROTO_HANDLE ProtoHandler)
//{
//
//    //todo LINUX
//
//}
#endif

