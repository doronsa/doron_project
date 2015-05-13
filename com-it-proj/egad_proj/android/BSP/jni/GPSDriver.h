/*
 * GPSDriver.h
 *
 *  Created on: Jul 6, 2014
 *      Author: doronsa
 */

#ifndef GPSDRIVER_H_
#define GPSDRIVER_H_

typedef struct {
	char lat[15] ;
	char lat_s[10] ;
	char lng[15] ;
	char lng_s[10] ;
	char date[10] ;
	char st_num[10];
}GPSdata;

void GetGPSLocation( GPSdata *data);
#endif /* GPSDRIVER_H_ */
