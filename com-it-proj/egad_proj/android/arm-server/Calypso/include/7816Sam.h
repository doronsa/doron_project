#ifndef _SC_H_
#define _SC_H_

#include <Core.h>

#define SAM_XTAL 3.56


typedef enum
{
    eSAMTypeCalypso,
    eSAMTypeSecurity, // AKA Sam Transway
    eSAMTypeLast,

}eSAMType;
////////////////////////////////////////////////////////////////////////////////////
//
// Function: SamInit
// Description: Init SAM 7816 API
// Parameters:  
// Return:  	0 - fail
//              1- success
//
////////////////////////////////////////////////////////////////////////////////////  

char SamInit(eSAMType SamType,eCoreUARTId UARTId);


////////////////////////////////////////////////////////////////////////////////////
//
// Function: SamReset
// Description: Reset the sam (and get the ATR)
// Parameters:  
// Return:  	0 - fail
//              1- success
//
//////////////////////////////////////////////////////////////////////////////////// 

char SamReset(eSAMType SamType,unsigned char *p_OutAtr,	        // [OUT] pointer to ATR string
	int MaxAtrAlocate,			// [IN] max le of ATR
	int *p_OutAtrLen			// [OUT] real len of ATR
	);


////////////////////////////////////////////////////////////////////////////////////
//
// Function: SamSetSpeed
// Description: Sets the sam  baud rate
// Parameters: When  Speed is 0 the function will set HSP flag and adjust uart speed
//             The selected reader id.  Speed could be 0 or eUBaudRate 
// Return:  	0 - fail
//              1- success
//
////////////////////////////////////////////////////////////////////////////////////

char SamSetSpeed(eSAMType SamType,unsigned long  BaudRate);  // [IN] Speed        

////////////////////////////////////////////////////////////////////////////////////
//
// Function: SamTransive
// Description: transmit and receive the data 
// Parameters:  
// Return:  	0 - fail
//              1- success
//
////////////////////////////////////////////////////////////////////////////////////  

char SamTransive(eSAMType SamType,unsigned char LenIn,	            // [IN]	in data len
	unsigned char *BuffIn,	 // [IN]	buffer of in data
	unsigned char LenOut,	 // [IN] data out len
	unsigned char *BuffOut,  // [OUT]buffer of out data 
	int _TimeOut			 // [IN] timeout
	);
#endif

