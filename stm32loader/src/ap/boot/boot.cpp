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
} device_tbl_t;


device_tbl_t device_tbl[] =
    {
        {0x440, "STM32F05xxx"},
        {0x444, "STM32F03xx4/6"},
        {0x442, "STM32F030C"},
        {0x445, "STM32F05xxx"},

        {0x414, "STM32F10xxx_High-density"},
        {0x000, ""},
    };




static bool is_open = false;
static bool is_log = true;

static uint8_t uart_ch = _DEF_UART2;
static uint32_t last_error = 0;


static bool bootSendCmd(enum BootCmd cmd, uint32_t timeout);




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

void bootPrintDevice(uint16_t pid)
{
  uint16_t i;

  i = 0;
  while(1)
  {
    if (device_tbl[i].pid == pid)
    {
      printf("devicde : %s\n", device_tbl[i].device_str);
    }
    if (device_tbl[i].pid == 0)
    {
      break;
    }

    i++;
  }
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

      printf("     0x%02X\n", read_byte);

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

bool bootPing(void)
{
  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }


  return bootSendCmd(Cmd_Ping, 100);
}

bool bootGet(resp_get_t *p_resp)
{
  bool ret = true;

  if (is_open != true)
  {
    last_error = BOOT_ERR_FAIL_OPEN;
    return false;
  }


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


  if (bootSendCmd(Cmd_Get_ID, 100) == true)
  {
    if (bootReadData(NULL, 1, 100) != true) // ACK
    {
      return false;
    }
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


  if (is_log == true)
  {
    printf("<- PID : 0x%04X\n", p_resp->pid);
    bootPrintDevice(p_resp->pid);
  }

  return ret;
}
