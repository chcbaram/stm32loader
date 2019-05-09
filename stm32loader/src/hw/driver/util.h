/*
 * util.h
 *
 *  Created on: 2018. 6. 1.
 *      Author: HanCheol Cho
 */

#ifndef SRC_HW_CORE_UTIL_H_
#define SRC_HW_CORE_UTIL_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


void delay(uint32_t delay_ms);
uint32_t millis(void);
uint32_t micros(void);

#ifdef __cplusplus
}
#endif


#endif /* SRC_HW_CORE_UTIL_H_ */
