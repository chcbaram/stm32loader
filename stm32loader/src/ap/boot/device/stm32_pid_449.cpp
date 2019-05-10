/*
 * stm32_pid_449.cpp
 *
 *  Created on: 2019. 5. 10.
 *      Author: HanCheol Cho
 */


#include "boot/boot.h"



static device_info_t stm32_pid_449[] =
    {
        {  0, 0x08000000, 32*1024 },
        {  1, 0x08008000, 32*1024 },
        {  2, 0x08010000, 32*1024 },
        {  3, 0x08018000, 32*1024 },
        {  4, 0x08020000, 128*1024 },
        {  5, 0x08040000, 256*1024 },
        {  6, 0x08080000, 256*1024 },
        {  7, 0x080C0000, 256*1024 },
        { -1, 0, 0 },
    };


device_info_t *stm32_pid_449_info(void)
{
  return stm32_pid_449;
}
