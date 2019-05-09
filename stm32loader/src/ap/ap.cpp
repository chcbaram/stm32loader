/*
 * ap.cpp
 *
 *  Created on: 2018. 6. 1.
 *      Author: HanCheol Cho
 */




#include "ap.h"
#include "boot/boot.h"






void apInit(void)
{
}

void apMain(int argc, char *argv[])
{
  bool is_open = false;


  if (argc >= 3)
  {
    char *port;
    int baud;

    port = argv[1];
    baud = (int)strtoul((const char * ) argv[2], (char **)NULL, (int) 0);


    if (bootOpen(port, baud) == true)
    {
      is_open = true;
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



  printf("start\n");

  if (bootPing())
  {
    printf("Ping : OK\n");

    bool ret;
    resp_get_t resp_get;
    resp_get_option_t resp_get_option;
    resp_get_id_t resp_get_id;

    ret = bootGet(&resp_get);
    ret = bootGetOption(&resp_get_option);
    ret = bootGetID(&resp_get_id);
  }
  else
  {
    printf("Ping : Fail\n");
  }

  printf("end\n");
}
