#define ENABLE_COMM
#include <Core.h>

#ifdef CORE_SUPPORT_SMARTCARD

#include <string.h>
#include <ClyTktOs.h>


#define TKT_ENC //use data decryption

#ifdef TKT_ENC
int DEBUG_PLAIN_DATA = 0;
#else
int DEBUG_PLAIN_DATA = 1;

#endif


// for work with 9 byte diversif in encrypt change this define to 1
#define ENCRYPT_BY_9_BYTE_DIVERS 1


/////////////////////////////////////////////
//////////////////////////defines
/////////////////////////////////////////////
///   sernumber r0,2,3,4
#define TISERIAL_NUMBER_WORD0   0xffff0000L ///byte0 r0
#define TISERIAL_NUMBER_WORD1   0xffff0000L ///byte4 r2
#define TISERIAL_NUMBER_WORD2_3 0xffffffffL ///byte6 r3,4

/// all r5-9
#define TC_KEY_INDEX       0xc0000000L   ///byte10 r5
#define TC_TARIFF          0x38000000L   ///byte10 r5
#define TC_PROVIDER        0x07f00000L   ///byte10 r5
#define TC_PROFILE         0x000f0000L   ///byte10 r5

////////////////////////MULTI TICKET////////////////
///Contract multi ticket r5-9
#define TMC_RELOAD_COUNT   0xf0000000L   ///byte12 r6
#define TMC_SALE_DATE      0x0ff80000L   ///byte12 r6
#define TMC_VALID_JOURN    0x0007c000L   ///byte12 r6
#define TMC_VALID_SPATIAL  0x00003fffL   ///byte12 r6
#define TMC_SIGNATURE      0xffffffffL   ///byte16 r8
///char recmap[9]={0,0,0,0,2,2,2,2,6};


///Validation multi ticket r10-13
#define TMF_SERVICE_PROVIDER    0xfe000000L///byte20 r10
#define TMF_DATE_STAMP          0x01fff800L///byte20 r10
#define TMF_TIME_STAMP          0x000007fcL///byte20 r10
#define TMF_LOCATION_STAMP      0x03ff0000L///byte23 r10+11
#define TMF_TOTAL_JOURNEUS      0x0000f800L///byte23 r
#define TMF_DIRECTION           0x00000400L///byte23 r
#define TMF_RFU                 0x00000300L///byte23 r
#define TMF_SIGNATURE           0xffff0000L///byte26 r

///location multi ticket r14-15
#define TML_FIRST_FLAG          0x80000000L///byte28
#define TML_LOCATION_ID         0x7fe00000L///byte28
#define TMLL_SERVICE_PROVIDER   0x001fc000L///byte28
#define TMLL_TIME_STAMP         0x00003fe0L///byte28
#define TMLL_CHECKSUM           0x0000001fL///byte28
///location backup multi ticket r14-15
#define TML_FIRST_FLAG_BCKUP        0x80000000L///byte28
#define TML_LOCATION_ID_BCKUP       0x7fe00000L///byte28
#define TMLL_JOURN_BCKUP            0x001f0000L///byte28
#define TMLL_SIGNUTE_BCKUP          0x0000ffffL///byte28

////////////////////////SEASON TICKET////////////////
///contract season ticket
#define TSC_RELOAD_COUNT        0xf0000000L /// byte12 r6
#define TSC_SLIDING             0x08000000L /// byte12 r
#define TSC_DATE                0x07ffe000L /// byte12 r
#define TSC_VALID_DURATION      0x00001fc0L /// byte12 r
#define TSC_VALID_SPATIAL       0x003fffffL /// byte14 r
#define TSC_SIGNATURE           0xffffffffL /// byte18 r

///Initial record season ticket
#define TSI_START           0xffe00000L ///byte22 r
#define TSI_RFU             0x001f0000L ///byte22 r
#define TSI_SIGNATURE       0xffff0000L ///byte24 r

///validation season ticket
#define TSL_VIRGIN_FLAG         0x80000000L ///byte26 r
#define TSLL_LOCATION_ID        0x7fe00000L ///byte26 r
#define TSLL_SERVICE_PROVIDER   0x001fc000L ///byte26 r
#define TSLL_TIME_STAMP         0x00003fe0L ///byte26 r
#define TSLL_DATE_OFFSET        0x001ff000L ///byte28 r
#define TSLL_RFU                0x00000f00L ///byte28 r
#define TSLL_CHECKSUM           0x000000ffL ///byte28 r
///validation backup season ticket
#define TSL_VIRGIN_FLAG_BCK     0x80000000L ///byte26 r
#define TSLL_RFU_BCK            0x7fff0000L ///byte26 r
#define TSLL_SIGN_BCK           0xffffffffL ///byte28 r


/// record(WORD) address by stucture of app
#define ISSUING_REC_ADDR                0
#define MULTI_CONTRACT_REC_ADDR         5
#define MULTI_VALIDATION_REC_ADDR       10
#define MULTI_LOCATION_REC_ADDR         14
#define SEASON_CONTRACT_REC_ADDR        5
#define SEASON_INITIAL_REC_ADDR         11
#define SEASON_VALIDATION_REC_ADDR      13


///   mask of season duration
#define CLYTKT_DURATION_MASK_TYPE 0x00000060L
#define CLYTKT_DURATION_MASK_VAL  0x0000001fL

///   masks of validity spatial of season
///   ASP-area season pass
#define VAL_SPAC_MASK_ASP_RSYS     0x003fc000L///0x000000ffL
#define VAL_SPAC_MASK_ASP_VALZONE  0x00003ffcL///0x000fff00L
///   P2P-point to point season pass
#define VAL_SPAC_MASK_P2P_RSYS     0x003fc000L///0x000000ffL
#define VAL_SPAC_MASK_P2P_ORIGIN   0x00003f80L///0x00007f00L
#define VAL_SPAC_MASK_P2P_DEST     0x0000007fL///0x003f8000L
/// not in use #define VAL_SPAC_MASK_PSP_CONTRACTID 

///   mask of spatial of multi ticket
///   fare code
#define MULTI_VAL_SPAC_MASK_FC_RSYS     0x00003fc0L///0x000000ffL
#define MULTI_VAL_SPAC_MASK_FC_FARCODE  0x0000003fL
///   poin to oint
#define MULTI_VAL_SPAC_MASK_P2P_ORIGN       0x00003f80L///0x0000007fL
#define MULTI_VAL_SPAC_MASK_P2P_DESTINATION 0x0000007fL///0x00003f80L

/////////////////////////////////////////////
/////////////////////////////local members
/////////////////////////////////////////////
typedef enum 
{
	eMulSign1,
	eMulSign2,
	eSeasSign1,
	eSeasSign2,
	eSeasSign3,
}SignList;
typedef struct 
{
	char mulsig1[14];
	char mulsig2[13];
	char seassig1[16];
	char seassig2[14];
	char seassig3[14];
	char s1_32m[4];
	char s2_16m[2];
	char s1_32[4];
	char s2_16[2];
	char s3_32[4];
}ContainSignes;
ContainSignes SaveSignes;
unsigned char *SaveSignesPtr=(unsigned char *)&SaveSignes;
SIGN_CALLBACK localSignProc=0;
TIME_DATE_CALLBACK localDateTimeProc=0;
DATE_PLUS_H_D_CALLBACK localHourDayProc=0;
static e_ClyTkt_ERR GetSignFromRAM(SignList eSign,clyTkt_BYTE* sigout,
	clyTkt_BYTE* SerNum,clyTkt_BYTE*signdata)
{
	e_ClyTkt_ERR err=e_ClyTkt_NO_ERROR;
	switch(eSign)
	{
	case eMulSign1:
		if(memcmp(SaveSignes.mulsig1,SerNum,8)==0 && memcmp(SaveSignes.mulsig1,signdata,14)==0)
		{
			memcpy(sigout,SaveSignes.s1_32m,4);
			return err;
		}
		err=localSignProc(signdata,14,ClyTkt_SignType_32,sigout);/// build sign
		if(! err)
		{
			memcpy(SaveSignes.mulsig1,signdata,14);
			memcpy(SaveSignes.s1_32m,sigout,4);
		}
		return err;
	case eMulSign2:
		if(memcmp(SaveSignes.mulsig2,SerNum,8)==0 && memcmp(SaveSignes.mulsig2,signdata,13)==0)
		{
			memcpy(sigout,SaveSignes.s2_16m,2);
			return err;
		}
		err=localSignProc(signdata,13,ClyTkt_SignType_16,sigout);/// build sign
		if(! err)
		{
			memcpy(SaveSignes.mulsig2,signdata,13);
			memcpy(SaveSignes.s2_16m,sigout,2);
		}
		return err;
	case eSeasSign1:
		if(memcmp(SaveSignes.seassig1,SerNum,8)==0 && memcmp(SaveSignes.seassig1,signdata,16)==0)
		{
			memcpy(sigout,SaveSignes.s1_32,4);
			return err;
		}
		err=localSignProc(signdata,16,ClyTkt_SignType_32,sigout);/// build sign
		if(! err)
		{
			memcpy(SaveSignes.seassig1,signdata,16);
			memcpy(SaveSignes.s1_32,sigout,4);
		}
		return err;
	case eSeasSign2:
		if(memcmp(SaveSignes.seassig2,SerNum,8)==0 && memcmp(SaveSignes.seassig2,signdata,14)==0)
		{
			memcpy(sigout,SaveSignes.s2_16,2);
			return err;
		}
		err=localSignProc(signdata,14,ClyTkt_SignType_16,sigout);/// build sign
		if(! err)
		{
			memcpy(SaveSignes.seassig2,signdata,14);
			memcpy(SaveSignes.s2_16,sigout,2);
		}
		return err;
	case eSeasSign3:
		if(memcmp(SaveSignes.seassig3,SerNum,8)==0 && memcmp(SaveSignes.seassig3,signdata,14)==0)
		{
			memcpy(sigout,SaveSignes.s3_32,4);
			return err;
		}
		err=localSignProc(signdata,14,ClyTkt_SignType_32,sigout);/// build sign
		if(! err)
		{           
			memcpy(SaveSignes.seassig3,signdata,13);
			memcpy(SaveSignes.s3_32,sigout,4);
		}
		return err;
	default:
		return e_ClyTkt_CHECK_SIGN_FAIL;
	}
}
///   address (in WORDS) of start in memory
clyTkt_BYTE RecordMemoryAdrr[e_ClyTkt_LastMediaType][e_ClyTkt_LastRecordType]=
{
	{
		ISSUING_REC_ADDR,MULTI_CONTRACT_REC_ADDR,MULTI_VALIDATION_REC_ADDR,
			MULTI_LOCATION_REC_ADDR,SEASON_CONTRACT_REC_ADDR,SEASON_INITIAL_REC_ADDR,
			SEASON_VALIDATION_REC_ADDR
	}
};


///   memory size (in WORDS) of every one structure
clyTkt_BYTE RecordMemorySize[e_ClyTkt_LastMediaType][e_ClyTkt_LastRecordType]=
{
	{
		5,/*issuing*/
			5,4,2,/*contract,validation,location of MULTI*/
			6,2,3/*contract,initial,validation of season*/
	}   
};

st_ClyTkt_KeyInfo localKeyArr[MAX_TIKET_KEYS];
clyTkt_BYTE localKeyLRC[MAX_TIKET_KEYS];
/////////////////////////////////////////////
/////////////////////////////local functions
/////////////////////////////////////////////

////////////////////////////////////////////
///   convert bits buffer to long by mask
////////////////////////////////////////////

void bit2long(unsigned long src,short nbits,unsigned long mask,unsigned char*res) //__thumb
{
	unsigned long tmp=0;
	memcpy(&tmp,res,4);
	tmp=(src&mask)>>(32-nbits);
	memcpy(res,&tmp,4);
}

///////////////////////////////////////////////
///   convert long to bits buffer by mask
///   whith saving previous value of converting
///////////////////////////////////////////////
void long2bit(unsigned long src,short nbits,unsigned long mask,unsigned long*res) //__thumb
{
	(*res)|=(src<<(32-nbits))&mask;
}

////////////////
///   reverse long
////////////////
static void localLrev(unsigned long *l)
{
	char c;
	char cc[4]={0};
	memcpy(cc,l,4);
	c=cc[0];
	cc[0]=cc[3];
	cc[3]=c;
	c=cc[1];
	cc[1]=cc[2];
	cc[2]=c;

	memcpy(l,cc,4);
}
////////////////
///   swap long
////////////////
static void localLSwap(unsigned long *l)
{
	char c;
	char cc[4]={0};
	memcpy(cc,l,4);
	c=cc[0];
	cc[0]=cc[2];
	cc[2]=c;
	c=cc[1];
	cc[1]=cc[3];
	cc[3]=c;
	memcpy(l,cc,4);
}

////////////////
///   get lrc
////////////////
clyTkt_BYTE GetLRC(clyTkt_BYTE * src,unsigned int len)
{
	clyTkt_BYTE lrc=0xff;
	unsigned int i;

	for(i=0;i<len;i++)
		lrc^=src[i];
	return lrc;
}

////////////////
///   encrypt
////////////////
#define kSG4KeyRounds (6*16) /* number of iteration for key diversification */
#define kSG4DtaRounds 12     /* number of iteration on each data byte */
static void SG4_enc(
	const   unsigned char inKey[16],
	const   unsigned char * inDiv,
	const   unsigned char   inDiversifierSize,
	unsigned char * ioData,
	const   unsigned char   inDataSize
	)

{
	unsigned char vDivKey[32]; /* diversified key */
	unsigned char a;
	unsigned char j;
	unsigned char p;
	unsigned char q;


	/* ---------------------------  */
	/* prepare the diversified key  */
	/* ---------------------------  */
	p = 16;

	/* spread inKey in vDivKey, in such a way that future interraction with inKey are with distant bytes */
	do
	{
		--p;
		vDivKey[p^2] = vDivKey[p+16] = inKey[p^9];
	}
	while (p);

	a = inDataSize;                                     /* extra safety: diversification changes with inDataSize */
	p = (q = inDiversifierSize) + kSG4KeyRounds;

	/* for each byte in inDiv, then kSG4KeyRounds extra steps */
	do
	{
		--p;
		vDivKey[p&31] += a ^= (((a>>1)+(q?inDiv[--q]:p))^inKey[p&15])+vDivKey[(p+17)&31];
	}
	while (p||q);


	/* ---------------------------  */
	/* encipher the data            */
	/* ---------------------------  */
	j = inDataSize >> 1;
	a = ioData[inDataSize-1];
	p = 0;

	do
	{
		q = 0;
		do
		{
			a = ioData[q] ^= (((a>>4)+ioData[j])^vDivKey[p&31])+a;
			if ((++j)==inDataSize)
				j = 0;

			++p;
		}
		while (++q!=inDataSize);                    /* inDataSize iterations */
		p -= q;
	}
	while ((p += 13)!=(unsigned char)(kSG4DtaRounds*13));   /* kSG4DtaRounds iterations */
}

static e_ClyTkt_ERR v_ClyTktEncrypt(clyTkt_BYTE * src
	,clyTkt_BYTE * trg,
	clyTkt_BYTE len,
	clyTkt_BYTE keyIndex,
	clyTkt_BYTE *Diver,
	clyTkt_BYTE Diverlen)
{
	if( DEBUG_PLAIN_DATA)
		memcpy(trg,src,len);
	else
	{

		unsigned char lrc=0xff;

		if(keyIndex>MAX_TIKET_KEYS || localKeyArr[keyIndex].b_IsKeyExist==0)
			return e_ClyTkt_KEY_NOT_EXIST;

		lrc=GetLRC(localKeyArr[keyIndex].ucp_Key,16);
		if(lrc!=localKeyLRC[keyIndex])
			return e_ClyTkt_KEY_LRC_FAIL;

		SG4_enc(localKeyArr[keyIndex].ucp_Key,Diver,Diverlen,trg,len);
	}
	return e_ClyTkt_NO_ERROR;
}

////////////////
///   decrypt
////////////////
static void SG4_dec
	(
	const   unsigned char   inKey[16],
	const   unsigned char * inDiv,
	const   unsigned char   inDiversifierSize,
	unsigned char * ioData,
	const   unsigned char   inDataSize
	)
{
	if ( DEBUG_PLAIN_DATA )
		return;
	//do nothing
	else
	{

		unsigned char vDivKey[32]; /* diversified key */
		unsigned char a;
		unsigned char j;
		unsigned char p;
		unsigned char q;

		/* ---------------------------  */
		/* prepare the diversified key */
		/* ---------------------------  */
		p = 16;

		/* spread inKey in vDivKey, in such a way that future interraction with inKey are with distant bytes */
		do
		{
			--p;
			vDivKey[p^2] = vDivKey[p+16] = inKey[p^9];
		}
		while (p);

		a = inDataSize;                                 /* extra safety: diversification changes with inDataSize */
		p = (q = inDiversifierSize) + kSG4KeyRounds;

		/* for each byte in inDiv, then kSG4KeyRounds extra steps */
		do
		{
			--p;
			vDivKey[p&31] += a ^= (((a>>1)+(q?inDiv[--q]:p))^inKey[p&15])+vDivKey[(p+17)&31];
		}
		while (p||q);


		/* ---------------------------  */
		/* decipher the data            */
		/* ---------------------------  */
		j = inDataSize>>1;
		p = (unsigned char)(kSG4DtaRounds*13);                  /* kSG4DtaRounds iterations */

		do
		{
			p += (q = inDataSize)-13;                   /* inDataSize iterations */
			do
			{
				if (j==0)
					j = inDataSize;

				if ((a = --q)==0)
					a = inDataSize;

				a = ioData[a-1];
				ioData[q] ^= (((a>>4)+ioData[--j])^vDivKey[(--p)&31])+a;
			}
			while (q);
		} while (p);
	}

}

static e_ClyTkt_ERR v_ClyTktDecrypt(clyTkt_BYTE * src,
	clyTkt_BYTE * trg,
	clyTkt_BYTE len,
	clyTkt_BYTE keyIndex,
	clyTkt_BYTE *Diver,
	clyTkt_BYTE Diverlen)
{

	if ( DEBUG_PLAIN_DATA )
		memcpy(trg,src,len);
	else
	{
		unsigned char lrc=0xff;

		if(keyIndex>MAX_TIKET_KEYS || localKeyArr[keyIndex].b_IsKeyExist==0)
			return e_ClyTkt_KEY_NOT_EXIST;

		lrc=GetLRC(localKeyArr[keyIndex].ucp_Key,16);
		if(lrc!=localKeyLRC[keyIndex])
			return e_ClyTkt_KEY_LRC_FAIL;

		SG4_dec(localKeyArr[keyIndex].ucp_Key,Diver,Diverlen,trg,len);
	}
	return e_ClyTkt_NO_ERROR;
}

/*
//////////////////////////////////////////////////////////////////////////////
API   FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
*/

e_ClyTkt_ERR  CLYTKT_STDCALL v_ClyTkt_InitInterface(st_ClyTkt_KeyInfo st_KeyInfoArr[MAX_TIKET_KEYS],
	SIGN_CALLBACK sprocIN,
	TIME_DATE_CALLBACK dtprocIN,
	DATE_PLUS_H_D_CALLBACK hprocIN)
{
	clyTkt_BYTE i;

	for(i=0;i<MAX_TIKET_KEYS;i++)
	{
		memcpy(&localKeyArr[i],&st_KeyInfoArr[i],sizeof(st_ClyTkt_KeyInfo));
		if(localKeyArr[i].b_IsKeyExist==ClyTkt_TRUE)
			localKeyLRC[i]=GetLRC(localKeyArr[i].ucp_Key,16);
	}
	if(sprocIN)
		localSignProc=sprocIN;
	else
		return e_ClyTkt_SIGN_CALLBACK_NULL;

	if(dtprocIN)
		localDateTimeProc=dtprocIN;
	else
		return e_ClyTkt_DATE_TIME_CALLBACK_NULL;

	if(hprocIN)
		localHourDayProc=hprocIN;
	else
		return e_ClyTkt_DATE_PLUS_H_D_CALLBACK_NULL;

	memset(&SaveSignes,0,sizeof(SaveSignes));
	return e_ClyTkt_NO_ERROR;
}


/***************************************/
// Get Ticket record Address 
/***************************************/
// Translate Ticket Record type to physical Address
e_ClyTkt_ERR  CLYTKT_STDCALL e_ClyTkt_GetTktRecAddress( e_ClyTkt_TicketMediaTypes e_TicketMediaTypes,//[IN] Ticket Media Types
	e_ClyTkt_TicketRecordType e_TicketRecordType,//[IN] Record Name
	clyTkt_BYTE *ucp_WordStartAddInCard,//[OUT] Record start physicl address
	clyTkt_BYTE *ucp_WordEndAddInCard,//[OUT] Record end physicl address
	clyTkt_BYTE *ucp_WordStartOffsetInBuff,//[OUT] the record offset in the translated buffer
	clyTkt_BYTE *ucp_WordRecLenInBuff)//[OUT] the len of the record in the translated buffer
{

	if(e_TicketMediaTypes>=e_ClyTkt_LastMediaType)
		return e_ClyTkt_MEDIA_INCORRECT;
	if(e_TicketRecordType>=e_ClyTkt_LastRecordType)
		return e_ClyTkt_RECORD_TYPE_INCORECT;

	///   start address in buffer & in card
	(*ucp_WordStartOffsetInBuff)=(*ucp_WordStartAddInCard)=RecordMemoryAdrr[e_TicketMediaTypes][e_TicketRecordType];
	///   end address in card
	(*ucp_WordEndAddInCard)=RecordMemoryAdrr[e_TicketMediaTypes][e_TicketRecordType]+RecordMemorySize[e_TicketMediaTypes][e_TicketRecordType];
	///   lenght in buff
	(*ucp_WordRecLenInBuff)=RecordMemorySize[e_TicketMediaTypes][e_TicketRecordType];

	return e_ClyTkt_NO_ERROR;
}


/***************************************/
// Convert ticket struct to binary Buff 
/***************************************/
// Convert Ticket struct to Binary buffer 
e_ClyTkt_ERR  CLYTKT_STDCALL e_ClyTkt_ConvertTktSt2BinBuff(struct_ClyTkt_Ticket *struct_Ticket,//[IN] Ticket struct input for translation
	CalypsoBinTktType ucp_BinBuffOut)//[OUT] Binary buff result
{
	unsigned long ltmp=0;
	unsigned char trg[32]={0};
	e_ClyTkt_ERR err=e_ClyTkt_NO_ERROR;
	//  unsigned long l=0;
	unsigned long res=0;
	char * ch=(char*)&res;
	unsigned long* lsrc=0;
	char season=0;/// multi
	clyTkt_BYTE signarr[32]={0};
	clyTkt_BYTE tmpsign[4]={0};
	clyTkt_BYTE Divers[9]={0};
	//  unsigned char * bout=ucp_BinBuffOut;
	unsigned short datetime=0;

	memset(ucp_BinBuffOut,0,sizeof(CalypsoBinTktType));

	//////////////////   convert serial number
	///   write 16 bits to byte 0
	lsrc=(unsigned long*)struct_Ticket->ucp_Sn;
	long2bit(*lsrc,16,TISERIAL_NUMBER_WORD0,&res);
	localLSwap(&res);
	memcpy((unsigned char *)(ucp_BinBuffOut),ch,4);
	///   write 16 bits to byte 4
	memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+4,4);
	localLSwap(&res);
	lsrc=(unsigned long*)((unsigned char*)(struct_Ticket->ucp_Sn)+2);
	long2bit(*lsrc,16,TISERIAL_NUMBER_WORD1,&res);
	localLSwap(&res);
	memcpy((unsigned char *)(ucp_BinBuffOut)+4,ch,4);
	///   write 32 bits to byte 6
	memcpy(&res,(unsigned char*)(ucp_BinBuffOut)+6,4);
	localLSwap(&res);
	lsrc=(unsigned long*)((unsigned char*)(struct_Ticket->ucp_Sn)+4);
	long2bit(*lsrc,32,TISERIAL_NUMBER_WORD2_3,&res);
	/// localLSwap(&res);
	memcpy((unsigned char*)(ucp_BinBuffOut)+6,ch,4);

	//////////////////   convert contract united
	///   write 2 bits to byte 10
	memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+10,4);
	localLrev(&res);///localLSwap(&res);
	lsrc=(unsigned long*)&struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex;
	long2bit(*lsrc,2,TC_KEY_INDEX,&res);
	///   write 3 bits to byte 10
	lsrc=(unsigned long*)&struct_Ticket->st_TicketContractCommonData.st_Tariff;
	long2bit(*lsrc,5,TC_TARIFF,&res);
	if(struct_Ticket->st_TicketContractCommonData.st_Tariff==4 || 
		struct_Ticket->st_TicketContractCommonData.st_Tariff==5 || 
		struct_Ticket->st_TicketContractCommonData.st_Tariff==6)
		season=1;///   season app on the ticket
	///   write 7 bits to byte 10
	lsrc=(unsigned long*)&struct_Ticket->st_TicketContractCommonData.uc_TC_Provider;
	long2bit(*lsrc,12,TC_PROVIDER,&res);
	///   write 4 bits to byte 10
	lsrc=(unsigned long*)&struct_Ticket->st_TicketContractCommonData.uc_TC_Profile;
	long2bit(*lsrc,16,TC_PROFILE,&res);
	localLrev(&res);///localLSwap(&res);
	memcpy((unsigned char *)(ucp_BinBuffOut)+10/*&bout[10]*/,ch,4);
	///   write 4 bits to byte 12
	memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+12,4);
	localLrev(&res);///localLSwap(&res);
	lsrc=(unsigned long*)&struct_Ticket->st_TicketContractCommonData.uc_ReloadCount;
	long2bit(*lsrc,4,TMC_RELOAD_COUNT,&res);
	localLrev(&res);///localLSwap(&res);
	memcpy((unsigned char *)(ucp_BinBuffOut)+12,ch,4);

	///////////////////////   convert all data
	if(season)
	{
		//////////////////   convert contract continue
		///   write 1 bits to byte 12
		memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+12,4);
		localLrev(&res);///localLSwap(&res);
		lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.e_TSC_Sliding;
		long2bit(*lsrc,5,TSC_SLIDING,&res);
		///   write 14 bits to byte 12
		localDateTimeProc(&datetime,0,0,&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date,e_ClyTkt_TktDTConv_CompactDate2Bit);
		lsrc=(unsigned long*)&datetime;///hren
		///     lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date;
		long2bit(*lsrc,19,TSC_DATE,&res);
		///   write 7 bits to byte 12
		ltmp=struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.uc_DurationValue&CLYTKT_DURATION_MASK_VAL;
		ltmp|=(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType<<5)&CLYTKT_DURATION_MASK_TYPE;
		lsrc=(unsigned long*)&ltmp;///hren struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration;
		long2bit(*lsrc,26,TSC_VALID_DURATION,&res);
		localLrev(&res);///localLSwap(&res);
		memcpy((unsigned char *)(ucp_BinBuffOut)+12,ch,4);

		///   write 22 bits to byte 14
		memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+14,4);
		localLrev(&res);///localLSwap(&res);
		if(struct_Ticket->st_TicketContractCommonData.st_Tariff==4)
		{
			ltmp=(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialAreaSP.uc_TSC_RoutesSystem<<14)&VAL_SPAC_MASK_ASP_RSYS;
			ltmp|=(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialAreaSP.ush_TSC_ValidityZones<<2)&VAL_SPAC_MASK_ASP_VALZONE;
			///Vaf          ltmp=struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialAreaSP.uc_TSC_RoutesSystem&VAL_SPAC_MASK_ASP_RSYS;
			///Vaf          ltmp|=(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialAreaSP.ush_TSC_ValidityZones<<8)&VAL_SPAC_MASK_ASP_VALZONE;
		}
		else 
			if(struct_Ticket->st_TicketContractCommonData.st_Tariff==5)
			{
				ltmp=(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialPoint2PointSP.uc_TSC_RoutesSystem<<14)&VAL_SPAC_MASK_P2P_RSYS;
				ltmp|=(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialPoint2PointSP.uc_TSC_Origin<<7)&VAL_SPAC_MASK_P2P_ORIGIN;
				ltmp|=struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialPoint2PointSP.uc_TSC_Destination&VAL_SPAC_MASK_P2P_DEST;
			}
			else///   must be 6
			{
				ltmp=struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialPredefinedSP.l_TSC_ContractTypelD;///&VAL_SPAC_MASK_PSP_CONTRACTID;
			}

			///     ltmp=struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialPredefinedSP.l_TSC_ContractTypelD;///&VAL_SPAC_MASK_PSP_CONTRACTID;
			lsrc=(unsigned long*)&ltmp;///struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial;
			long2bit(*lsrc,32,TSC_VALID_SPATIAL,&res);
			localLrev(&res);///localLSwap(&res);
			memcpy((unsigned char *)(ucp_BinBuffOut)+14,ch,4);

			//////////////////   get first sign
			memcpy(signarr,struct_Ticket->ucp_Sn,8);/// build buffer
			memcpy(signarr+8,(unsigned char *)(ucp_BinBuffOut)+10,8);/// build buffer
			err=GetSignFromRAM(eSeasSign1,tmpsign,struct_Ticket->ucp_Sn,signarr);
			///         err=localSignProc(signarr,16,ClyTkt_SignType_32,tmpsign);/// build sign
			if(err!=e_ClyTkt_NO_ERROR)
				return err;
			lsrc=(unsigned long*)tmpsign;
			memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+18,4);
			localLrev(&res);///localLSwap(&res);
			localLrev(lsrc);///localLSwap(lsrc);
			long2bit(*lsrc,32,TSC_SIGNATURE,&res);
			localLrev(&res);///localLSwap(&res);
			memcpy((unsigned char *)(ucp_BinBuffOut)+18,ch,4);

			//////////////////   convert initial
			///   write 11 bits to byte 22
			memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+22,4);
			localLrev(&res);///localLSwap(&res);
			if(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType==e_ClyTkt_DurationInHours)
			{
				localHourDayProc(&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start,&datetime,
					&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date,e_ClyTkt_TktDTConv_stDate2HourInWORD);
				///localDateTimeProc(&datetime,0,&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.union_Start.stTime,0,e_ClyTkt_TktDTConv_Bit2CompactTime);
			}
			else 
				if(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.e_TSC_Sliding==0)
					memset(&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start,0,sizeof(st_Cly_DateAndTime));
				else
					localHourDayProc(&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start,&datetime,
					&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date,e_ClyTkt_TktDTConv_stDate2DayInWORD);
			lsrc=(unsigned long*)&datetime;///hren struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.union_Start.ush_TSI_Start;///???
			long2bit(*lsrc,11,TSI_START,&res);
			localLrev(&res);///localLSwap(&res);
			memcpy((unsigned char *)(ucp_BinBuffOut)+22,ch,4);

			//////////////////   get second sign
			memcpy(signarr,struct_Ticket->ucp_Sn,8);/// build buffer
			memcpy(signarr+8,(unsigned char *)(ucp_BinBuffOut)+18,6);
			err=GetSignFromRAM(eSeasSign2,tmpsign,struct_Ticket->ucp_Sn,signarr);
			///     err=localSignProc(signarr,14,ClyTkt_SignType_16,tmpsign);
			if(err!=e_ClyTkt_NO_ERROR)
				return err;
			lsrc=(unsigned long*)tmpsign;
			memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+24,4);
			localLrev(&res);///localLSwap(&res);
			///     localLrev(lsrc);///localLSwap(lsrc);
			localLSwap(lsrc);///beseder
			localLrev(lsrc);

			long2bit(*lsrc,16,TSI_SIGNATURE,&res);
			localLrev(&res);///localLSwap(&res);
			memcpy((unsigned char *)(ucp_BinBuffOut)+24,ch,2);

			//////////////////   convert validation
			///   write 1 bits to byte 26
			memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+26,4);
			localLrev(&res);///localLSwap(&res);
			lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.TSL_IsVirginFlag;
			long2bit(*lsrc,1,TSL_VIRGIN_FLAG,&res);
			localLrev(&res);///localLSwap(&res);
			memcpy((unsigned char *)(ucp_BinBuffOut)+26,ch,2);
			if(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.TSL_IsVirginFlag)
			{
				///   ticket is not yet used once
				memcpy(signarr,struct_Ticket->ucp_Sn,8);
				memcpy(signarr+8,(unsigned char *)(ucp_BinBuffOut)+18,4);
				signarr[12]=0x80;///flag 1 , rfu(15bits)0
				signarr[13]=0;///flag 1 , rfu(15bits)0 ///sabla
				err=GetSignFromRAM(eSeasSign3,tmpsign,struct_Ticket->ucp_Sn,signarr);
				///         err=localSignProc(signarr,13,ClyTkt_SignType_32,tmpsign);
				if(err!=e_ClyTkt_NO_ERROR)
					return err;
				lsrc=(unsigned long*)tmpsign;
				memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+28,4);
				localLrev(&res);///localLSwap(&res);
				localLrev(lsrc);///localLSwap(lsrc);
				long2bit(*lsrc,32,TSLL_SIGN_BCK,&res);
				localLrev(&res);///localLSwap(&res);
				memcpy((unsigned char *)(ucp_BinBuffOut)+28,ch,4);
			}
			else
			{
				///   ticket has been used
				///   write 10 bits to byte 26
				lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.ush_TSLL_Locationld;
				long2bit(*lsrc,11,TSLL_LOCATION_ID,&res);
				///   write 7 bits to byte 26
				lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.uc_TSLL_ServiceProvider;
				long2bit(*lsrc,18,TSLL_SERVICE_PROVIDER,&res);
				///   write 9 bits to byte 26
				localDateTimeProc(&datetime,0,&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.st_TSLL_TimeStamp,0,e_ClyTkt_TktDTConv_CompactTime2Bit);
				lsrc=(unsigned long*)&datetime;///hren
				///         lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.st_TSLL_TimeStamp;
				long2bit(*lsrc,27,TSLL_TIME_STAMP,&res);
				localLrev(&res);///localLSwap(&res);
				memcpy((unsigned char *)(ucp_BinBuffOut)+26,ch,4);
				///   write 9 bits to byte 28
				memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+28,4);
				localLrev(&res);///localLSwap(&res);
				lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.ush_TSSL_DateOffset;
				long2bit(*lsrc,20,TSLL_DATE_OFFSET,&res);
				localLrev(&res);///localLSwap(&res);
				memcpy((unsigned char *)(ucp_BinBuffOut)+28,ch,4);
			}

			//////////////////   encrypt
			memcpy(trg,(unsigned char *)(ucp_BinBuffOut),32);
			memcpy(Divers+ENCRYPT_BY_9_BYTE_DIVERS,struct_Ticket->ucp_Sn,8);
#if ENCRYPT_BY_9_BYTE_DIVERS==1
			Divers[0]=1;
#endif
			///   contract block len is 6
			err=v_ClyTktEncrypt(trg+12,(clyTkt_BYTE *)(unsigned char *)(ucp_BinBuffOut)+12,
				6,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
				Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
			///   initial block len is 2
#if ENCRYPT_BY_9_BYTE_DIVERS==1
			Divers[0]=4;
#endif
			err|=v_ClyTktEncrypt(trg+22,(clyTkt_BYTE *)(unsigned char *)(ucp_BinBuffOut)+22,
				2,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
				Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
			///   validation block len 6
#if ENCRYPT_BY_9_BYTE_DIVERS==1
			Divers[0]=5;
#endif
			err|=v_ClyTktEncrypt(trg+26,(clyTkt_BYTE *)(unsigned char *)(ucp_BinBuffOut)+26,
				6,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
				Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
			if(err!=e_ClyTkt_NO_ERROR)
				return e_ClyTkt_ENCRYPT_FAIL;
	}///   if(season)
	else///if(season)==multi
	{
		//////////////////   convert contract continue
		///   write 9 bits to byte 12
		memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+12,4);
		localLrev(&res);///localLSwap(&res);
		localDateTimeProc(&datetime,&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.st_TMC_SaleDate,0,0,e_ClyTkt_TktDTConv_ShortDate2Bit);
		lsrc=(unsigned long*)&datetime;///hren
		///     lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.st_TMC_SaleDate;
		long2bit(*lsrc,13,TMC_SALE_DATE,&res);
		///   write 5 bits to byte 12
		lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.uc_TMC_ValidityJourneys;
		long2bit(*lsrc,18,TMC_VALID_JOURN,&res);

		///   write 14 bits to byte 12
		if(struct_Ticket->st_TicketContractCommonData.st_Tariff==0)
		{

			ltmp=struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialFareCode.uc_TCM_FareCode&MULTI_VAL_SPAC_MASK_FC_FARCODE;
			ltmp|=(struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialFareCode.uc_TMC_RoutesSystem<<6)&MULTI_VAL_SPAC_MASK_FC_RSYS;
			///Vaf          ltmp=struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialFareCode.uc_TMC_RoutesSystem&MULTI_VAL_SPAC_MASK_FC_RSYS;
			///Vaf          ltmp|=(struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialFareCode.uc_TCM_FareCode<<8)&MULTI_VAL_SPAC_MASK_FC_FARCODE;
		}
		else 
			if(struct_Ticket->st_TicketContractCommonData.st_Tariff==1)
			{
				ltmp=struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialPoint2Point.uc_TCM_Destination&MULTI_VAL_SPAC_MASK_P2P_DESTINATION;
				ltmp|=(struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialPoint2Point.uc_TMC_Origin<<7)&MULTI_VAL_SPAC_MASK_P2P_ORIGN;
				///Vaf          ltmp=struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialPoint2Point.uc_TMC_Origin&MULTI_VAL_SPAC_MASK_P2P_ORIGN;
				///Vaf          ltmp|=(struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialPoint2Point.uc_TCM_Destination<<7)&MULTI_VAL_SPAC_MASK_P2P_DESTINATION;
			}
			else///   must be 2
			{
				ltmp=struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialPredefined.ush_ContractTypelD;
			}

			///     ltmp=struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialPredefined.ush_ContractTypelD;
			lsrc=(unsigned long*)&ltmp;///struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial;
			long2bit(*lsrc,32,TMC_VALID_SPATIAL,&res);
			localLrev(&res);///localLSwap(&res);
			memcpy((unsigned char *)(ucp_BinBuffOut)+12,ch,4);

			//////////////////   get first sign
			memcpy(signarr,struct_Ticket->ucp_Sn,8);/// build buffer
			memcpy(signarr+8,(unsigned char *)(ucp_BinBuffOut)+10,6);/// build buffer
			err=GetSignFromRAM(eMulSign1,tmpsign,struct_Ticket->ucp_Sn,signarr);
			///     err=localSignProc(signarr,14,ClyTkt_SignType_32,tmpsign);/// build sign
			if(err!=e_ClyTkt_NO_ERROR)
				return err;
			lsrc=(unsigned long*)tmpsign;
			memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+16,4);
			localLrev(&res);///localLSwap(&res);
			localLrev(lsrc);///localLSwap(lsrc);
			long2bit(*lsrc,32,TMC_SIGNATURE,&res);
			localLrev(&res);///localLSwap(&res);
			memcpy((unsigned char *)(ucp_BinBuffOut)+16,ch,4);

			//////////////////   convert validation
			///   write 7 bits to byte 20
			memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+20,4);
			localLrev(&res);///localLSwap(&res);
			lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_ServiceProvider;
			long2bit(*lsrc,7,TMF_SERVICE_PROVIDER,&res);
			///   write 14 bits to byte 20
			localDateTimeProc(&datetime,0,0,&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_DateStamp,e_ClyTkt_TktDTConv_CompactDate2Bit);
			lsrc=(unsigned long*)&datetime;///hren
			///lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_DateStamp;
			long2bit(*lsrc,21,TMF_DATE_STAMP,&res);
			///   write 9 bits to byte 20
			localDateTimeProc(&datetime,0,&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_TimeStamp,0,e_ClyTkt_TktDTConv_CompactTime2Bit);
			lsrc=(unsigned long*)&datetime;///hren
			///     lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_TimeStamp;
			long2bit(*lsrc,30,TMF_TIME_STAMP,&res);
			localLrev(&res);///localLSwap(&res);
			memcpy((unsigned char *)(ucp_BinBuffOut)+20,ch,4);
			///   write 10 bits to byte 23
			memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+23,4);
			localLrev(&res);///localLSwap(&res);
			lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.ush_TMF_LocationStamp;
			long2bit(*lsrc,16,TMF_LOCATION_STAMP,&res);
			///   write 5 bits to byte 23
			lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys;
			long2bit(*lsrc,21,TMF_TOTAL_JOURNEUS,&res);
			///   write 1 bits to byte 23
			lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.e_TMF_Direction;
			long2bit(*lsrc,22,TMF_DIRECTION,&res);
			localLrev(&res);///localLSwap(&res);
			memcpy((unsigned char *)(ucp_BinBuffOut)+23,ch,4);

			//////////////////   get second sign
			memcpy(signarr,struct_Ticket->ucp_Sn,8);/// build buffer
			memcpy(signarr+8,(unsigned char *)(ucp_BinBuffOut)+16,4);
			signarr[12]=struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys;
			err=GetSignFromRAM(eMulSign2,tmpsign,struct_Ticket->ucp_Sn,signarr);
			///     err=localSignProc(signarr,13,ClyTkt_SignType_16,tmpsign);
			if(err!=e_ClyTkt_NO_ERROR)
				return err;
			lsrc=(unsigned long*)tmpsign;
			memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+26,4);
			localLrev(&res);///localLSwap(&res);
			localLSwap(lsrc);///beseder
			localLrev(lsrc);
			long2bit(*lsrc,16,TMF_SIGNATURE,&res);
			localLrev(&res);///localLSwap(&res);
			memcpy((unsigned char *)(ucp_BinBuffOut)+26,ch,2);

			///////////////   location
			///   write 1 bits to byte 28
			memcpy(&res,(unsigned char *)(ucp_BinBuffOut)+28,4);
			localLrev(&res);///localLSwap(&res);
			lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.uc_FirstFlag;
			long2bit(*lsrc,1,TML_FIRST_FLAG,&res);
			///   write 10 bits to byte 28
			lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.ush_TML_LocationId;
			long2bit(*lsrc,11,TML_LOCATION_ID,&res);
			if(struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.uc_FirstFlag)
			{
				///   write 5 bits to byte 28
				lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.uc_JourneysBck;
				long2bit(*lsrc,16,TMLL_JOURN_BCKUP,&res);
				///   write 16 bits to byte 28
				lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.ush_SignatureBkp;
				long2bit(*lsrc,32,TMLL_SIGNUTE_BCKUP,&res);
			}
			else
			{
				///   write 7 bits to byte 28
				lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRec.uc_TMLL_ServiceProvider;
				long2bit(*lsrc,18,TMLL_SERVICE_PROVIDER,&res);
				///   write 9 bits to byte 28  
				localDateTimeProc(&datetime,0,&struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRec.st_TimeStamp,0,e_ClyTkt_TktDTConv_CompactTime2Bit);
				lsrc=(unsigned long*)&datetime;///hren
				///lsrc=(unsigned long*)&struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRec.st_TimeStamp;
				long2bit(*lsrc,27,TMLL_TIME_STAMP,&res);
			}
			localLrev(&res);///localLSwap(&res);
			memcpy((unsigned char *)(ucp_BinBuffOut)+28,ch,4);


			//////////////////   encrypt
			memcpy(trg,(unsigned char *)(ucp_BinBuffOut),32);
			memcpy(Divers+ENCRYPT_BY_9_BYTE_DIVERS,struct_Ticket->ucp_Sn,8);
#if ENCRYPT_BY_9_BYTE_DIVERS==1
			Divers[0]=1;
#endif
			///   contract block len is 4
			err=v_ClyTktEncrypt(trg+12,(clyTkt_BYTE *)(unsigned char *)(ucp_BinBuffOut)+12,
				4,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
				Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
			///   validation block len is 6
#if ENCRYPT_BY_9_BYTE_DIVERS==1
			Divers[0]=2;
#endif
			err|=v_ClyTktEncrypt(trg+20,(clyTkt_BYTE *)(unsigned char *)(ucp_BinBuffOut)+20,
				6,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
				Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
			///   location block len is 4
#if ENCRYPT_BY_9_BYTE_DIVERS==1
			Divers[0]=3;
#endif
			err|=v_ClyTktEncrypt(trg+28,(clyTkt_BYTE *)(unsigned char *)(ucp_BinBuffOut)+28,
				4,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
				Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
			if(err!=e_ClyTkt_NO_ERROR)
				return e_ClyTkt_ENCRYPT_FAIL;
	}/// else of if(season)
	return err;
}
/***************************************/
// Convert binary Buff to ticket Struct 
/***************************************/
// Translate Ticket struct Binary buffer
e_ClyTkt_ERR  CLYTKT_STDCALL e_ClyTkt_ConvertBinBuff2TktSt(
	struct_ClyTkt_Ticket *struct_Ticket,//[OUT] Ticket struct output 
	CalypsoBinTktType ucp_BinBuffIn,//[IN] Binary buff to translate
	CalypsoBinTktType ucp_BinBuffOut)//[OUT] Binary buff plantex result
{
	unsigned short datetime=0;
	e_ClyTkt_ERR err=e_ClyTkt_NO_ERROR;
	/// clyTkt_BYTE trg[32]={0};
	clyTkt_BYTE *trg=ucp_BinBuffOut;
	clyTkt_BYTE signarr[32]={0};
	clyTkt_BYTE tmpsign[4]={0};
	clyTkt_BYTE Divers[9]={0};
	/// clyTkt_BYTE keyIndex;
	unsigned long l=0;
	unsigned long res=0;
	unsigned long resbak=0;
	clyTkt_BYTE season=0;
	int multisign2;
	//debug
	unsigned char* ptr = (unsigned char*)&struct_Ticket->ucp_Sn;
	//  struct_ClyTkt_Ticket *struct_Ticket_debug = struct_Ticket;

	//debug end

	if(localSignProc==0)
		return e_ClyTkt_NO_SIGN_CALBACK;

	///  get working copy of buffer
	memcpy(trg,ucp_BinBuffIn,32);
	/////////////////////get serial number
	///   read 16 bits from byte 0 
	memcpy(&l,ucp_BinBuffIn,4);
	localLSwap(&l);
	bit2long(l,16,TISERIAL_NUMBER_WORD0,ptr);
	///   read 16 bits from byte 4 
	memcpy(&l,ucp_BinBuffIn+4,4);
	localLSwap(&l);
	bit2long(l,16,TISERIAL_NUMBER_WORD1,(ptr+2));
	///   read 32 bits from byte 6 
	memcpy(&l,ucp_BinBuffIn+6,4);
	/// localLSwap(&l);
	bit2long(l,32,TISERIAL_NUMBER_WORD2_3,(ptr+4));


	////////////////// read the united & plain data
	/////////////////////get key ondex
	///   read 2 bits from byte 10 
	memcpy(&l,ucp_BinBuffIn+10,4);
	localLrev(&l);///localLSwap(&l);
	bit2long(l,2,TC_KEY_INDEX,(unsigned char*)&res);
	struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex=(unsigned char)res;
	/////////////////////get tariff - app type
	///   read 3 bits from byte 10 
	bit2long(l,5,TC_TARIFF,(unsigned char*)&res);
	struct_Ticket->st_TicketContractCommonData.st_Tariff = (e_ClyTkt_TicketTariffAppType)res;
	if(res==4 || res==5 || res==6)
		season=1;///   season app on the ticket
	/////////////////////get provider
	///   read 7 bits from byte 10 
	bit2long(l,12,TC_PROVIDER,(unsigned char*)&res);
	struct_Ticket->st_TicketContractCommonData.uc_TC_Provider=(unsigned char)res;
	/////////////////////get profile
	///   read 4 bits from byte 10
	bit2long(l,16,TC_PROFILE,(unsigned char*)&res);
	struct_Ticket->st_TicketContractCommonData.uc_TC_Profile=(unsigned char)res;


	/////////////////////decrypt data
	if(season)
	{
		memcpy(Divers+ENCRYPT_BY_9_BYTE_DIVERS,struct_Ticket->ucp_Sn,8);
		///   contract block len is 6
#if ENCRYPT_BY_9_BYTE_DIVERS==1
		Divers[0]=1;
#endif
		err=v_ClyTktDecrypt((clyTkt_BYTE *)ucp_BinBuffIn+12,trg+12,
			6,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
			Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
		///   initial block len is 2
#if ENCRYPT_BY_9_BYTE_DIVERS==1
		Divers[0]=4;
#endif
		err|=v_ClyTktDecrypt((clyTkt_BYTE *)ucp_BinBuffIn+22,trg+22,
			2,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
			Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
		///   validation block len 6
#if ENCRYPT_BY_9_BYTE_DIVERS==1
		Divers[0]=5;
#endif
		err|=v_ClyTktDecrypt((clyTkt_BYTE *)ucp_BinBuffIn+26,trg+26,
			6,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
			Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
	}
	else
	{
		memcpy(Divers+ENCRYPT_BY_9_BYTE_DIVERS,struct_Ticket->ucp_Sn,8);
#if ENCRYPT_BY_9_BYTE_DIVERS==1
		Divers[0]=1;
#endif
		///   contract block len is 4
		err=v_ClyTktDecrypt((clyTkt_BYTE *)ucp_BinBuffIn+12,trg+12,
			4,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
			Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
		///   validation block len is 6
#if ENCRYPT_BY_9_BYTE_DIVERS==1
		Divers[0]=2;
#endif
		err|=v_ClyTktDecrypt((clyTkt_BYTE *)ucp_BinBuffIn+20,trg+20,
			6,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
			Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
		///   location block len is 4
#if ENCRYPT_BY_9_BYTE_DIVERS==1
		Divers[0]=3;
#endif
		err|=v_ClyTktDecrypt((clyTkt_BYTE *)ucp_BinBuffIn+28,trg+28,
			4,struct_Ticket->st_TicketContractCommonData.uc_TC_KeyIndex,
			Divers,8+ENCRYPT_BY_9_BYTE_DIVERS);
	}
	if(err!=e_ClyTkt_NO_ERROR)
		return e_ClyTkt_DECRYPT_FAIL;

	/////////////////////check first sign
	memcpy(signarr,struct_Ticket->ucp_Sn,8);
	if(season)
	{
		///   first sign
		memcpy(signarr+8,trg+10,8);
		err=GetSignFromRAM(eSeasSign1,tmpsign,struct_Ticket->ucp_Sn,signarr);
		///     err=localSignProc(signarr,16,ClyTkt_SignType_32,tmpsign);
		res=memcmp(tmpsign,trg+18,4);
		if(res!=0)
			return e_ClyTkt_CHECK_SIGN_FAIL;
		///   second sign
		memcpy(signarr+8,trg+18,6);
		err|=GetSignFromRAM(eSeasSign2,tmpsign,struct_Ticket->ucp_Sn,signarr);
		///     err|=localSignProc(signarr,14,ClyTkt_SignType_16,tmpsign);
		multisign2=memcmp(tmpsign,trg+24,2);
	}
	else
	{
		///   first sign
		memcpy(signarr+8,trg+10,6);
		err=GetSignFromRAM(eMulSign1,tmpsign,struct_Ticket->ucp_Sn,signarr);
		///     err=localSignProc(signarr,14,ClyTkt_SignType_32,tmpsign);
		res=memcmp(tmpsign,trg+16,4);
		if(res!=0)
			return e_ClyTkt_CHECK_SIGN_FAIL;
		///   second sign
		memcpy(signarr+8,trg+16,4);
		///   read 5 bits from byte 23 
		memcpy(&l,trg+23,4);
		///     localLSwap(&l);
		localLrev(&l);
		bit2long(l,21,TMF_TOTAL_JOURNEUS,(unsigned char*)&res);
		struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_TotalJourneys=(unsigned char)res;
		signarr[12]=(unsigned char)res;
		err|=GetSignFromRAM(eMulSign2,tmpsign,struct_Ticket->ucp_Sn,signarr);
		///     err|=localSignProc(signarr,13,ClyTkt_SignType_16,tmpsign);
		multisign2=memcmp(tmpsign,trg+26,2);
	}
	if(err!=e_ClyTkt_NO_ERROR)
		return err;
	/// if(res!=0)
	///     return e_ClyTkt_CHECK_SIGN_FAIL;

	/////////////////////convert data
	if(season)
	{
		///////////////contract
		///   read 4 bits from byte 12 
		memcpy(&l,trg+12,4);
		localLrev(&l);///localLSwap(&l);
		bit2long(l,4,TSC_RELOAD_COUNT,(unsigned char*)&res);
		struct_Ticket->st_TicketContractCommonData.uc_ReloadCount=(unsigned char)res;
		///   read 1 bits from byte 12 
		bit2long(l,5,TSC_SLIDING,(unsigned char*)&res);
		struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.e_TSC_Sliding = (e_ClyTkt_ValidityStartsType)res;
		///   read 14 bits from byte 12 
		bit2long(l,19,TSC_DATE,(unsigned char*)&res);
		datetime=(unsigned short)res;///hren
		localDateTimeProc(&datetime,0,0,&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date,e_ClyTkt_TktDTConv_Bit2CompactDate);
		///     struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date=res;

		///   read 7 bits from byte 12 
		bit2long(l,26,TSC_VALID_DURATION,(unsigned char*)&res);///hren
		struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType=(e_ClyTkt_TicketDurationType)((res&CLYTKT_DURATION_MASK_TYPE)>>5);
		struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.uc_DurationValue=(unsigned char)(res&CLYTKT_DURATION_MASK_VAL);

		///   read 22 bits from byte 14 
		memcpy(&l,trg+14,4);
		localLrev(&l);///localLSwap(&l);
		bit2long(l,32,TSC_VALID_SPATIAL,(unsigned char*)&res);///hren
		if(struct_Ticket->st_TicketContractCommonData.st_Tariff==4)
		{
			struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialAreaSP.uc_TSC_RoutesSystem=(unsigned char)((res&VAL_SPAC_MASK_ASP_RSYS)>>14);
			struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialAreaSP.ush_TSC_ValidityZones=(unsigned short)((res&VAL_SPAC_MASK_ASP_VALZONE)>>2);
			///Vaf          struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialAreaSP.uc_TSC_RoutesSystem=(unsigned char)(res&VAL_SPAC_MASK_ASP_RSYS);
			///Vaf          struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialAreaSP.ush_TSC_ValidityZones=(unsigned short)((res&VAL_SPAC_MASK_ASP_VALZONE)>>8);
		}
		else 
			if(struct_Ticket->st_TicketContractCommonData.st_Tariff==5)
			{
				struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialPoint2PointSP.uc_TSC_RoutesSystem=(unsigned char)((res&VAL_SPAC_MASK_P2P_RSYS)>>14);
				struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialPoint2PointSP.uc_TSC_Origin=(unsigned char)((res&VAL_SPAC_MASK_P2P_ORIGIN)>>7);
				struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialPoint2PointSP.uc_TSC_Destination=(unsigned char)(res&VAL_SPAC_MASK_P2P_DEST);
			}
			else///   must be 6
			{
				struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialPredefinedSP.l_TSC_ContractTypelD=res;///&VAL_SPAC_MASK_PSP_CONTRACTID;
			}
			///     struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.union_TSC_ValiditySpatial.s_ValiditySpatialPredefinedSP.l_TSC_ContractTypelD=res;///&VAL_SPAC_MASK_PSP_CONTRACTID;
			///////////////initial
			///   read 11 bits from byte 22 
			memcpy(&l,trg+22,4);
			localLrev(&l);///localLSwap(&l);
			bit2long(l,11,TSI_START,(unsigned char*)&res);
			datetime=(unsigned short)res;///hren
			if(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TicketDuration.e_TicketDurationType==e_ClyTkt_DurationInHours)
			{
				localHourDayProc(&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start,&datetime,
					&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date,e_ClyTkt_TktDTConv_HourInWORD2stDate);
				///localDateTimeProc(&datetime,0,&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.union_Start.stTime,0,e_ClyTkt_TktDTConv_Bit2CompactTime);
			}
			else 
				if(struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.e_TSC_Sliding==0)
					memset(&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start,0,sizeof(st_Cly_DateAndTime));
				else
					localHourDayProc(&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassInitialRec.stTSI_start,&datetime,
					&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassContractRec.st_TSC_Date,e_ClyTkt_TktDTConv_DayInWORD2stDate);

			///////////////validation
			///   read 1 bits from byte 26 
			memcpy(&l,trg+26,4);
			localLrev(&l);///localLSwap(&l);
			bit2long(l,1,TSL_VIRGIN_FLAG,(unsigned char*)&res);
			struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.TSL_IsVirginFlag=(ClyTkt_BOOL)res;
			if(res)
			{
				memcpy(signarr,struct_Ticket->ucp_Sn,8);
				memcpy(signarr+8,trg+18,4);
				signarr[12]=0x80;///flag 1 , rfu(15bits)0
				signarr[13]=0;///flag 1 , rfu(15bits)0
				err=GetSignFromRAM(eSeasSign3,tmpsign,struct_Ticket->ucp_Sn,signarr);
				///         err=localSignProc(signarr,13,ClyTkt_SignType_32,tmpsign);
				if(err!=e_ClyTkt_NO_ERROR)
					return err;
				if(memcmp(tmpsign,trg+28,4))
					return e_ClyTkt_CHECK_SEASON_THERD_SIGN_FAIL;
			}
			else
			{
				///   read 10 bits from byte 26 
				bit2long(l,11,TSLL_LOCATION_ID,(unsigned char*)&res);
				struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.ush_TSLL_Locationld=(unsigned short)res;
				///   read 7 bits from byte 26 
				bit2long(l,18,TSLL_SERVICE_PROVIDER,(unsigned char*)&res);
				struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.uc_TSLL_ServiceProvider=(unsigned char)res;
				///   read 9 bits from byte 26 
				bit2long(l,27,TSLL_TIME_STAMP,(unsigned char*)&res);
				datetime=(unsigned short)res;///hren
				localDateTimeProc(&datetime,0,&struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.st_TSLL_TimeStamp,0,e_ClyTkt_TktDTConv_Bit2CompactTime);
				///hara         struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.st_TSLL_TimeStamp=res;
				///   read 9 bits from byte 28 
				memcpy(&l,trg+28,4);
				localLrev(&l);///localLSwap(&l);
				bit2long(l,20,TSLL_DATE_OFFSET,(unsigned char*)&res);
				struct_Ticket->union_TicketAppType.st_TicketSeasonPassTicket.st_TicketSeasonPassValidationRec.ush_TSSL_DateOffset=(unsigned short)res;
				if(multisign2)///sign fail
					return e_ClyTkt_CHECK_SEASON_THERD_SIGN_FAIL;

			}
	}///   if(season) of convert data
	else///   if(season) of convert data
	{
		///////////////contract
		///   read 4 bits from byte 12 
		memcpy(&l,trg+12,4);
		localLrev(&l);///localLSwap(&l);
		bit2long(l,4,TMC_RELOAD_COUNT,(unsigned char*)&res);
		struct_Ticket->st_TicketContractCommonData.uc_ReloadCount=(unsigned char)res;
		///   read 9 bits from byte 12 
		bit2long(l,13,TMC_SALE_DATE,(unsigned char*)&res);
		datetime=(unsigned short)res;///hren
		localDateTimeProc(&datetime,&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.st_TMC_SaleDate,0,0,e_ClyTkt_TktDTConv_Bit2ShortDate);
		///     struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.st_TMC_SaleDate=res;
		///   read 5 bits from byte 12 
		bit2long(l,18,TMC_VALID_JOURN,(unsigned char*)&res);
		struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.uc_TMC_ValidityJourneys=(unsigned char)res;
		///   read 14 bits from byte 12 
		bit2long(l,32,TMC_VALID_SPATIAL,(unsigned char*)&res);///hren       
		if(struct_Ticket->st_TicketContractCommonData.st_Tariff==0)
		{

			struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialFareCode.uc_TMC_RoutesSystem=(unsigned char)((res&MULTI_VAL_SPAC_MASK_FC_RSYS)>>6);
			struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialFareCode.uc_TCM_FareCode=(unsigned char)((res&MULTI_VAL_SPAC_MASK_FC_FARCODE));
		}
		else 
			if(struct_Ticket->st_TicketContractCommonData.st_Tariff==1)
			{
				struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialPoint2Point.uc_TMC_Origin=(unsigned char)((res&MULTI_VAL_SPAC_MASK_P2P_ORIGN)>>7);
				struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialPoint2Point.uc_TCM_Destination=(unsigned char)((res&MULTI_VAL_SPAC_MASK_P2P_DESTINATION));
			}
			else///   must be 2
			{
				struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideContractRec.union_TMC_ValiditySpatial.s_ValiditySpatialPredefined.ush_ContractTypelD=(unsigned short)res;
			}

			///////////////   validation
			///   read 7 bits from byte 20 
			memcpy(&l,trg+20,4);
			///     localLSwap(&l);
			localLrev(&l);
			bit2long(l,7,TMF_SERVICE_PROVIDER,(unsigned char*)&res);
			struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.uc_TMF_ServiceProvider=(unsigned char)res;
			///   read 14 bits from byte 20 
			bit2long(l,21,TMF_DATE_STAMP,(unsigned char*)&res);
			datetime=(unsigned short)res;///hren
			localDateTimeProc(&datetime,0,0,&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_DateStamp,e_ClyTkt_TktDTConv_Bit2CompactDate);
			///     struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_DateStamp=res;
			///   read 9 bits from byte 20 
			bit2long(l,30,TMF_TIME_STAMP,(unsigned char*)&res);
			datetime=(unsigned short)res;///hren                                                                                                            
			localDateTimeProc(&datetime,0,&struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_TimeStamp,0,e_ClyTkt_TktDTConv_Bit2CompactTime);
			///     struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.st_TMF_TimeStamp=res;
			///   read 10 bits from byte 23 
			memcpy(&l,trg+23,4);
			///     localLSwap(&l);
			localLrev(&l);
			bit2long(l,16,TMF_LOCATION_STAMP,(unsigned char*)&res);
			struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.ush_TMF_LocationStamp=(unsigned short)res;
			///   read 1 bits from byte 23 
			bit2long(l,22,TMF_DIRECTION,(unsigned char*)&res);
			struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.e_TMF_Direction=(e_ClyTkt_Direction)res;
			///   read 16 bits from byte 26
			memcpy(&l,trg+26,4);
			///     localLSwap(&l);
			localLrev(&l);
			bit2long(l,16,TMF_SIGNATURE,(unsigned char*)&res);
			struct_Ticket->union_TicketAppType.st_MultiRideTicket.st_TicketMultiRideFirstValidationRec.us_TMF_Sig=(unsigned short)res;
			///////////////   location
			///   read 1 bits from byte 28 
			memcpy(&l,trg+28,4);
			localLrev(&l);///localLSwap(&l);
			bit2long(l,1,TML_FIRST_FLAG,(unsigned char*)&res); 
			struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.uc_FirstFlag=(unsigned char)res;
			///   read 10 bits from byte 28 
			///     memcpy(&l,trg+28,4);
			///     localLSwap(&l);
			bit2long(l,11,TML_LOCATION_ID,(unsigned char*)&res);
			struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.ush_TML_LocationId=(unsigned short)res;
			if(struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.uc_FirstFlag)
			{
				///   read 5 bits from byte 28 
				bit2long(l,16,TMLL_JOURN_BCKUP,(unsigned char*)&res);
				resbak=struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.uc_JourneysBck=(unsigned char)res;
				///   read 16 bits from byte 28 
				bit2long(l,32,TMLL_SIGNUTE_BCKUP,(unsigned char*)&res);
				struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRecBackUp.ush_SignatureBkp=(unsigned short)res;
				if(multisign2!=0)///sign success
				{
					///   check backup sign
					///   get serial number
					memcpy(signarr,struct_Ticket->ucp_Sn,8);
					///   get first sign
					memcpy(signarr+8,trg+16,4);
					///   get total jornes/backup jornes
					signarr[12]=(unsigned char)resbak;
					///   build sign
					err=localSignProc(signarr,13,ClyTkt_SignType_16,tmpsign);/// build sign
					///   check backup sign
					if(err!=e_ClyTkt_NO_ERROR || memcmp(tmpsign,trg+30,2))
						return e_ClyTkt_SECOND_MULTI_SIGN_FAIL_BUT_BKPFLAG_FALSE;
					return e_ClyTkt_BKFLGTRU_SECMULSIGNFAIL;///e_ClyTkt_SECOND_MULTI_SIGN_FAIL_BUT_BKPFLAG_TRUE;
				}
			}
			else
			{
				///   read 7 bits from byte 28 
				bit2long(l,18,TMLL_SERVICE_PROVIDER,(unsigned char*)&res);
				struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRec.uc_TMLL_ServiceProvider=(unsigned char)res;
				///   read 9 bits from byte 28 
				bit2long(l,27,TMLL_TIME_STAMP,(unsigned char*)&res);
				datetime=(unsigned short)res;///hren
				localDateTimeProc(&datetime,0,&struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRec.st_TimeStamp,0,e_ClyTkt_TktDTConv_Bit2CompactTime);
				///         struct_Ticket->union_TicketAppType.st_MultiRideTicket.union_TicketMultiRideLocationRec.st_TicketMultiRideLocationRec.st_TimeStamp=res;
				if(multisign2)///sign fail
					return e_ClyTkt_SECOND_MULTI_SIGN_FAIL_BUT_BKPFLAG_FALSE;
			}
	}///   else of if(season) of convert data
	return err;
}

#endif // #ifdef CORE_SUPPORT_SMARTCARD
