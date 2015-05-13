/*
 ============================================================================
 Name        : BSPutil.c.c
 Author      : doron sandroy
 Version     :
 Copyright   : Your copyright notice
 Description : timer util
 ============================================================================
 */
#include "BSPutil.h"
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>


//#include <android/log.h>

//#define  LOG_TAG    "BSP"

timer_t gTimerid;

//void timer_handler (int sig)
//{
//	static int count = 0;
//	printf("timer expired %d times\n", ++count);
//	stop_timer();
//}



void init_timer(void)
{
	timer_create (CLOCK_REALTIME, NULL, &gTimerid);
}

void start_timer(long time)
{
	long time_temp = time * 1000*1000;
	struct itimerspec value;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = time_temp;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = time_temp;
	timer_settime (gTimerid, 0, &value, NULL);
}

void stop_timer(void)
{
	struct itimerspec value;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 0;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;
	timer_settime (gTimerid, 0, &value, NULL);
	//timer_delete(gTimerid);
}



