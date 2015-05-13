

#include <Core.h>
#if defined(CORE_SUPPORT_SMARTCARD) && defined(CORE_SUPPORT_CALYPSO) 


#include <Bit2Byte.h>


////////////////////////////////////
//      DEFINES
///////////////////////////////////

#define NUMBER_OF_BYTES_IN_ON_ROW(num) strcat( cp_strOut, i%(num*8) == num*8-1?",\n":", "); 
#define MAX_DATA_BIT_LENGHT (50*8) // 50 bytes

////////////////////////////////////
//      GLOBAL
///////////////////////////////////

// Not used at the moment  - for future use
e_CompilerByteOriented Global_CompilerByteOriented;


/////////////////////////////////////////////////////////////////////////////////////////////////// 
//FUNCTION:
//              v_Bit2Field
//
//DESCRITION:
//
//			  Convert bit stream to byte base type 
//			  
//RETURN:
//
//				Non
//LOGIC :
//Copy the bit stream to the beggining of the byte filed  - 
//the start Bit Offset  is the LAST bit in the filed
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static void v_Bit2Field( const unsigned char* ucp_BitStream,//[IN] The Bit Stream input 
	unsigned char uc_StreamBitCount,//[IN] Field Bit Count
	unsigned short us_StreamBitOffset,//[IN] Field Bit Offset
	void *vp_FieldOut)//[OUT] field after translation 
{
	int i,shift;
	char IsBitSet,val;
	//calculate minimum byte lan of the byte field 
	unsigned char byteLen = uc_StreamBitCount%8==0? uc_StreamBitCount/8 : (uc_StreamBitCount/8)+1;
	//clear Field bytes
	memset(vp_FieldOut,0,byteLen);
	for(i=0;i<uc_StreamBitCount;i++)
	{
		//for each bit in the byte stream check if it is set or reset
		IsBitSet = ucp_BitStream[(us_StreamBitOffset+i)/8] & (1<<(7-((us_StreamBitOffset+i)%8)));
		//if bit is set  - set bit to 1 in the FieldOut
		if( IsBitSet )
		{
			//calculate the location of the bit to set in the FieldOut
			shift = (uc_StreamBitCount-i)%8?(uc_StreamBitCount-i)%8:8;
			//create value 
			val = (1<<((shift-1)));
			//add value to the FieldOut
			if((uc_StreamBitCount-i)%8 )
				((char*)vp_FieldOut)[(uc_StreamBitCount-i)/8] |= val;
			else
				((char*)vp_FieldOut)[((uc_StreamBitCount-i)-1)/8] |= val;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////// 
//FUNCTION:
//				v_Field2Bit
//
//DESCRITION:
//
//				Convert byte base type to bit stream   
//			  
//RETURN:
//
//				Non
//LOGIC :
//				Copy the byte field into the  bit stream - 
//				Bit 0 in the field the LAST bit offset in the bit stream
//
///////////////////////////////////////////////////////////////////////////////////////////////////


static void v_Field2Bit( unsigned char* ucp_BitStream,//[OUT] The Bit Stream output 
	unsigned char uc_StreamBitCount,//[IN] Field Bit Count
	unsigned short us_StreamBitOffset,//[IN] Field Bit Offset
	void *vp_FieldIn)//[IN] byte field input
{
	long l_Mask;
	long l_tmp = *((long*)vp_FieldIn);
	//	unsigned char *cp_ptr = (char*)&l_tmp ;
	int i;
	//cleare all erelevant bits to 0
	if(uc_StreamBitCount<32)
		l_tmp =	(l_tmp & (~((long)(0xffffffff<<(uc_StreamBitCount))))) ;
	//if mask needed - visual studio bug
	for(i=0;i<uc_StreamBitCount;i++)
	{
		l_Mask = ((long)((long)1<<(uc_StreamBitCount-i-1)));
		if( l_tmp & l_Mask )
			//set the correspond bit in the BitStream output
			ucp_BitStream[(us_StreamBitOffset+i)/8] |= (1<<(7-((us_StreamBitOffset+i)%8)));
		else// clear the correspond bit in the BitStream output
			ucp_BitStream[(us_StreamBitOffset+i)/8] &= ~(1<<(7-((us_StreamBitOffset+i)%8)));
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////// 
//FUNCTION:
//              v_Internal_ConvertData
//
//DESCRITION:
//
//			  call the right function  
//			  
//RETURN:
//
//				Non
//LOGIC :
//
//////////////////////////////////////////////////////////////////////////////////////////////////


#define  v_Internal_ConvertData(  ConvertType, ucp_BitStream,uc_StreamBitCount,us_StreamBitOffset,vp_Field)\
{\
	if( ConvertType == e_BitStream2St )\
	v_Bit2Field(  ucp_BitStream,\
	uc_StreamBitCount,\
	us_StreamBitOffset,\
	vp_Field);\
	else\
	v_Field2Bit( ucp_BitStream,\
	uc_StreamBitCount,\
	us_StreamBitOffset,\
	vp_Field);\
}\


////////////////////////////////
//       API
////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////// 
//FUNCTION:
//              v_Bit2Byte_InitInterface
//
//DESCRITION:
//
//			  set the compiler Byte Oriented - to know if flip byte is needed 
//
//			  
//RETURN:
//
//				Non
//LOGIC : 
//			  Not used at the moment  - for future use
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void v_Bit2Byte_InitInterface(e_CompilerByteOriented CompilerByteOriented)
{
	Global_CompilerByteOriented = CompilerByteOriented;
}

/////////////////////////////////////////////////////////////////////////////////////////////////// 
//FUNCTION:
//              v_Bit2Byte_Convert
//
//DESCRITION:
//
//			  convert Bit2Byte of byte to bit accurding to the ConvertType parameter
//
//RETURN:
//
//				Non
//LOGIC : 
//			  Not used at the moment  - for future use
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void v_Bit2Byte_Convert( e_ConvertType ConvertType,//[IN] convert direction - bit stream to struct OR struct to bit stream
	st_FieldDescriptor *stp_FieldDescriptorArr,//[IN] Field Descriptor Array
	unsigned char uc_DescriptorArrLen,//[IN]Field Descriptor Array len   
	unsigned char *ucp_BitStream)//[IN if e_BitStream2St OUT if e_St2BitStream]bit stream   
{
	unsigned char i;
	for( i=0;i<uc_DescriptorArrLen;i++)
	{
		v_Internal_ConvertData( ConvertType,ucp_BitStream,stp_FieldDescriptorArr[i].uc_StreamBitCount,stp_FieldDescriptorArr[i].us_StreamBitOffset,stp_FieldDescriptorArr[i].vp_StFieldPtr);//[OUT] field after translation
	}

}

#endif // #ifdef CORE_SUPPORT_CALYPSO
