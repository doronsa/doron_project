/*
 * demon_util.c
 *
 *  Created on: Apr 4, 2014
 *      Author: doronsa
 */

#include "TW_K10232_p.h"
#include "P_TwMtrBase.h"
#include "demon_util.h"
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
int GetPackeOk = 1;
extern CoreReturnVal SendCommandToK10 ( int UARTId, void *Bytes, int Length, int Timeout);
typedef struct _AppCmdData
{
    //In
    unsigned long InDataSize;
    void*         pInData;
    //Out
    unsigned long OutDataSize;
    void*         pOutData;
}AppCmdData;

static AppCmdData CmdData;

int K10_Download(void)
{

	FILE* fp;
	BinFileHeader SDHeader;

	int status;
	unsigned long Len;
	unsigned long TotalLen;
	St_K10_DownLoad DataIn;  //e_CmdK10_DownLoad=16,//[IN]St_K10_DownLoad, [OUT]void
	const char * FileName = "FLASH.BIN";
	unsigned long t1,POffset = 0;
	char *buffer = (char*)malloc(100000);
	//Open file
	fp = fopen(FileName, "rb");
	if(fp == NULL)
	{
		printf("fail to open %s" ,FileName);
		return 0;
	}
	//Get header
	Len = fread(&SDHeader,1,sizeof(SDHeader),fp);
	if(Len != sizeof(SDHeader))
	{
		printf("fail to read header" );
		fclose(fp);
		return 0;
	}
	//TBD: check crc32
	//TBD: check file Length
	TotalLen = SDHeader.Length;
	// set in data
	DataIn.AppVersion = (unsigned short)SDHeader.Version;
	DataIn.AppCRC32   = SDHeader.CRC32;  // after write all data the loader check if the crc32 that sent is equal  to for actual  dtat written
	DataIn.PacketNum  = 0;// run form 0 to n
	DataIn.Offset     = SDHeader.Offset;// the offset in application area to write PacketSize bytes

	DataIn.IsLast    = 0;// 0 not last 1 last (when it last the the loader update header and return ack
	DataIn.Buffer[MAX_BUFFER_SIZE_FOR_DOWNLOAD];
	DataIn.PacketSize;// MX6 does not have to use all the allocated size (MAX_BUFFER_SIZE_FOR_DOWNLOAD)

	//t0 = GetTickCount();
	//IN
	CmdData.InDataSize = sizeof(DataIn);
	CmdData.pInData	= &DataIn;
	//Out
	CmdData.OutDataSize = 0;
	CmdData.pOutData	= 0;
	Len = fread(buffer,1,TotalLen,fp);
	while(TotalLen)
	{
		while(GetPackeOk == 0);
		GetPackeOk = 0;
		DataIn.PacketSize = (unsigned short)(TotalLen<MAX_BUFFER_SIZE_FOR_DOWNLOAD)?TotalLen:MAX_BUFFER_SIZE_FOR_DOWNLOAD;
		TotalLen -= DataIn.PacketSize;
		//printf("packet %d size %d",DataIn.PacketNum,DataIn.PacketSize );
		if(TotalLen == 0)
			DataIn.IsLast = 1;
		//read packet
		//usleep(100000);
		//Len = fread(DataIn.Buffer,1,DataIn.PacketSize,fp);
		memcpy( DataIn.Buffer, (buffer + POffset), DataIn.PacketSize);
//		if(DataIn.PacketSize != Len)
//		{
//			perror("fail to read packet :\n");
//			printf("fail to read packet %d size %d len %d",DataIn.PacketNum,DataIn.PacketSize , Len );
//
//			fclose(fp);
//			return status;
//		}

		//send packet
		status = e_SendMessage(DeviceFd.K10Fd, e_CmdK10_DownLoad,
								1,//e_IsRequesType
								0,//ApplicationResult
								0,//ApplicationStatusBits
								CmdData.InDataSize, CmdData.pInData,
								DataIn.PacketNum==0?1200:0);
		/*
		 * ComResult = e_SendMessage( DeviceFd.DCUKeyDrv, SandDtataToSDU[var].i_cmd,
											SandDtataToSDU[var].e_IsRequesType,
											SandDtataToSDU[var].ApplicationResult,
											SandDtataToSDU[var].ApplicationStatusBits,
											SandDtataToSDU[var].datalen, SandDtataToSDU[var].data,
					                        (SandDtataToSDU[var].datalen * 10));
		 * */
		//DataIn.PacketNum==0?1200:0;
		//status = SendCommandToK10 ( DeviceFd.K10Fd, &CmdData, DataIn.PacketSize, 1000);

		//Check output
		if(status > 0)
		{
			printf("fail to Send packet %d size %d",DataIn.PacketNum,DataIn.PacketSize );
			break;
		}
		//INC packet num
		DataIn.PacketNum++;
		//pdate offset
		DataIn.Offset += DataIn.PacketSize;
		POffset += DataIn.PacketSize;
		//usleep(1000000);
	}
	fclose(fp);
	//t1 = GetTickCount();
//	diff = t1 - t0;
//	printf("Total time %d",diff);

	return status;

}
