
#include <Config.h>
#ifdef CORE_SUPPORT_LIBCRYPTO

////////////////////////////////////////////////////////////////////////////////////
//
//  
//  DES - A portable, public domain version of the Data Encryption Standard.
//
//
////////////////////////////////////////////////////////////////////////////////////

#include <CoreTypes.h>
#include <string.h>


// Prototypes
static void     des_scrunch             (uint8_tw *, uint32_tw *);
static void     des_unscrun             (uint32_tw *, uint8_tw *);
void            des_func                (uint32_tw *, uint32_tw *);
static void     des_cookey              (uint32_tw *);
void            des_usekey              (register uint32_tw *from);

// Static paras and globals
static uint32_tw                    KnL[32] = { 0L };
static int32_tw                     Batir   = 0;

static const uint16_tw bytebit[8]  =
{
	0200, 0100, 040, 020, 010, 04, 02, 01
};

static const uint32_tw bigbyte[24] =
{
	0x800000L,  0x400000L,  0x200000L,  0x100000L,
	0x80000L,   0x40000L,   0x20000L,   0x10000L,
	0x8000L,    0x4000L,    0x2000L,    0x1000L,
	0x800L,     0x400L,     0x200L,     0x100L,
	0x80L,      0x40L,      0x20L,      0x10L,
	0x8L,       0x4L,       0x2L,       0x1L
};

// Use the key schedule specified in the Standard (ANSI X3.92-1981).
static const uint8_tw pc1[56] =
{
	56, 48, 40, 32, 24, 16,  8, 0,  57, 49, 41, 33, 25, 17,
	9,  1,  58, 50, 42, 34, 26, 18, 10, 2,  59, 51, 43, 35,
	62, 54, 46, 38, 30, 22, 14, 6,  61, 53, 45, 37, 29, 21,
	13, 5,  60, 52, 44, 36, 28, 20, 12, 4,  27, 19, 11, 3
};

static const uint8_tw totrot[16] =
{
	1, 2, 4, 6, 8, 10, 12, 14, 15, 17, 19, 21, 23, 25, 27, 28 
};

static const uint8_tw pc2[48] =
{
	13, 16, 10, 23, 0,  4,  2,  27, 14, 5,  20, 9,
	22, 18, 11, 3,  25, 7,  15, 6,  26, 19, 12, 1,
	40, 51, 30, 36, 46, 54, 29, 39, 50, 44, 32, 47,
	43, 48, 38, 55, 33, 52, 45, 41, 49, 35, 28, 31 
};

///////////////////////////////////////////////////////////////////////////////////////
//
// SP1
//
///////////////////////////////////////////////////////////////////////////////////////
static const uint32_tw SP1[64] =
{
	0x01010400L, 0x00000000L, 0x00010000L, 0x01010404L,
	0x01010004L, 0x00010404L, 0x00000004L, 0x00010000L,
	0x00000400L, 0x01010400L, 0x01010404L, 0x00000400L,
	0x01000404L, 0x01010004L, 0x01000000L, 0x00000004L,
	0x00000404L, 0x01000400L, 0x01000400L, 0x00010400L,
	0x00010400L, 0x01010000L, 0x01010000L, 0x01000404L,
	0x00010004L, 0x01000004L, 0x01000004L, 0x00010004L,
	0x00000000L, 0x00000404L, 0x00010404L, 0x01000000L,
	0x00010000L, 0x01010404L, 0x00000004L, 0x01010000L,
	0x01010400L, 0x01000000L, 0x01000000L, 0x00000400L,
	0x01010004L, 0x00010000L, 0x00010400L, 0x01000004L,
	0x00000400L, 0x00000004L, 0x01000404L, 0x00010404L,
	0x01010404L, 0x00010004L, 0x01010000L, 0x01000404L,
	0x01000004L, 0x00000404L, 0x00010404L, 0x01010400L,
	0x00000404L, 0x01000400L, 0x01000400L, 0x00000000L,
	0x00010004L, 0x00010400L, 0x00000000L, 0x01010004L 
};

///////////////////////////////////////////////////////////////////////////////////////
//
// SP2
//
///////////////////////////////////////////////////////////////////////////////////////
static const uint32_tw SP2[64] =
{
	0x80108020L, 0x80008000L, 0x00008000L, 0x00108020L,
	0x00100000L, 0x00000020L, 0x80100020L, 0x80008020L,
	0x80000020L, 0x80108020L, 0x80108000L, 0x80000000L,
	0x80008000L, 0x00100000L, 0x00000020L, 0x80100020L,
	0x00108000L, 0x00100020L, 0x80008020L, 0x00000000L,
	0x80000000L, 0x00008000L, 0x00108020L, 0x80100000L,
	0x00100020L, 0x80000020L, 0x00000000L, 0x00108000L,
	0x00008020L, 0x80108000L, 0x80100000L, 0x00008020L,
	0x00000000L, 0x00108020L, 0x80100020L, 0x00100000L,
	0x80008020L, 0x80100000L, 0x80108000L, 0x00008000L,
	0x80100000L, 0x80008000L, 0x00000020L, 0x80108020L,
	0x00108020L, 0x00000020L, 0x00008000L, 0x80000000L,
	0x00008020L, 0x80108000L, 0x00100000L, 0x80000020L,
	0x00100020L, 0x80008020L, 0x80000020L, 0x00100020L,
	0x00108000L, 0x00000000L, 0x80008000L, 0x00008020L,
	0x80000000L, 0x80100020L, 0x80108020L, 0x00108000L 
};

///////////////////////////////////////////////////////////////////////////////////////
//
// SP3
//
///////////////////////////////////////////////////////////////////////////////////////
static const uint32_tw SP3[64] =
{
	0x00000208L, 0x08020200L, 0x00000000L, 0x08020008L,
	0x08000200L, 0x00000000L, 0x00020208L, 0x08000200L,
	0x00020008L, 0x08000008L, 0x08000008L, 0x00020000L,
	0x08020208L, 0x00020008L, 0x08020000L, 0x00000208L,
	0x08000000L, 0x00000008L, 0x08020200L, 0x00000200L,
	0x00020200L, 0x08020000L, 0x08020008L, 0x00020208L,
	0x08000208L, 0x00020200L, 0x00020000L, 0x08000208L,
	0x00000008L, 0x08020208L, 0x00000200L, 0x08000000L,
	0x08020200L, 0x08000000L, 0x00020008L, 0x00000208L,
	0x00020000L, 0x08020200L, 0x08000200L, 0x00000000L,
	0x00000200L, 0x00020008L, 0x08020208L, 0x08000200L,
	0x08000008L, 0x00000200L, 0x00000000L, 0x08020008L,
	0x08000208L, 0x00020000L, 0x08000000L, 0x08020208L,
	0x00000008L, 0x00020208L, 0x00020200L, 0x08000008L,
	0x08020000L, 0x08000208L, 0x00000208L, 0x08020000L,
	0x00020208L, 0x00000008L, 0x08020008L, 0x00020200L 
};

///////////////////////////////////////////////////////////////////////////////////////
//
// SP4
//
///////////////////////////////////////////////////////////////////////////////////////
static const uint32_tw SP4[64] =
{
	0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
	0x00802080L, 0x00800081L, 0x00800001L, 0x00002001L,
	0x00000000L, 0x00802000L, 0x00802000L, 0x00802081L,
	0x00000081L, 0x00000000L, 0x00800080L, 0x00800001L,
	0x00000001L, 0x00002000L, 0x00800000L, 0x00802001L,
	0x00000080L, 0x00800000L, 0x00002001L, 0x00002080L,
	0x00800081L, 0x00000001L, 0x00002080L, 0x00800080L,
	0x00002000L, 0x00802080L, 0x00802081L, 0x00000081L,
	0x00800080L, 0x00800001L, 0x00802000L, 0x00802081L,
	0x00000081L, 0x00000000L, 0x00000000L, 0x00802000L,
	0x00002080L, 0x00800080L, 0x00800081L, 0x00000001L,
	0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
	0x00802081L, 0x00000081L, 0x00000001L, 0x00002000L,
	0x00800001L, 0x00002001L, 0x00802080L, 0x00800081L,
	0x00002001L, 0x00002080L, 0x00800000L, 0x00802001L,
	0x00000080L, 0x00800000L, 0x00002000L, 0x00802080L 
};

///////////////////////////////////////////////////////////////////////////////////////
//
// SP5
//
///////////////////////////////////////////////////////////////////////////////////////
static const uint32_tw SP5[64] =
{
	0x00000100L, 0x02080100L, 0x02080000L, 0x42000100L,
	0x00080000L, 0x00000100L, 0x40000000L, 0x02080000L,
	0x40080100L, 0x00080000L, 0x02000100L, 0x40080100L,
	0x42000100L, 0x42080000L, 0x00080100L, 0x40000000L,
	0x02000000L, 0x40080000L, 0x40080000L, 0x00000000L,
	0x40000100L, 0x42080100L, 0x42080100L, 0x02000100L,
	0x42080000L, 0x40000100L, 0x00000000L, 0x42000000L,
	0x02080100L, 0x02000000L, 0x42000000L, 0x00080100L,
	0x00080000L, 0x42000100L, 0x00000100L, 0x02000000L,
	0x40000000L, 0x02080000L, 0x42000100L, 0x40080100L,
	0x02000100L, 0x40000000L, 0x42080000L, 0x02080100L,
	0x40080100L, 0x00000100L, 0x02000000L, 0x42080000L,
	0x42080100L, 0x00080100L, 0x42000000L, 0x42080100L,
	0x02080000L, 0x00000000L, 0x40080000L, 0x42000000L,
	0x00080100L, 0x02000100L, 0x40000100L, 0x00080000L,
	0x00000000L, 0x40080000L, 0x02080100L, 0x40000100L 
};

///////////////////////////////////////////////////////////////////////////////////////
//
// SP6
//
///////////////////////////////////////////////////////////////////////////////////////
static const uint32_tw SP6[64] =
{
	0x20000010L, 0x20400000L, 0x00004000L, 0x20404010L,
	0x20400000L, 0x00000010L, 0x20404010L, 0x00400000L,
	0x20004000L, 0x00404010L, 0x00400000L, 0x20000010L,
	0x00400010L, 0x20004000L, 0x20000000L, 0x00004010L,
	0x00000000L, 0x00400010L, 0x20004010L, 0x00004000L,
	0x00404000L, 0x20004010L, 0x00000010L, 0x20400010L,
	0x20400010L, 0x00000000L, 0x00404010L, 0x20404000L,
	0x00004010L, 0x00404000L, 0x20404000L, 0x20000000L,
	0x20004000L, 0x00000010L, 0x20400010L, 0x00404000L,
	0x20404010L, 0x00400000L, 0x00004010L, 0x20000010L,
	0x00400000L, 0x20004000L, 0x20000000L, 0x00004010L,
	0x20000010L, 0x20404010L, 0x00404000L, 0x20400000L,
	0x00404010L, 0x20404000L, 0x00000000L, 0x20400010L,
	0x00000010L, 0x00004000L, 0x20400000L, 0x00404010L,
	0x00004000L, 0x00400010L, 0x20004010L, 0x00000000L,
	0x20404000L, 0x20000000L, 0x00400010L, 0x20004010L 
};

///////////////////////////////////////////////////////////////////////////////////////
//
// SP7
//
///////////////////////////////////////////////////////////////////////////////////////
static const uint32_tw SP7[64] =
{
	0x00200000L, 0x04200002L, 0x04000802L, 0x00000000L,
	0x00000800L, 0x04000802L, 0x00200802L, 0x04200800L,
	0x04200802L, 0x00200000L, 0x00000000L, 0x04000002L,
	0x00000002L, 0x04000000L, 0x04200002L, 0x00000802L,
	0x04000800L, 0x00200802L, 0x00200002L, 0x04000800L,
	0x04000002L, 0x04200000L, 0x04200800L, 0x00200002L,
	0x04200000L, 0x00000800L, 0x00000802L, 0x04200802L,
	0x00200800L, 0x00000002L, 0x04000000L, 0x00200800L,
	0x04000000L, 0x00200800L, 0x00200000L, 0x04000802L,
	0x04000802L, 0x04200002L, 0x04200002L, 0x00000002L,
	0x00200002L, 0x04000000L, 0x04000800L, 0x00200000L,
	0x04200800L, 0x00000802L, 0x00200802L, 0x04200800L,
	0x00000802L, 0x04000002L, 0x04200802L, 0x04200000L,
	0x00200800L, 0x00000000L, 0x00000002L, 0x04200802L,
	0x00000000L, 0x00200802L, 0x04200000L, 0x00000800L,
	0x04000002L, 0x04000800L, 0x00000800L, 0x00200002L 
};

///////////////////////////////////////////////////////////////////////////////////////
//
// SP8
//
///////////////////////////////////////////////////////////////////////////////////////
static const uint32_tw SP8[64] =
{
	0x10001040L, 0x00001000L, 0x00040000L, 0x10041040L,
	0x10000000L, 0x10001040L, 0x00000040L, 0x10000000L,
	0x00040040L, 0x10040000L, 0x10041040L, 0x00041000L,
	0x10041000L, 0x00041040L, 0x00001000L, 0x00000040L,
	0x10040000L, 0x10000040L, 0x10001000L, 0x00001040L,
	0x00041000L, 0x00040040L, 0x10040040L, 0x10041000L,
	0x00001040L, 0x00000000L, 0x00000000L, 0x10040040L,
	0x10000040L, 0x10001000L, 0x00041040L, 0x00040000L,
	0x00041040L, 0x00040000L, 0x10041000L, 0x00001000L,
	0x00000040L, 0x10040040L, 0x00001000L, 0x00041040L,
	0x10001000L, 0x00000040L, 0x10000040L, 0x10040000L,
	0x10040040L, 0x10000000L, 0x00040000L, 0x10001040L,
	0x00000000L, 0x10041040L, 0x00040040L, 0x10000040L,
	0x10040000L, 0x10001000L, 0x10001040L, 0x00000000L,
	0x10041040L, 0x00041000L, 0x00041000L, 0x00001040L,
	0x00001040L, 0x00040040L, 0x10000000L, 0x10041000L 
};

///////////////////////////////////////////////////////////////////////////////////////
//
//
// 3DES Implementation and optimization related
//
//
///////////////////////////////////////////////////////////////////////////////////////

#define DES3_REMEMBER_KEY    1
#define DES3_INIT_CODE       0xa5a5a5L
#define INV(val) ((e_Oparation)(!(val)))
typedef uint32_tw CookedKey[32];        // Define type of 32 permutations for

typedef struct 
{
	CookedKey      EncryptCook[DES3_REMEMBER_KEY];
	CookedKey      DecryptCook[DES3_REMEMBER_KEY];   // The Decrypt permutation
	long           Init;                             // Init value
	int32_tw       indexLocalKey;
	uint8_tw  LocalKey[DES3_REMEMBER_KEY][16];  // The key
} StDesOptimize;    // Local object that holds the permutation for des

// Operation type
typedef enum 
{
	e_Encrypt = 0,
	e_Decrypt = 1
} e_Oparation;

typedef struct  
{
	StDesOptimize  DesArray[2];
} StDes3Optimize;   // The object that hold the des3 optimization data

StDes3Optimize  Obj_Des3;
StDesOptimize   Obj_Des;


// Prototypes
void des3_xor               (uint8_tw *In1, uint8_tw *In2, uint8_tw *OuT, int32_tw len);
void des3_init_optimize     (StDesOptimize  *desoptim);
void des3_optimized_des     (StDesOptimize  *desoptim,int8_tw *InData,int8_tw *OutData,int8_tw *Key,e_Oparation Operation);

///////////////////////////////////////////////////////////////////////////////////////
//
//
// CRC32 Static tables
//
//
///////////////////////////////////////////////////////////////////////////////////////

static const uint32_tw crcTable[] =
{ 
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535,
	0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD,
	0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D,
	0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4,
	0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
	0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC,
	0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB,
	0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F,
	0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB,
	0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA,
	0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE,
	0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A,
	0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409,
	0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
	0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739,
	0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344, 0x8708A3D2, 0x1E01F268,
	0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0,
	0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8,
	0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF,
	0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703,
	0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7,
	0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE,
	0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
	0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6,
	0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D,
	0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5,
	0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605,
	0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D 
}; 


///////////////////////////////////////////////////////////////////////////////////////
//
//
// Pseudo RNG - generate 2**31-2 random numbers
// based on "Random Number Generators: Good Ones Are Hard to Find",
// S.K. Park and K.W. Miller, Communications of the ACM 31:10 (Oct 1988),
//
///////////////////////////////////////////////////////////////////////////////////////

#define PRNG_A      16807         // Multiplier 
#define PRNG_M      2147483647L   // 2**31 - 1 
#define PRNG_Q      127773L       // PRNG_M div PRNG_A 
#define PRNG_R      2836          // PRNG_M mod PRNG_A 
static  long        RandomNum       = 1;

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   prng_nextrand
// Notes    :
//
///////////////////////////////////////////////////////////////////////////////////////

long prng_nextrand(long seed)
{
	uint32_tw lo, hi;

	lo = PRNG_A * (long)(seed & 0xFFFF);
	hi = PRNG_A * (long)((uint32_tw)seed >> 16);
	lo += (hi & 0x7FFF) << 16;
	if (lo > PRNG_M)
	{
		lo &= PRNG_M;
		++lo;
	}
	lo += hi >> 15;
	if (lo > PRNG_M)
	{
		lo &= PRNG_M;
		++lo;
	}
	return (long)lo;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   prng_rand
// Notes    :   Returns next random long
//
///////////////////////////////////////////////////////////////////////////////////////

long prng_rand(void)            
{
	RandomNum = prng_nextrand(RandomNum);
	return RandomNum;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   prng_srand
// Notes    :   Seeds the PRNG algorithm
//
///////////////////////////////////////////////////////////////////////////////////////
void prng_srand(uint32_tw seed)  // to seed it
{
	RandomNum = seed ? (seed & PRNG_M) : 1;  // nonzero seed 
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :
// Notes    :   - DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des_deskey(uint8_tw *key, int16_tw edf)
{
	register int32_tw i, j, l, m, n;
	uint8_tw pc1m[56], pcr[56];
	uint32_tw kn[32];

	for (j = 0; j < 56; j++) 
	{
		l = pc1[j];
		m = l & 07;
		pc1m[j] = (key[l >> 3] & bytebit[m]) ? 1 : 0;
	}
	for (i = 0; i < 16; i++) 
	{
		if (edf == e_Decrypt ) 
		{
			m = (15 - i) << 1;
		}
		else 
		{
			m = i << 1;
		}
		n = m + 1;
		kn[m] = kn[n] = 0L;
		for (j = 0; j < 28; j++) 
		{
			l = j + totrot[i];
			if (l < 28 ) 
			{
				pcr[j] = pc1m[l];
			}
			else 
			{
				pcr[j] = pc1m[l - 28];
			}
		}
		for (j = 28; j < 56; j++) 
		{
			l = j + totrot[i];
			if (l < 56 )
			{
				pcr[j] = pc1m[l];
			}
			else
			{
				pcr[j] = pc1m[l - 28];
			}
		}
		for (j = 0; j < 24; j++) 
		{
			if (pcr[pc2[j]] )
			{
				kn[m] |= bigbyte[j];
			}
			if (pcr[pc2[j+24]] )
			{
				kn[n] |= bigbyte[j];
			}
		}
	}
	des_cookey(kn);
	return;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :
// Notes    :   - DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

static void des_cookey(register uint32_tw *raw1)
{
	register uint32_tw *cook, *raw0;
	uint32_tw dough[32];
	register int32_tw i;

	cook = dough;
	for (i = 0; i < 16; i++, raw1++) 
	{
		raw0     = raw1++;
		*cook    = (*raw0 & 0x00fc0000L) << 6;
		*cook   |= (*raw0 & 0x00000fc0L) << 10;
		*cook   |= (*raw1 & 0x00fc0000L) >> 10;
		*cook++ |= (*raw1 & 0x00000fc0L) >> 6;
		*cook    = (*raw0 & 0x0003f000L) << 12;
		*cook   |= (*raw0 & 0x0000003fL) << 16;
		*cook   |= (*raw1 & 0x0003f000L) >> 4;
		*cook++ |= (*raw1 & 0x0000003fL);
	}
	des_usekey(dough);
	return;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :
// Notes    :   - DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des_cpkey(register uint32_tw *into)
{
	register uint32_tw *from, *endp;

	from = KnL, endp = &KnL[32];
	while (from < endp)
	{
		*into++ = *from++;
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des_usekey
// Notes    :   - DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des_usekey(register uint32_tw *from)
{
	register uint32_tw *to, *endp;

	to = KnL, endp = &KnL[32];
	while (to < endp)
	{
		*to++ = *from++;
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des
// Notes    :   - DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////
void des(uint8_tw *inblock, uint8_tw *outblock)
{
	uint32_tw work[2];

	des_scrunch(inblock, work);
	des_func(work, KnL);
	des_unscrun(work, outblock);
	return;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des_scrunch
// Notes    :   - DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

static void des_scrunch(register uint8_tw *outof,
	register uint32_tw *into)

{
	*into    = (*outof++ & 0xffL) << 24;
	*into   |= (*outof++ & 0xffL) << 16;
	*into   |= (*outof++ & 0xffL) << 8;
	*into++ |= (*outof++ & 0xffL);
	*into    = (*outof++ & 0xffL) << 24;
	*into   |= (*outof++ & 0xffL) << 16;
	*into   |= (*outof++ & 0xffL) << 8;
	*into   |= (*outof   & 0xffL);
	return;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des_unscrun
// Notes    :   - DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

static void des_unscrun(register uint32_tw *outof,
	register uint8_tw *into)

{
	*into++ = (uint8_tw)((*outof >> 24) & 0xffL);
	*into++ = (uint8_tw)((*outof >> 16) & 0xffL);
	*into++ = (uint8_tw)((*outof >>  8) & 0xffL);
	*into++ = (uint8_tw)(*outof++   & 0xffL);
	*into++ = (uint8_tw)((*outof >> 24) & 0xffL);
	*into++ = (uint8_tw)((*outof >> 16) & 0xffL);
	*into++ = (uint8_tw)((*outof >>  8) & 0xffL);
	*into   = (uint8_tw)(*outof & 0xffL);
	return;
}


///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des_func
// Notes    :   - DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des_func(register uint32_tw *block, register uint32_tw *keys)
{
	register uint32_tw fval, work, right, leftt;
	register int32_tw round;

	leftt  = block[0];
	right  = block[1];
	work   = ((leftt >> 4) ^ right) & 0x0f0f0f0fL;
	right ^= work;
	leftt ^= (work << 4);
	work   = ((leftt >> 16) ^ right) & 0x0000ffffL;
	right ^= work;
	leftt ^= (work << 16);
	work   = ((right >> 2) ^ leftt) & 0x33333333L;
	leftt ^= work;
	right ^= (work << 2);
	work   = ((right >> 8) ^ leftt) & 0x00ff00ffL;
	leftt ^= work;
	right ^= (work << 8);
	right  = ((right << 1) | ((right >> 31) & 1L)) & 0xffffffffL;
	work   = (leftt ^ right) & 0xaaaaaaaaL;
	leftt ^= work;
	right ^= work;
	leftt  = ((leftt << 1) | ((leftt >> 31) & 1L)) & 0xffffffffL;

	for (round = 0; round < 8; round++) 
	{
		work   = (right << 28) | (right >> 4);
		work  ^= *keys++;
		fval   = SP7[ work    & 0x3fL];
		fval  |= SP5[(work >>  8) & 0x3fL];
		fval  |= SP3[(work >> 16) & 0x3fL];
		fval  |= SP1[(work >> 24) & 0x3fL];
		work   = right ^ *keys++;
		fval  |= SP8[ work    & 0x3fL];
		fval  |= SP6[(work >>  8) & 0x3fL];
		fval  |= SP4[(work >> 16) & 0x3fL];
		fval  |= SP2[(work >> 24) & 0x3fL];
		leftt ^= fval;
		work   = (leftt << 28) | (leftt >> 4);
		work  ^= *keys++;
		fval   = SP7[ work    & 0x3fL];
		fval  |= SP5[(work >>  8) & 0x3fL];
		fval  |= SP3[(work >> 16) & 0x3fL];
		fval  |= SP1[(work >> 24) & 0x3fL];
		work   = leftt ^ *keys++;
		fval  |= SP8[ work    & 0x3fL];
		fval  |= SP6[(work >>  8) & 0x3fL];
		fval  |= SP4[(work >> 16) & 0x3fL];
		fval  |= SP2[(work >> 24) & 0x3fL];
		right ^= fval;
	}

	right  = (right << 31) | (right >> 1);
	work   = (leftt ^ right) & 0xaaaaaaaaL;
	leftt ^= work;
	right ^= work;
	leftt  = (leftt << 31) | (leftt >> 1);
	work   = ((leftt >> 8) ^ right) & 0x00ff00ffL;
	right ^= work;
	leftt ^= (work << 8);
	work   = ((leftt >> 2) ^ right) & 0x33333333L;
	right ^= work;
	leftt ^= (work << 2);
	work   = ((right >> 16) ^ leftt) & 0x0000ffffL;
	leftt ^= work;
	right ^= (work << 16);
	work   = ((right >> 4) ^ leftt) & 0x0f0f0f0fL;
	leftt ^= work;
	right ^= (work << 4);
	*block++ = right;
	*block = leftt;
	return;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des_encipher
// Notes    :   - DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des_encipher(uint8_tw *plaintext, uint8_tw *ciphertext, uint8_tw *key)
{
	int32_tw i;

	if (Obj_Des.Init != DES3_INIT_CODE)
	{
		des3_init_optimize(&Obj_Des);
	}

	if (Batir)
	{
		for (i = 0; i < 8; i++)
		{
			ciphertext[i] = plaintext[i] ^ key[i];
		}
		return;
	}

	// Call  des object des function
	des3_optimized_des(&Obj_Des, (int8_tw*)plaintext, (int8_tw*)ciphertext, (int8_tw*)key, e_Encrypt);

	return;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des_decipher
// Notes    :   - DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////
    
void des_decipher(uint8_tw *ciphertext, uint8_tw *plaintext, uint8_tw *key)
{
	int32_tw i;
	if (Obj_Des.Init != DES3_INIT_CODE)
	{
		des3_init_optimize(&Obj_Des);
	}

	// Call  des object des function
	if (Batir)
	{
		for (i = 0; i < 8; i++)
		{
			ciphertext[i] = plaintext[i] ^ key[i];
		}
		return;
	}

	des3_optimized_des(&Obj_Des, (int8_tw*)ciphertext, (int8_tw*)plaintext, (int8_tw*)key, e_Decrypt);

	return;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des_mac_with_residue
// Notes    :   - DES signature Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

static void des_mac_with_residue(uint8_tw *plaintext, // [IN]  Cipher-text
	int32_tw lenDataIn,                               // [IN]  The data length
	uint8_tw *key,                                    // [IN]  Key
	uint8_tw *Signiture)                              // [OUT] Signiture
{

	int32_tw resiude,i;
	uint8_tw ResidueChar[8];
	uint8_tw InitValue[8], tmp[8];

	resiude = lenDataIn%8;

	memset(InitValue, 0, 8);
	memset(ResidueChar, 0, 8);

	for (i = 0; i + 8 <= lenDataIn; i += 8)
	{
		// XOR the plain text with the previous result or initiate with the InitValue
		des3_xor(&plaintext[i], InitValue, tmp, 8);

		// Encrypt the xor result with the key and put it in the InitValue
		des_encipher(tmp, InitValue, key);

	}
	if (resiude)
	{
		memcpy(ResidueChar, &plaintext[i], resiude);
		// XOR the plan text with the previous result or at first with the InitValue
		des3_xor(ResidueChar, InitValue, tmp, 8);
		// Encrypt the xor result with the key and put it in the InitValue
		des_encipher(tmp, InitValue, key);
	}

	// Modify Signiture
	memcpy(Signiture, InitValue, 8);
}

/////////////////////////////////////////////////////////////////////////////////////
// 
//     3DES Implementation and optimization 
//     
/////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des3_init_optimize
// Notes    :   - 3DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des3_init_optimize(StDesOptimize *desoptim)
{
	memset(desoptim, 0, sizeof(*desoptim));
	desoptim->Init = 0;// 0xa5a5a5L;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des3_init
// Notes    :   - 3DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des3_init(StDes3Optimize *des3)
{
	des3_init_optimize(&des3->DesArray[0]);
	des3_init_optimize(&des3->DesArray[1]);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des3_xor
// Notes    :   - 3DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des3_xor(uint8_tw *In1, uint8_tw *In2, uint8_tw *OuT, int32_tw len)
{
	int32_tw i;
	for (i = 0; i < len; i++)
	{
		OuT[i] = In1[i] ^ In2[i];
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des3_find_optimized_key
// Notes    :   - 3DES Internal implementation -
//              check if key was loaded for optimization
// return :     0 - key was found
//              Number - key was not found so return the next empty index 
///////////////////////////////////////////////////////////////////////////////////////

int32_tw des3_find_optimized_key(StDesOptimize *desoptim, int8_tw *Key)
{
	int32_tw i;
	i = desoptim->indexLocalKey;
	if (desoptim->Init == DES3_INIT_CODE)
	{
		for (desoptim->indexLocalKey = 0; desoptim->indexLocalKey < DES3_REMEMBER_KEY; desoptim->indexLocalKey++)
		{
			if (memcmp(Key, desoptim->LocalKey[desoptim->indexLocalKey], 8) == 0)
			{
				return -1;
			}
		}
	}
	if ((i + 1) == DES3_REMEMBER_KEY)
	{
		i = 0;
	}
	else
	{
		i++;
	}
	desoptim->indexLocalKey = i;
	return desoptim->indexLocalKey;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des3_load_optimized_key
// Notes    :   - 3DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des3_load_optimized_key(StDesOptimize *desopt, int8_tw *Key)
{
	// Make optimization
	if (des3_find_optimized_key(desopt, Key) == -1)
	{
		return;
	}

	// Modify Init to YES
	desopt->Init= DES3_INIT_CODE;

	// Store key
	memcpy(desopt->LocalKey[desopt->indexLocalKey], Key, 8);

	// Cooked key permutation for encrypt
	des_deskey((uint8_tw*)desopt->LocalKey[desopt->indexLocalKey], e_Encrypt);

	// Copy the permutation from device (d3des.h)
	des_cpkey(desopt->EncryptCook[desopt->indexLocalKey]);

	// Cooked key permutation for decrypt
	des_deskey(desopt->LocalKey[desopt->indexLocalKey], e_Decrypt);

	// Copy the permutation from device (d3des.h)
	des_cpkey(desopt->DecryptCook[desopt->indexLocalKey]);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des3_load_key
// Notes    :   - 3DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des3_load_key(StDes3Optimize *des3, int8_tw *Key)
{
	// Load the left key
	des3_load_optimized_key(&des3->DesArray[0], Key);
	// Load the right key
	des3_load_optimized_key(&des3->DesArray[1], Key + 8);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des3_optimized_des
// Notes    :   - 3DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des3_optimized_des(StDesOptimize *desoptim, int8_tw *InData, int8_tw *OutData, int8_tw *Key, e_Oparation Operation)
{
	CookedKey *arry[2];

	// Load key if need
	des3_load_optimized_key(desoptim, Key);

	// Permutations arrays for enc/decr
	arry[0] = &desoptim->EncryptCook[desoptim->indexLocalKey];
	arry[1] = &desoptim->DecryptCook[desoptim->indexLocalKey];

	// Store cooked key in driver pool
	des_usekey(*arry[Operation]);

	// Calculate des
	des((uint8_tw*)InData, (uint8_tw*)OutData);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function :   des3
// Notes    :   - 3DES Internal implementation -
//
///////////////////////////////////////////////////////////////////////////////////////

void des3(StDes3Optimize *des3, int8_tw *InData, int8_tw *OutData, int8_tw *Key, e_Oparation Operation)
{
	int8_tw SaveIn[8];

	memcpy(SaveIn, InData, 8);

	// Load key
	des3_load_key(des3, Key);

	// Left key
	des3_optimized_des(&des3->DesArray[0], InData, OutData, Key, Operation);
	// Right key
	des3_optimized_des(&des3->DesArray[1], OutData, InData, Key + 8, INV(Operation));
	// Left key
	des3_optimized_des(&des3->DesArray[0], InData, OutData, Key, Operation);

	// Restore output
	memcpy(InData, SaveIn, 8);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : des3_encipher
// Notes    : see h file, ciphertext length must be ptlen!
//
///////////////////////////////////////////////////////////////////////////////////////

int32_tw des3_encipher(const uint8_tw *plaintext,
	int32_tw ptlen,
	uint8_tw *key,
	uint8_tw *ciphertext)
{
	int32_tw  encryptedlen  = 0;
	uint8_tw *pt_ptr       = (uint8_tw *)plaintext;
	uint8_tw *ct_ptr       = ciphertext;

	// Make sure we have valid params
	if (!plaintext || !key || !ciphertext)
	{
		return 0;
	}

	if (Obj_Des3.DesArray[0].Init != DES3_INIT_CODE || Obj_Des3.DesArray[1].Init != DES3_INIT_CODE)
	{
		des3_init(&Obj_Des3);
	}

	// Get the needed buffer length for the output  
	if (ptlen % 8)
	{
		return 0;
	}

	// preform des3 
	while (encryptedlen < ptlen)
	{
		des3(&Obj_Des3, (int8_tw*)pt_ptr, (int8_tw*)ct_ptr, (int8_tw*)key, e_Encrypt);
		pt_ptr += 8;
		ct_ptr += 8;
		encryptedlen += 8;
	}

	return (int32_tw)encryptedlen;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : des3_decipher
// Notes    : see h file, plaintext length must be ctlen!
//
///////////////////////////////////////////////////////////////////////////////////////

int32_tw des3_decipher(const uint8_tw *ciphertext,
	int32_tw ctlen,
	uint8_tw *key,
	uint8_tw *plaintext)
{
	int32_tw  decryptedlen  = 0;
	uint8_tw *pt_ptr       = plaintext;
	uint8_tw *ct_ptr       = (uint8_tw *)ciphertext;

	// Make sure we have valid params
	if (!plaintext || !key || !ciphertext)
	{
		return 0;
	}

	if (Obj_Des3.DesArray[0].Init != DES3_INIT_CODE || Obj_Des3.DesArray[1].Init != DES3_INIT_CODE)
	{
		des3_init(&Obj_Des3);
	}

	// Get the needed buffer length for the output  
	if (ctlen % 8)
	{
		return 0;
	}

	// preform des3 
	while (decryptedlen < ctlen)
	{
		// preform des3 
		des3(&Obj_Des3, (int8_tw*)ct_ptr, (int8_tw*)pt_ptr, (int8_tw*)key, e_Decrypt);
		pt_ptr += 8;
		ct_ptr += 8;
		decryptedlen += 8;
	}   

	return (int32_tw)decryptedlen;
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : des3_mac_with_residue
// Notes    : signature  with length that not divide by 8 and with Various signature 
//            length
//
///////////////////////////////////////////////////////////////////////////////////////

void des3_mac_with_residue(uint8_tw *plaintext,       // [IN] the plan text
	uint8_tw lenDataIn,        // [IN] the data length
	uint8_tw key[16],          // [IN] the key
	e_CryptoSignType SignType,      // [IN] the signature type
	uint8_tw *SignitureResult) //[OUT] the result
{
	uint8_tw Tmp[8];
	uint8_tw Signiture[8];

	// Make sure we have valid parameters
	if (!plaintext || !SignitureResult)
	{
		return;
	}

	// do the mac  into Signature
	des_mac_with_residue(plaintext, lenDataIn, key, Signiture);

	// do des -1  with the right key
	des_decipher(Signiture, Tmp, key + 8);
	// do des   with the left key
	des_encipher(Tmp, Signiture, key);

	memcpy(SignitureResult, Signiture, SignType);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : crc32_compute
// Notes    : Return a 32-bit CRC of the contents of the buffer
//
///////////////////////////////////////////////////////////////////////////////////////

uint32_tw crc32_compute(const void *buf, uint32_tw bufLen)
{
	uint32_tw crc32;
	uint32_tw i;
	uint8_tw *byteBuf;

	// Accumulate crc32 for a buffer 
	crc32 = 0 ^ 0xFFFFFFFF;
	byteBuf = (uint8_tw*)buf;

	for (i = 0; i < bufLen; i++) 
	{
		crc32 = (crc32 >> 8) ^ crcTable[(crc32 ^ byteBuf[i]) & 0xFF];
	}

	return (crc32 ^ 0xFFFFFFFF);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Function : crc32_check
// Notes    : Validates a crc32
// Returns  : 0 - Error
//            1 - Successes
//
///////////////////////////////////////////////////////////////////////////////////////

uint8_tw crc32_check(const void *buf, uint32_tw bufLen, uint32_tw crc32)
{
	if (crc32_compute(buf, bufLen) == crc32)
	{
		return eCoreTrue;
	}
	else
	{
		return eCoreFalse;
	}
}
#endif // CORE_SUPPORT_LIBCRYPTO
