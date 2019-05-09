/*
 * uart.h
 *
 *  Created on: 2018. 6. 1.
 *      Author: HanCheol Cho
 */

#ifndef SRC_HW_CORE_UART_H_
#define SRC_HW_CORE_UART_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"



#define UART_CH_MAX    8



#define UART_PARITY_NONE  0
#define UART_PARITY_EVEN  1
#define UART_PARITY_ODD   2




bool        uartInit(void);
uint32_t    uartOpen(uint8_t channel, uint32_t baud);
void        uartSetPortName(uint8_t channel, char *port_name);
void        uartSetParity(uint8_t channel, uint8_t parity);
uint32_t    uartClose(uint8_t channel);
uint32_t    uartAvailable(uint8_t channel);
void        uartWaitForEnable(uint8_t channel, uint32_t timeout);
void        uartPutch(uint8_t channel, uint8_t ch);
uint8_t     uartGetch(uint8_t channel);
int32_t     uartWrite(uint8_t channel, uint8_t *p_data, uint32_t length);
uint8_t     uartRead(uint8_t channel);
int32_t     uartPrintf(uint8_t channel, const char *fmt, ...);
int32_t     uartPrint(uint8_t channel, uint8_t *p_str);
void        uartFlush(uint8_t channel);


#ifdef __cplusplus
}
#endif


#endif /* SRC_HW_CORE_UART_H_ */
