/*
 * gpio_util.c
 *
 *  Created on: Jan 19, 2014
 *      Author: doronsa
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <android/log.h>
#include "P_TwMtrBase.h"
#include "gpio_util.h"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64
#define  LOG_TAG    "BSP"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define DEBUG
/*
 * from SOMIDD-A2
 * GPIO[7]_12 corresponds to (7-1)*32 + 12 =  204 GPIO number in Linux
 * 23	IN  GPIO[5]12=140	IOEXTPWR.ACCSNS	  Ignition detect 	IN	High	Hi-Z
 * 124	OUT GPIO[5]30=158	CELL_ON			  Ignition Cellular Modem Pulse >10s = Low-High-Low	Out			ignition
 * 17	PWM PWM0	    GLCD_BL_CTL		  PWM LCD 7' Backlight - light power depends on light sensor	PWM			50%	** (200HZ, can be constant)
 * 173	IN  GPIO[6]2=162	Prt_MMI.P_SNS_SDA Miniature Transmissive Photomicrosensor paper present on the printer out lips (can be recipt that didn’t taken by customer)	IN	high	low	hi-z
 * 120	OUT GPIO[5]20=148	USDSOM_SDOE		  uSD output enable enable the signals to uSD when OE=high	out	high	low	high
 * 81	OUT GPIO5[19]=147 	CPT_RST			  Reset pin fot Capacirve touch screen	Out
 * 42	IN  BOOT_SEL0	CPT_INT			  Interrupt from capacitive 	in
 * 48	OUT GPIO4[10]=106 	K10_nRST		  Reset K10	out
 * 73	IN  GPIO1[21]=21 	D_MMI2MB.KEY_NINT key interrupt for driver hard keys	in	low	high
 * 22	IN  GPIO5[17]=145 	DRV_reader.HALL1  hall sensor input indicates driver module present	in	?			*	(need to read before enabling authentication to appliction
 * 69	IN  GPIO1[18]=18 	Prt_MMI.HALL2	  hall sensor input indicates printer drawer opent					*
 * 77	OUT GPIO3[22]=86	Prt_MMI.LED_G_CTL light the lips of the presentor paper of the printer	out	high	Hi-Z		*
 * 24	OUT GPIO5[16]=144 	Prt_MMI.nPRESET	  Reset pin fot the CPU printer 	Out	low	hi-z		*
 * 175	OUT GPIO[5]31=159	SPKEN			  speaker audio amp. Enable	out	high	low	high
 *----------------------------------------------------------------------------------------
 *"edge" ... reads as either "none", "rising", "falling", or
		"both". Write these strings to select the signal edge(s)
		that will make poll(2) on the "value" file return.

		This file exists only if the pin can be configured as an
		interrupt generating input pin.

	"active_low" ... reads as either 0 (false) or 1 (true).  Write
		any nonzero value to invert the value attribute both
		for reading and writing.  Existing and subsequent
		poll(2) support configuration via the edge attribute
		for "rising" and "falling" edges will follow this
		setting.
 * */




//extern pthread_mutex_t lock;
//extern GPIOSetup SandGPIOGUI[NUM_OFF_GPIO];

//extern int Sendudp(DataCommand SandDataToKGUI);

/****************************************************************
 * gpio_export
 ****************************************************************/
int ExportGPIO( int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

	return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int SetDirection( int gpio, int direction )
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}

	if (direction)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int SetGpioValue( int gpio, int value)
{
	int fd, len;
	char buf[MAX_BUF];
    printf("SetGpioValue %d value:%d",gpio, value);
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}

	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);
	return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int GetGpioValue( int gpio )
{
	int fd, len;
	char buf[MAX_BUF];
	char ch;
	int value;

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}

	read(fd, &ch, 1);

	if (ch != '0') {
		value = 1;
	} else {
		value = 0;
	}
	LOGI("IN Get GpioValue GPIO %d Value %d",gpio,value);
	close(fd);
	return value;
}

/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int gpio, char *edge)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}

	write(fd, edge, strlen(edge) + 1);
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio_fd_open(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY | O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd)
{
	return close(fd);
}


/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio_unexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/unexport");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

/****************************************************************
 * gpio poling and get value
 ****************************************************************/
int gpio_init(void)
{

}



