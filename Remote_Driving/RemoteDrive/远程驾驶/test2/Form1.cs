using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;


namespace test2
{
    
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            this.FormBorderStyle = FormBorderStyle.None;     //设置窗体为无边框样式
            this.WindowState = FormWindowState.Maximized;    //最大化窗体 
            this.BackColor = Color.LimeGreen;
            this.TransparencyKey = Color.LimeGreen;
        }
        

        private void Form1_Load(object sender, EventArgs e)
        {
            ////System.Diagnostics.Process.Start("CommandG29.exe");
            System.Diagnostics.Process.Start("start1.exe");
            System.Diagnostics.Process.Start("start2.exe");
            label7.Text = DateTime.Now.ToShortDateString();

        }

        public int old = 1;
        public int speedold = 0;
        public int angleold = 0;


        private void label4_Click(object sender, EventArgs e)
        {

        }


        private void label3_Click(object sender, EventArgs e)
        {

        }


        private void label2_Click(object sender, EventArgs e)
        {

        }

        private void Form1_MouseEnter(object sender, EventArgs e)
        {
            //label9.Text = "P";
            
        }

         void timer1_Tick(object sender, EventArgs e)
        {
            label11.Text=DateTime.Now.ToString();
            int result = CPPDLL.add(1, 299);
            if (result == 1)
            {
                result = old; 
            }
            else
            {
                old = result;
            }//防止空显示
            label12.Text = "a="+result;
            int c = result / 10000000;
            switch (c)
            {
                case 0:
                    label9.Text = "N";
                    break;
                case 1:
                    label9.Text = "D";
                    break;
                case 2:
                    label9.Text = "P";
                    break;
                case 3:
                    label9.Text = "R";
                    break;
                default:
                    break;
            }
            int speed = (result % 10000000) / 10000;
           // while (speed != speedold)
            //{
             //   if (speed > speedold)
               // {
             //       speedold = speedold + 1;
             //       label3.Text = " " + speedold;
               // }
             //   else
               // {
                //    speedold = speedold - 1;
             //       label3.Text = " " + speedold;
              //  }
           // }//速度连续化

            label3.Text =" "+speed;
            int d = (result % 10000) / 1000;//转角方向标志位
            
            int angle = (result % 1000);



            label4.Text = " " + angle;
            switch (d)
            {
                case 0:
                    label5.Text = "左" ;
                    break;
                case 1:
                    label5.Text = "右";
                    break;
                default:
                    break;
            }
        }

        private void label7_Click(object sender, EventArgs e)
        {

        }

        private void label3_Click_1(object sender, EventArgs e)
        {

        }

        private void timer2_Tick(object sender, EventArgs e)
        {
            Random rd = new Random();
            int i = rd.Next(1, 6);
            switch (i)
            {
                case 2:
                    label1.Text = "CMCC 5G    ●●○○";
                    break;
                case 1:
                    label1.Text = "CMCC 5G    ●●●○";
                    break;
                case 3:
                    label1.Text = "CMCC 5G    ●●●○";
                    break;
                case 4:
                    label1.Text = "CMCC 5G    ●●●●";
                    break;
                case 5:
                    label1.Text = "CMCC 5G    ●●●●";
                    break;
                default:
                    break;
            }
            
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }
    }
    public class CPPDLL
    {
        [DllImport("C1p.dll", EntryPoint = "?add@@YAHHH@Z", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
        public static extern int add(int x, int y);
    }

    //public class DATA
   // {
        
    //    public int d = CPPDLL.add(1, 299);
   // }
}
