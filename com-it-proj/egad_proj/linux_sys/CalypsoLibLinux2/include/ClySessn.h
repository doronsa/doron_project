
///////////////////////////////////////////////////////////////////////////////////////
//
//     Abstract: ClySessn.h
//         
//         
//
///////////////////////////////////////////////////////////////////////////////////////

#ifndef CLYSESSION_H
    #define CLYSESSION_H

#include <ClyCrdOs.h>

//////////////////////////////////////////////////////////////////////////////
//
//                                     DEFINES
//
//////////////////////////////////////////////////////////////////////////////

#define IS_RES_OK(obj) (obj && obj->sw1_sw2[0]==0x90 && obj->sw1_sw2[1]==0 )

//////////////////////////////////////////////////////////////////////////////
//
//                                     API
//
//////////////////////////////////////////////////////////////////////////////


RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_OpenSecureSession(e_7816_DEVICE CardReaderId,      // [IN]  card reader id
	e_7816_DEVICE SamReaderId,                                                                    // [IN]  sam reader id
	St_clyCard_OpenSessionInput *St_OpenSessionInput,                                   // [IN] Open Session Input parameters
	e_clySam_KeyAccessType KeyAccessType,                                               // [IN] choose SAM access type -  index \ KIF+KVC  are only available  
	Union_clySam_WorKeyParamsAcess SessionWorkKey,                                      // [IN] the SAM session work key ( index \ KIF+KVC ) found in the sam work keys												
	St_clyCard_OpenSessionOutput *St_OpenSessionOutput);                                // [OUT]Open Session output parameters

// Close Secure Session
RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_CloseSecureSession(e_7816_DEVICE CardReaderId,     // [IN]  card reader id
	e_7816_DEVICE SamReaderId,                                                                    // [IN]  sam reader id
	clyCard_BOOL b_IsRatifyImmediatly                                                   // [IN] //1= the session will be immediately Ratified
	);

// Change keys
RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_ChangeKeys(
	e_7816_DEVICE CardReaderId,                                                                   // [IN]  card reader id
	e_7816_DEVICE SamReaderId,                                                                    // [IN]  sam reader id
	e_clyCard_KeyType keytype,                                                          // [IN] type of key
	st_CipherDataCard s_CipherDataCard
	);

//Write Sam Data
RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_WriteSamData (e_7816_DEVICE SamTargReaderId,       // [IN]  target sam reader id
	e_7816_DEVICE SamControlReaderId,                                                             // [IN]  control sam reader id
	e_clySam_WriteDataMode WriteMode,                                                   // Dynamic or static mode
	e_clySam_WriteDataSamP2 e_WriteDataSamP2,                                           // [IN] enum to fill P2
	st_clySam_CipherDataSam s_CipherDataSam);                                           // [IN] struct to fill command parameters (PACKET_7816)


// Add value to ceiling service
RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_SamClCeilingUpdate(
	e_7816_DEVICE CardReaderId,                                                                   // [IN] card reader id
	e_7816_DEVICE SamReaderId,                                                                    // [IN] sam reader id
	unsigned long ul_Val2Add);                                                          // [IN] Value to add to ceiling

// Read Remain value :  ceiling value - counter value = Remain value
RESPONSE_OBJ* CLY_CARD_STDCALL  pSt_ClySession_SamClCeilingRemain(
	e_7816_DEVICE CardReaderId,                                                                   // [IN]  card reader id
	e_7816_DEVICE SamReaderId,                                                                    // [IN]  sam reader id
	unsigned long *ulp_ValRemain);                                                      // [OUT] ceiling Value Remain


// Terminallogging helper
void ClyTerminalDebug(char *pFunctionName, PACKET_7816  *pReq, RESPONSE_OBJ *p_Answer);

#endif // CLYSESSION_H
