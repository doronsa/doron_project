#ifndef BIT2BYTE_H
	#define BIT2BYTE_H

/*
/////////////////////////////////////////////////////////////////////////
// File name:	    BIT2BYTE.h


FUNCTIONS:

v_Bit2Byte_ConvertData 
/////////////////////////////////////////////////////////////////////////
*/

/*
//////////////////////////////////////////////////////////////////////////////
                                       DEFINES
//////////////////////////////////////////////////////////////////////////////
*/
#ifdef WIN32
#ifndef NDEBUG // print to screen active only when compiled in debug mode NOT ACTIVE in release mode
 #define SHOW_BIT2BYTE_CONVERT 0//1
#else
 #define SHOW_BIT2BYTE_CONVERT 0//1
#endif
#else
 #define SHOW_BIT2BYTE_CONVERT 0
#endif



/*
//////////////////////////////////////////////////////////////////////////////
                                       ENUMS
//////////////////////////////////////////////////////////////////////////////
*/

//convert direction - struct to binary Or binary to struct
typedef enum
{
	e_BitStream2St,
	e_St2BitStream,
}e_ConvertType;

typedef enum
{
	e_IntelType, // set when using visual studio , borland 
	e_MotorolaType,//  set when using 8051
}e_CompilerByteOriented;


typedef enum
{
	e_BitStreamType,//don't care - flip byte is not needed - 
	e_VariableType,// types that their byte oriented are subjected to the Compiler Byte Oriented ( big \ littel indian )
}e_Bit2Byte_Fieltype;

typedef enum
{
	e_Bit2Byte_base10,//base10
	e_Bit2Byte_Hex,//hex
}e_Bit2Byte_Base;

typedef enum
{
	e_Bit2Byte_MainField,
	e_Bit2Byte_SubField,
}e_Bit2Byte_FieldType;

/*
//////////////////////////////////////////////////////////////////////////////
                                      STRUCTS
//////////////////////////////////////////////////////////////////////////////
*/

typedef struct
{
	char* cp_FieldName; // field name to print to screen
	e_Bit2Byte_Base e_Base; // HEX or Base 10 printing format
	e_Bit2Byte_FieldType e_FieldType; // Main Or Sub Type
	char* cp_MainFieldName; // relevat only for Sub Field type
	unsigned char c_TabCount; // to print sub filed  
}st_Bit2Byte_PrintInfo;

typedef struct
{
	unsigned char uc_StreamBitCount;//Field Bit Count
	unsigned short us_StreamBitOffset;// Field Bit Offset
	void *vp_StFieldPtr;
	e_Bit2Byte_Fieltype  e_Fieltype;
	st_Bit2Byte_PrintInfo st_PrintInfo;
}st_FieldDescriptor;


/*
//////////////////////////////////////////////////////////////////////////////
                           API   FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
*/
void v_Swap(void *i_RewLenSwap,int i_Count);

//set the compiler Byte Oriented - to know if flip byte is needed 
void v_Bit2Byte_InitInterface(e_CompilerByteOriented CompilerByteOriented);

void v_Bit2Byte_Convert( e_ConvertType ConvertType,//[IN] convert direction - bit stream to struct OR struct to bit stream
					     st_FieldDescriptor *stp_FieldDescriptorArr,//[IN] Field Descriptor Array
					     unsigned char uc_DescriptorArrLen,//[IN]Field Descriptor Array len   
						 unsigned char *ucp_BitStream);//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream   




#if SHOW_BIT2BYTE_CONVERT

void v_Debug_bin2AsciiBase2( unsigned char *ucp_BinDataInput,//[IN] binary data 
					   unsigned short ush_BinBitLen,//[IN] binary data bit length  
					   unsigned short ush_BinBitOffset,//[IN] binary data bit offset length  
					   char *cp_strOut);//[OUT] output string out ( len = strlen) 

//call back to print to screen / terminal 
typedef int (*Myprintf)( const char *format , ... );
void v_SetPrintfCallback(Myprintf UserCallback);
void v_IsPrintAllow(char b_IsPrintAllow);//[IN] set print ON/OFF

#endif

#endif
