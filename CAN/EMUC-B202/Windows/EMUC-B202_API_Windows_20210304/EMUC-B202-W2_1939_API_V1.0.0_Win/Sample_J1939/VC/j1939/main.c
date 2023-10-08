#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#include "lib_j1939.h"

#define   MAX_COM_NUM   256

#if defined(_M_X64) || defined(__amd64__)
  #define  LIB_J1939  "lib_j1939_64.dll"
#else
  #define  LIB_J1939  "lib_j1939_32.dll"
#endif


/* To record the SA of each port */
uint8_t sa_record [CAN_PORT_NUM];

/* To load emuc library */
HMODULE hDLL = NULL;

/* static function prototype */
static int  find_emuc     (void);
static int  load_dll      (void);
static void print_cb_data (J1939_FRAME_INFO frame);
static void proc_norm_pgn (J1939_CB_INFO  *cb_info);
static void proc_req_pgn  (J1939_CB_INFO  *cb_info);
static void proc_change_sa(J1939_CB_INFO  *cb_info);
static void proc_cmd_sa   (J1939_CB_INFO  *cb_info);
static void j1939_cb_handler (void *ptr);


/*-----------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
  int               i;
  int               rtn;
  int               com_port;
  uint8_t           default_sa [CAN_PORT_NUM] = {20, 30};
  uint8_t           buf_req_1  [3]  = {0x04, 0xF0, 0x00};
  uint8_t           buf_req_2  [3]  = {0x03, 0xF0, 0x00};
  uint8_t           buf_norm   [8]  = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  uint8_t           buf_cmd_sa [9]  = {0xC9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA};
  uint8_t           buf_multi  [16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                       0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

  J1939_CB_INFO     cb_info;
  J1939_INIT_INFO   init;
  J1939_FRAME_INFO  frame;


  /* find EMUC device */
  com_port = find_emuc();

  if(com_port != -1)
    printf("Find EMUC device: COM %d\n", com_port + 1);
  else
  {
    printf("No EMUC device !\n");
    return 1;
  }


  /* load j1939 library */
  rtn = load_dll();
  if(rtn)
  {
    printf("Load %s failed !\n", LIB_J1939);
    getchar();
    return 1;
  }


  /* EMUCJ1939RegCbFunc */
  cb_info.cb_func = j1939_cb_handler;
  EMUCJ1939RegCbFunc(&cb_info);


  /* EMUCJ1939Init */
  init.com_port = com_port;

  for(i=0; i<CAN_PORT_NUM; i++)
  {
    sa_record[i] = default_sa[i];

    init.sa[i] = default_sa[i];
    init.name[i].aac          = 0;
    init.name[i].ind_grp      = 0;
    init.name[i].veh_sys_inst = 0;
    init.name[i].veh_sys      = 0;
    init.name[i].func         = 0;
    init.name[i].func_inst    = 0;
    init.name[i].ecu_inst     = 0;
    init.name[i].mfg_code     = 0;
    init.name[i].identy_num   = 200 + i;
  }

  rtn = EMUCJ1939Init (init);

  if(rtn == 0)
  {
    printf("J1939 init successfully !\n");

    for(i=0; i<CAN_PORT_NUM; i++)
    {
      printf("\n");
      printf("CAN %d\n", i+1);
      printf("------------------------------\n");
      printf("Source Address            = %u\n", init.sa[i]);
      printf("Arbitrary Address Capable = %u\n", init.name[i].aac);
      printf("Industry Group            = %u\n", init.name[i].ind_grp);
      printf("Vehicle System Instance   = %u\n", init.name[i].veh_sys_inst);
      printf("Vehicle System            = %u\n", init.name[i].veh_sys);
      printf("Function                  = %u\n", init.name[i].func);
      printf("Function Instance         = %u\n", init.name[i].func_inst);
      printf("ECU Instance              = %u\n", init.name[i].ecu_inst);
      printf("Manufacturer Code         = %u\n", init.name[i].mfg_code);
      printf("Identity Number           = %u\n", init.name[i].identy_num);
    }
  }
  else
  {
    printf("J1939 init failed ! (error code: %d)\n", rtn);
    return 1;
  }

  /* EMUCJ1939Send */
  /* PDU 1 */
  Sleep(1000);
  frame.pgn     = 256;
  frame.buf     = buf_norm;
  frame.buf_len = 8;
  frame.dst     = 30;
  frame.pri     = 6;
  frame.port    = CAN_1;

  rtn = EMUCJ1939Send(frame);
  if(rtn)
    printf("EMUCJ1939Send PDU1 failed !\n");


  /* PDU 2 */
  Sleep(1000);
  frame.pgn     = 61444;
  frame.buf     = buf_norm;
  frame.buf_len = 8;
  frame.pri     = 6;
  frame.port    = CAN_1;

  rtn = EMUCJ1939Send(frame);
  if(rtn)
    printf("EMUCJ1939Send PDU2 failed !\n");


  /* Multi-byte (> 8 bytes) */
  Sleep(1000);
  frame.pgn     = 256;
  frame.buf     = buf_multi;
  frame.buf_len = 16;
  frame.dst     = 255;
  frame.pri     = 6;
  frame.port    = CAN_1;

  rtn = EMUCJ1939Send(frame);
  if(rtn)
    printf("EMUCJ1939Send Multi-byte failed !\n");


  /* REQUEST 1 */
  Sleep(1000);
  frame.pgn     = J1939_PGN_REQUEST;
  frame.buf     = buf_req_1;
  frame.buf_len = 3;
  frame.dst     = 255;
  frame.pri     = 6;
  frame.port    = CAN_1;

  rtn = EMUCJ1939Send(frame);
  if(rtn)
    printf("EMUCJ1939Send REQUEST failed !\n");


  /* REQUEST 2 */
  Sleep(1000);
  frame.pgn     = J1939_PGN_REQUEST;
  frame.buf     = buf_req_2;
  frame.buf_len = 3;
  frame.dst     = 255;
  frame.pri     = 6;
  frame.port    = CAN_1;

  rtn = EMUCJ1939Send(frame);
  if(rtn)
    printf("EMUCJ1939Send REQUEST failed !\n");


  /* Commanded SA */
  Sleep(1000);
  frame.pgn     = J1939_PGN_COMMANDED_ADDRESS;
  frame.buf     = buf_cmd_sa;
  frame.buf_len = 9;
  frame.dst     = 255;
  frame.pri     = 6;
  frame.port    = CAN_1;

  rtn = EMUCJ1939Send(frame);
  if(rtn)
    printf("EMUCJ1939Send Commanded SA failed !\n");


  while(1)
    Sleep(1);

  /* EMUCJ1939Stop */
  EMUCJ1939Stop(com_port);

  return 0;

} /* END: main() */



/*-----------------------------------------------------------------------*/
static int find_emuc (void)
{
  typedef int (*EMUC_OPEN_DEVICE)  (int com_port);
  typedef int (*EMUC_CLOSE_DEVICE) (int com_port);

  int                 i;
  int                 rtn;
  HMODULE             hDLL = NULL;
  EMUC_OPEN_DEVICE    EMUCOpenDevice;
  EMUC_CLOSE_DEVICE   EMUCCloseDevice;
  
  #if defined(_M_X64) || defined(__amd64__)
    const char *lib_emuc = "lib_emuc2_64.dll";
  #else
    const char *lib_emuc = "lib_emuc2_32.dll";
  #endif

  hDLL = LoadLibrary(lib_emuc);

  if(hDLL)
  {
    EMUCOpenDevice  = (EMUC_OPEN_DEVICE)  GetProcAddress(hDLL, "EMUCOpenDevice");
    EMUCCloseDevice = (EMUC_CLOSE_DEVICE) GetProcAddress(hDLL, "EMUCCloseDevice");
  }
  else
  {
    printf("Load %s failed !\n", lib_emuc);
	return -1;
  }

  for(i=0; i<MAX_COM_NUM; i++)
  {
    rtn = EMUCOpenDevice(i);

    if(rtn == 0)
    {
      EMUCCloseDevice(i);
      FreeLibrary(hDLL);
      return i;
    }
  }

  FreeLibrary(hDLL);
  return -1;

} /* END: find_emuc() */



/*-----------------------------------------------------------------------*/
static int load_dll (void)
{
  hDLL = LoadLibrary(LIB_J1939);

  if(hDLL == NULL)
    return 1;
  else
  {
    EMUCJ1939Init      = (EMUC_J1939_INIT)        GetProcAddress(hDLL, "EMUCJ1939Init");
    EMUCJ1939Send      = (EMUC_J1939_SEND)        GetProcAddress(hDLL, "EMUCJ1939Send");
    EMUCJ1939RegCbFunc = (EMUC_J1939_REG_CB_FUNC) GetProcAddress(hDLL, "EMUCJ1939RegCbFunc");
    EMUCJ1939Stop      = (EMUC_J1939_STOP)        GetProcAddress(hDLL, "EMUCJ1939Stop");

    if(!EMUCJ1939Init || !EMUCJ1939Send || !EMUCJ1939RegCbFunc || !EMUCJ1939Stop)
    {
      printf("Get function from %s failed !\n", LIB_J1939);
      return 1;
    }

    return 0;
  }

  return 1;

} /* END: load_dll() */




/*-----------------------------------------------------------------------*/
static void print_cb_data (J1939_FRAME_INFO  frame)
{
  int  i;

  printf("\n=============================================\n");
  printf("PGN:  %u\n", frame.pgn);
  printf("Len:  %u\n", frame.buf_len);
  printf("DA:   %u\n", frame.dst);
  printf("SA:   %u\n", frame.src);
  printf("Pri:  %u\n", frame.pri);
  printf("Port: %u\n", frame.port + 1);
  printf("Data: ");
  for(i=0; i<frame.buf_len; i++)
    printf("%02X ", frame.buf[i]);
  printf("\n---------------------------------------------\n");


} /* END: print_cb_data() */



/*-----------------------------------------------------------------------*/
static void proc_norm_pgn (J1939_CB_INFO *cb_info)
{
  print_cb_data(cb_info->frame);

  switch(cb_info->frame.pgn)
  {
    case 59392:
                  {
                    char *ACK_type[] = {"Positive ACK", "Negative ACK", "Access Denied", "Cannot Respond"};
                    printf("Acknowledgment\n");
                    printf("ACK type: %s\n", ACK_type[cb_info->frame.buf[0]]);
                    break;
                  }

    case 60928:   /* Address Claimed */
                  printf("Address Claimed\n");
                  break;

    case 61443:   /* Electronic Engine Controller 2 */
                  printf("Electronic Engine Controller 2\n");
                  break;

    case 61444:   /* Electronic Engine Controller 1 */
                  printf("Electronic Engine Controller 1\n");
                  break;

    case 65262:   /* Engine Temperature */
                  printf("Engine Temperature\n");
                  break;

    case 65269:   /* Ambient Conditions */
                  printf("Ambient Conditions\n");
                  break;

    case 65263:   /* Engine Fluid Level/Pressure */
                  printf("Engine Fluid Level/Pressure\n");
                  break;

    default:
                  printf("Please look up SAE J1939 PGN table\n");
                  break;
  }


} /* END: proc_norm_pgn() */



/*-----------------------------------------------------------------------*/
static void proc_req_pgn (J1939_CB_INFO *cb_info)
{
  static uint32_t serv_pgn[CAN_PORT_NUM] = {61443, 61444};
  uint8_t         req_port;

  req_port = cb_info->sa_req_port;

  if(serv_pgn[req_port] == cb_info->req_pgn)
    cb_info->ack_type = ACK_P;
  else
    cb_info->ack_type = ACK_N;

  print_cb_data (cb_info->frame);
  printf("Requested PGN: %u\n", cb_info->req_pgn);

} /* END: proc_req_pgn() */



/*-----------------------------------------------------------------------*/
static void proc_change_sa (J1939_CB_INFO *cb_info)
{
  static uint8_t sa_1 = 253;
  static uint8_t sa_2 = 0;
  uint8_t        sa_port;
  uint8_t        sa_ori;
  uint8_t        sa_change;

  sa_port = cb_info->sa_req_port;

  /* CAN1: 253, 252, ..., 0 */
  if(sa_port == CAN_1)
  {
    sa_change          = sa_1;
    sa_ori             = sa_record[sa_port];
    sa_record[sa_port] = sa_change;

    cb_info->sa = sa_change;
  }

  /* CAN2: 0, 1, ..., 253 */
  if(sa_port == CAN_2)
  {
    sa_change          = sa_2;
    sa_ori             = sa_record[sa_port];
    sa_record[sa_port] = sa_change;

    cb_info->sa = sa_change;
  }

  printf("\n=============================================\n");
  printf("CAN %u lost the competition\n", sa_port + 1);
  printf("Change SA from %u to %u\n", sa_ori, sa_change);

  if(sa_1 == 0)    sa_1 = 254;
  else             sa_1--;

  if(sa_2 == 253)  sa_2 = 254;
  else             sa_2++;


} /* END: proc_change_sa() */



/*-----------------------------------------------------------------------*/
static void proc_cmd_sa (J1939_CB_INFO *cb_info)
{
  uint8_t   sa_port;
  uint8_t   sa_ori;
  uint8_t   sa_change;

  sa_port   = cb_info->sa_req_port;
  sa_change = cb_info->sa;
  sa_ori    = sa_record[sa_port];

  printf("\n=============================================\n");
  printf("CAN %u rececive a commanded address (PGN = 65240)\n", sa_port + 1);
  printf("Change SA from %u to %u\n", sa_ori, sa_change);


} /* END: proc_cmd_sa() */




/*-----------------------------------------------------------------------*/
static void j1939_cb_handler (void *ptr)
{
  J1939_CB_INFO  *cb_info = (J1939_CB_INFO *) ptr;

  switch(cb_info->msg_type)
  {
    case NORMAL_PGN:
                      proc_norm_pgn(cb_info);
                      break;

    case REQUEST_PGN:
                      proc_req_pgn(cb_info);
                      break;

    case CHANGE_SA:
                      proc_change_sa(cb_info);
                      break;

    case CMD_SA:
                      proc_cmd_sa(cb_info);
                      break;

    default:
                      break;
  }


} /* END: j1939_cb_handler() */