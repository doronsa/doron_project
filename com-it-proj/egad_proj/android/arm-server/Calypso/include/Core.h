

#if !defined(CORE_H_)
#define CORE_H_ // Symbol preventing repeated inclusion

///////////////////////////////////////////////////////////////////////////////
//
// File: Core.h
// Version 1.0 
// The one and only include file for all of the BSP low level API's
// Eitan Michaelson, Transway 2011.
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Section  : 
//
///////////////////////////////////////////////////////////////////////////////

#include <CoreTypes.h>
#include <Config.h>

// Standard C include files
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#ifdef 	WIN32
	#include <windows.h>
#endif



///////////////////////////////////////////////////////////////////////////////
//
// Section  : Core System Ticks API
//
///////////////////////////////////////////////////////////////////////////////

#if linux
	#define         CoreDelay(x)            //nothing
#else
	#define         CoreDelay                       Sleep
#endif
#define         CoreGetTickCount                GetTickCount                                            // Get the current tick (each tick is set to 1 millisecond)
#define         eCoreUARTId                     unsigned short
#define         eCoreUARTNone                   -1 // Invalid uart (comm) id

////////////////////////////////////////////////////////////////////////////////////
//
// Function:    Stdc lost & found
//
////////////////////////////////////////////////////////////////////////////////////

uint8_tw         *strltrim                       (uint8_tw *str, const uint8_tw*trim);    // Left trim
uint8_tw         *strrtrim                       (uint8_tw *str, const uint8_tw *trim);    // Right trim
uint8_tw         *strtrim                        (uint8_tw *str, const uint8_tw *trim);    // Trim - left and right
uint8_tw         *strrepl                        (uint8_tw *Str, uint8_tw *OldStr, uint8_tw *NewStr);   // Replace OldStr by NewStr in string Str.
void             memrev                          (uint8_tw *buf, uint32_tw count);         // Reverse "count" bytes starting at "buf"

////////////////////////////////////////////////////////////////////////////////////
//
// Function:    DES & 3DES Encryption support
//
////////////////////////////////////////////////////////////////////////////////////

void            des_encipher                        (uint8_tw *plaintext, uint8_tw *ciphertext, uint8_tw *key);
void            des_decipher                        (uint8_tw *ciphertext, uint8_tw *plaintext, uint8_tw *key);
int32_tw         des3_encipher                       (const uint8_tw *plaintext, int32_tw ptlen, uint8_tw *key, uint8_tw *ciphertext);
int32_tw         des3_decipher                       (const uint8_tw *ciphertext, int32_tw ctlen, uint8_tw *key,uint8_tw *plaintext);


////////////////////////////////////////////////////////////////////////////////////
//
// Function:    Extended time support API
//
////////////////////////////////////////////////////////////////////////////////////


int             i_GetMaxDateInMonth                 (st_Time tm);                                           // Returns days in month
long            l_TimeH_ConvertStTime2SecFrom2000   (const st_Time *stp_TimeIn);                            // Convert Struct Time to Seconds From 2000
long            l_GetSecFrom2000Now                 (void);
int             b_TimeH_Convert2000Sec2StTime       (const unsigned long l_SecIn,st_Time *stp_TimeOut);     // Convert 2000 Seconds to Struct Time
void            v_TimeH_Time2String                 (const st_Time *stp_TimeIn,char *cp_StringOut);         // Convert the   time struct  into string
void            v_ConvertSt_Time2YYYYMMDDhhmm       (st_Time* tm,char* OutTime,int len);                    // Converts time to YYYYMMDDhhmm format
st_Time         ov_Str2DateTime                     (char* cp_DateStr, e_ModeDate e_Mode);                  // Converts time string format to st_Time structure
int             i_TimeH_GetIsraelGMTOffset          (st_Time *Now);                                         // Get gmt offset
int             i_TimeHGetNow                       (st_Time *Now);                                         // Return current time ( Now )           
int             i_TimeSet                           (st_Time *Now);                                         // Set the current time 

#endif // CORE_H_
