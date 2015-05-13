/*
 * gpio_util.h
 *
 *  Created on: Jan 19, 2014
 *      Author: doronsa
 */

#ifndef GPIO_UTIL_H_
#define GPIO_UTIL_H_

int ExportGPIO( int gpio);
int SetDirection( int gpio, int direction );
int SetGpioValue( int gpio, int value);
int GetGpioValue( int gpio);
int gpio_unexport(unsigned int gpio);
int gpio_fd_open(unsigned int gpio);
int gpio_fd_close(int fd);
int gpio_set_edge(unsigned int gpio, char *edge);
#define NUM_OFF_GPIO 2
typedef struct
{
	int gpio_num ;
	int value;
	int SendOK;
	int cmd;
}GPIOSetup;

typedef enum
{
	e_BSPKeyBoardled = 234,//befor enum off calypso=60 and after events k10  K10 send event =33
	e_BSPKeyBoardSetup,
	e_BSPGPIO_1,
	e_BSPGPIO_2,
	e_BSPGPIO_3,
	e_BSPGPIO_4,
	e_BSPGPIO_5,
	e_BSPGPIO_6,
	e_BSPGPIO_7,
	e_BSPGPIO_8,
	e_BSPGPIO_9,
	e_BSP_GET_VERSION,
	e_BSPLaset,//  Mast be last

}e_BSPcommand;

typedef enum
{
 e_CmdGPIOSetOn=2,
 e_CmdGPIOSetOff=3
}e_GPIOCmd;
#endif /* GPIO_UTIL_H_ */
