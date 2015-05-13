#ifndef BSP_UTIL_H_
#define BSP_UTIL_H_

//void timer_handler (int sig);
void init_timer(void);
void start_timer(long time);
void stop_timer(void);
int InitGPIOuart( void );
void GetLocation(int num);
void *GPS_Recive_message_function( void *ptr );
#endif
