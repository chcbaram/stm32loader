/*
 * ap.cpp
 *
 *  Created on: 2018. 6. 1.
 *      Author: HanCheol Cho
 */




#include "ap.h"


uint8_t uart_ch = _DEF_UART2;




void apInit(void)
{
}

void apMain(int argc, char *argv[])
{
  bool uart_is_open = false;



  if (argc >= 3)
  {
    char *port;
    int baud;

    port = argv[1];
    baud = (int)strtoul((const char * ) argv[2], (char **)NULL, (int) 0);


    uartSetPortName(uart_ch, port);
    uartSetParity(uart_ch, UART_PARITY_EVEN);

    if (uartOpen(uart_ch, baud) == OK)
    {
      uart_is_open = true;
      printf("uart open : %s, %d\n", port, baud);
    }
    else
    {
      printf("uart open fail\n");
    }
  }
  else
  {
    printf(".exe com1 115200\n");
    return;
  }

  uint32_t pre_time;


  //uartPutch(uart_ch, 0x7F);
  //uartPutch(uart_ch, 0x7F);
  //uartPutch(uart_ch, 0x7F);
  //uartPutch(uart_ch, 0x7F);
  uartPutch(uart_ch, 0x7F);
  uartPutch(uart_ch, 0xFF);
  delay(100);

  uartPutch(uart_ch, 0x00);
  uartPutch(uart_ch, 0xFF);

  printf("start\n");

  pre_time = millis();
  while(uart_is_open)
  {
    if (uartAvailable(uart_ch) > 0)
    {
      printf("rx : 0x%X\n", uartRead(uart_ch));
      pre_time = millis();
    }

    if (millis()-pre_time >= 100)
    {
      break;
    }
  }

  printf("end\n");

  if (uart_is_open == false)
  {
    return;
  }
}
