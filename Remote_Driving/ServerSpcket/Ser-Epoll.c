/*  epollServer.c  */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>             //socket need
#include <sys/epoll.h>              //epoll
#include <fcntl.h>                  //nonblocking
#include <sys/resource.h>           //设置最大连接数需要setrlimit

#define     MAXEPOLL    10000       //epoll监听数量上限
#define     MAXLINE     1024        //数据接收BUF大小
#define     PORT        1987        //监听端口
#define     MAXBACK     1000        //监听队列上限

int main(int argc , char**argv)
{
    int listen_fd;
    int conn_fd;
    int epoll_fd;
    int nread;
    int cur_fds;                    //当前已经存在的数量
    int wait_fds;                   //epoll_wait 的返回值
    int i;
    int yes = 1;
    struct sockaddr_in  servaddr;
    struct sockaddr_in  cliaddr;
    struct epoll_event  ev;
    struct epoll_event  evs[MAXEPOLL];
    struct rlimit       rlt;        //设置连接数所需
    char   buf[MAXLINE];
    socklen_t len = sizeof(struct sockaddr_in);

    //设置每个进程允许打开的最大文件数，每个主机是不一样的，一般服务器应该很大
    rlt.rlim_max = rlt.rlim_cur = MAXEPOLL;
    if(-1 == setrlimit(RLIMIT_NOFILE,&rlt))
    {
        perror("setrlimit");
        exit(EXIT_FAILURE);
    }

    //server套接口
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    //建立套接字
    if(-1 == (listen_fd = socket(AF_INET,SOCK_STREAM,0)))
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    //设置成非阻塞模式
    if(-1 == fcntl(listen_fd,F_SETFL,fcntl(listen_fd,F_GETFD,0)|O_NONBLOCK))
    {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

    //端口复用
    if(-1 == setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,(void *)&yes,sizeof(yes)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //绑定
    if(-1 == bind(listen_fd,(struct sockaddr *)&servaddr,sizeof(struct sockaddr)))
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    //监听
    if(-1 == listen(listen_fd,MAXBACK))
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //创建EPOLL
    epoll_fd = epoll_create(MAXEPOLL);
    ev.events = EPOLLIN|EPOLLET;
    ev.data.fd = listen_fd;
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_fd,&ev) < 0)
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
    cur_fds = 1;

    while(1)
    {
        if(-1 == (wait_fds = epoll_wait(epoll_fd,evs,cur_fds,-1)))
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for(i = 0 ; i < wait_fds ; i++)
        {
            if(evs[i].data.fd == listen_fd && cur_fds < MAXEPOLL)         //如果监听窗口有事情发生
            {
                if(-1 == (conn_fd = accept(listen_fd,(struct sockaddr *)&cliaddr,&len)))
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                printf("Server get connection from client %s:%d,new socket %d!\n",(char *)inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port,conn_fd);

                ev.events = EPOLLIN|EPOLLET;        //关心刻度（ET模式）
                ev.data.fd = conn_fd;
                if(epoll_ctl(epoll_fd,EPOLL_CLT_ADD,conn_fd,&ev) < 0)
                {
                    perror("epoll_ctl");
                    exit(EXIT_FAILURE);
                }
                ++cur_fds;
                continue;
            }
            //处理数据部分
            memset(buf,0,sizeof(buf));
            nread = read(evs[i].data.fd,buf,sizeof(buf));
            if(nread <= 0)      //结束或者出错
            {
                printf("Disconnected with the socket %d,so close it!\n",evs[i].data.fd);
                close(evs[i].data.fd);
                epoll_ctl(epoll_fd,EPOLL_CTL_DEL,evs[i].data.fd,&ev);       //删除对应fd，不再监控
                --cur_fds;
                continue;
            }
            printf("Received: %s\n",buf);
            write(evs[i].data.fd,buf,nread);
        }
    }
    close(listen_fd);
    return 0;
}