/*
 * stm32_pid_410.cpp
 *
 *  Created on: 2019. 5. 10.
 *      Author: HanCheol Cho
 */


#include "boot/boot.h"



static device_info_t stm32_pid_410[129];



device_info_t *stm32_pid_410_info(void)
{
  int i;

  for (i=0; i<128; i++)
  {
    stm32_pid_410[i].sector_index = i;
    stm32_pid_410[i].sector_addr = 0x08000000 + 1024 * i;
    stm32_pid_410[i].sector_length = 1024;
  }
  stm32_pid_410[i].sector_index = -1;
  stm32_pid_410[i].sector_addr = 0;
  stm32_pid_410[i].sector_length = 0;

  return stm32_pid_410;
}
