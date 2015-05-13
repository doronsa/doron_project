/*
 * demon_util.c
 *
 *  Created on: Apr 4, 2014
 *      Author: doronsa
 */

#include "TW_K10232_p.h"
#include "P_TwMtrBase.h"
#include "daemon_util.h"
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

int GetPackeOk = 1;
extern int DelayForLongData;
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

/******************************************************************
 *  Function name: K10_Download
 *  Description: Util for update the K10 cpu
 *  in :/sdcard/FLASH.BIN
 *  out
 ******************************************************************/
int K10_Download(void)
{

	FILE* fp;
	BinFileHeader SDHeader;

	int status;
	unsigned long Len;
	unsigned long TotalLen;
	St_K10_DownLoad DataIn;  //e_CmdK10_DownLoad=16,//[IN]St_K10_DownLoad, [OUT]void
	const char * FileName = "/sdcard/FLASH.BIN";
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
		DelayForLongData = 9;
		GetPackeOk = 0;
		DataIn.PacketSize = (unsigned short)(TotalLen<MAX_BUFFER_SIZE_FOR_DOWNLOAD)?TotalLen:MAX_BUFFER_SIZE_FOR_DOWNLOAD;
		TotalLen -= DataIn.PacketSize;

		if(TotalLen == 0)
			DataIn.IsLast = 1;

		memcpy( DataIn.Buffer, (buffer + POffset), DataIn.PacketSize);

		//send packet
		status = e_SendMessage(DeviceFd.K10Fd, e_CmdK10_DownLoad,
								1,//e_IsRequesType
								0,//ApplicationResult
								0,//ApplicationStatusBits
								CmdData.InDataSize, CmdData.pInData,
								DataIn.PacketNum==0?1200:0);

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
	}
	fclose(fp);

	return status;

}
