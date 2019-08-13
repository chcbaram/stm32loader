/*
 * boot.cpp
 *
 *  Created on: 2019. 5. 9.
 *      Author: Baram
 */

#include "boot.h"



enum BootCmd
{
  Cmd_Get             = 0x00,
  Cmd_Get_Version     = 0x01,
  Cmd_Get_ID          = 0x02,
  Cmd_Read_Memory     = 0x11,
  Cmd_Go              = 0x21,
  Cmd_Write_Memory    = 0x31,
  Cmd_Erase           = 0x43,
  Cmd_Extended_Erase  = 0x44,
  Cmd_Write_Protect   = 0x63,
  Cmd_Write_Unprotect = 0x73,
  Cmd_Readout_Protect = 0x82,
  Cmd_Readout_Unprotect = 0x92,
  Cmd_Ping = 0x7F
};

enum BootResp
{
  Resp_None  = 0x00,
  Resp_ACK   = 0x79,
  Resp_NACK  = 0x1F,
};


typedef struct
{
  uint16_t pid;
  char device_str[64];
  device_info_t* (*p_info)();

} device_tbl_t;


extern device_info_t *stm32_pid_449_info();
extern device_info_t *stm32_pid_410_info();
extern device_info_t *stm32_pid_414_info();
extern device_info_t *stm32_pid_450_info();

device_tbl_t device_tbl[] =
    {
        {0x440, "STM32F05xxx",              NULL},
        {0x444, "STM32F03xx4/6",            NULL},
        {0x442, "STM32F030C",               NULL},
        {0x445, "STM32F05xxx",              NULL},

        {0x410, "STM32F10xxx_Medium-density", stm32_pid_410_info},
        {0x414, "STM32F10xxx_High-density",   stm32_pid_414_info},
        {0x449, "STM32F74xxx/75xxx",          stm32_pid_449_info},
        {0x450, "STM32H74xxx/75xxx",          stm32_pid_450_info},

        {0x000, " ", NULL},
    };



static bool is_open = false;
static bool is_log = true;

static uint8_t uart_ch = _DEF_UART2;
static uint32_t last_error = 0;


static bool bootSendCmd(enum BootCmd cmd, uint32_t timeout);
static bool bootWaitAck(uint32_t timeout, bool log_progress=false);
static device_tbl_t *bootGetDevice(uint16_t pid);
static bool bootGetFlashInfo(uint16_t pid, uint32_t addr, uint32_t length, flash_info_t *p_resp);
static void bootFlush(void);






bool bootOpen(char *port, uint32_t baud)
{
  bool ret = false;

  last_error = OK;


  uartSetPortName(uart_ch, port);
  uartSetParity(uart_ch, UART_PARITY_EVEN);

  if (uartOpen(uart_ch, baud) == OK)
  {
    is_open = true;
    ret = true;
  }
  else
  {
    last_error = BOOT_ERR_FAIL_OPEN;
  }

  return ret;
}

bool bootClose(void)
{
  return uartClose(uart_ch);
}

uint32_t bootGetLastError(void)
{
  return last_error;
}

void bootPrintError(void)
{
  printf("error_code \t: 0x%X(%d), ", last_error, last_error);

  switch(last_error)
  {
    case BOOT_ERR_NOT_OPEN:
      printf("BOOT_ERR_NOT_OPEN");
      break;
    case BOOT_ERR_FAIL_OPEN:
      printf("BOOT_ERR_FAIL_OPEN");
      break;
    case BOOT_ERR_NACK:
      printf("BOOT_ERR_NACK");
      break;
    case BOOT_ERR_NO_RESP:
      printf("BOOT_ERR_NO_RESP");
      break;
    case BOOT_ERR_TIMEOUT:
      printf("BOOT_ERR_TIMEOUT");
      break;
    case BOOT_ERR_WRITE_ADDR:
      printf("BOOT_ERR_WRITE_ADDR");
      break;
    case BOOT_ERR_WRITE_ADDR_ACK:
      printf("BOOT_ERR_WRITE_ADDR_ACK");
      break;
    case BOOT_ERR_WRITE_LEN:
      printf("BOOT_ERR_WRITE_LEN");
      break;
    case BOOT_ERR_WRITE_LEN_ACK:
      printf("BOOT_ERR_WRITE_LEN_ACK");
      break;
    case BOOT_ERR_WRITE_READ_DATA:
      printf("BOOT_ERR_WRITE_READ_DATA");
      break;
    case BOOT_ERR_CMD_READ_MEMORY_RDP:
      printf("BOOT_ERR_CMD_READ_MEMORY_RDP");
      break;
    case BOOT_ERR_CMD_GET:
      printf("BOOT_ERR_CMD_GET");
      break;
    case BOOT_ERR_CMD_GET_ID:
      printf("BOOT_ERR_CMD_GET_ID");
      break;
    case BOOT_ERR_CMD_GET_OPTION:
      printf("BOOT_ERR_CMD_GET_OPTION");
      break;
    case BOOT_ERR_CMD_PING:
      printf("BOOT_ERR_CMD_PING");
      break;
    case BOOT_ERR_NOT_EXTENDED_ERASE:
      printf("BOOT_ERR_NOT_EXTENDED_ERASE");
      break;
    case BOOT_ERR_NOT_SUPPORT:
      printf("BOOT_ERR_NOT_SUPPORT");
      break;
    case BOOT_ERR_CMD_GO:
      printf("BOOT_ERR_CMD_GO");
      break;
    case BOOT_ERR_CMD_EX_ERASE:
      printf("BOOT_ERR_CMD_EX_ERASE");
      break;
    case BOOT_ERR_INVAILD_ADDR:
      printf("BOOT_ERR_INVAILD_ADDR");
      break;
    case BOOT_ERR_ERASE_TIMEOUT:
      printf("BOOT_ERR_ERASE_TIMEOUT");
      break;
  }

  printf("\n");
}

void bootPrintDevice(uint16_t pid)
{
  uint16_t i;

  i = 0;
  while(1)
  {
    if (device_tbl[i].pid == pid)
    {
      printf("%s", device_tbl[i].device_str);
    }
    if (device_tbl[i].pid == 0)
    {
      break;
    }

    i++;
  }
}

device_tbl_t *bootGetDevice(uint16_t pid)
{
  uint16_t i;
  device_tbl_t *p_ret = NULL;

  i = 0;
  while(1)
  {
    if (device_tbl[i].pid == pid)
    {
      p_ret = &device_tbl[i];
      break;
    }
    if (device_tbl[i].pid == 0)
    {
      break;
    }

    i++;
  }

  return p_ret;
}

bool bootIsSupportMCU(void)
{
  resp_get_id_t resp_get_id;
  device_tbl_t *p_device;


  if (bootGetID(&resp_get_id) != true)
  {
    last_error = BOOT_ERR_CMD_GET_ID;
    return false;
  }

  p_device = bootGetDevice(resp_get_id.pid);
  if (p_device == NULL)
  {
    last_error = BOOT_ERR_NOT_SUPPORT;
    return false;
  }

  if (p_device->p_info == NULL)
  {
    last_error = BOOT_ERR_NOT_SUPPORT;
    return false;
  }

  return true;
}

bool bootSendCmd(enum BootCmd cmd, uint32_t timeout)
{
  uint32_t pre_time;
  uint8_t  cmd_byte;
  uint8_t  resp_byte;
  bool ret = false;


  cmd_byte = (uint8_t)cmd;


  if (cmd == Cmd_Ping)
  {
    uartPutch(uart_ch, cmd_byte);
  }
  else
  {
    uartPutch(uart_ch, cmd_byte);
    uartPutch(uart_ch, ~cmd_byte);
  }

  if (is_log == true)
  {
    printf("-> SendCmd : 0x%02X\n", cmd_byte);
  }

  pre_time = millis();

  while(1)
  {
    if (uartAvailable(uart_ch) > 0)
    {
      resp_byte = uartRead(uart_ch);

      if (resp_byte == Resp_ACK)
      {
        ret = true;
      }
      if (resp_byte == Resp_NACK)
      {
        last_error = BOOT_ERR_NACK;
      }

      if (is_log == true)
      {
        printf("<- RespCmd : 0x%02X\n", resp_byte);
      }

      break;
    }

    if (millis()-pre_time >= timeout)
    {
      last_error = BOOT_ERR_NO_RESP;
      break;
    }
  }

  return ret;
}

bool bootReadData(uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  uint32_t pre_time;
  uint32_t index;
  bool ret = false;
  uint8_t read_byte;


  index = 0;
  pre_time = millis();
  while(1)
  {
    if (uartAvailable(uart_ch) > 0)
    {
      read_byte = uartRead(uart_ch);

      if (is_log)
      {
        printf("     0x%02X\n", read_byte);
      }

      if (p_data != NULL)
      {
        p_data[index] = read_byte;
      }
      index++;

      if (index >= length)
      {
        ret = true;
        break;
      }
    }
    if (millis()-pre_time >= timeout)
    {
      last_error = BOOT_ERR_TIMEOUT;
      break;
    }
  }

  return ret;
}

bool bootWriteData(uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = false;

  if (uartWrite(uart_ch, p_data, length) == (int32_t)length)
  {
    ret = true;
  }

  return ret;
}

bool bootWaitAck(uint32_t timeout, bool log_progress)
{
  uint32_t pre_time;
  uint32_t pre_time_cnt;
  uint8_t  rx_byte;
  bool ret = false;

  if (log_progress == true)
  {
    printf("#");
  }

  pre_time = millis();
  pre_time_cnt = millis();
  while(1)
  {
    if (uartAvailable(uart_ch) > 0)
    {
      rx_byte = uartRead(uart_ch);

      if (rx_byte == Resp_ACK)
      {
        ret = true;
      }
      if (rx_byte == Resp_NACK)
      {
        last_error = BOOT_ERR_NACK;
      }

      if (is_log == true)
      {
        printf("<- RespCmd : 0x%02X\n", rx_byte);
      }

      break;
    }

    if (millis()-pre_time >= timeout)
    {
      last_error = BOOT_ERR_NO_RESP;
      break;
    }

    if (millis()-pre_time_cnt >= 1000)
    {
      pre_time_cnt = millis();

      if (log_progress == true)
      {
        printf("#");
      }
    }
  }

  return ret;
}

void bootFlush(void)
{
  uartFlush(uart_ch);
}

void bootLogEnable(bool enable)
{
  is_log = enable;
}

bool bootPing(void)
{
  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }

  if (is_log == true)
  {
    printf("# bootPing()\n");
  }

  if (bootSendCmd(Cmd_Ping, 500) != true)
  {
    last_error = BOOT_ERR_CMD_PING;
    return false;
  }

  return true;
}

bool bootGetBoardName(char *board_str)
{
  resp_get_id_t resp_get_id;
  resp_get_option_t resp_get_option;
  device_tbl_t *p_device;


  if (bootGetOption(&resp_get_option) != true)
  {
    last_error = BOOT_ERR_CMD_GET_OPTION;
    return false;
  }

  if (bootGetID(&resp_get_id) != true)
  {
    last_error = BOOT_ERR_CMD_GET_ID;
    return false;
  }

  p_device = bootGetDevice(resp_get_id.pid);
  if (p_device == NULL)
  {
    last_error = BOOT_ERR_NOT_SUPPORT;
    return false;
  }

  strcpy(board_str, p_device->device_str);

  return true;
}


bool bootGetBootVersion(uint8_t *p_version)
{
  resp_get_t resp_get;

  if (bootGet(&resp_get) != true)
  {
    last_error = BOOT_ERR_CMD_GET;
    return false;
  }

  *p_version = resp_get.boot_version;

  return true;
}

bool bootGet(resp_get_t *p_resp)
{
  bool ret = true;

  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }

  if (is_log == true)
  {
    printf("# bootGet()\n");
  }
  bootFlush();


  if (bootSendCmd(Cmd_Get, 100) == true)
  {
    if (bootReadData(&p_resp->length, 1, 100) != true)
    {
      return false;
    }
    if (bootReadData(&p_resp->boot_version, 1, 100) != true)
    {
      return false;
    }
    if (bootReadData(&p_resp->supported_cmd[0], p_resp->length, 100) != true)
    {
      return false;
    }

    p_resp->support_erase = false;
    for (int i=0; i<p_resp->length; i++)
    {
      if (p_resp->supported_cmd[i] == Cmd_Erase)
      {
        p_resp->support_erase = true;
      }
    }
  }
  else
  {
    last_error = BOOT_ERR_CMD_GET;
    return false;
  }


  if (is_log == true)
  {
    printf("<- Len : %d\n", p_resp->length);
    printf("<- Ver : 0x%02X\n", p_resp->boot_version);
    for (int i=0; i<p_resp->length; i++)
    {
      printf("<- Cmd : 0x%02X\n", p_resp->supported_cmd[i]);
    }
  }

  return ret;
}

bool bootGetOption(resp_get_option_t *p_resp)
{
  bool ret = true;

  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }

  if (is_log == true)
  {
    printf("# bootGetOption()\n");
  }
  bootFlush();


  if (bootSendCmd(Cmd_Get_Version, 100) == true)
  {
    if (bootReadData(&p_resp->boot_version, 1, 100) != true)
    {
      return false;
    }
    if (bootReadData(&p_resp->option_byte_1, 1, 100) != true)
    {
      return false;
    }
    if (bootReadData(&p_resp->option_byte_2, 1, 100) != true)
    {
      return false;
    }
    if (bootReadData(NULL, 1, 100) != true)
    {
      return false;
    }
  }
  else
  {
    last_error = BOOT_ERR_CMD_GET_OPTION;
    return false;
  }

  if (is_log == true)
  {
    printf("<- Ver : 0x%02X\n", p_resp->boot_version);
    printf("<- Op1 : 0x%02X\n", p_resp->option_byte_1);
    printf("<- Op2 : 0x%02X\n", p_resp->option_byte_2);
  }

  return ret;
}

bool bootGetID(resp_get_id_t *p_resp)
{
  bool ret = true;
  uint8_t len = 0;
  uint8_t pid[2];

  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }

  if (is_log == true)
  {
    printf("# bootGetID()\n");
  }
  bootFlush();


  if (bootSendCmd(Cmd_Get_ID, 100) == true)
  {
    if (bootReadData(&len, 1, 100) != true || len != 1)
    {
      return false;
    }
    if (bootReadData((uint8_t *)&pid[0], 2, 100) != true)
    {
      return false;
    }
    p_resp->pid = (pid[0]<<8) | (pid[1]<<0);

    if (bootReadData(NULL, 1, 100) != true) // ACK
    {
      return false;
    }
  }
  else
  {
    last_error = BOOT_ERR_CMD_GET_ID;
    return false;
  }


  if (is_log == true)
  {
    printf("<- PID : 0x%04X\n", p_resp->pid);
    printf("\n");
    bootPrintDevice(p_resp->pid);
    printf("\n");
  }

  return ret;
}

bool bootWriteUnprotect(void)
{
  bool ret = true;


  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }

  if (is_log == true)
  {
    printf("# bootWriteUnprotect()\n");
  }
  bootFlush();


  if (bootSendCmd(Cmd_Write_Unprotect, 100) == true)
  {
    if (bootWaitAck(500) != true)
    {
      return false;
    }
  }
  else
  {
    last_error = BOOT_ERR_CMD_WRITE_UNPROTECT;
    return false;
  }

  return ret;
}

bool bootReadUnprotect(void)
{
  bool ret = true;


  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }

  if (is_log == true)
  {
    printf("# bootReadUnprotect()\n");
  }
  bootFlush();


  if (bootSendCmd(Cmd_Readout_Unprotect, 100) == true)
  {
    if (bootWaitAck(500) != true)
    {
      return false;
    }
  }
  else
  {
    last_error = BOOT_ERR_CMD_READ_UNPROTECT;
    return false;
  }

  return ret;
}

bool bootReadMemory(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  uint16_t rx_len;
  uint8_t tx_buf[8];
  uint32_t rx_addr;
  uint32_t sent_len;
  bool log_save;


  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }

  if (is_log == true)
  {
    printf("# bootReadMemoy()\n");
  }
  bootFlush();

  rx_addr = addr;
  sent_len = 0;
  while(sent_len < length)
  {

    if ((length-sent_len) > 256)
    {
      rx_len = 256;
    }
    else
    {
      rx_len = length-sent_len;
    }
    sent_len += rx_len;

    if (bootSendCmd(Cmd_Read_Memory, 100) == true)
    {
      tx_buf[0] = (rx_addr >> 24) & 0xFF;
      tx_buf[1] = (rx_addr >> 16) & 0xFF;
      tx_buf[2] = (rx_addr >>  8) & 0xFF;
      tx_buf[3] = (rx_addr >>  0) & 0xFF;
      tx_buf[4] = 0;
      for (int i=0; i<4; i++)
      {
        tx_buf[4] ^= tx_buf[i];
      }

      if (bootWriteData(tx_buf, 5, 100) != true)
      {
        last_error = BOOT_ERR_WRITE_ADDR;
        return false;
      }
      if (bootWaitAck(100) != true)
      {
        last_error = BOOT_ERR_WRITE_ADDR_ACK;
        return false;
      }

      tx_buf[0] = (rx_len - 1);
      tx_buf[1] = ~(rx_len - 1);
      if (bootWriteData(tx_buf, 2, 100) != true)
      {
        last_error = BOOT_ERR_WRITE_LEN;
        return false;
      }

      if (bootWaitAck(100) != true)
      {
        last_error = BOOT_ERR_WRITE_LEN_ACK;
        return false;
      }

      log_save = is_log;
      is_log = false;
      if (bootReadData(p_data, rx_len, 100) != true)
      {
        is_log = log_save;
        last_error = BOOT_ERR_WRITE_READ_DATA;
        return false;
      }
      is_log = log_save;
    }
    else
    {
      last_error = BOOT_ERR_CMD_READ_MEMORY_RDP;
      return false;
    }


    if (is_log == true)
    {
      printf("<- addr : 0x%X\n", rx_addr);
      printf("<- len  : %d\n", rx_len);
    }

    rx_addr += rx_len;
  }


  return ret;
}

bool bootGo(uint32_t addr)
{
  bool ret = true;
  uint8_t tx_buf[8];


  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }

  if (is_log == true)
  {
    printf("# bootGo()\n");
  }
  bootFlush();


  if (bootSendCmd(Cmd_Go, 100) == true)
  {
    tx_buf[0] = (addr >> 24) & 0xFF;
    tx_buf[1] = (addr >> 16) & 0xFF;
    tx_buf[2] = (addr >>  8) & 0xFF;
    tx_buf[3] = (addr >>  0) & 0xFF;
    tx_buf[4] = 0;
    for (int i=0; i<4; i++)
    {
      tx_buf[4] ^= tx_buf[i];
    }

    if (bootWriteData(tx_buf, 5, 100) != true)
    {
      last_error = BOOT_ERR_WRITE_ADDR;
      return false;
    }
    if (bootWaitAck(100) != true)
    {
      last_error = BOOT_ERR_WRITE_ADDR_ACK;
      return false;
    }
  }
  else
  {
    last_error = BOOT_ERR_CMD_GO;
    return false;
  }

  if (is_log == true)
  {
    printf("<- addr : 0x%X\n", addr);
  }

  return ret;
}

bool bootGetFlashInfo(uint16_t pid, uint32_t addr, uint32_t length, flash_info_t *p_resp)
{
  bool ret = true;
  device_tbl_t *p_device;
  uint32_t i;
  device_info_t *p_info;


  p_device = bootGetDevice(pid);
  if (p_device == NULL)
  {
    last_error = BOOT_ERR_NOT_SUPPORT;
    return false;
  }

  if (p_device->p_info == NULL)
  {
    last_error = BOOT_ERR_NOT_SUPPORT;
    return false;
  }


  p_info = p_device->p_info();
  i = 0;
  p_resp->pages = 0;

  while(p_info[i].sector_index >= 0)
  {
    bool need_erase;
    uint32_t start_sector_addr;
    uint32_t end_sector_addr;
    uint32_t start_addr;
    uint32_t end_addr;

    need_erase = false;
    start_sector_addr = p_info[i].sector_addr;
    end_sector_addr = p_info[i].sector_addr + p_info[i].sector_length - 1;
    start_addr = addr;
    end_addr = addr + length - 1;

    if (start_addr >= start_sector_addr && start_addr <= end_sector_addr)
    {
      need_erase = true;
    }
    if (end_addr >= start_sector_addr && end_addr <= end_sector_addr)
    {
      need_erase = true;
    }
    if (start_sector_addr >= start_addr && start_sector_addr <= end_addr)
    {
      need_erase = true;
    }
    if (end_sector_addr >= start_addr && end_sector_addr <= end_addr)
    {
      need_erase = true;
    }

    if (need_erase == true)
    {
      p_resp->number[p_resp->pages++] = p_info[i].sector_index;

      if (is_log == true)
      {
        printf("-> Page : %d\n", p_info[i].sector_index);
        printf("<- Addr : 0x%X\n", p_info[i].sector_addr);
      }
    }

    i++;
  }

  if (p_resp->pages == 0)
  {
    last_error = BOOT_ERR_INVAILD_ADDR;
    return false;
  }

  return ret;
}

bool bootEraseMemory(uint32_t addr, uint32_t length, uint32_t timeout)
{
  resp_get_t resp_get;


  if (bootGet(&resp_get) != true)
  {
    last_error = BOOT_ERR_CMD_ERASE;
    return false;
  }

  if (resp_get.support_erase == true)
  {
    return bootErase(addr, length, timeout);
  }
  else
  {
    return bootExtendedErase(addr, length, timeout);
  }
}

bool bootErase(uint32_t addr, uint32_t length, uint32_t timeout)
{
  bool ret = true;
  uint8_t tx_buf[8];
  resp_get_t resp_get;
  resp_get_id_t resp_get_id;
  device_tbl_t *p_device;
  uint32_t i;
  flash_info_t flash_info;

  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }

  if (is_log == true)
  {
    printf("# bootErase()\n");
  }
  bootFlush();


  if (bootGet(&resp_get) != true)
  {
    last_error = BOOT_ERR_CMD_ERASE;
    return false;
  }

  if (resp_get.support_erase != true)
  {
    last_error = BOOT_ERR_NOT_ERASE_CMD;
    return false;
  }

  if (bootGetID(&resp_get_id) != true)
  {
    last_error = BOOT_ERR_CMD_GET_ID;
    return false;
  }

  p_device = bootGetDevice(resp_get_id.pid);
  if (p_device == NULL)
  {
    last_error = BOOT_ERR_NOT_SUPPORT;
    return false;
  }


  if (bootGetFlashInfo(resp_get_id.pid, addr, length, &flash_info) != true)
  {
    return false;
  }


  if (bootSendCmd(Cmd_Erase, 100) == true)
  {
    uint8_t checksum = 0;

    tx_buf[0] = ((flash_info.pages-1) >>  0) & 0xFF;
    if (bootWriteData(tx_buf, 1, 100) != true)
    {
      last_error = BOOT_ERR_WRITE_ADDR;
      return false;
    }
    checksum ^= tx_buf[0];

    for (i=0; i<flash_info.pages; i++)
    {
      tx_buf[0] = (flash_info.number[i] >>  0) & 0xFF;
      if (bootWriteData(tx_buf, 1, 100) != true)
      {
        last_error = BOOT_ERR_WRITE_ADDR;
        return false;
      }
      checksum ^= tx_buf[0];
    }

    tx_buf[0] = checksum;
    if (bootWriteData(tx_buf, 1, 100) != true)
    {
      last_error = BOOT_ERR_WRITE_ADDR;
      return false;
    }

    if (bootWaitAck(timeout, true) != true)
    {
      last_error = BOOT_ERR_ERASE_TIMEOUT;
      return false;
    }
  }
  else
  {
    last_error = BOOT_ERR_CMD_ERASE;
    return false;
  }

  return ret;
}

bool bootExtendedErase(uint32_t addr, uint32_t length, uint32_t timeout)
{
  bool ret = true;
  uint8_t tx_buf[8];
  resp_get_t resp_get;
  resp_get_id_t resp_get_id;
  device_tbl_t *p_device;
  uint32_t i;
  flash_info_t flash_info;

  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }

  if (is_log == true)
  {
    printf("# bootExtendedErase()\n");
  }
  bootFlush();


  if (bootGet(&resp_get) != true)
  {
    last_error = BOOT_ERR_CMD_GET;
    return false;
  }

  if (resp_get.support_erase == true)
  {
    last_error = BOOT_ERR_NOT_EXTENDED_ERASE;
    return false;
  }

  if (bootGetID(&resp_get_id) != true)
  {
    last_error = BOOT_ERR_CMD_GET_ID;
    return false;
  }

  p_device = bootGetDevice(resp_get_id.pid);
  if (p_device == NULL)
  {
    last_error = BOOT_ERR_NOT_SUPPORT;
    return false;
  }


  if (bootGetFlashInfo(resp_get_id.pid, addr, length, &flash_info) != true)
  {
    return false;
  }


  if (bootSendCmd(Cmd_Extended_Erase, 100) == true)
  {
    uint8_t checksum = 0;

    tx_buf[0] = ((flash_info.pages-1) >>  8) & 0xFF;
    tx_buf[1] = ((flash_info.pages-1) >>  0) & 0xFF;
    if (bootWriteData(tx_buf, 2, 100) != true)
    {
      last_error = BOOT_ERR_WRITE_ADDR;
      return false;
    }
    checksum ^= tx_buf[0];
    checksum ^= tx_buf[1];

    for (i=0; i<flash_info.pages; i++)
    {
      tx_buf[0] = (flash_info.number[i] >>  8) & 0xFF;
      tx_buf[1] = (flash_info.number[i] >>  0) & 0xFF;
      if (bootWriteData(tx_buf, 2, 100) != true)
      {
        last_error = BOOT_ERR_WRITE_ADDR;
        return false;
      }
      checksum ^= tx_buf[0];
      checksum ^= tx_buf[1];
    }

    tx_buf[0] = checksum;
    if (bootWriteData(tx_buf, 1, 100) != true)
    {
      last_error = BOOT_ERR_WRITE_ADDR;
      return false;
    }

    if (bootWaitAck(timeout, true) != true)
    {
      last_error = BOOT_ERR_ERASE_TIMEOUT;
      return false;
    }
  }
  else
  {
    last_error = BOOT_ERR_CMD_EX_ERASE;
    return false;
  }

  return ret;
}

bool bootWriteMemory(uint32_t addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = true;
  uint16_t tx_len;
  uint8_t tx_buf[256+2];
  uint32_t tx_addr;
  uint32_t sent_len;
  uint8_t checksum;
  int i;
  uint32_t block_size;
  int32_t  block_count = -1;


  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }

  if (is_log == true)
  {
    printf("# bootWriteMemoy()\n");
  }
  bootFlush();

  block_size = length / 10;
  if (block_size == 0)
  {
    block_size = 1;
  }

  tx_addr = addr;
  sent_len = 0;
  while(sent_len < length)
  {

    if ((length-sent_len) > 256)
    {
      tx_len = 256;
    }
    else
    {
      tx_len = length-sent_len;
    }

    if (bootSendCmd(Cmd_Write_Memory, 100) == true)
    {
      tx_buf[0] = (tx_addr >> 24) & 0xFF;
      tx_buf[1] = (tx_addr >> 16) & 0xFF;
      tx_buf[2] = (tx_addr >>  8) & 0xFF;
      tx_buf[3] = (tx_addr >>  0) & 0xFF;
      tx_buf[4] = 0;
      for (i=0; i<4; i++)
      {
        tx_buf[4] ^= tx_buf[i];
      }

      if (bootWriteData(tx_buf, 5, 100) != true)
      {
        last_error = BOOT_ERR_WRITE_ADDR;
        return false;
      }
      if (bootWaitAck(100) != true)
      {
        last_error = BOOT_ERR_WRITE_ADDR_ACK;
        return false;
      }

      checksum = 0;
      tx_buf[0] = (tx_len - 1);
      checksum ^= tx_buf[0];
      for (i=0; i<tx_len; i++)
      {
        tx_buf[1+i] = p_data[sent_len + i];
        checksum ^= tx_buf[1+i];
      }
      tx_buf[1+i] = checksum;

      if (bootWriteData(tx_buf, 2+tx_len, 100) != true)
      {
        last_error = BOOT_ERR_WRITE_LEN;
        return false;
      }

      if (bootWaitAck(timeout) != true)
      {
        last_error = BOOT_ERR_WRITE_LEN_ACK;
        return false;
      }
    }
    else
    {
      last_error = BOOT_ERR_CMD_READ_MEMORY_RDP;
      return false;
    }


    if (is_log == true)
    {
      printf("<- addr : 0x%X\n", tx_addr);
      printf("<- len  : %d\n", tx_len);
    }

    if ((int32_t)(sent_len/block_size) != block_count)
    {
      block_count++;
      printf("%d%% ", block_count * 10);
    }

    tx_addr  += tx_len;
    sent_len += tx_len;
  }


  return ret;
}

