/*
 * uart_win.c
 *
 *  Created on: 2018. 6. 1.
 *      Author: HanCheol Cho
 */




#include "uart.h"
#include "util.h"

#if defined (__WIN32__) || (__WIN64__)

#include <conio.h>

typedef struct
{
  bool     is_open;
  bool     is_set_port_name;
  bool     is_consol;
  uint8_t  channel;
  uint32_t baud;
  uint8_t  parity;
  uint8_t  received_data;
  HANDLE  serial_handle;
  LARGE_INTEGER freq, counter;
  char     port_name[256];
} uart_t;

uart_t uart_tbl[UART_CH_MAX];




bool uartInit(void)
{
  uint32_t i;


  for(i=0; i<UART_CH_MAX; i++)
  {
    uart_tbl[i].is_open = false;
    uart_tbl[i].is_set_port_name = false;
    uart_tbl[i].is_consol = false;
    uart_tbl[i].parity = UART_PARITY_NONE;
  }


  return true;
}

bool uartOpen(uint8_t channel, uint32_t baud)
{
  uint32_t err_code  = OK;
  uart_t *p_uart = &uart_tbl[channel];
  bool ret = false;
  
  DCB dcb;
  COMMTIMEOUTS timeouts;
  DWORD dwError;


  if (channel >= UART_CH_MAX)
  {
    return false;
  }


  if (channel == _DEF_UART1)
  {
    p_uart->is_consol = true;
    p_uart->is_open = true;
    return true;
  }


  if (p_uart->is_set_port_name == false)
  {
    return false;
  }

  //strcpy(p_uart->port_name, port_name);
  //sprintf(p_uart->port_name, "\\\\.\\%s", port_name);


  p_uart->baud = baud;


  p_uart->serial_handle = CreateFileA(p_uart->port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (p_uart->serial_handle == INVALID_HANDLE_VALUE)
  {
    printf("Error opening serial port!\n");
    return false;
  }

  while(1)
  {
    dcb.DCBlength = sizeof(DCB);
    if (GetCommState(p_uart->serial_handle, &dcb) == FALSE)
    {
      err_code = 1;
      break;
    }

    // Set baudrate
    dcb.BaudRate = (DWORD)baud;
    dcb.ByteSize = 8;                    // Data bit = 8bit

    dcb.StopBits = ONESTOPBIT;           // Stop bit = 1

    if (p_uart->parity == UART_PARITY_EVEN)
    {
      dcb.Parity  = EVENPARITY;          // Even parity
      dcb.fParity = 1;
    }
    else
    {
      dcb.Parity  = NOPARITY;            // No parity
      dcb.fParity = 0;                   // No Parity check
    }

    dcb.fBinary  = 1;                    // Binary mode
    dcb.fNull    = 0;                    // Get Null byte
    dcb.fAbortOnError = 0;
    dcb.fErrorChar    = 0;
    // Not using XOn/XOff
    dcb.fOutX = 0;
    dcb.fInX  = 0;
    // Not using H/W flow control
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fDsrSensitivity = 0;
    dcb.fOutxDsrFlow = 0;
    dcb.fOutxCtsFlow = 0;

    if (SetCommState(p_uart->serial_handle, &dcb) == FALSE)
    {
      DWORD dwError = GetLastError();
      err_code = 2;

      printf("SetCommState err: %d\n", (int)dwError);
      break;
    }

    if (SetCommMask(p_uart->serial_handle, 0) == FALSE) // Not using Comm event
    {
      err_code = 3;
      break;
    }
    if (SetupComm(p_uart->serial_handle, 4096, 4096) == FALSE) // Buffer size (Rx,Tx)
    {
      err_code = 4;
      break;
    }
    if (PurgeComm(p_uart->serial_handle, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR) == FALSE) // Clear buffer
    {
      err_code = 5;
      break;
    }
    if (ClearCommError(p_uart->serial_handle, &dwError, NULL) == FALSE)
    {
      err_code = 6;
      break;
    }

    if (GetCommTimeouts(p_uart->serial_handle, &timeouts) == FALSE)
    {
      err_code = 7;
      break;
    }
    // Timeout (Not using timeout)
    // Immediatly return
    timeouts.ReadIntervalTimeout         = 0;
    timeouts.ReadTotalTimeoutMultiplier  = 0;
    timeouts.ReadTotalTimeoutConstant    = 1; // must not be zero.
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant   = 0;
    if (SetCommTimeouts(p_uart->serial_handle, &timeouts) == FALSE)
    {
      err_code = 8;
      break;
    }
    break;
  }

  if (err_code != OK)
  {
    CloseHandle(p_uart->serial_handle);
    printf("uartOpen err : %x \n", err_code);
  }
  else
  {
    p_uart->is_open = true;
  }

  if (err_code == OK)
    ret = true;

  return ret;
}

bool uartClose(uint8_t channel)
{
  uint32_t err_code  = OK;
  uart_t *p_uart = &uart_tbl[channel];
  bool ret = false;

  if (channel >= UART_CH_MAX)
  {
    return false;
  }

  if (p_uart->is_open == true)
  {
    CloseHandle(p_uart->serial_handle);
    //printf("close : %d\n", p_uart->serial_handle);
    p_uart->is_open = false;
  }

  if (err_code == OK)
    ret = true;

  return ret;
}

void uartSetPortName(uint8_t channel, char *port_name)
{
  uart_t *p_uart = &uart_tbl[channel];


  if (channel >= UART_CH_MAX)
  {
    return;
  }

  sprintf(p_uart->port_name, "\\\\.\\%s", port_name);
  p_uart->is_set_port_name = true;
}

void uartSetParity(uint8_t channel, uint8_t parity)
{
  uart_t *p_uart = &uart_tbl[channel];


  if (channel >= UART_CH_MAX)
  {
    return;
  }

  p_uart->parity = parity;
}

#if 1
bool uartIsEnable(uint8_t channel)
{
  if (channel >= UART_CH_MAX)
  {
    return false;
  }

  return uart_tbl[channel].is_open;
}

void uartWaitForEnable(uint8_t channel, uint32_t timeout)
{
  uint32_t t_time;


  t_time = millis();
  while(1)
  {
    if (uartIsEnable(channel) == true)
    {
      break;
    }
    if ((millis()-t_time) >= timeout)
    {
      break;
    }
  }
}

uint32_t uartAvailable(uint8_t channel)
{
  int32_t length = 0;
  uart_t *p_uart = &uart_tbl[channel];
  DWORD dwRead = 0;
  uint8_t data;

  //length = GetFileSize(p_uart->serial_handle, NULL);


  if (p_uart->is_consol == true)
  {
    if (kbhit())
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }


  if (ReadFile(p_uart->serial_handle, &data, (DWORD)1, &dwRead, NULL) == TRUE)
  {
    if (dwRead != 1)
    {
      length = 0;
    }
    else
    {
      length = 1;
      p_uart->received_data = data;
    }
  }

  return length;
}

void uartPutch(uint8_t channel, uint8_t ch)
{
  uartWrite(channel, &ch, 1 );
}

uint8_t uartGetch(uint8_t channel)
{
  if (uartIsEnable(channel) == false ) return 0;


  while(1)
  {
    if( uartAvailable(channel) ) break;
  }


  return uartRead(channel);
}

int32_t uartWrite(uint8_t channel, uint8_t *p_data, uint32_t length)
{
  int32_t  ret = 0;
  uart_t *p_uart = &uart_tbl[channel];
  DWORD dwWrite = 0;

  if (uartIsEnable(channel) == false ) return 0;


  if (p_uart->is_consol == true)
  {
    for (int i=0; i<length; i++)
    {
      putc(p_data[i], stdout);
    }
    ret = length;

    return ret;
  }

  if (WriteFile(p_uart->serial_handle, p_data, (DWORD)length, &dwWrite, NULL) == FALSE)
  {
    ret = 0;
  }
  else
  {
    ret = dwWrite;
  }

  return ret;
}

uint8_t uartRead(uint8_t channel)
{
  uint8_t ret = 0;
  uart_t *p_uart = &uart_tbl[channel];


  if (p_uart->is_consol == true)
  {
    return getch();
  }

  ret = p_uart->received_data;

  return ret;
}

int32_t uartPrintf(uint8_t channel, const char *fmt, ...)
{
  int32_t ret = 0;
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  static char print_buffer[255];


  if (uartIsEnable(channel) == false ) return 0;


  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  ret = uartWrite(channel, (uint8_t *)print_buffer, len);

  return ret;
}

int32_t uartPrint(uint8_t channel, uint8_t *p_str)
{
  int32_t index = 0;

  if (uartIsEnable(channel) == false ) return 0;

  while(1)
  {
    uartPutch(channel, p_str[index]);

    if (p_str[index] == 0)
    {
      break;
    }

    index++;

    if (index > 255)
    {
      break;
    }
  }


  return index;
}

void uartFlush(uint8_t channel)
{
  uint32_t pre_time;

  pre_time = millis();

  while(uartAvailable(channel) > 0)
  {
    uartRead(channel);
    if (millis()-pre_time >= 100)
    {
      break;
    }
  }
}
#endif

#endif