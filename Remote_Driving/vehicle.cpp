#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>  //多线程编程
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>

#define BUF_SIZE 21
#define SERV_PORT 8888
#define CHK_ERR(err, s) \
    if ((err) < 0)      \
    {                   \
        perror(s);      \
        exit(1);        \
    }
using namespace std;

char buf[21];//指明一个缓冲区，该缓冲区用来存放recv函数接收到的数据；
int can_send_wheel_high = 7400 / 256;
int can_send_wheel_low = 7400 % 256;
int can_send_speed_high = 0;
int can_send_speed_low = 0;
int break_amount = 0;
int V_shift = 0;
bool break_signal = false;

long timestamp() //获取时间戳
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void wait(float seconds)
{
    clock_t endwait;
    endwait = clock() + seconds * CLOCKS_PER_SEC;
    while (clock() < endwait)
    {
    }
}
void convertStrToUnChar(char *str, unsigned char *UnChar);
void Send301(unsigned &count301, int &sd, can_frame &pframe, sockaddr_can &paddr); //50ms
void Send131(unsigned &count131, int &sd, can_frame &pframe, sockaddr_can &paddr); //20ms
void Send703(int &sd, can_frame &pframe, sockaddr_can &paddr);                     //10ms
void *mythread(void *);                                                            //子线程执行的程序

//主函数
int main()
{
    int err;
    pthread_t id;
    int ret;//线程函数返回值
    int cnt;
    int on = 1;
    int sockfd;//创建的socket
    int fd_num = 0;
    struct sockaddr_in serv_addr;
    fd_set readfds;
    struct timeval select_timeval;
    ret = pthread_create(&id, NULL, &mythread, NULL); //创建新的线程,子线程执行函数mythread，线程创建成功，返回0；
    if (ret != 0)
    {
        printf("Create pthread error.\n");
        exit(1);
    }

    //主线程
    //主线程与服务器通讯，将获得到的数据放到全局变量中（车速、转向、制动、档位）
    //非阻塞模式用select实现·，收到信息就储存数组，收不到就收can线的数据
    sockfd = socket(AF_INET, SOCK_STREAM, 0);//创建失败返回-1，
    if (sockfd == -1)
    {
        printf("socket error\n");
        exit(1);
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    //与服务器建立连接部分
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = inet_addr("81.68.251.140");

    err = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
    CHK_ERR(err, "connect");
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        //每次select之前都需要设置下一次的超时时间
        select_timeval.tv_sec = 0;
        select_timeval.tv_usec = 5000;
        fd_num = select(sockfd + 1, &readfds, NULL, NULL, &select_timeval);
        /*
        到监视文件描述符集合中某个文件描述符发生变化为止；第二，若将时间值设为0秒0毫秒，就变成一个纯粹的非阻塞函数，不管文件描述符是否有变化，都立刻返回继续执行，文件无变化返回0，有变化返回一个正值；第三，timeout的值大于0，这就是等待的超时时间，即 select在timeout时间内阻塞，超时时间之内有事件到来就返回了，否则在超时后不管怎样一定返回，返回值同上述。

        */
        CHK_ERR(fd_num, "select");
        if (0 == fd_num)
        {
            usleep(100000);     //没有接收到消息就休息100ms,之后代码可以扩充在此
            continue;
        }
        else
        {
            //接收服务器信息
            cnt = recv(sockfd, buf, BUF_SIZE, 0);//返回实际接收的字节数，   
            cout << "cnt:" << cnt << "Get data from server:" << buf << "timestamp :" << timestamp() << endl;
            unsigned char Logic[10];
            convertStrToUnChar(buf, Logic);
            int zhuanxiang = Logic[1] * 256 + Logic[2]; //目前范围0~65535
            int youmen = Logic[4] * 256 + Logic[5];     //目前范围0~65535
            int shache = Logic[7] * 256 + Logic[8];     //目前范围0~65535
            int sha = shache / 256; 
            int dangwei = Logic[9]; //目前范围0~3
            switch (dangwei)
            {
            case 2:
                V_shift = 0; //前进档
                break;
            case 0:
                V_shift = 1; //倒挡
                break;
            default:
                V_shift = 2; //P和N均为N档
            }
            break_amount = (255 - sha) / 30; //制动压力
            if(break_amount > 0)
                break_signal = true;
            else
                break_signal = false;
            can_send_wheel_high = (zhuanxiang - 32768) / 1600 + 29;
            can_send_wheel_low = zhuanxiang % 256;
            can_send_speed_high = (65535 - youmen) / 18000;
            can_send_speed_low = (65535 - youmen) % 256;
        }
    }

    return 0;
}

void Send301(unsigned &count301, int &sd, can_frame &pframe, sockaddr_can &paddr) //50ms
{
    if (count301 > 15)
        count301 = 0;
    pframe.can_id = 0x301;
    if (break_signal == true)
        pframe.data[0] = 0b10000000; //N挡，128,80
    else
    {
        switch (V_shift)
        {
        case 0:
            pframe.data[0] = 0b10100000; //前进档，160，a0
            break;
        case 1:
            pframe.data[0] = 0b11000000; //倒挡,192,c0
            break;
        default:
            pframe.data[0] = 0b10000000; //P和N均为N档
        }
    }
    if (break_signal == true)
    {
        pframe.data[1] = 0x00; //速度0
        pframe.data[2] = 0x00; //速度0
    }
    else
    {
        pframe.data[1] = can_send_speed_high; //速度高位
        pframe.data[2] = can_send_speed_low;  //速度低位
    }
    pframe.data[3] = 0x04;                                                                                                        //期望加速度
    pframe.data[4] = 0x20;                                                                                                        //刹车量
    pframe.data[5] = count301 << 4;                                                                                               //循环计数位
    pframe.data[6] = (pframe.data[0] + pframe.data[1] + pframe.data[2] + pframe.data[3] + pframe.data[4] + pframe.data[5]) % 256; //校验和
    pframe.data[7] = 0x00;                                                                                                        //空
    pframe.can_dlc = 8;
    int nbyte = write(sd, &pframe, sizeof(pframe));
    if(nbyte != sizeof(pframe)) printf("Send error\n");
    count301++;
}

void Send131(unsigned &count131, int &sd, can_frame &pframe, sockaddr_can &paddr) //20ms
{
    if (count131 > 15)
        count131 = 0;
    pframe.can_id = 0x131;
    if (break_signal == true)
        pframe.data[0] = 0b01000000; //bit1(0:制动压力有效1:制动压力无效)
    else
        pframe.data[0] = 0b01000010; //bit1(0:制动压力有效1:制动压力无效)
    pframe.data[1] = 0x00;           //固定0
    if (break_signal == true)
        pframe.data[2] = break_amount; //制动压力
    else
        pframe.data[2] = 0x00; //制动压力
    pframe.data[3] = 0x00;
    pframe.data[4] = 0x00;
    pframe.data[5] = 0x00;
    pframe.data[6] = count131;                                                                                                                     //循环计数位
    pframe.data[7] = (pframe.data[0] + pframe.data[1] + pframe.data[2] + pframe.data[3] + pframe.data[4] + pframe.data[5] + pframe.data[6]) % 256; //空
    pframe.can_dlc = 8;
    int nbyte = write(sd, &pframe, sizeof(pframe));
    if(nbyte != sizeof(pframe)) printf("Send error\n");
    count131++;
}

void Send703(int &sd, can_frame &pframe, sockaddr_can &paddr) //10ms
{
    pframe.can_id = 0x703;
    pframe.data[0] = 0x01;
    pframe.data[1] = can_send_wheel_high; //转向盘高位
    pframe.data[2] = can_send_wheel_low;  //转向盘低位
    pframe.data[3] = 0x20;                //转向速度
    pframe.data[4] = 0x00;
    pframe.data[5] = 0x00;
    pframe.data[6] = 0x00;
    pframe.data[7] = (pframe.data[0] + pframe.data[1] + pframe.data[2] + pframe.data[3]) % 256;
    pframe.can_dlc = 8;
    int nbyte = write(sd, &pframe, sizeof(pframe));
    if(nbyte != sizeof(pframe)) printf("Send error\n");    
}

void *mythread(void *) //子线程执行的程序
{

    int sd;
    struct sockaddr_can paddr;
    struct ifreq pifr;
    struct can_frame pframe;
    if ((sd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        perror("Socket");
    }
    strcpy(pifr.ifr_name, "can0");
    ioctl(sd, SIOCGIFINDEX, &pifr);
    memset(&paddr, 0, sizeof(paddr));
    paddr.can_family = AF_CAN;
    paddr.can_ifindex = pifr.ifr_ifindex;
    if (bind(sd, (struct sockaddr *)&paddr, sizeof(paddr)) < 0)
    {
        perror("Bind");
    }
    unsigned count301 = 0, count131 = 0;
    while (1)
    {
        //00
        Send301(count301, sd, pframe, paddr); //50ms
        Send703(sd, pframe, paddr);           //10ms
        usleep(9600);
        //10
        Send131(count131, sd, pframe, paddr); //20ms    
        Send703(sd, pframe, paddr);           //10ms
        usleep(9600);
        //20
        Send703(sd, pframe, paddr); //10ms
        usleep(9600);
        //30
        Send131(count131, sd, pframe, paddr); //20ms
        Send703(sd, pframe, paddr);           //10ms
        usleep(9600);
        //40
        Send703(sd, pframe, paddr); //10ms
        usleep(9600);
        //50
        Send301(count301, sd, pframe, paddr); //50ms
        Send131(count131, sd, pframe, paddr); //20ms
        Send703(sd, pframe, paddr);           //10ms
        usleep(9600);
        //60
        Send703(sd, pframe, paddr); //10ms
        usleep(9600);
        //70
        Send131(count131, sd, pframe, paddr); //20ms
        Send703(sd, pframe, paddr);           //10ms
        usleep(9600);
        //80
        Send703(sd, pframe, paddr); //10ms
        usleep(9600);
        //90
        Send131(count131, sd, pframe, paddr); //20ms
        Send703(sd, pframe, paddr);           //10ms
        usleep(9600);
    }
}

void convertStrToUnChar(char *str, unsigned char *UnChar)
{
    int i = strlen(str), j = 0, counter = 0;
    char c[2];
    unsigned int bytes[2];

    for (j = 0; j < i; j += 2)
    {
        if (0 == j % 2)
        {
            c[0] = str[j];
            c[1] = str[j + 1];
            ++(c, "%02x", &bytes[0]);
            UnChar[counter] = bytes[0];
            counter++;
        }
    }
    return;
}