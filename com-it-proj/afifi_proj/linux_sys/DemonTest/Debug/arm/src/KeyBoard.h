/*
 * KeyBoard.h
 *
 *  Created on: Jan 30, 2014
 *      Author: doronsa
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_


int InitKeyBoard( void );
//void free_adapters(struct i2c_adap *adapters);
//int set_slave_addr(int fd, int address, int force);
int SetSubSystem( int reg );
void closedev(void);
int SetVal( int addreg,int val);
int getVal( int reg ,int module);
int SetLED(int lednum, int val);
int GetKey(void);
typedef enum
{
 e_CmdKeyBoardSetLedOn=0,
 e_CmdKeyBoardSetLedOff=1
}e_KeyBoardLedCmd;

#endif /* KEYBOARD_H_ */
