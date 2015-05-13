
///////////////////////////////////////////////////////////////////////////////
//
// Projekt         : TR1000
// Files           : ISO14443.h ISO14443.c
// Created         : 03.04.01
//
// COMMENT: 
//	  This header file can be used either for a uC environment or for 
//        Win32 DLL. Therefore the header MfOsDefs.h is included. Depending 
//        on the environment different definitions are used for "char". 
//        For the microcontroller system, all functions have the return type
//        "char". For the Win32 DLL the return types are changed to "long". 
//        Further more the DLL functions need to be exported, so the 
//        corresponding declarations are made. 
// MODIFICATIONS:
// 01.04.00  HB  first issue
///////////////////////////////////////////////////////////////////////////////
#ifndef ISO14443_H
#define ISO14443_H


#define CID_MASK                0x02
/////////////////////////   ????????????????????????????????
// Mifare error codes. 
#define MIF_OK                 (   0)//0 //OK
#define MIF_NOTAGERR           0xff///(  -1)// FF//No card in Field
#define MIF_CRCERR             (  -2)//FE //wrong CRC received from thecard
#define MIF_AUTHERR            (  -4)//FC //no authentication possible
#define MIF_PARITYERR          (  -5)//FB //wrong parity received from thecard
#define MIF_CODEERR            (  -6)//FA //communication problem
#define MIF_SERNRERR           (  -8)//F8 //wrong serial number read during anticoll
#define MIF_NOTAUTHERR         ( -10)//F6 //card is not authenticated
#define MIF_BITCOUNTERR        ( -11)//F5  //wrong number of bits received
#define MIF_BYTECOUNTERR       ( -12)//F4 //wrong number of bytes received
#define MIF_TRANSERR           ( -14)//F2  //problem during transfer
#define MIF_WRITEERR           ( -15)//F1  //problem during write
#define MIF_OVFLERR            ( -19)//ED //value overflow
#define MIF_FRAMINGERR         ( -21)//EB
#define MIF_UNKNOWN_COMMAND    ( -23)//E9
#define MIF_COLLERR            ( -24)//E8
#define MIF_RESETERR           ( -25)//E7
#define MIF_INTERFACEERR       ( -26)//E6
#define MIF_ACCESSTIMEOUT      ( -27)//E5
#define MIF_CODINGERR          ( -31)//
#define MIF_NYIMPLEMENTED      (-100)//9C
#define MIF_VALERR             (-124)// Problem during write!
#define MIF_RECBUF_OVERFLOW    (-255)
/////////////////////////   ????????????????????????????????
#define DEFAULT_WTXM			0x9///0x01//Multiple the time extension by 1=>ramains the same.
#define NUM_OF_CHAINING_RETRANSMISSIONS 2
#define APP_BRAKE			   0x7e//(-130)//7E
#define WTX						3
#define DECELECT				0x00
#define I_BLOCK 				0
#define R_BLOCK					2
#define S_BLOCK					3
#define MAX_TOTAL_RES			0xff//the maximum respone size of data define by ISO14443/4
#define SW1_SW2_SIZE			2 //SYMBIOS OS adds to status bytes to the data response
#define CARD_BUFF_LEN			0xff //Max data from card=0xff
#define WTX_INFO_BYTE_MASK		0x3f




 







typedef enum {I00, 		//0
              I01, 		//1
              I10,		//2
			  I11,		//3
			  R_ACK0,	//4
			  R_ACK1,	//5
			  R_ERR0,	//6
			  R_ERR1,	//7
			  WTX_RES,	//8
			  S_DESELECT_REQ//9			  
             } HOST_SEND;

typedef enum {I00c, //0
              I01c,	//1
              I10c,	//2
			  I11c,	//3
			  R_ACK0c,	//4
			  R_ACK1c,	//5
		      WTX_REQ,	//6
			  S_DESELECT_RES,	//7
			  RES_ERR,	//8	
			  APP_BRAKEc, //9
			  UNKNOWN_ERR  //10			  
             } CARD_RES;


typedef struct {
	unsigned char b1	:1;
	unsigned char b2	:1;
	unsigned char b3	:1;
	unsigned char b4	:1;
	unsigned char b5	:1;
	unsigned char b6	:1;
	unsigned char b7	:1;
	unsigned char b8	:1;
	} OneBit;
typedef struct {
    unsigned char b12 :2;
  	unsigned char b34 :2;
  	unsigned char b56 :2;
  	unsigned char b78 :2;
    }TwoBit;
typedef struct {
    unsigned char b1234 :4;
  	unsigned char b5678 :4;  	
    }FourBit;

typedef union {
	unsigned char oneByte;
	OneBit	      oneBit;
	TwoBit	      twoBit;
	FourBit		  fourBit;
    unsigned char sixBit :6;
	} ONE_BYTE;



//_____________________________________________________________________________
//
//3.6.8. FUNCTION: 	ProCardTransmit
//PURPOSE: 		Transmit data to the Mifare Pro card.
//Parameters:		
//Parameter1:
//Name:		dataSend 
//Description:	is a pointer to the byte data block that shall be 
//                    		 	written to the card ( 1 - 256 bytes).
//Values:	/
//Parameter2:
//Name:		dataReseive 
//Description:	is a pointer to the byte data block that returns from  the card ( 1 - 256 bytes).
//Values:	/
//Parameter3:
//Name:		cid
//Description:	is the requested card CID
//Values:	/
//Parameter4:
//Name:		ForcedFWT
//Description:	is the requested card CID
//Values:	0 - ignore other (1-14) will cause the transmit process to ignore
//			the FWT declared by the card, and use the given value instead.
//			this option is used mainly for a fast DETECT process, for avoiding 
//			unnecessary long timeout. 
//			
//Call:		MFTC_Cmd  			(MFRC 500 Layer 4) 	
//RETURN: 	char
//
//char ProCardTransmit (void);

char ProCardTransmit (  unsigned char *SendToCardBuf,
                        unsigned char sendBufLen,
                        unsigned char **CardBufOut,
                        unsigned char *RecBufLen,
                        unsigned char cid,unsigned char SpecialFWI,
                        int iTimeout
                        );

#endif
