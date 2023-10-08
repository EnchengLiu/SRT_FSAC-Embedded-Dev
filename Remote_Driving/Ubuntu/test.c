#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<net/if.h>
#include<sys/ioctl.h>
#include<sys/socket.h>
#include<linux/can.h>
#include<linux/can/raw.h>


int main()
{
	int s,nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame[2]={{0}};
	s=socket(PF_CAN,SOCK_RAW,CAN_RAW);//创建套接字
	strcpy(ifr.ifr_name,"can0");
	ioctl(s,SIOCGIFINDEX,&ifr);//指定CAN0设备
	addr.can_family=AF_CAN;
	addr.can_ifindex=ifr.ifr_ifindex;
	bind(s,(struct sockaddr *)&addr,sizeof(addr));//将套接字与can0捆绑
	//禁用过滤原则，本进程不接受报文，只负责发送
	setsockopt(s,SOL_CAN_RAW,CAN_RAW_FILTER,NULL,0);
	//生成两个报文
	frame[0].can_id=0x11;
	frame[0].can_dlc=1;
	frame[0].data[0]='Y';
	frame[1].can_id=0x302;
	frame[1].can_dlc=1;
	frame[1].data[0]='N';
	//循环发送两个报文
	while(1)
	{
		nbytes=write(s,&frame[0],sizeof(frame[0]));//发送frame[0]
		if(nbytes != sizeof(frame[0]))
		{
			printf("send Error frame[0]\n!");
			break;//发送错误，退出
		}
		sleep(1);
		nbytes=write(s,&frame[1],sizeof(frame[0]));
		if(nbytes != sizeof(frame[1]))
		{
			printf("send Error frame[1]\n!");
			break;
		}
		sleep(1);
	}
	close(s);
	
	
	
	return 0;
}
