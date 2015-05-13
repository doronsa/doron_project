/*
 * gpio_util.h
 *
 *  Created on: Jan 19, 2014
 *      Author: doronsa
 */

#ifndef GPIO_UTIL_H_
#define GPIO_UTIL_H_

void ExportGPIO( int gpio);
void SetDirection( int gpio, int direction );
void SetGpioValue( int gpio, int value);
int GetGpioValue( int gpio );
void UnExportGPIO( int gpio);
typedef enum
{
 e_CmdGPIOSetOn=2,
 e_CmdGPIOSetOff=3
}e_GPIOCmd;
#endif /* GPIO_UTIL_H_ */
