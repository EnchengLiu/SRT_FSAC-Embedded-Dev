#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

 
int main()
{
   
    int s, nbytes;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame[2] = {{0}};
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);//创建套接字
    strcpy(ifr.ifr_name, "can0" );
    ioctl(s, SIOCGIFINDEX, &ifr); //指定 can0 设备
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(s, (struct sockaddr *)&addr, sizeof(addr));//将套接字与 can0 绑定
    //setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);//设置过滤规则，取消当前注释为禁用过滤规则，即不接收所有报文，不设置此项（即如当前代码被注释）为接收所有ID的报文。
    frame[0].can_id = 0x666;
    frame[0]. can_dlc = 8;
    frame[0].data[0] = 0x40;
    frame[0].data[1] = 0x20;
    frame[0].data[2] = 0x10;
    frame[0].data[3] = 0x00;
    frame[0].data[4] = 0x03;
    frame[0].data[5] = 0x04;
    frame[0].data[6] = 0x05;
    frame[0].data[7] = 0x06;
    for(int i=0;i<100;i++)
    {
        frame[0].data[7]++;
        nbytes = write(s, &frame[0], sizeof(frame[0])); //发送 frame[0]
        if(nbytes != sizeof(frame[0]))
        {
            printf("Send Error frame[0]\n!");
        }
 
        nbytes = read(s, &frame[1], sizeof(frame[1]));//接收总线上的报文保存在frame[1]中
        printf("the nbytes:%d\n", nbytes);
        printf("length:%lu", sizeof(frame[1]));
        printf("ID=0x%X DLC=%d\n", frame[1].can_id, frame[1].can_dlc);
        printf("data0=0x%02x\n",frame[1].data[0]);
        printf("data1=0x%02x\n",frame[1].data[1]);
        printf("data2=0x%02x\n",frame[1].data[2]);
        printf("data3=0x%02x\n",frame[1].data[3]);
        printf("data4=0x%02x\n",frame[1].data[4]);
        printf("data5=0x%02x\n",frame[1].data[5]);
        printf("data6=0x%02x\n",frame[1].data[6]);
        printf("data7=0x%02x\n",frame[1].data[7]);
        //sleep(1);
    }
 
    close(s);
    return 0;
}
