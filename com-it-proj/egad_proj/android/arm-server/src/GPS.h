/*
 * GPS.h
 *
 *  Created on: Jul 3, 2014
 *      Author: doronsa
 */

#ifndef GPS_H_
#define GPS_H_

#include <time.h>

void gps_setup();
void  gps_data();
int get_day();
int get_month();
int get_year();
int get_hour();
int get_min();
int get_sec();
float get_latitude();
float get_longitude();
float get_longitude();
time_t gpsTimeToArduinoTime();
time_t gpsTimeSync();

#endif /* GPS_H_ */
