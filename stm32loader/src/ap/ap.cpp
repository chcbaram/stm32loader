/*
 * ap.cpp
 *
 *  Created on: 2018. 6. 1.
 *      Author: HanCheol Cho
 */




#include "ap.h"
#include "boot/boot.h"



#define FLASH_TX_BLOCK_LENGTH       (1024)



int32_t getFileSize(char *file_name);



void apInit(void)
{
}

void apMain(int argc, char *argv[])
{
  char *port_name;
  char *file_name;
  int baud;
  int fw_size;
  char board_str[128];
  static FILE *fp;
  size_t readbytes;
  uint32_t flash_addr;
  uint8_t jump_flag;
  uint8_t log_print = 0;
  uint8_t retry_cnt = 0;
  uint8_t retry;


  setbuf(stdout, NULL);


  if (argc != 6 && argc != 7)
  {
    printf("stm32loader port baud flash_addr file_name jump[0|1] debug[0|1]\n\n");
    return;
  }


  port_name  = (char *)argv[ 1 ];
  baud       = (uint32_t)strtoul(argv[ 2 ], NULL, 0 );
  flash_addr = (uint32_t)strtoul(argv[ 3 ], NULL, 0 );
  file_name  = (char *)argv[ 4 ];
  jump_flag  = (uint8_t)strtoul(argv[ 5 ], NULL, 0 );

  if (argc == 7)
  {
    log_print  = (uint8_t)strtoul(argv[ 6 ], NULL, 0 );
  }

  fw_size = getFileSize(file_name);

  printf("stm32loader...\n\n");

  printf("file open\n");
  printf("  file name \t: %s \n", file_name);
  printf("  file size \t: %d KB\n", fw_size/1024);
  printf("portOpen \t: %s\n", port_name);
  printf("flash addr\t: 0x%08X\n", flash_addr);


  if (bootOpen(port_name, baud) == true)
  {
    printf("bootOpen \t: OK, %s, %d\n", port_name, baud);
  }
  else
  {
    printf("bootOpen \t: Fail, %s, %d\n", port_name, baud);
    return;
  }

  bootLogEnable(log_print);

  retry = 3;

  while(1)
  {
    if (bootPing() == true)
    {
      printf("bootPing \t: OK\n");
      break;
    }
    else
    {
      printf("bootPing \t: Fail\n");

      if (retry == 0)
      {
        bootClose();
        return;
      }

      retry--;
    }
  }

  while(1)
  {
    //-- Boart Name
    //
    if (bootGetBoardName(board_str) == true)
    {
      printf("MCU     \t: %s\n", board_str);
    }
    else
    {
      bootPrintError();
      break;
    }

    resp_get_id_t get_id;
    if (bootGetID(&get_id) == true)
    {
      printf("PID       \t: 0x%03X\n", get_id.pid);
    }
    else
    {
      bootPrintError();
      break;
    }

    if (bootIsSupportMCU() == true)
    {
      printf("Supported  \t: OK\n");
    }
    else
    {
      printf("Supported  \t: Not Support\n");
      break;
    }


    //-- Version
    //
    uint8_t version;
    if (bootGetBootVersion(&version) == true)
    {
      printf("BootVersion \t: %d.%d\n", version>>4, version&0x0F);
    }
    else
    {
      bootPrintError();
      break;
    }


    //-- Flash Erase
    //
    uint32_t timeout;

    timeout = 5000 + ((fw_size/1024) * 10);

    printf("timeout \t: %d ms\n", timeout);
    printf("erase fw \t: ");
    if (bootEraseMemory(flash_addr, fw_size, timeout) == true)
    {
      printf(", OK\n");
    }
    else
    {
      bootPrintError();

      if (retry_cnt == 0)
      {
        retry_cnt++;

        //-- Read Unprotect
        //
        if (bootReadUnprotect() == true)
        {
          printf("ReadUnprotect \t: OK\n");
        }
        else
        {
          bootPrintError();
          break;
        }
        delay(100);
        continue;
      }
      retry_cnt++;
      break;
    }


    //-- Flash Write
    //
    if( ( fp = fopen( file_name, "rb" ) ) == NULL )
    {
      fprintf( stderr, "Unable to open %s\n", file_name );
      bootClose();
      exit( 1 );
    }

    uint8_t *file_buf = (uint8_t *)malloc(fw_size);

    if (file_buf == NULL)
    {
      printf("file malloc fail\n");
      bootClose();
      break;
    }

    readbytes = fread(file_buf, 1, fw_size, fp);
    if (readbytes != (size_t)fw_size)
    {
      free(file_buf);
      printf("file read fail\n");
      break;
    }


    printf("flash fw \t: ");
    if (bootWriteMemory(flash_addr, file_buf, fw_size, timeout) == true)
    {
      printf(", OK\n");
    }
    else
    {
      free(file_buf);
      bootPrintError();
      break;
    }
    free(file_buf);


    //-- Jump Fw
    //
    if (jump_flag == true)
    {
      uint32_t jump_addr;

      printf("jump fw \t: ");

      if (bootReadMemory(flash_addr + 4, (uint8_t *)&jump_addr, 4) != true)
      {
        bootPrintError();
        break;
      }
      if (bootGo(jump_addr) == true)
      {
        printf("OK\n");
      }
      else
      {
        bootPrintError();
        break;
      }
    }

    break;
  }

  bootClose();
}



int32_t getFileSize(char *file_name)
{
  int32_t ret = -1;
  static FILE *fp;


  if( ( fp = fopen( file_name, "rb" ) ) == NULL )
  {
    fprintf( stderr, "Unable to open %s\n", file_name );
    exit( 1 );
  }
  else
  {
    fseek( fp, 0, SEEK_END );
    ret = ftell( fp );
    fseek( fp, 0, SEEK_SET );
    fclose(fp);
  }

  return ret;
}
