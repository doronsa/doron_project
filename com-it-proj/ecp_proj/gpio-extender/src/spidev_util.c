/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define IODIR 		0x00
#define IPOL 		0x01
#define GPINTEN 	0x02
#define DEFVAL 		0x03
#define INTCON 		0x04
#define IOCON 		0x05
#define GPPU 		0x06
#define INTF 		0x07
#define INTCAP 		0x08
#define GPIO 		0x09
#define OLAT 		0xA
#define DELAY 		1000000

#define VERSION    1.1
static void pabort(const char *s)
{
	perror(s);
	abort();
}

//static char *device ;//= "/dev/spidev0.0";
char device[40];
static uint32_t mode = SPI_MODE_0;
static uint8_t bits = 8;
static uint32_t speed = 500000;

static uint16_t delay;

static uint16_t data1;
static uint16_t data2;



static void transfer(int fd ,char add, char reg, int data)
{
	int ret;
	uint8_t tx[4];

	tx[0] = add;
	tx[1] = reg;
	tx[2] = data;
	tx[3] = data >> 8;
        //printf("data send %x, %x, %x, %x,\n",tx[0],tx[1],tx[2],tx[3]);
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };
	struct spi_ioc_transfer tr =
	{
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");
/*
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx[ret]);
	}
	puts("");*/
}

static char read_spi(int fd ,char add, char reg )
{
	int ret;
	uint8_t tx[4];

	tx[0] = add;
	tx[1] = reg;
	tx[2] = 0;
	tx[3] = 0;
        //printf("data send %x, %x, %x, %x,\n",tx[0],tx[1],tx[2],tx[3]);
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };
	struct spi_ioc_transfer tr =
	{
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

	return (rx[2]);
}



//arg1 = CS
//arg2 = port A I/O
//arg3 = port B I/O
//arg4 = Port A data
//arg5 = Port B data
int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;
	sprintf(device, "/dev/spidev0.%d", 0);
    int PortAread = 0;
    int PortBread = 0;
	data1 = 0 ;
	data2 = 0 ;

          
	if(argc > 1)
	  sprintf(device, "/dev/spidev0.%s", argv[1]);
	if(argc > 2)
	   PortAread = strtol(argv[2], NULL, 16);
	if(argc > 3)
	   PortBread = strtol(argv[3], NULL, 16);
	if(argc > 4)
	   data1 = strtol(argv[4], NULL, 16);
	if(argc > 5)
	   data2 = strtol(argv[5], NULL, 16);


	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

    //************************************************************************************//


    // SET direction PORT A
    if (PortAread == 0)
	{
		transfer(fd ,0x40, 0, 0);//dir port a
		usleep(DELAY);
		transfer(fd ,0x40, 0x12, data1);//set port a
	}else
	{
		transfer(fd ,0x40, 0, 0xFF);//dir port a
		usleep(DELAY);
		data1 = read_spi(fd ,0x41, 0x12 );	
	}


	// SET direction PORT B
    if (PortBread == 0)
	{
		transfer(fd ,0x40, 0x01, 0);//dir port a
		usleep(DELAY);
		transfer(fd ,0x40, 0x13, data2);//set port a
		
	}else
	{
		transfer(fd ,0x40, 0x01, 0xFF);//dir port a
		usleep(DELAY);
		data2 = read_spi(fd ,0x41, 0x13 );	
	}
		//printf("spi device: %s\n", device);
		if (PortAread == 1)
			printf("PORT A : 0x%2x\n", data1);
		if (PortBread == 1)
			printf("PORT B : 0x%2x\n", data2);
	return 1;
}
