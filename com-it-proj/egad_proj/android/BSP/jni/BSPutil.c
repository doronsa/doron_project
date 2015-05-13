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
#include <android/log.h>
#include "BSPutil.h"
#define  LOG_TAG    "BSP"

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

timer_t gTimerid;

//void timer_handler (int sig)
//{
//	static int count = 0;
//	LOGI("timer expired %d times\n", ++count);
//	stop_timer();
//}
void init_timer(void)
{
	timer_create (CLOCK_REALTIME, NULL, &gTimerid);
}

void start_timer(long int time)
{
	long time_temp = time ;//* 1000;
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


