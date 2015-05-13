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
//
#include "P_TwMtrBase.h"
//

#define LINUX

#ifndef LINUX
#include "i2c-dev.h"
#include "i2cbusses.h"
#endif
#define I2CNUMBER 2
#define I2C_SMBUS_BYTE_DATA	    2
#define KEYBOARDADD 0x38
#define LEDADD 1234
#if 0
int InitKeyBoard( void )
{
	int fd;
#ifndef LINUX
	fd = open("/dev/i2c-0", O_RDWR | O_NOCTTY | O_NDELAY);

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
	getVal(0,0);
}


int InitKeyBoard(void)
{
    //char *end;
    int i2cbus, address;
    //int daddress;
    char filename[20];
   // int pec = 0;
    //int flags = 0;
    int force = 1;
    int file=0;

    //size = I2C_SMBUS_BYTE_DATA;
    i2cbus = 0;//lookup_i2c_bus(TWL4030ADD);
    address = KEYBOARDADD;
    //daddress = LEDADD;//parse_i2c_address(KEYPADS1TOS3);

#ifndef LINUX
    DeviceFd.KeyBoard = open_i2c_dev(i2cbus, filename, 0);
    if (DeviceFd.KeyBoard < 0 || set_slave_addr(file, address, force))
    {
            //exit(1);
            //send_error("UDP server");
    }
#endif
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
#ifndef LINUX
	res = i2c_smbus_write_byte_data(DeviceFd.KeyBoard, addreg, val);
	if (res < 0)
    {
            fprintf(stderr, "Error: write failed\n");
            //exit(2);
    }
#endif
    return (1);
}
int getVal( int reg ,int module)
{
    int res = 0;
#ifndef LINUX
    res = i2c_smbus_read_byte_data(DeviceFd.KeyBoard, reg);
    usleep(100);
    if (res < 0)
    {
            fprintf(stderr, "Error: Read failed\n");
            //exit(2);
    }
#endif
    return (res);
}
