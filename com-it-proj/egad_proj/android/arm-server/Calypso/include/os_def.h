#ifndef OS_DEF_H
 #define OS_DEF_H
#ifdef WIN32
#define linux 0
#else
 #define linux 1

#endif
#ifndef NULL
 #define NULL 0
#endif
#define ANDROID 0
#if ANDROID
 #define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#else
 #define  LOGI(...)
#endif
#endif
