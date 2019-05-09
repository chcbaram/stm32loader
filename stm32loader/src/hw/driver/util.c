/*
 * util.c
 *
 *  Created on: 2018. 6. 1.
 *      Author: HanCheol Cho
 */


#include "util.h"



void delay(uint32_t delay_ms)
{
  uint32_t time_pre;

  time_pre = millis();
  while(1)
  {
    if (millis()-time_pre >= delay_ms)
    {
      break;
    }
  }
}


#if defined (__WIN32__) || (__WIN64__)
uint32_t millis(void)
{
  double ret;

  LARGE_INTEGER freq, counter;

  QueryPerformanceCounter(&counter);
  QueryPerformanceFrequency(&freq);

  ret = (double)counter.QuadPart / (double)freq.QuadPart * 1000.0;

  return (uint32_t)ret;
}

uint32_t micros(void)
{
  LARGE_INTEGER freq, counter;
  double ret;

  QueryPerformanceCounter(&counter);
  QueryPerformanceFrequency(&freq);

  ret = (double)counter.QuadPart / (double)freq.QuadPart * 1000000.0;

  return (uint32_t)ret;
}

#endif


#if defined (__linux__)

 #include <stdio.h>
 #include <fcntl.h>
 #include <string.h>
 #include <unistd.h>
 #include <termios.h>
 #include <time.h>
 #include <sys/time.h>
 #include <sys/ioctl.h>


uint32_t millis(void)
{
  double ret;
  struct timespec tv;
  clock_gettime( CLOCK_REALTIME, &tv);


  ret = ((double)tv.tv_sec*1000.0 + (double)tv.tv_nsec*0.001*0.001);

  return (uint32_t)ret;
}
#endif
