#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <process.h>
#include <windows.h>

#include "lib_emuc_2.h"

/* global variables */
EMUC_OPEN_DEVICE       EMUCOpenDevice      = NULL;
EMUC_CLOSE_DEVICE      EMUCCloseDevice     = NULL;
EMUC_SHOW_VER          EMUCShowVer         = NULL;
EMUC_RESET_CAN         EMUCResetCAN        = NULL;
EMUC_CLEAR_FILTER      EMUCClearFilter     = NULL;
EMUC_INIT_CAN          EMUCInitCAN         = NULL;
EMUC_SET_BAUDRATE      EMUCSetBaudRate     = NULL;
EMUC_SET_MODE          EMUCSetMode         = NULL;
EMUC_SET_FILTER        EMUCSetFilter       = NULL;
EMUC_SET_ERROR_TYPE    EMUCSetErrorType    = NULL;
EMUC_GET_CFG           EMUCGetCfg          = NULL;
EMUC_EXP_CFG           EMUCExpCfg          = NULL;
EMUC_IMP_CFG           EMUCImpCfg          = NULL;
EMUC_SEND              EMUCSend            = NULL;
EMUC_RECEIVE           EMUCReceive         = NULL;
EMUC_RECEIVE_NON_BLOCK EMUCReceiveNonblock = NULL;

int              com_port;
int              CAN_port;
const char      *file_name = "emuc_config";
unsigned char    bld_buf[VER_LEN];
unsigned char    data_send[DATA_LEN] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
HINSTANCE        hDLL = NULL;

VER_INFO         ver_info;
CFG_INFO         cfg_info;
FILTER_INFO      filter_info;
CAN_FRAME_INFO   frame_send;
CAN_FRAME_INFO   frame_recv;
NON_BLOCK_INFO   non_block_info;


/* static function prototype */
static void TRD_recv_fx(void *ptr);
static int  load_dll (void);
static int  find_emuc (void);
static void print_data (CAN_FRAME_INFO frame_info);


/*----------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
  int   i;
  int   j;
  int   rtn;


  rtn = load_dll();
  if(rtn)
  {
    printf("Load EMUC .dll failed !\n");
    system("pause");
    return 1;
  }

  com_port = find_emuc();

  if(com_port == -1)
  {
    printf("No EMUC device !\n");
    system("pause");
    return 1;
  }

  /* ----- EMUCOpenDevice() ----- */
  rtn = EMUCOpenDevice(com_port);
  if(rtn)
  {
    printf("Open COM %d failed !\n", com_port + 1);
    system("pause");
    return 1;
  }
  else
  {
    printf("Open COM %d successfully !\n", com_port + 1);
    printf("==============================================\n");
  }


  /* ----- EMUCInitCAN() ----- */
  rtn = EMUCInitCAN(com_port, EMUC_INACTIVE, EMUC_INACTIVE);
  if(rtn)
    printf("EMUC initial CAN failed !\n");
  else
  {
    printf("EMUC initial CAN successfully !\n");
    printf("==============================================\n");
  }

  /* ----- EMUCShowVer() ----- */
  rtn = EMUCShowVer(com_port, &ver_info);
  if(rtn)
    printf("EMUC show version failed !\n");
  else
  {    
    printf("EMUC show version successfully !\n");
    printf("FW ver: %s\n",  ver_info.fw);
    printf("LIB ver: %s\n", ver_info.api);
    printf("Model: %s\n",   ver_info.model);
    printf("==============================================\n");
  }

  /* ----- EMUCResetCAN() ----- */
  rtn = EMUCResetCAN(com_port);
  if(rtn)
    printf("EMUC reset CAN failed !\n");
  else
  {
    printf("EMUC reset CAN successfully !\n");
    printf("==============================================\n");
  }


  /* ----- EMUCClearFilter() ----- */
  CAN_port = EMUC_CAN_1;
  rtn = EMUCClearFilter(com_port, CAN_port);
  if(rtn)
    printf("EMUC clear filter failed !\n");
  else
  {
    printf("EMUC clear filter successfully !\n");
    printf("==============================================\n");
  }


  /* ----- EMUCSetBaudRate() ----- */
  rtn = EMUCSetBaudRate(com_port, EMUC_BAUDRATE_1M, EMUC_BAUDRATE_1M);
  if(rtn)
    printf("EMUC set baud rate failed !\n");
  else
  {
    printf("EMUC set baud rate successfully !\n");
    printf("==============================================\n");
  }


  /* ----- EMUCSetErrorType() ----- */
  rtn = EMUCSetErrorType(com_port, EMUC_DIS_ALL);
  if(rtn)
    printf("EMUC set error type failed !\n");
  else
  {
    printf("EMUC set error type successfully !\n");
    printf("==============================================\n");
  }


  /* ----- EMUCSetMode() ----- */
  rtn = EMUCSetMode(com_port, EMUC_NORMAL, EMUC_NORMAL);
  if(rtn)
    printf("EMUC set mode failed !\n");
  else
  {
    printf("EMUC set mode successfully !\n");
    printf("==============================================\n");
  }


  /* ----- EMUCSetFilter() ----- */
  filter_info.CAN_port = EMUC_CAN_1;
  filter_info.flt_type = EMUC_EID;
  filter_info.flt_id   = 0x0000ABCD;
  filter_info.flt_id   = 0x0012ABCD;
  filter_info.mask     = 0x1FFFFFFF;

  rtn = EMUCSetFilter(com_port, &filter_info);
  if(rtn)
    printf("EMUC set CAN 1 filter failed !\n");
  else
  {
    printf("EMUC set CAN 1 filter successfully !\n");
    printf("==============================================\n");
  }

  filter_info.CAN_port = EMUC_CAN_2;
  filter_info.flt_type = EMUC_EID;
  filter_info.flt_id   = 0x00001234;
  filter_info.mask     = 0x00FFEEEE;

  rtn = EMUCSetFilter(com_port, &filter_info);
  if(rtn)
    printf("EMUC set CAN 2 filter failed !\n");
  else
  {
    printf("EMUC set CAN 2 filter successfully !\n");
    printf("==============================================\n");
  }


  /* ----- EMUCGetCfg() ----- */
  rtn = EMUCGetCfg(com_port, &cfg_info);
  if(rtn)
    printf("EMUC get config. failed !\n");
  else
  {
    printf("EMUC get config. successfully !\n");
    for(i=0; i<CAN_NUM; i++)
    {
      printf("----------------------------------------------\n");
      printf("CAN %d:\n", i+1);
      printf("baud rate = %d\n", cfg_info.baud[i]);
      printf("mode = %d\n", cfg_info.mode[i]);
      printf("filter type = %d\n", cfg_info.flt_type[i]);
      printf("filter id = %08X\n", cfg_info.flt_id[i]);
      printf("filter mask = %08X\n", cfg_info.flt_mask[i]);

    }
    printf("----------------------------------------------\n");
    printf("error set = %d\n", cfg_info.err_set);
    printf("==============================================\n");
  }


  /* ----- EMUCExpCfg() ----- */
  rtn = EMUCExpCfg(com_port, file_name);
  if(rtn)
    printf("EMUC export config. failed !\n");
  else
  {
    printf("EMUC export config. successfully !\n");
    printf("==============================================\n");
  }


  /* ----- EMUCImpCfg() ----- */
  rtn = EMUCImpCfg(com_port, file_name);
  if(rtn)
    printf("EMUC import config. failed !\n");
  else
  {
    printf("EMUC import config. successfully !\n");
    printf("==============================================\n");
  }


  /* Send & Receive must initial CAN to EMUC_ACTIVE */
  /*-----------------------------------------------------------------------------------------*/
  EMUCClearFilter(com_port, EMUC_CAN_1);  /* not necessary */
  EMUCClearFilter(com_port, EMUC_CAN_2);  /* not necessary */
  EMUCInitCAN    (com_port, EMUC_ACTIVE, EMUC_ACTIVE);


  /* ----- EMUCSend ----- */
  memset(&frame_send, 0, sizeof(CAN_FRAME_INFO));
  frame_send.CAN_port = EMUC_CAN_1;
  frame_send.id_type  = EMUC_EID;
  frame_send.rtr      = EMUC_DIS_RTR;
  frame_send.dlc      = 8;
  frame_send.id       = 0x001234AB;
  memcpy(frame_send.data, data_send, DATA_LEN);

  for(i=0; i<2000; i++)
  {
    Sleep(1);
    EMUCSend(com_port, &frame_send);
  }

  /* ----- EMUCReceiveNonblock() ----- */
  non_block_info.cnt = 5;
  non_block_info.interval = 10 * 1000;
  non_block_info.can_frame_info = (CAN_FRAME_INFO *) malloc(non_block_info.cnt * sizeof(CAN_FRAME_INFO));

  printf("Non-block receive ------> Time start !\n");
  rtn = EMUCReceiveNonblock(com_port, &non_block_info);
  if(rtn)
  {
    printf("Non-block receive ------> Time out or Data enough !\n");
    for(i=0; i<rtn; i++)
    {
      switch (non_block_info.can_frame_info[i].msg_type)
      {
        case EMUC_DATA_TYPE:
                                print_data(non_block_info.can_frame_info[i]);
                                break;
        case EMUC_EEERR_TYPE:
                                printf("EEPROM Error !\n");
                                break;
        case EMUC_BUSERR_TYPE:
                                printf("Bus Error !\n");

                                printf("error data (CAN 1): ");
                                for(j=0; j<DATA_LEN_ERR; j++)
                                  printf("%02X ", frame_recv.data_err[EMUC_CAN_1][j]);
                                printf("\n");

                                printf("error data (CAN 2): ");
                                for(j=0; j<DATA_LEN_ERR; j++)
                                  printf("%02X ", frame_recv.data_err[EMUC_CAN_2][j]);
                                printf("\n");

                                break;
      }
    }
    printf("==============================================\n");
  }
  else
  {
    printf("Non-block receive ------> Time out (No data) !\n");
    printf("==============================================\n");
  }

  free(non_block_info.can_frame_info);


  /* ----- EMUCReceive() ----- */
  _beginthread(TRD_recv_fx, 0, NULL);
  printf("EMUC reveice start ...\n");


#if 0
  /* ----- EMUCCloseDevice() ----- */
  EMUCCloseDevice(com_port);
#endif

  while(1)
    Sleep(10000);

  return 0;

} /* END: main() */



/*----------------------------------------------------------------------*/
static void TRD_recv_fx (void *ptr)
{
  int  i;
  int  rtn;

  while(1)
  {
    memset(&frame_recv, 0, sizeof(CAN_FRAME_INFO));
    rtn = EMUCReceive(com_port, &frame_recv);

    if(rtn == 1)
    {
      switch (frame_recv.msg_type)
      {
        case EMUC_DATA_TYPE:
                                print_data(frame_recv);
                                break;
        case EMUC_EEERR_TYPE:
                                printf("EEPROM Error !\n");
                                break;
        case EMUC_BUSERR_TYPE:
                                printf("Bus Error !\n");
                                printf("error data (CAN 1): ");
                                for(i=0; i<DATA_LEN_ERR; i++)
                                  printf("%02X ", frame_recv.data_err[EMUC_CAN_1][i]);
                                printf("\n");

                                printf("error data (CAN 2): ");
                                for(i=0; i<DATA_LEN_ERR; i++)
                                  printf("%02X ", frame_recv.data_err[EMUC_CAN_2][i]);
                                printf("\n");
                                break;
      }
    }
  }

} /* END: TRD_recv_fx() */


/*----------------------------------------------------------------------*/
static int load_dll (void)
{
  #if defined(_M_X64) || defined(__amd64__)
    hDLL = LoadLibrary(TEXT("lib_emuc2_64.dll"));
  #else
    hDLL = LoadLibrary(TEXT("lib_emuc2_32.dll"));
  #endif

  if(hDLL == NULL)
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

    if(!EMUCOpenDevice || !EMUCCloseDevice || !EMUCShowVer || !EMUCResetCAN  || !EMUCClearFilter  ||
       !EMUCInitCAN    || !EMUCSetBaudRate || !EMUCSetMode || !EMUCSetFilter || !EMUCSetErrorType ||
       !EMUCGetCfg     || !EMUCExpCfg      || !EMUCImpCfg  || !EMUCSend      || !EMUCReceive      ||
       !EMUCReceiveNonblock)
    {
      return 1;
    }

    return 0;
  }

  return 1;

} /* END: load_dll() */



/*----------------------------------------------------------------------*/
static int find_emuc (void)
{
  int  i;
  int  rtn;

  for(i=0; i<MAX_COM_NUM; i++)
  {
    rtn = EMUCOpenDevice(i);

    if(rtn == 0)
    {
      EMUCCloseDevice(i);
      return i;
    }
  }

  return -1;

} /* END: find_emuc() */



/*----------------------------------------------------------------------*/
static void print_data (CAN_FRAME_INFO frame_info)
{
  static unsigned long cnt_1 = 0;
  static unsigned long cnt_2 = 0;

  int i;

  /* CAN port & cnt */
  if(frame_info.CAN_port == EMUC_CAN_1)
  {
    cnt_1++;
    printf("(CAN 1) %lu. ", cnt_1);
  }
  else if(frame_info.CAN_port == EMUC_CAN_2)
  {
    cnt_2++;
    printf("(CAN 2) %lu. ", cnt_2);
  }

  /* ID */
  if(frame_info.id_type == EMUC_SID)
    printf("ID: %03X; ", frame_info.id);
  else if(frame_info.id_type == EMUC_EID)
    printf("ID: %08X; ", frame_info.id);
  
  /* Data */
  printf("Data: ");
  for(i=0; i<frame_info.dlc; i++)
    printf("%02X ", frame_info.data[i]);
  printf("\n");

} /* END: print_data() */