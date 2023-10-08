#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>

#include "lib_emuc_2.h"


#if defined(_M_X64) || defined(__amd64__)
  #define  LIB_EMUC  "lib_emuc2_64.dll"
#else
  #define  LIB_EMUC  "lib_emuc2_32.dll"
#endif

#define  RECV_TORLENCE  1  /* [sec] */
#define  INI_FILE       "setup.ini"

#define  DONT_SHOW_DEBUG_MSG
#define  USE_OTHER_CONFIG


/* global variables */
int              com_port;
int              baudrate;
int              interval;
int              test_time;
char             test_file[128];
char             log_file[128];
unsigned int    *id_send;
unsigned char  **data_send;
bool             is_pass = false;
HMODULE          hDLL = NULL;


/* static function */
static int  load_dll(void);
static int  open_ini(void);
static int  start_testing(void);
static void id_switch(char **id, int cnt);
static void data_switch(char **data, int cnt);
static unsigned char hex_2_dec(char ch_high, char ch_low);
static void print_data(CAN_FRAME_INFO frame_info, bool is_send, bool is_new);


/*----------------------------------------------------------------*/
int main(int argc, char *argv[])
{
  int     rtn;
  FILE   *fptr = NULL;

  /* load dll */
  rtn = load_dll();
  if (rtn)
  {
    printf("Open %s failed !\n", LIB_EMUC);
    system("pause");
    return 1;
  }

  /* open ini */
  rtn = open_ini();
  if (rtn)
  {
    if (rtn == 1)
      printf("Open %s failed !\n", INI_FILE);
    else if (rtn == 2)
      printf("Open test file failed !\n");

    system("pause");
    return 1;
  }

  /* start testing */
  rtn = start_testing();
  if (rtn == 0)  is_pass = true;
  else          is_pass = false;

  /* Write to log file */
  fptr = fopen(log_file, "w");
  if (fptr)
  {
    fprintf(fptr, "COM port  = %d\n", com_port + 1);
    fprintf(fptr, "baudrate  = %d\n", baudrate);
    fprintf(fptr, "interval  = %d [ms]\n", interval);
    fprintf(fptr, "test_time = %d [min]\n", test_time);
    fprintf(fptr, "\n");

    if (is_pass)
    {
      fprintf(fptr, "Pass !\n");
      printf("Pass !\n");
    }
    else
    {
      fprintf(fptr, "Failed !\n");
      printf("Failed !\n");
    }

    fclose(fptr);
  }
  else
  {
    printf("Save to %s failed !\n", log_file);
    system("pause");
    return 1;
  }

  FreeLibrary(hDLL);

  system("pause");
  return 0;

} /* END: main() */



/*----------------------------------------------------------------*/
static int load_dll(void)
{
  hDLL = LoadLibrary(LIB_EMUC);

  if (hDLL == NULL)
    return 1;
  else
  {
    EMUCOpenDevice      = (EMUC_OPEN_DEVICE)       GetProcAddress(hDLL, "EMUCOpenDevice");
    EMUCCloseDevice     = (EMUC_CLOSE_DEVICE)      GetProcAddress(hDLL, "EMUCCloseDevice");
    EMUCShowVer         = (EMUC_SHOW_VER)          GetProcAddress(hDLL, "EMUCShowVer");
    EMUCResetCAN        = (EMUC_RESET_CAN)         GetProcAddress(hDLL, "EMUCResetCAN");
    EMUCClearFilter     = (EMUC_CLEAR_FILTER)      GetProcAddress(hDLL, "EMUCClearFilter");
    EMUCInitCAN         = (EMUC_INIT_CAN)          GetProcAddress(hDLL, "EMUCInitCAN");
    EMUCSetBaudRate     = (EMUC_SET_BAUDRATE)      GetProcAddress(hDLL, "EMUCSetBaudRate");
    EMUCSetMode         = (EMUC_SET_MODE)          GetProcAddress(hDLL, "EMUCSetMode");
    EMUCSetFilter       = (EMUC_SET_FILTER)        GetProcAddress(hDLL, "EMUCSetFilter");
    EMUCSetErrorType    = (EMUC_SET_ERROR_TYPE)    GetProcAddress(hDLL, "EMUCSetErrorType");
    EMUCGetCfg          = (EMUC_GET_CFG)           GetProcAddress(hDLL, "EMUCGetCfg");
    EMUCExpCfg          = (EMUC_EXP_CFG)           GetProcAddress(hDLL, "EMUCExpCfg");
    EMUCImpCfg          = (EMUC_IMP_CFG)           GetProcAddress(hDLL, "EMUCImpCfg");
    EMUCSend            = (EMUC_SEND)              GetProcAddress(hDLL, "EMUCSend");
    EMUCReceive         = (EMUC_RECEIVE)           GetProcAddress(hDLL, "EMUCReceive");
    EMUCReceiveNonblock = (EMUC_RECEIVE_NON_BLOCK) GetProcAddress(hDLL, "EMUCReceiveNonblock");

    if (!EMUCOpenDevice || !EMUCCloseDevice || !EMUCShowVer || !EMUCResetCAN  || !EMUCClearFilter  ||
        !EMUCInitCAN    || !EMUCSetBaudRate || !EMUCSetMode || !EMUCSetFilter || !EMUCSetErrorType ||
        !EMUCGetCfg     || !EMUCExpCfg      || !EMUCImpCfg  || !EMUCSend      || !EMUCReceive      ||
        !EMUCReceiveNonblock)
    {
      printf("Get function from %s failed !\n", LIB_EMUC);
      return 1;
    }
    return 0;
  }
  return 1;

} /* END: load_dll() */



/*----------------------------------------------------------------*/
static int open_ini(void)
{
  FILE  *fptr_1 = NULL;
  FILE  *fptr_2 = NULL;

  fptr_1 = fopen(INI_FILE, "r");

  if (fptr_1)
  {
    fscanf(fptr_1, "%d %d %d %d %s %s", &com_port,
      &baudrate,
      &interval,
      &test_time,
      test_file,
      log_file);

#ifndef DONT_SHOW_DEBUG_MSG
    printf("COM port  = %d\n", com_port);
    printf("baudrate  = %d\n", baudrate);
    printf("interval  = %d\n", interval);
    printf("test_time = %d\n", test_time);
    printf("test_file : %s\n", test_file);
    printf("log_file  : %s\n", log_file);
#endif

    com_port = com_port - 1;
    fclose(fptr_1);

    /* check test_file */
    fptr_2 = fopen(test_file, "r");

    if (!fptr_2)
      return 2;

    fclose(fptr_2);

    return 0;
  }

  return 1;

} /* END: open_ini() */


/*----------------------------------------------------------------*/
static int start_testing(void)
{
  int                  i;
  int                  j;
  int                  rtn;
  int                  CAN_port;
  int                  total_send = -2;
  char                 buf_temp[128];
  char               **id;
  char               **data;
  bool                 is_failed = false;
  bool                 is_new    = false;
  unsigned int         send_can  = EMUC_CAN_1;
  clock_t              period_t  = test_time * 1000 * 60;  /* t: total */
  clock_t              period_r  = RECV_TORLENCE * 1000;   /* r: recv */
  clock_t              start_t;  /* t: total */
  clock_t              start_r;  /* r: recv */
  FILE                *fptr = NULL;
  CAN_FRAME_INFO       frame_send, frame_recv;


  /* open com port */
  if(com_port == -1)
  {
    for(i=0; i<MAX_COM_NUM; i++)
    {
      rtn = EMUCOpenDevice(i);
      if(rtn == 0)
        break;
    }
    if(rtn)
    {
      printf("Can not find EMUC2.0 device !\n");
      return 1;
    }
    else
      com_port = i;
  }
  else
  {
    rtn = EMUCOpenDevice(com_port);
    if (rtn)
    {
      printf("Open COM port %d failed !\n", com_port + 1);
      return 1;
    }
  }


  /* initial CAN */
  rtn = EMUCInitCAN(com_port, EMUC_INACTIVE, EMUC_INACTIVE);
  if (rtn)
  {
    printf("EMUC initial CAN failed !\n");
    return 1;
  }

  /* set baudrate */
  rtn = EMUCSetBaudRate(com_port, baudrate, baudrate);
  if (rtn)
  {
    printf("EMUC set baud rate failed !\n");
    return 1;
  }

#ifdef USE_OTHER_CONFIG
  /* set error type */
  rtn = EMUCSetErrorType(com_port, EMUC_DIS_ALL);
  if (rtn)
  {
    printf("EMUC set error type failed !\n");
    return 1;
  }

  /* set mode */
  rtn = EMUCSetMode(com_port, EMUC_NORMAL, EMUC_NORMAL);
  if (rtn)
  {
    printf("EMUC set mode failed !\n");
    return 1;
  }

  /* clear filter */
  CAN_port = EMUC_CAN_1;
  rtn = EMUCClearFilter(com_port, CAN_port);
  if (rtn)
  {
    printf("EMUC CAN 1 clear filter failed !\n");
    return 1;
  }

  CAN_port = EMUC_CAN_2;
  rtn = EMUCClearFilter(com_port, CAN_port);
  if (rtn)
  {
    printf("EMUC CAN 2 clear filter failed !\n");
    return 1;
  }
#endif

  /* initial CAN */
  rtn = EMUCInitCAN(com_port, EMUC_ACTIVE, EMUC_ACTIVE);
  if (rtn)
  {
    printf("EMUC initial CAN failed !\n");
    return 1;
  }

  /* send setting */
  fptr = fopen(test_file, "r");

  if (!fptr)
  {
    printf("Open %s failed !\n", test_file);
    return 1;
  }
  else
  {
    while (!feof(fptr))
    {
      memset(buf_temp, 0, sizeof(buf_temp));
      fgets(buf_temp, sizeof(buf_temp), fptr);

      if (buf_temp[0] != 0 && buf_temp[0] != '\n')
        total_send++;
    }

    /* malloc id & data */
    id = malloc(total_send * sizeof(char *));
    data = malloc(total_send * sizeof(char *));

    for (i = 0; i<total_send; i++)
    {
      id[i] = malloc(128 * sizeof(char));
      data[i] = malloc(128 * sizeof(char));
    }

    /* back to head of file */
    fseek(fptr, 0, 0);
    for (i = 0; i<2; i++)  fgets(buf_temp, sizeof(buf_temp), fptr);

    for (i = 0; i<total_send; i++)
      fscanf(fptr, "%s %s", id[i], data[i]);

    id_switch(id, total_send);
    data_switch(data, total_send);

#ifndef DONT_SHOW_DEBUG_MSG
    for (int i = 0; i<total_send; i++)
      printf("id_send[%d] = 0x%08X\n", i, id_send[i]);

    for (int i = 0; i<total_send; i++)
    {
      printf("data_send[%d] = ", i);
      for (int j = 0; j<8; j++)
        printf("%02X ", data_send[i][j]);
      printf("\n");
    }
#endif

    fclose(fptr);
  }

  memset(&frame_send, 0, sizeof(CAN_FRAME_INFO));
  frame_send.CAN_port = EMUC_CAN_1;
  frame_send.id_type = EMUC_EID;
  frame_send.rtr = EMUC_DIS_RTR;
  frame_send.dlc = 8;


  /* start sending */
  start_t = clock();

  while (1)
  {
    is_new = true;

    for (i = 0; i<total_send;)
    {
      frame_send.CAN_port = send_can % 2;
      if (frame_send.CAN_port == EMUC_CAN_1)
      {
        memcpy(frame_send.data, &data_send[i][0], DATA_LEN);
        frame_send.id = id_send[i];
      }
      else
      {
        memcpy(frame_send.data, frame_recv.data, DATA_LEN);
        frame_send.id = frame_recv.id;
        i++;
      }

      /* send data */
      Sleep(interval);
      EMUCSend(com_port, &frame_send);
      print_data(frame_send, true, is_new);

      is_new = false;

      /* recv data */
      start_r = clock();
      while (1)
      {
        memset(&frame_recv, 0, sizeof(CAN_FRAME_INFO));
        rtn = EMUCReceive(com_port, &frame_recv);

        if (rtn == 1 && frame_recv.msg_type == EMUC_DATA_TYPE)
        {
          print_data(frame_recv, false, is_new);

          for (j = 0; j<DATA_LEN; j++)
          {
            if (frame_recv.data[j] != frame_send.data[j])
              break;
          }
          break;
        }

        if (clock() > start_r + period_r)  /* time out */
          return 1;
      }

      if (j != 8)
        return 1;
      else
        send_can++;
    }

    if (test_time == 0)  /* test_time = 0: once */
      break;

    if (clock() > (start_t + period_t))  /* time out */
      break;
  }

  return 0;

} /* END: start_testing() */



/*----------------------------------------------------------------*/
static void id_switch(char **id, int cnt)
{
  int i, j, shift = 0;

  id_send = (unsigned int *)malloc(cnt * sizeof(unsigned int));

  for (i = 0; i<cnt; i++) id_send[i] = 0;

  for (i = 0; i<cnt; i++)
    for (j = 7; j >= 0; j -= 2) /* ID: 4 bytes -> 8 char */
      id_send[i] = id_send[i] + (hex_2_dec(id[i][j - 1], id[i][j]) << (8 * (shift++)));

} /* END: id_switch() */



/*----------------------------------------------------------------*/
static void data_switch(char **data, int cnt)
{
  int i, j;

  data_send = malloc(cnt * sizeof(unsigned char *));

  for (i = 0; i<cnt; i++)
    data_send[i] = malloc(DATA_LEN * sizeof(unsigned char));

  for (i = 0; i<cnt; i++)
    for (j = 0; j<DATA_LEN * 2; j += 2)
      data_send[i][j / 2] = hex_2_dec(data[i][j], data[i][j + 1]);

} /* END: data_switch() */



/*----------------------------------------------------------------*/
static unsigned char hex_2_dec(char ch_high, char ch_low)
{
  unsigned int rtn_high;
  unsigned int rtn_low;

  /* A ~ F */
  if (65 <= ch_low)     rtn_low = (unsigned int)ch_low - 55;
  /* 0 ~ 9 */
  else if (48 <= ch_low) rtn_low = (unsigned int)ch_low - 48;

  /* A ~ F */
  if (65 <= ch_high)    rtn_high = (unsigned int)ch_high - 55;
  /* 0 ~ 9 */
  else if (48 <= ch_high) rtn_high = (unsigned int)ch_high - 48;

  return (unsigned char)((rtn_high * 16) + (rtn_low));

} /* END: hex_2_dec() */



/*----------------------------------------------------------------*/
static void print_data(CAN_FRAME_INFO frame_info, bool is_send, bool is_new)
{
  static unsigned int round = 1;

  int i;

  if(is_new)
  {
    system("cls");
    printf("Round %u:\n", round);
    printf("===========");
    round++;
  }

  if (is_send)
  {
    printf("\n---------------------------------------------------------\n");
    printf("Send: ");
  }
  else
    printf("Recv: ");

  /* CAN port & cnt */
  if (frame_info.CAN_port == EMUC_CAN_1)
  {
    printf("(CAN 1) ");
  }
  else if (frame_info.CAN_port == EMUC_CAN_2)
  {
    printf("(CAN 2) ");
  }

  /* ID */
  printf("ID: %08X; ", frame_info.id);

  /* Data */
  printf("Data: ");
  for (i = 0; i<frame_info.dlc; i++)
    printf("%02X ", frame_info.data[i]);
  printf("\n");



} /* END: print_data() */