
#ifndef Contactless_H
 #define Contactless_H

#include <Iso7816.h>

////////////////////////////////////////////////////////////////////////////////////

#define         MAX_CARD_UID_SIZE       10
#define         MAX_CARD_INFO_SIZE      32
#define	        MAX_DATA_BUF	        270      // Must be 256+6
#define         SelectIdle              0x12
#define         Halt                    0x15


////////////////////////////////////////////////////////////////////////////////////

typedef enum // API Errors
{
	 e_Contacless_OK                       = 0,
     e_Contacless_InitErr,
     e_Contacless_MFRCErr,
     e_Contactless_ReadTicketErr,
     e_Contactless_WriteTicketErr,
     e_Contactless_WriteCardErr,
     e_Contactless_ForgetCardErr,
     e_Contactless_BufferSizeErr,
     e_Contactless_CardAccessTimeoutErr,

}e_ContacLessErr;

////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
    e_C_CARD         = 0x30,                        // FULL ISO14443 B COMPATIBLE Igor 17/11/2005
    e_C_TICKET       = 0x31,                        // FULL ISO14443 B COMPATIBLE Igor 17/11/2005
	e_MIF_STD        = 0x08,
	e_MIF_PRO        = 0x20,                        // FULL ISO14443A COMPATIBLE!
	e_MIF_DES_PRO    = 0x24,                        // FULL ISO14443A COMPATIBLE!
	e_MIF_PLUS       = 0x10,
	e_MIF_STD_PLUS   = 0x18,
	e_MIF_ULTRALIGHT = 0x04,
	e_MIF_LIGHT      = 0x01,
	e_NO_MIF         = -1
}e_ClCardType;

////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
	 e_SwitchOFF    = 0,
	 e_SwitchON     = 1
}e_Switch;

////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	 e_ClCardType   e_ClType;                       // MIF1 or PRO or None
	 unsigned char  b_CollDetect;                   // Is collision detected
	 unsigned char  c_ClUidLen;                     // Cl Card size of UID
	 unsigned char  c_ClInfoLen;                    // Cl Card size of information field
	 unsigned char  cp_ClUid[MAX_CARD_UID_SIZE];    // Cl Card UID
	 unsigned char  cp_ClInfo[MAX_CARD_INFO_SIZE];  // Cl Card INFO. field
	 unsigned char  b_ClCardIn;                     // Is Cl Card in field or abscent
     unsigned long  SerialNumber;                   // Redable (reversed) serial
}ContactlessInfo;


////////////////////////////////////////////////////////////////////////////////////
//
// Function: ContactlessInit  
// Description: Setup Mfrc api and make it ready for card
// Parameters:  e_ContacLessErr - error type,comm port (win only)
// Return:      0 Success, 1 Error 
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  


short ContactlessInit(e_ContacLessErr *e_CLessError);// tr1020

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ContactlessSetIdle
// Description: This function sends a Switch2idleState command to the Reader
//	            according to which, the Reader shut OFF its RF field, this
//              function analyzes Reader answer and returns if in Loader or
//              Application state
// Parameters: e_Switch
// Return:     e_ContacLessErr 
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

e_ContacLessErr  ContactlessSetIdle(e_Switch e_SwitchOnOff);


////////////////////////////////////////////////////////////////////////////////////
//
// Function: ContactlessDetect
// Description: This function returns 1 if card detected and the reader
//              interrupted the Host with a Card detected.
//              returns 1 if the Reader interrupted the Host with Card detected
//              0 otherwise.
// Parameters: none 
// Return: 
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

unsigned char  ContactlessDetect(void);

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ContactlessTicketRead
// Description: This function reads a block from the Calypso Ticket 
// Parameters:  unsigned short us_FirstAddr  - the number of a Cticket Memory Block read from
//              unsigned short  BlockSize  -   the size of block (in unsigned chars)
//              (in application or in loader)
//              cp_OutData - holds the information that was read
// Return: 
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

e_ContacLessErr  ContactlessTicketRead(unsigned char c_BlockNum,
							 unsigned char BlockSize ,
						     unsigned char *cp_OutData);

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ContactlessTicketWrite
// Description: This function writes a block from the Calypso Ticket
// Parameters:  unsigned char c_EnVrf    - ignored this is allways being preformed.
//              unsigned char c_BlockNum  - the number of block to write into
//              unsigned char  BlockSize  - the size of block (in unsigned chars)
//              unsigned char *cp_InData  - holds the information that will writen
//              (in application or in loader)
// Return: e_ContacLessErr
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

e_ContacLessErr  ContactlessTicketWrite(unsigned char c_EnVrf,
							 unsigned char c_BlockNum,
                             unsigned char BlockSize ,
						     unsigned char *cp_InData);

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ContactlessForgetCard
// Description: Sends a command to the Reader to erase all data about a selected card
//              and return a card to Idle state by turn RF field "Off" for few ms.
// Parameters: none
// Return: e_ContacLessErr
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

e_ContacLessErr  ContactlessForgetCard(void);

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ContactlessGetCardInfo
// Description: This function returns a pointer to an object containing
//				details on the current card 
// Parameters: none 
// Return: sInfo - Card information struct 
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

ContactlessInfo*  ContactlessGetCardInfo(void); 


////////////////////////////////////////////////////////////////////////////////////
//
// Function: ContactlessTransive
// Description: Transmit data and get the response over 7816 protocol 
// Parameters: 7816 Protocol buffers (in ands out) 
// Return: e_ContacLessErr 
// Notes: 
//
////////////////////////////////////////////////////////////////////////////////////  

e_ContacLessErr  ContactlessTransive(PACKET_7816 *Input,
								   RESPONSE_OBJ *OutPut,int TimeOut);


#endif // Contactless_H
