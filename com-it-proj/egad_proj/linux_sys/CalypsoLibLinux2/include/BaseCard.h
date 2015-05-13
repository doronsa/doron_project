 /******************************************************************************
 *
 *    ORIGINAL PATH: D:\Projects\CALYPSO\TR1000\src\TR1000\SRC\CCard.h
 *
 *    DESCRIPTION: CCard(R)(aka CALYPSO Memory Cards) Support Interfaces.
 *
 *    AUTHOR:   Igor.
 *
 *    HISTORY: Created 02/01/2006.   
 *
 ******************************************************************************/
#ifndef CCARD_H
#define CCARD_H

#define MAX_SERNUM_SIZE			10
#define MAX_NUM_SELECTED_CARDS  2
#define  SND_BUF_LEN			50
#define  RCV_BUF_LEN        50//30   // Maximal Receive buffer length
#define ISO_B_CARD                     0x30
#define TA1_DEFAULT_DsDr        0x80
#define TB1_DEFAULT_FWT_SFGI    0x90///0x40//SFGI b1-b4=0,FWT b5-b8=4 





typedef struct
{
  unsigned char   CardType                :8;
  unsigned char SerNumSize                :8;
  unsigned char     SerNum [MAX_SERNUM_SIZE];
  unsigned char       FLAG                :1;
  unsigned char         T0                :8;
  unsigned char        TA1                :8;
  unsigned char        TB1                :8;
  unsigned char        TC1                :8;
} st_CardData;


/*============================================================================*/
/*                             P U B L I C   D A T A                          */
/*============================================================================*/
extern st_CardData IsoCardsData[MAX_NUM_SELECTED_CARDS];
extern unsigned char MSndBuffer[SND_BUF_LEN];


/*============================================================================*/
/*                        P U B L I C   F U N C T I O N S                     */
/*============================================================================*/
char HostCmdCCard_HaltB(void);
char HostCmdCCard_SelectAll(unsigned char cmd,unsigned char *RecData, unsigned short * RecLen);
char HostCmdCCard_SelectIdle(unsigned char cmd,unsigned char *RecData, unsigned short * RecLen); 

#endif


