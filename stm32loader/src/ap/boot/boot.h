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
#define BOOT_ERR_WRITE_ADDR         6
#define BOOT_ERR_WRITE_ADDR_ACK     7
#define BOOT_ERR_WRITE_LEN          8
#define BOOT_ERR_WRITE_LEN_ACK      9
#define BOOT_ERR_WRITE_READ_DATA    10
#define BOOT_ERR_CMD_READ_MEMORY_RDP 11
#define BOOT_ERR_CMD_GET            12
#define BOOT_ERR_CMD_GET_ID         13
#define BOOT_ERR_CMD_GET_OPTION     14
#define BOOT_ERR_CMD_PING           15
#define BOOT_ERR_NOT_EXTENDED_ERASE 16
#define BOOT_ERR_NOT_SUPPORT        17
#define BOOT_ERR_CMD_GO             18
#define BOOT_ERR_CMD_EX_ERASE       19
#define BOOT_ERR_INVAILD_ADDR       20
#define BOOT_ERR_ERASE_TIMEOUT      21
#define BOOT_ERR_CMD_ERASE          22
#define BOOT_ERR_NOT_ERASE_CMD      23
#define BOOT_ERR_CMD_WRITE_UNPROTECT 24
#define BOOT_ERR_CMD_READ_UNPROTECT 25

typedef struct
{
  int16_t  sector_index;
  uint32_t sector_addr;
  uint32_t sector_length;
} device_info_t;

typedef struct
{
  uint16_t pages;
  uint16_t number[512];
} flash_info_t;


typedef struct
{
  uint8_t boot_version;
  uint8_t length;
  uint8_t supported_cmd[32];

  bool    support_erase;
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
void bootPrintError(void);
void bootLogEnable(bool enable);
bool bootIsSupportMCU(void);
bool bootPing(void);
bool bootWriteUnprotect(void);
bool bootReadUnprotect(void);
bool bootGetBoardName(char *board_char);
bool bootGetBootVersion(uint8_t *p_version);
bool bootGet(resp_get_t *p_resp);
bool bootGetOption(resp_get_option_t *p_resp);
bool bootGetID(resp_get_id_t *p_resp);
bool bootReadMemory(uint32_t addr, uint8_t *p_data, uint32_t length);
bool bootGo(uint32_t addr);
bool bootErase(uint32_t addr, uint32_t length, uint32_t timeout);
bool bootExtendedErase(uint32_t addr, uint32_t length, uint32_t timeout);
bool bootEraseMemory(uint32_t addr, uint32_t length, uint32_t timeout);
bool bootWriteMemory(uint32_t addr, uint8_t *p_data, uint32_t length, uint32_t timeout);

uint32_t bootGetLastError(void);


#endif /* SRC_AP_BOOT_BOOT_H_ */
