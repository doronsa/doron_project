

#if !defined(CONFIG_H_)
#define CONFIG_H_ // Symbol preventing repeated inclusion

///////////////////////////////////////////////////////////////////////////////
//
// File: Config.h
// Configuration file
// Eitan Michaelson, Transway 2011.
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// Section  : Hardware defaults defines
//
///////////////////////////////////////////////////////////////////////////////

// Product type, this will define the matrix table that will be used
#define CORE_BUILD_TR1020M  

#define CORE_USES_XTAL
#define CORE_CLK_MHZ                    PLL96                                       // XTAL Mhz
#define CORE_REF_CLK                    XTAL8                                       // Crystal 
#define CORE_MEM_SIZE_OF_RAM            (128 * 1024)                                // 128k SRAM
#define CORE_MEM_SIZE_OF_FLASH          (512 * 1024)                                // 512K
#define CORE_MEM_SIZE_OF_SECTOR         (2 * 1024)                                  // Flash sector size


///////////////////////////////////////////////////////////////////////////////
//
// Section  : Basic modules (not for micro build!)
//
///////////////////////////////////////////////////////////////////////////////

#define CORE_SUPPORT_MFRC
#define CORE_SUPPORT_LIBC
#define CORE_SUPPORT_LIBCRYPTO
#define CORE_SUPPORT_LIBTIME
#define CORE_SUPPORT_SMARTCARD
#define CORE_SUPPORT_SAM
#define CORE_SUPPORT_CALYPSO
#define CORE_SUPPORT_CONTACTLESS
#define CORE_SUPPORT_RTC
//#define CORE_SUPPORT_TICKET

///////////////////////////////////////////////////////////////////////////////
//
// Section  : Winows/Linux Porting
//
///////////////////////////////////////////////////////////////////////////////

//#if this is a linux machine
//	#define linux 1
//#endif
#ifdef FLAG_ENABLE_COMM  //should be defined if in TIM7020 code
	#define ENABLE_COMM
#endif
#endif // CONFIG_H_
