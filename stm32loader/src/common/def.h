/*
 * def.h
 *
 *  Created on: 2018. 6. 1.
 *      Author: HanCheol Cho
 */

#ifndef SRC_COMMON_DEF_H_
#define SRC_COMMON_DEF_H_


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#include "error_code.h"
#include "cmdif/cmdif.h"



#if defined (__linux__)
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

#elif defined (__WIN32__) || (__WIN64__)
#include <Windows.h>

#endif



#define OK                    0


#define _DEF_UART1            0
#define _DEF_UART2            1
#define _DEF_UART3            2
#define _DEF_UART4            3
#define _DEF_UART5            4
#define _DEF_UART6            5


#define _DEF_LEFT             0
#define _DEF_RIGHT            1

#define _DEF_INPUT            0
#define _DEF_INPUT_PULLUP     1
#define _DEF_INPUT_PULLDOWN   2
#define _DEF_OUTPUT           3
#define _DEF_OUTPUT_PULLUP    4
#define _DEF_OUTPUT_PULLDOWN  5

#define _DEF_EXTI_RISING      0
#define _DEF_EXTI_FALLING     1
#define _DEF_EXTI_BOTH        2
#define _DEF_EVT_RISING       3
#define _DEF_EVT_FALLING      4
#define _DEF_EVT_BOTH         5

#define _DEF_FLOAT            2
#define _DEF_HIGH             1
#define _DEF_LOW              0


#define _DEF_TIMER1           0
#define _DEF_TIMER2           1
#define _DEF_TIMER3           2
#define _DEF_TIMER4           3


#define _DEF_I2C1             0
#define _DEF_I2C2             1
#define _DEF_I2C3             2
#define _DEF_I2C4             3

#define _DEF_DXL1             0
#define _DEF_DXL2             1
#define _DEF_DXL3             2
#define _DEF_DXL4             3
#define _DEF_DXL5             4

#define _DEF_BUTTON1          0
#define _DEF_BUTTON2          1
#define _DEF_BUTTON3          2
#define _DEF_BUTTON4          3
#define _DEF_BUTTON5          4

#define _DEF_ADC1             0
#define _DEF_ADC2             1
#define _DEF_ADC3             2
#define _DEF_ADC4             3
#define _DEF_ADC5             4
#define _DEF_ADC6             5
#define _DEF_ADC7             6
#define _DEF_ADC8             7

#define _DEF_DAC_CH1          0
#define _DEF_DAC_CH2          1

#define _DEF_PWM1             0
#define _DEF_PWM2             1
#define _DEF_PWM3             2
#define _DEF_PWM4             3
#define _DEF_PWM5             4
#define _DEF_PWM6             5
#define _DEF_PWM7             6
#define _DEF_PWM8             7

#define _DEF_I2S1             0
#define _DEF_I2S2             1

#define _DEF_SPI1             0
#define _DEF_SPI2             1
#define _DEF_SPI3             2
#define _DEF_SPI4             3
#define _DEF_SPI5             4
#define _DEF_SPI6             5
#define _DEF_SPI7             6
#define _DEF_SPI8             7

#define _DEF_EXTI1            0
#define _DEF_EXTI2            1
#define _DEF_EXTI3            2
#define _DEF_EXTI4            3
#define _DEF_EXTI5            4
#define _DEF_EXTI6            5
#define _DEF_EXTI7            6
#define _DEF_EXTI8            7
#define _DEF_EXTI9            8
#define _DEF_EXTI10           9
#define _DEF_EXTI11           10
#define _DEF_EXTI12           11
#define _DEF_EXTI13           12
#define _DEF_EXTI14           13
#define _DEF_EXTI15           14
#define _DEF_EXTI16           15





typedef uint32_t  err_code_t;




typedef void (*voidFuncPtr)(void);



typedef union
{
  uint8_t  u8Data[4];
  uint16_t u16Data[2];
  uint32_t u32Data;

  int8_t   s8Data[4];
  int16_t  s16Data[2];
  int32_t  s32Data;

  uint8_t  u8D;
  uint16_t u16D;
  uint32_t u32D;

  int8_t   s8D;
  int16_t  s16D;
  int32_t  s32D;
} data_t;


typedef struct
{
  data_t data;
  bool   ret;
} data_ret_t;


typedef struct
{
  uint32_t  ptr_in;
  uint32_t  ptr_out;
  uint32_t  length;
  uint8_t  *p_buf;
} ring_buf_t;


typedef struct
{
  uint32_t addr;
  uint32_t end;
  uint32_t length;
} flash_attr_t;



#endif /* SRC_COMMON_DEF_H_ */
