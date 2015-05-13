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
