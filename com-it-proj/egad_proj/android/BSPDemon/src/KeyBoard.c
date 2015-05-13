/*
 * KeyBoard.c
 *
 *  Created on: Jan 30, 2014
 *      Author: doronsa
 */

#include "KeyBoard.h"
//#include <linux/i2c-dev.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include"/home/doron-linux/workspace/var_jb_422_110/jb_422_110_build/kernel_imx/include/linux/i2c-dev.h"
#include "i2c-dev.h"
#include "P_TwMtrBase.h"
#include "i2cbusses.h"
//#include <linux/i2c-dev.h>

#define I2C_BUS 0
#define I2C_SMBUS_BYTE_DATA	    2
#define KEYBOARDADD 0x38
#define LEDADD 0x50//

/******************************************************************
 *  Function name: GetKey
 *  Description: Get Key Value
 *  in
 *  out
 ******************************************************************/
int GetKey(void)
{
	return GetVal(0);
}

/******************************************************************
 *  Function name: InitKeyBoard
 *  Description: init the I2C port for the keyboard
 *  in
 *  out
 ******************************************************************/
int InitKeyBoard( void )
{
	printf("InitKeyBoard \n");
	int i2cbus = 1;//lookup_i2c_bus((const char)I2C_BUS);
	printf("InitKeyBoard i2cbus %hhx\n",i2cbus);
	int address = KEYBOARDADD;//parse_i2c_address((const char)KEYBOARDADD);
	printf("InitKeyBoard address %hhx\n",address);
	int size = I2C_SMBUS_BYTE_DATA;//I2C_SMBUS_BYTE;
	char filename[20];
	DeviceFd.KeyBoard = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
	printf("InitKeyBoard KeyBoard hndel= %x\n",DeviceFd.KeyBoard);
	set_slave_addr(DeviceFd.KeyBoard, address, 1);
	if (DeviceFd.KeyBoard < 0 )
	{
		printf("cannot open i2c port\n");
		//exit(1);
	}
	/* init key board REG */
	printf("Set Controler Reg\n");
	SetVal(0x2 ,0x0);
	SetVal(0x4 ,0xfe);
	SetVal(0x40 ,0x10);
	SetVal(0x41 ,0xff);
	SetVal(0x43 ,0x2);
	SetVal(0x44 ,0xff);
	SetVal(0x45 ,0xff);
	printf("Set LED Reg\n");
	SetLED(0,0xff);
	printf("InitKeyBoard end\n");
	return DeviceFd.KeyBoard;

}

/******************************************************************
 *  Function name
 *  Description
 *  in
 *  out
 ******************************************************************/
int SetSubSystem( int reg )
{
	return 0;
}

/******************************************************************
 *  Function name
 *  Description
 *  in
 *  out
 ******************************************************************/
void closedev(void)
{

}

/******************************************************************
 *  Function name: SetLED
 *  Description: Init The KeyBoard LED
 *  in
 *  out
 ******************************************************************/
int SetLED(int lednum, int val)
{
	SetVal(0x50 ,val);
	SetVal(0x51 ,val);
	SetVal(0x52 ,val);
	SetVal(0x53 ,val);
	SetVal(0x54 ,val);
	SetVal(0x55 ,val);
	SetVal(0x56 ,val);
	SetVal(0x57 ,val);
	return(0);
}

/******************************************************************
 *  Function name: GetVal
 *  Description: Get the KeyBoard pressed
 *  in
 *  out
 ******************************************************************/
int GetVal(int reg)
{
	char *end;
	int res, i2cbus, address, size, file;
	int daddress;
	char filename[20];
	int pec = 0;
	int flags = 0;
	int force = 1, yes = 1 ;
	size = I2C_SMBUS_BYTE_DATA;//I2C_SMBUS_BYTE;
	daddress = reg;
    if (DeviceFd.KeyBoard==0)
    {
    	fprintf(stderr, "no hendel failed\n");
    }
	switch (size) {
	case I2C_SMBUS_BYTE:
		if (daddress >= 0) {
			res = i2c_smbus_write_byte(DeviceFd.KeyBoard, daddress);
			if (res < 0)
				fprintf(stderr, "Warning - write failed\n");
		}
		res = i2c_smbus_read_byte(DeviceFd.KeyBoard);
		break;
	case I2C_SMBUS_WORD_DATA:
		res = i2c_smbus_read_word_data(DeviceFd.KeyBoard, daddress);
		break;
	default: /* I2C_SMBUS_BYTE_DATA */
		res = i2c_smbus_read_byte_data(DeviceFd.KeyBoard, daddress);
	}
	close(file);
	//printf("0x%0*x\n", size == I2C_SMBUS_WORD_DATA ? 4 : 2, res);

	return (res);
}

/******************************************************************
 *  Function name: SetVal
 *  Description: send data to the KeyBoard REG
 *  in
 *  out
 ******************************************************************/
int SetVal( int addreg,int value)
{
	char *end;
	const char *maskp = NULL;
	int res, i2cbus, address, size, file;
	int daddress, vmask = 0;
	char filename[20];
	int pec = 0;
	int flags = 0;
	unsigned char block[I2C_SMBUS_BLOCK_MAX];
	int len;
	size = I2C_SMBUS_BYTE_DATA;
	/* handle (optional) flags first */
	i2cbus = 0;
	address = KEYBOARDADD;

	daddress = addreg;
	len = 0; /* Must always initialize len since it is passed to confirm() */

	file = DeviceFd.KeyBoard;//open_i2c_dev(i2cbus, filename, sizeof(filename), 0);

	res = i2c_smbus_write_byte_data(file, daddress, value);

	if (res < 0) {
		fprintf(stderr, "Error: Write failed\n");
	}

	switch (size) {
	case I2C_SMBUS_BYTE:
		res = i2c_smbus_read_byte(file);
		value = daddress;
		break;
	case I2C_SMBUS_WORD_DATA:
		res = i2c_smbus_read_word_data(file, daddress);
		break;
	default: /* I2C_SMBUS_BYTE_DATA */
		res = i2c_smbus_read_byte_data(file, daddress);
	}

	if (res < 0) {
		printf("Warning - readback failed\n");
	} else
	if (res != value) {
		printf("Warning - data mismatch - wrote "
		       "0x%0*x, read back 0x%0*x\n",
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, value,
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, res);
	} else {
		printf("Value 0x%0*x written, readback matched\n",
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, value);
	}

	usleep(3000);
	return(0);
}
