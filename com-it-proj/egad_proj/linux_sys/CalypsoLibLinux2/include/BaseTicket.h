/******************************************************************************
 *
 *    ORIGINAL PATH: D:\Projects\CALYPSO\TR1000\src\TR1000\SRC\CTicket.h
 *
 *    DESCRIPTION: CTicket(R)(aka CALYPSO Memory Cards) Support Interfaces.
 *
 *    AUTHOR:   Igor.
 *
 *    HISTORY: Created 02/01/2006.   
 *
 ******************************************************************************/
#ifndef CTICKET_H
#define CTICKET_H

#define NAK_TYPE		        0x51    // 
#define ACK_TYPE		        0x50    //  
#define C_TICKET_READ            0x62 // App
#define C_TICKET_WRITE           0x63 // App
#define C_TICKET_SELECT          0x60 // App
#define C_TICKET_HALT            0x61 // App

#define ISO_B_TICKET                   0x31





typedef enum 
{
	OK=0,                      //0
	
	BAD_CHKSUM,                //1 //communuication error
	BAD_CRC,                   //2 //communuication error
	INVL_LENGTH,               //3 //communuication error
	TIMEOUT_ERR,               //4 //communuication error
	UNKNOWE_TYPE,              //5 //communuication error
	MSG_TO_BIG,                //6 //communuication error
	INVALID_HEAD,              //7 //communuication error
	Ns_ERR,                    //8
	INVALID_DATA_FIELD,        //9 //communuication error
	
	CARD_NOTAGERR,             //a  
	CARD_CRCERR,               //b
	CARD_EMPTY,                //c
	CARD_AUTHERR,              //d     
	CARD_PARITYERR,            //e
	CARD_CODEERR,              //f
	CARD_SERNRERR,             //10
	CARD_NOTAUTHERR,           //11
	CARD_BITCOUNTERR,          //12
	CARD_BYTECOUNTERR,         //13
	CARD_IDLE,                 //14
	CARD_TRANSERR,         	 //15
	CARD_WRITEERR,          	 //16
	CARD_VALERR,           	 //17
	CARD_KEYERR,             	 //18
	CARD_READERR,            	 //19
	CARD_OVFLERR,            	 //1A
	CARD_UNKNOWN_ERR,		     //1B
	CARD_FRAMINGERR,           //1C
	CARD_UNKNOWN_COMMAND,      //1D
	CARD_COLLERR,              //1E
	CARD_ACCESSTIMEOUT,        //1F
	CARD_MAX_CARD_CAPACITY_ERR,//20 
	UNKNOWN_UID,				 //21
	ILLEGAL_UID,				 //22
	CARD_APP_BRAKE,			 //23 for debug only 
	CARD_CODINGERR     = 25,   //25 //Must be in Hex - Igor
	CARD_NYIMPLEMENTED = 49,   //49
	// ILAN's additionals
	
	BAD=70,					 //70
	FLASH_ERASE_ERR,           //71
	FLASH_WRITE_ERR,			 //72
	FLASH_WRITE_TIME_OUT,      //73
	FLASH_NO_APP,              //74
	FLASH_NO_APP_CPY,           //75
	
	/* Igor 2/01/06 CTicket Errors */
	
	ODD_DATA_SIZE_E = 0x80,
	BIG_DATA_SIZE_E,
	BIG_BLOCK_NUM_E,
	NO_TICKET_E,
	BLOCK_WRITE_FLT_E,
	READ_ONLY_AREA_E,
	WRITE_VER_FLT_E,
	
	MFRC_FAULT = 0xA0,
	MFRC_TYPE_B_NOT_SUPPORT
} MSG_ERR;


/*============================================================================*/
/*                             P U B L I C   D A T A                          */
/*============================================================================*/


/*============================================================================*/
/*                        P U B L I C   F U N C T I O N S                     */
/*============================================================================*/
 unsigned char HostCommCTicket_Read(unsigned char FirstBlock  , unsigned char DataSize, unsigned char *DataBuff)  ;
 unsigned char HostCommCTicket_Write(unsigned char  BlockNum,unsigned char DataSize,unsigned char  *DataBuff);
 char HostCommCTicket_Halt(unsigned char cmd,unsigned char *DataBuff)  ;
/// char HostCommCTicket_Select(unsigned char xdata *Buffer); // ???
 char HostCommCTicket_SelectIdle(unsigned char  *DataBuff);
#endif

