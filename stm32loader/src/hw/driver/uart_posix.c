/*
 * uart_posix.c
 *
 *  Created on: 2020. 12. 8.
 *      Author: baram
 */


#include "uart.h"
#include "util.h"

#if defined (__WIN32__) || (__WIN64__)
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <string.h>


#ifdef _USE_HW_UART


#define UART_MAX_CH     2
#define _USE_UART1
#define _USE_UART2



bool log_err = true;


typedef struct
{
  bool     is_open;
  bool     is_consol;
  bool     is_port_name;
  uint8_t  ch;
  uint32_t baud;
  uint8_t  parity;
  int      serial_handle;
  char     port_name[256];
  uint8_t  received_data;
} uart_t;

static uart_t uart_tbl[UART_MAX_CH];



static uint32_t uartOpenPC(uint8_t channel, char *port_name, uint32_t baud);
static int uartGetCFlagBaud(int baudrate);
static int kbhit(void);
static int getch(void);




bool uartInit(void)
{
  for (int i=0; i<UART_MAX_CH; i++)
  {
    uart_tbl[i].is_open   = false;
    uart_tbl[i].is_consol = false;
    uart_tbl[i].is_port_name = false;
    uart_tbl[i].parity = UART_PARITY_NONE;
  }


  return true;
}

bool uartOpen(uint8_t ch, uint32_t baud)
{
  bool ret = false;


  switch(ch)
  {
    case _DEF_UART1:
      uart_tbl[ch].is_open = true;
      uart_tbl[ch].ch = ch;
      ret = true;
      break;

    case _DEF_UART2:
      if (uart_tbl[ch].is_port_name == true)
      {
        if (uart_tbl[ch].is_open == true)
        {
          uartClose(uart_tbl[ch].ch);
        }
        if (uartOpenPC(ch, uart_tbl[ch].port_name, baud) == 0)
        {
          uart_tbl[ch].is_open = true;
          uart_tbl[ch].ch = ch;
          ret = true;
        }
      }
      break;
  }

  return ret;
}

bool uartIsOpen(uint8_t ch)
{
  return uart_tbl[ch].is_open;
}

bool uartOpenPort(uint8_t ch, char *port_name, uint32_t baud)
{
  bool ret = false;

  uartSetPortName(ch, port_name);

  ret = uartOpen(ch, baud);

  return ret;
}

void uartSetPortName(uint8_t ch, char *port_name)
{
  if (ch >= UART_MAX_CH)
  {
    return;
  }

  sprintf(uart_tbl[ch].port_name, "%s", port_name);
  uart_tbl[ch].is_port_name  = true;
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

const char *uartGetPortName(uint8_t ch)
{
  return (const char *)uart_tbl[ch].port_name;
}

uint32_t uartOpenPC(uint8_t ch, char *port_name, uint32_t baud)
{
  uint32_t err_code  = 0;
  uart_t *p_uart = &uart_tbl[ch];

  struct termios newtio;
  uint32_t option = 0;


  if (ch >= UART_MAX_CH)
  {
    return 1;
  }


  p_uart->baud = baud;


  p_uart->serial_handle = open(port_name, O_RDWR|O_NOCTTY|O_NONBLOCK);
  if(p_uart->serial_handle < 0)
  {
    if (log_err)
    {
      //printf("Error opening serial port!\n");
    }
    return 2;
  }

  bzero(&newtio, sizeof(newtio)); // clear struct for new port settings

  if (option & (1<<0))
  {
    newtio.c_cflag = uartGetCFlagBaud(baud)| CS8 | CLOCAL | CREAD | CSTOPB;
  }
  else
  {
    newtio.c_cflag = uartGetCFlagBaud(baud) | CS8 | CLOCAL | CREAD;
  }

  if (p_uart->parity == UART_PARITY_EVEN)
  {
    newtio.c_cflag |= PARENB;      /* enable parity */
    newtio.c_cflag &= ~PARODD;     /* Even parity */
  }
  if (p_uart->parity == UART_PARITY_ODD)
  {
    newtio.c_cflag |= PARENB;      /* enable parity */
    newtio.c_cflag |= PARODD;      /* Odd parity */    
  }

  newtio.c_iflag = IGNPAR;
  newtio.c_oflag      = 0;
  newtio.c_lflag      = 0;
  newtio.c_cc[VTIME]  = 0;
  newtio.c_cc[VMIN]   = 0;

  #if defined(__linux__)
  #else
  cfsetispeed(&newtio, uartGetCFlagBaud(baud) );
  cfsetospeed(&newtio, uartGetCFlagBaud(baud) );
  #endif
  // clean the buffer and activate the settings for the port
  tcflush(p_uart->serial_handle, TCIFLUSH);
  tcsetattr(p_uart->serial_handle, TCSANOW, &newtio);


  p_uart->is_open = true;

  return err_code;
}

bool uartClose(uint8_t ch)
{
  bool ret = false;

  switch(ch)
  {
    case _DEF_UART1:
      uart_tbl[ch].is_open = false;
      break;

    case _DEF_UART2:
      if (uart_tbl[ch].is_open == true)
      {
        close(uart_tbl[ch].serial_handle);
        uart_tbl[ch].is_open = false;
        ret = true;
      }
      break;
  }

  return ret;
}

uint32_t uartAvailable(uint8_t ch)
{
  uint32_t ret = 0;
  int length = 0;
  uart_t *p_uart = &uart_tbl[ch];


  if (uart_tbl[ch].is_open != true)
  {
    return 0;
  }

  switch(ch)
  {
    case _DEF_UART1:
      if (kbhit())
      {
        ret = 1;
      }
      break;

    case _DEF_UART2:
        
      ioctl(p_uart->serial_handle, FIONREAD, &length);

      ret = length;
      break;
  }

  return ret;
}

uint8_t uartRead(uint8_t ch)
{
  uint8_t ret = 0;
  uint8_t data[1];
  uart_t *p_uart = &uart_tbl[ch];

  if (uart_tbl[ch].is_open != true)
  {
    return 0;
  }

  switch(ch)
  {
    case _DEF_UART1:
      ret = getch();
      break;

    case _DEF_UART2:
      if (read(p_uart->serial_handle, data, 1) == 1)
      {
        ret = data[0];
      }
      break;
  }

  return ret;
}

int32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t length)
{
  int32_t ret = 0;
  uart_t *p_uart = &uart_tbl[ch];


  if (uart_tbl[ch].is_open != true)
  {
    return 0;
  }

  switch(ch)
  {
    case _DEF_UART1:
      for (int i=0; i<length; i++)
      {
        putc(p_data[i], stdout);
      }
      ret = length;
      break;

    case _DEF_UART2:
      ret = write(p_uart->serial_handle, p_data, length);
      if (ret != length)
      {
        ret = 0;
      }
      break;
  }

  return ret;
}

void uartPutch(uint8_t ch, uint8_t data)
{
  uartWrite(ch, &data, 1 );
}

uint8_t uartGetch(uint8_t ch)
{
  while(1)
  {
    if( uartAvailable(ch) ) break;
  }

  return uartRead(ch);
}

int32_t uartPrintf(uint8_t ch, const char *fmt, ...)
{
  char buf[256];
  va_list args;
  int len;
  int32_t ret;

  va_start(args, fmt);
  len = vsnprintf(buf, 256, fmt, args);

  ret = uartWrite(ch, (uint8_t *)buf, len);

  va_end(args);


  return ret;
}

int32_t uartPrint(uint8_t channel, uint8_t *p_str)
{
  int32_t index = 0;

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

uint32_t uartGetBaud(uint8_t ch)
{
  uint32_t ret = 0;


  ret = uart_tbl[ch].baud;

  return ret;
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

int uartGetCFlagBaud(int baudrate)
{
  switch(baudrate)
  {
    case 1200:
      return B1200;    
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 38400:
      return B38400;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    case 230400:
      return B230400;
    
#if defined (__linux__)    

    case 460800:
      return B460800;
    case 500000:
      return B500000;      
    case 576000:
      return B576000;      
    case 921600:
      return B921600;      
    case 1000000:
      return B1000000;      
    case 1152000:    
      return B1152000;            
    case 1500000:
      return B1500000;      
    case 2000000:
      return B2000000;
    case 2500000:
      return B2500000;
    case 3000000:
      return B3000000;
    case 3500000:
      return B3500000;
    case 4000000:
      return B4000000;      
#endif

    default:
      return -1;
  }
}

int getch(void)
{
  int ch;
  struct termios buf, save;
  tcgetattr(0,&save);
  buf = save;
  buf.c_lflag &= ~(ICANON|ECHO);
  buf.c_cc[VMIN] = 1;
  buf.c_cc[VTIME] = 0;
  tcsetattr(0, TCSAFLUSH, &buf);
  ch = getchar();
  tcsetattr(0, TCSAFLUSH, &save);
  return ch;
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;

  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;

  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );

  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );

  return ch;
}


#endif
#endif