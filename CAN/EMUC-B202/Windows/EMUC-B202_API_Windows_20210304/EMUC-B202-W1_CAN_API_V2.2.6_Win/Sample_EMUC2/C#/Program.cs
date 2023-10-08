using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace helloword_cs
{
  class Program
  {
    #if __WIN32__
      const string LibName = "lib_emuc2_32.dll";
    #elif __WIN64__
      const string LibName = "lib_emuc2_64.dll";
    #endif

    /* load .dll */
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCOpenDevice       (int com_port);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCCloseDevice      (int com_port);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCShowVer          (int com_port, ref VER_INFO ver_info);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCResetCAN         (int com_port);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCClearFilter      (int com_port, int CAN_port);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCInitCAN          (int com_port, int CAN1_sts, int CAN2_sts);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCSetBaudRate      (int com_port, int CAN1_baud, int CAN2_baud);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCSetMode          (int com_port, int CAN1_mode, int CAN2_mode);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCSetFilter        (int com_port, ref FILTER_INFO filter_info);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCSetErrorType     (int com_port, int err_type);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCGetCfg           (int com_port, ref CFG_INFO cfg_info);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCExpCfg           (int com_port, [MarshalAs(UnmanagedType.LPStr)] string file_name);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCImpCfg           (int com_port, [MarshalAs(UnmanagedType.LPStr)] string file_name);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCSend             (int com_port, ref CAN_FRAME_INFO can_frame_info);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCReceive          (int com_port, ref CAN_FRAME_INFO can_frame_info);
    [DllImport(LibName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
    public static extern int EMUCReceiveNonblockCS(int com_port, uint cnt, uint interval, [Out] CAN_FRAME_INFO[] can_frame_info);

    /* structure */
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct VER_INFO
    {
      [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
      public string fw;
      [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
      public string api;
      [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
      public string model;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct  FILTER_INFO
    {
      public int   CAN_port;
      public int   flt_type;
      public uint  flt_id;
      public uint  mask;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct CFG_INFO
    {
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
      public byte[]  baud;
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
      public byte[]  mode;
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
      public byte[]  flt_type;
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
      public uint[]  flt_id;
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
      public uint[]  flt_mask;

      public byte    err_set;
    }


    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct CAN_FRAME_INFO
    {
      public int     CAN_port;
      public int     id_type;
      public int     rtr;
      public int     dlc;
      public int     msg_type;

      [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 13)]
      public string  recv_time;  /* e.g., 15:30:58:789 (h:m:s:ms) */
      
      public uint    id;
      
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
      public byte[]  data;
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 12)]
      public byte[]  data_err;
    }

    /* enumeration */
    enum CANPort : int
    {
      EMUC_CAN_1 = 0,
      EMUC_CAN_2
    };

    /*----------------------*/
    enum BaudRate : int
    {
      EMUC_BAUDRATE_100K = 4,
      EMUC_BAUDRATE_125K,
      EMUC_BAUDRATE_250K,
      EMUC_BAUDRATE_500K,
      EMUC_BAUDRATE_800K,
      EMUC_BAUDRATE_1M
    };

    /*----------------------*/
    enum CANStatus : int
    {
      EMUC_INACTIVE = 0,
      EMUC_ACTIVE
    };

    /*----------------------*/
    enum CANMode : int
    {
      EMUC_NORMAL = 0,
      EMUC_LISTEN
    };

    /*----------------------*/
    enum IDType : int
    {
      EMUC_SID = 1,
      EMUC_EID
    };

    /*----------------------*/
    enum RTRSelect : int
    {
      EMUC_DIS_RTR = 0,
      EMUC_EN_RTR
    };

    /*----------------------*/
    enum ErrType : int
    {
      EMUC_DIS_ALL = 0,
      EMUC_EE_ERR,
      EMUC_BUS_ERR,
      EMUC_EN_ALL = 255
    };

    /*----------------------*/
    enum MsgType : int
    {
      EMUC_DATA_TYPE = 0,
      EMUC_EEERR_TYPE,
      EMUC_BUSERR_TYPE
    };


    /* global variables */
    public class Global
    {
      public const  int MAX_COM_NUM  = 256;
      public const  int CAN_NUM      = 2;
      public const  int DATA_LEN     = 8;
      public const  int DATA_LEN_ERR = 6;

      public static int com_port;
      public static int CAN_port;

      [MarshalAs(UnmanagedType.LPStr)]
      public static string file_name = "emuc_config";

      public static int data_send_cnt = 500;
      public static byte[] data_send  = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

      public static uint recv_cnt1 = 0;
      public static uint recv_cnt2 = 0;

      public static VER_INFO       ver_info       = new VER_INFO();
      public static FILTER_INFO    filter_info    = new FILTER_INFO();
      public static CFG_INFO       cfg_info       = new CFG_INFO();
      public static CAN_FRAME_INFO frame_send     = new CAN_FRAME_INFO();
      public static CAN_FRAME_INFO frame_recv     = new CAN_FRAME_INFO();      

      public static Thread TRDRecv = new Thread(TRDRecvFx);
    }

    /* Entry point */
    /*---------------------------------------------------------------------*/
    static void Main (string[] args)
    {
      int   i;
      int   j;
      int   rtn;

      /* for EMUCReceiveNonblockCS() */
      uint               cnt;
      uint               interval;
      CAN_FRAME_INFO[]   frame_non_block;
      /*-----------------------------*/


      Global.com_port = FindEmucDev();

      if(Global.com_port >= 0)
      {
        Console.WriteLine("EMUC open device (COM {0}) successfully !\n", Global.com_port + 1);


        /* ----- EMUCInitCAN() ----- */
        rtn = EMUCInitCAN(Global.com_port, CANStatus.EMUC_INACTIVE.GetHashCode(), CANStatus.EMUC_INACTIVE.GetHashCode());
        if(rtn == 0)
          Console.WriteLine("EMUC initial CAN successfully !\n");
        else
          Console.WriteLine("EMUC initial CAN failed !\n");


        /* ----- EMUCShowVer() ----- */
        rtn = EMUCShowVer(Global.com_port, ref Global.ver_info);
        if(rtn == 0)
        {
          Console.WriteLine("EMUC show version successfully !");
          Console.WriteLine("FW ver:  {0}", Global.ver_info.fw);
          Console.WriteLine("LIB ver: {0}", Global.ver_info.api);
          Console.WriteLine("Model:   {0}\n", Global.ver_info.model);
        }
        else
          Console.WriteLine("EMUC show version failed !\n");

        /* ----- EMUCResetCAN() ----- */
        rtn = EMUCResetCAN(Global.com_port);
        if(rtn == 0)
          Console.WriteLine("EMUC reset CAN successfully !\n");
        else
          Console.WriteLine("EMUC reset CAN failed !\n");


        /* ----- EMUCClearFilter() ----- */
        Global.CAN_port = CANPort.EMUC_CAN_1.GetHashCode();
        rtn = EMUCClearFilter(Global.com_port, Global.CAN_port);
        if(rtn == 0)
          Console.WriteLine("EMUC clear filter successfully !\n");
        else
          Console.WriteLine("EMUC clear filter failed !\n");


        /* ----- EMUCSetBaudRate() ----- */
        rtn = EMUCSetBaudRate(Global.com_port, BaudRate.EMUC_BAUDRATE_1M.GetHashCode(), BaudRate.EMUC_BAUDRATE_1M.GetHashCode());
        if(rtn == 0)
          Console.WriteLine("EMUC set baud rate successfully !\n");
        else
          Console.WriteLine("EMUC set baud rate failed !\n");


        /* ----- EMUCSetErrorType() ----- */
        rtn = EMUCSetErrorType(Global.com_port, ErrType.EMUC_DIS_ALL.GetHashCode());
        if(rtn == 0)
          Console.WriteLine("EMUC set error type successfully !\n");
        else
          Console.WriteLine("EMUC set error type failed !\n");


        /* ----- EMUCSetMode() ----- */
        rtn = EMUCSetMode(Global.com_port, CANMode.EMUC_NORMAL.GetHashCode(), CANMode.EMUC_NORMAL.GetHashCode());
        if(rtn == 0)
          Console.WriteLine("EMUC set mode successfully !\n");
        else
          Console.WriteLine("EMUC set mode failed !\n");


        /* ----- EMUCSetFilter() ----- */
        Global.filter_info.CAN_port = CANPort.EMUC_CAN_1.GetHashCode();
        Global.filter_info.flt_type = IDType.EMUC_EID.GetHashCode();
        Global.filter_info.flt_id   = 0x0000ABCD;
        Global.filter_info.mask     = 0x1FFFFFFF;

        rtn = EMUCSetFilter(Global.com_port, ref Global.filter_info);
        if(rtn == 0)
          Console.WriteLine("EMUC set CAN 1 filter successfully !\n");
        else
          Console.WriteLine("EMUC set CAN 1 filter failed !\n");

        Global.filter_info.CAN_port = CANPort.EMUC_CAN_2.GetHashCode();
        Global.filter_info.flt_type = IDType.EMUC_EID.GetHashCode();
        Global.filter_info.flt_id   = 0x00001234;
        Global.filter_info.mask     = 0x00FFEEEE;

        rtn = EMUCSetFilter(Global.com_port, ref Global.filter_info);
        if(rtn == 0)
          Console.WriteLine("EMUC set CAN 2 filter successfully !\n");
        else
          Console.WriteLine("EMUC set CAN 2 filter failed !\n");


        /* ----- EMUCGetCfg() ----- */
        rtn = EMUCGetCfg(Global.com_port, ref Global.cfg_info);
        if(rtn == 0)
        {
          Console.WriteLine("EMUC get config. successfully !");

          for(i=0; i<Global.CAN_NUM; i++)
          {
            Console.WriteLine("CAN {0}:", i+1);
            Console.WriteLine("baud rate   = {0}",   Global.cfg_info.baud[i]);
            Console.WriteLine("mode        = {0}",   Global.cfg_info.mode[i]);
            Console.WriteLine("filter type = {0}",   Global.cfg_info.flt_type[i]);
            Console.WriteLine("filter id   = {0:X8}",   Global.cfg_info.flt_id[i]);
            Console.WriteLine("filter mask = {0:X8}\n", Global.cfg_info.flt_mask[i]);
          }

          Console.WriteLine("error set = {0}\n", Global.cfg_info.err_set);
        }
        else
          Console.WriteLine("EMUC get config. failed !\n");


        /* ----- EMUCExpCfg() ----- */
        rtn = EMUCExpCfg(Global.com_port, Global.file_name);
        if(rtn == 0)
          Console.WriteLine("EMUC export config. successfully !\n");
        else
          Console.WriteLine("EMUC export config. failed !\n");


        /* ----- EMUCImpCfg() ----- */
        rtn = EMUCImpCfg(Global.com_port, Global.file_name);
        if(rtn == 0)
          Console.WriteLine("EMUC import config. successfully !\n");
        else
          Console.WriteLine("EMUC import config. failed !\n");


        /* Send & Receive must initial CAN to EMUC_ACTIVE */
        /*---------------------------------------------------------------------------*/
        EMUCClearFilter(Global.com_port, CANPort.EMUC_CAN_1.GetHashCode());  /* not necessary */
        EMUCClearFilter(Global.com_port, CANPort.EMUC_CAN_2.GetHashCode());  /* not necessary */
        EMUCInitCAN    (Global.com_port, CANStatus.EMUC_ACTIVE.GetHashCode(), CANStatus.EMUC_ACTIVE.GetHashCode());


        /* ----- EMUCSend ----- */
        Global.frame_send.CAN_port = CANPort.EMUC_CAN_1.GetHashCode();
        Global.frame_send.id_type  = IDType.EMUC_EID.GetHashCode();
        Global.frame_send.rtr      = RTRSelect.EMUC_DIS_RTR.GetHashCode();
        Global.frame_send.dlc      = 8;
        Global.frame_send.id       = 0x001234AB;
        Global.frame_send.data     = Global.data_send;

        Console.WriteLine("EMUC send started !");
        for(i=0; i<Global.data_send_cnt; i++)
        {
          Thread.Sleep(1);
          EMUCSend(Global.com_port, ref Global.frame_send);
        }
        Console.WriteLine("EMUC send finished (with {0} data) !\n", Global.data_send_cnt);


        /* ----- EMUCReceiveNonblock() ----- */
        cnt = 5;
        interval = 5 * 1000;
        frame_non_block = new CAN_FRAME_INFO[cnt];

        Console.WriteLine("Non-block ---------------------> Time start !");

        rtn = EMUCReceiveNonblockCS(Global.com_port, cnt, interval, frame_non_block);

        if(rtn > 0)
        {
          Console.WriteLine("Non-block ---------------------> Time out or Data enough !\n");
          for(i=0; i<rtn; i++)
          {
            switch (frame_non_block[i].msg_type)
            {
              case (int) MsgType.EMUC_DATA_TYPE:
                   PrintData(frame_non_block[i]);
                   break;

              case (int) MsgType.EMUC_EEERR_TYPE:
                   Console.WriteLine("EEPROM Error !");
                   break;

              case (int) MsgType.EMUC_BUSERR_TYPE:
                    Console.WriteLine("Bus Error !");

                    System.Console.Write("error data (CAN 1): ");
                    for(j=0; j<Global.DATA_LEN_ERR; j++)
                      System.Console.Write("{0:X2} ", frame_non_block[i].data_err[j]);
                    Console.WriteLine();

                    System.Console.Write("error data (CAN 2): ");
                    for(j=0; j<Global.DATA_LEN_ERR; j++)
                      System.Console.Write("{0:X2} ", frame_non_block[i].data_err[j + Global.DATA_LEN_ERR]);
                    Console.WriteLine();
                    break;

              default:
                   break;
            }
          }
        }
        else
          Console.WriteLine("Non-block ---------------------> Time out (No data) !\n");


        /* ----- EMUCReceive() ----- */
        Global.TRDRecv.Start();
        Console.WriteLine("\nEMUC reveice start ...\n");


        /* ----- EMUCCloseDevice() ----- */
        /*
        rtn = EMUCCloseDevice(Global.com_port);
        if(rtn == 0)
          Console.WriteLine("EMUC close device (COM {0}) successfully !\n", Global.com_port + 1);
        else
          Console.WriteLine("EMUC close device failed !\n");
        */

        while(true)
          Thread.Sleep(10000);
      }
      else
        Console.WriteLine("No EMUC device !\n");

      Console.ReadLine();

    } /* END: Main() */




    /* success: return COM ID, failed: return -1 */
    /*---------------------------------------------------------------------*/
    static int FindEmucDev ()
    {
      int   i;
      int   rtn = -1;

      for(i=0; i<Global.MAX_COM_NUM; i++)
      {
        if(EMUCOpenDevice(i) == 0)
        {
          rtn = i;
          break;
        }
      }

      return rtn;

    } /* END: FindEmucDev() */


    /*---------------------------------------------------------------------*/
    static void TRDRecvFx ()
    {
      int  i;
      int  rtn;

      while (true)
      {
        rtn = EMUCReceive(Global.com_port, ref Global.frame_recv);

        if(rtn == 1)
        {
          switch (Global.frame_recv.msg_type)
          {
            case  (int) MsgType.EMUC_DATA_TYPE:
                  PrintData(Global.frame_recv);
                  break;

            case  (int) MsgType.EMUC_EEERR_TYPE:
                  Console.WriteLine("EEPROM Error !");
                  break;

            case  (int) MsgType.EMUC_BUSERR_TYPE:
                  Console.WriteLine("Bus Error !");

                  System.Console.Write("error data (CAN 1): ");
                  for(i=0; i<Global.DATA_LEN_ERR; i++)
                    System.Console.Write("{0:X2} ", Global.frame_recv.data_err[i]);
                  Console.WriteLine();

                  System.Console.Write("error data (CAN 2): ");
                  for(i=0; i<Global.DATA_LEN_ERR; i++)
                    System.Console.Write("{0:X2} ", Global.frame_recv.data_err[i + Global.DATA_LEN_ERR]);
                  Console.WriteLine();
                  break;

            default:
                  break;
          }
        }
      }
    } /* END: TRDRecvFx() */


    /*---------------------------------------------------------------------*/
    static void PrintData (CAN_FRAME_INFO frame_recv)
    {
      int i;

      /* CAN port & cnt */
      if(frame_recv.CAN_port == CANPort.EMUC_CAN_1.GetHashCode())
      {
        Global.recv_cnt1++;
        System.Console.Write("(CAN 1) {0}. ", Global.recv_cnt1);
      }
      else if(frame_recv.CAN_port == CANPort.EMUC_CAN_2.GetHashCode())
      {
        Global.recv_cnt2++;
        System.Console.Write("(CAN 2) {0}. ", Global.recv_cnt2);
      }

      /* ID */
      if(frame_recv.id_type == IDType.EMUC_SID.GetHashCode())
        System.Console.Write("ID: {0:X3}; ", frame_recv.id);
      else if(frame_recv.id_type == IDType.EMUC_EID.GetHashCode())
        System.Console.Write("ID: {0:X8}; ", frame_recv.id);

      /* Data */
      System.Console.Write("Data: ");
      for(i=0; i<frame_recv.dlc; i++)
        System.Console.Write("{0:X2} ", frame_recv.data[i]);
      Console.WriteLine();

    } /* END: PrintData() */
  }
}