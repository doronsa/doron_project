/*
 * gpio_util.c
 *
 *  Created on: Jan 19, 2014
 *      Author: doronsa
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#define DEBUG
/*
 * from SOMIDD-A2
 * 23	IN  GPIO[5]12	IOEXTPWR.ACCSNS	  Ignition detect 	IN	High	Hi-Z
 * 124	OUT GPIO[5]30	CELL_ON			  Ignition Cellular Modem Pulse >10s = Low-High-Low	Out			ignition
 * 17	PWM PWM0	    GLCD_BL_CTL		  PWM LCD 7' Backlight - light power depends on light sensor	PWM			50%	** (200HZ, can be constant)
 * 173	IN  GPIO[6]2	Prt_MMI.P_SNS_SDA Miniature Transmissive Photomicrosensor paper present on the printer out lips (can be recipt that didn’t taken by customer)	IN	high	low	hi-z
 * 120	OUT GPIO[5]21	USDSOM_SDOE		  uSD output enable enable the signals to uSD when OE=high	out	high	low	high
 * 81	OUT GPIO5[19] 	CPT_RST			  Reset pin fot Capacirve touch screen	Out
 * 42	IN  BOOT_SEL0	CPT_INT			  Interrupt from capacitive 	in
 * 48	OUT GPIO4[10] 	K10_nRST		  Reset K10	out
 * 73	IN  GPIO1[21] 	D_MMI2MB.KEY_NINT key interrupt for driver hard keys	in	low	high
 * 22	IN  GPIO5[17] 	DRV_reader.HALL1  hall sensor input indicates driver module present	in	?			*	(need to read before enabling authentication to appliction
 * 69	IN  GPIO1[18] 	Prt_MMI.HALL2	  hall sensor input indicates printer drawer opent					*
 * 77	OUT GPIO3[22]	Prt_MMI.LED_G_CTL light the lips of the presentor paper of the printer	out	high	Hi-Z		*
 * 24	OUT GPIO5[16] 	Prt_MMI.nPRESET	  Reset pin fot the CPU printer 	Out	low	hi-z		*
 * 175	OUT GPIO[5]31	SPKEN			  speaker audio amp. Enable	out	high	low	high
 *
 * */
void ExportGPIO( int gpio)
{
	int fd;
	char buf[10];
	fd = open("/sys/class/gpio/export", O_WRONLY);
	sprintf(buf, "%d", gpio);
	write(fd, buf, strlen(buf));
	close(fd);
}

void SetDirection( int gpio, int direction )
{
	int fd;
	char buf[100];
	sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);
	fd = open(buf, O_WRONLY);
	// Set out direction
	write(fd, "out", 3);
	// Set in direction
	write(fd, "in", 2);

	close(fd);
}

void SetGpioValue( int gpio, int value)
{
	int fd;
	char buf[100];
	sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
	fd = open(buf, O_WRONLY);

	if (value)
	{
		// Set GPIO high status
		write(fd, "1", 1);
	}
	else
	{
		// Set GPIO low status
		write(fd, "0", 1);
	}

	close(fd);
}

int GetGpioValue( int gpio )
{
	int fd;
	char buf[100];
	char value;
	sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
	fd = open(buf, O_RDONLY);
	read(fd, &value, 1);
	if(value == '0')
	{
	     // Current GPIO status low
		return (0);
	}
	else
	{
	     // Current GPIO status high
		return (1);
	}

	close(fd);
	return (1);
}


void UnExportGPIO( int gpio)
{
	int fd;
	char buf[100];
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	sprintf(buf, "%d", gpio);
	write(fd, buf, strlen(buf));
	close(fd);
}
