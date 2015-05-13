/*
 * dcu_util.h
 *
 *  Created on: Mar 10, 2014
 *      Author: doronsa
 */
#include "P_TwMtrBase.h"
#include "TW_K10232_p.h"
#ifndef DCU_UTIL_H_
#define DCU_UTIL_H_
int DCU_SerialPortSearch(void);
int DCU_SerialPortInit(void);
void CloseDCUUart(void);
int AddDCUQue( St_ProtocolHeader *sSDUProtocolHeader, void *data );
void *DCURecive_message_function( void *ptr );
int DCUData_Decode( int cmd, int TypeRespond, void *data,int AplicationStatus, int data_len);


#endif /* DCU_UTIL_H_ */
