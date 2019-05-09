/*
 * boot.h
 *
 *  Created on: 2019. 5. 9.
 *      Author: Baram
 */

#ifndef SRC_AP_BOOT_BOOT_H_
#define SRC_AP_BOOT_BOOT_H_


#include "ap.h"


#define BOOT_ERR_NOT_OPEN           1
#define BOOT_ERR_FAIL_OPEN          2
#define BOOT_ERR_NACK               3
#define BOOT_ERR_NO_RESP            4
#define BOOT_ERR_TIMEOUT            5


typedef struct
{
  uint8_t boot_version;
  uint8_t length;
  uint8_t supported_cmd[32];
} resp_get_t;

typedef struct
{
  uint8_t boot_version;
  uint8_t option_byte_1;
  uint8_t option_byte_2;
} resp_get_option_t;

typedef struct
{
  uint16_t pid;
} resp_get_id_t;


bool bootOpen(char *port, uint32_t baud);
bool bootClose(void);
bool bootPing(void);
bool bootGet(resp_get_t *p_resp);
bool bootGetOption(resp_get_option_t *p_resp);
bool bootGetID(resp_get_id_t *p_resp);

uint32_t bootGetLastError(void);


#endif /* SRC_AP_BOOT_BOOT_H_ */
