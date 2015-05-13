

#if !defined(CORE_TYPES_H_)
#define CORE_TYPES_H_ // Symbol preventing repeated inclusion

///////////////////////////////////////////////////////////////////////////////
//
// File: CoreTypes.h
// Core basic types
// Eitan Michaelson, Transway 2011.
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// Section  : Basic types
//
///////////////////////////////////////////////////////////////////////////////


//#define uint8_tw   unsigned char
//#define int8_tw    char
//#define uint16_tw  unsigned short
//#define int16_tw   short
//#define uint32_tw  unsigned long
//#define int32_tw   long


typedef unsigned char 	uint8_tw;
typedef char 		 	int8_tw;
typedef unsigned short	uint16_tw;
typedef short 			int16_tw;
typedef unsigned long	uint32_tw;
typedef long			int32_tw;


///////////////////////////////////////////////////////////////////////////////
//
// Section  : Global HAL return value
//
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
    eCoreFalse = 0,
    eCoreTrue,

}CoreBool;

////////////////////////////////////////////////////////////////////////////////////
//
// Function:    DES & 3DES Encryption support
//
////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
    e_CryptoSign8bit  = 1,
    e_CryptoSign16bit = 2,
    e_CryptoSign32bit = 4,
    e_CryptoSign64bit = 8
} e_CryptoSignType;

////////////////////////////////////////////////////////////////////////////////////
//
// Function:    Extended time support API
//
////////////////////////////////////////////////////////////////////////////////////

typedef  struct
{
    unsigned int  ui_Year;        //  Year      : for example 20000
    unsigned char uc_Month;       //  Month.    [1-12]
    unsigned char uc_Day;         //  Day.      [1-31]
    unsigned char uc_Hour;        //  Hours.    [0-23]
    unsigned char uc_Minute;      //  Minutes.  [0-59]
    unsigned char uc_Second;      //  Seconds.  [0-59] (1 leap second)
} st_Time;

typedef enum
{
    e_DocsMode,                     // YYYYMMDDhhmm
    e_WaitMode,                     // dd:mm:yyyy:hh:mm
    e_DocsMode2,                    // YYYYMMDDhhmmss
}e_ModeDate;


#endif // CORE_TYPES_H_
