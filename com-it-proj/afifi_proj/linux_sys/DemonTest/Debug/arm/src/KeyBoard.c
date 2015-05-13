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
#include"/media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/var_jb_422_110/jb_422_110_build/kernel_imx/include/linux/i2c-dev.h"
#include "i2c-dev.h"
#include "P_TwMtrBase.h"
#include "i2cbusses.h"

//#define LINUX
#define I2CNUMBER 0
#define I2C_SMBUS_BYTE_DATA	    2
#define KEYBOARDADD 0x38
#define LEDADD 0x50//
#if 0
int InitKeyBoard( void )
{
	int fd;
#ifndef LINUX
	fd = open("/dev/i2c-2", O_RDWR | O_NOCTTY | O_NDELAY);

	if (fd == -1)
	{
		/* Could not open the port */
		fprintf(stderr, "open_port: Unable to open /dev/i2c-0 - %s\n", strerror(errno));
	}
#else
	fd = 1;
#endif
	return (fd);
}
#endif


//struct i2c_adap *gather_i2c_busses(void);

int SetLED(int lednum, int val)
{
	int reg = 1;
	/*need to add case for the reg of the led*/
	SetSubSystem( reg );
	if ( SetVal( KEYBOARDADD, val) == 1)
	{
		return 0;
	}
	return 1;
}

int GetKey(void)
{
	return (getVal(0,0));
}


int InitKeyBoard(void)
{
    int i2cbus, address;
    char filename[20];
    int force = 1;
    int rval=0;
    //size = I2C_SMBUS_BYTE_DATA;
    i2cbus = I2CNUMBER;//lookup_i2c_bus(I2CNUMBER);
    address = KEYBOARDADD;
    //daddress = LEDADD;//parse_i2c_address(KEYPADS1TOS3);


    DeviceFd.KeyBoard = open("/dev/i2c-0", O_RDWR | O_NOCTTY | O_NDELAY);//open_i2c_dev(i2cbus, filename, 0);

    rval = set_slave_addr(DeviceFd.KeyBoard, address, force);
    printf("InitKeyBoard open hndel :%d rval:%d\n",DeviceFd.KeyBoard,rval);
    if (DeviceFd.KeyBoard < 0 || rval)
    {
            exit(1);
    }

}

int SetSubSystem( int reg )
{
	set_slave_addr(DeviceFd.KeyBoard, reg, 1);
	return 1;
}

void closedev(void)
{

}
int SetVal( int addreg,int val)
{
	int res;
	res = i2c_smbus_write_byte_data(DeviceFd.KeyBoard, addreg, val);
	if (res < 0)
    {
            fprintf(stderr, "Error: write failed\n");
            //exit(2);
    }
    return (1);
}
int getVal( int reg ,int module)
{
    int res;
    res = i2c_smbus_read_byte_data(DeviceFd.KeyBoard, reg);
    usleep(100);
    if (res < 0)
    {
            fprintf(stderr, "Error: Read failed\n");
            //exit(2);
    }
    return (res);
}
