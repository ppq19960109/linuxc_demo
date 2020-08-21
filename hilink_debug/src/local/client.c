#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "client.h"
#include "protocol_cover.h"
#include "socket.h"

#define SERVER_PORT 7000 //7000

#define RECVLEN 4096
char tcpBuf[RECVLEN + 1];

const char *heart =
    {"{ \
    \"Command\":\"TcpBeatHeart\", \
    \"Period\":\"60\" \
    }"};

void timer_thread(union sigval v)
{
    printf("timer_thread function! %s\n", heart);
}
void timer_signal_handler(int signal)
{
    printf("timer_signal_handler function! %s\n", heart);
    writeToHaryan(heart, protocol_data.socketfd, protocol_data.sendData, SENDTOLOCAL_SIZE);
}

timer_t start_timer()
{
    struct sigaction act;
    act.sa_handler = timer_signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);

    timer_t timerid;
    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent)); //清零初始化

    // evp.sigev_value.sival_int = 1;                //也是标识定时器的，回调函数可以获得
    // evp.sigev_notify = SIGEV_THREAD;                //线程通知的方式，派驻新线程
    // evp.sigev_notify_function = timer_thread;       //线程函数地址
    evp.sigev_signo = SIGUSR1;
    evp.sigev_notify = SIGEV_SIGNAL;

    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)
    {
        perror("fail to timer_create");
        return NULL;
    }

    /* 第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长,就是说it.it_value变0的时候会>装载it.it_interval的值 */
    struct itimerspec it;
    it.it_interval.tv_sec = 60; // 回调函数执行频率为1s运行1次
    it.it_interval.tv_nsec = 0;
    it.it_value.tv_sec = 5; // 倒计时3秒开始调用回调函数
    it.it_value.tv_nsec = 0;

    if (timer_settime(timerid, 0, &it, NULL) == -1)
    {
        perror("fail to timer_settime");
        return NULL;
    }

    return timerid;
}

int net_client_srart()
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;                     //簇
    server.sin_port = htons(SERVER_PORT);            //端口
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); //ip地址

    int sockfd = Socket(AF_INET, SOCK_STREAM);
    // Bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    while (Connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) != 0)
    {
        sleep(2);
    }
    return sockfd;
}

void *thread_hander(void *arg)
{
    int *fd = ((int *)arg);
    int i;
    while (1)
    {
        *fd = net_client_srart();
        int sockfd = *fd;
        int readLen = 0;
        timer_t timerid = start_timer();
        write_cmd("Add", NULL);
        write_cmd("DevsInfo", NULL);
        while (1)
        {
            readLen = Recv(sockfd, tcpBuf, RECVLEN, 0);
            if (readLen == 0)
            {
                log_debug("client close");
                break;
            }
            else if (readLen < 0)
            {
                break;
            }
            else
            {
                if (tcpBuf[0] == 0x02)
                {
                    tcpBuf[readLen] = '\0';
                    log_debug("%d,%s", readLen, &tcpBuf[1]);
                    for (i = 0; i < readLen; ++i)
                    {
                        if (tcpBuf[i] == 2)
                            read_from_local(&tcpBuf[i + 1]);
                    }
                }
                else
                {
                    tcpBuf[readLen] = '\0';
                    log_debug("%s", tcpBuf);
                    read_from_local(tcpBuf);
                }
            }
        }
        close(sockfd);
        timer_delete(timerid);
    }
    pthread_exit(0);
}

void net_client(int *sockfd)
{

    pthread_t id;
    //clientMethod为此线程客户端，要执行的程序。
    pthread_create(&id, NULL, (void *)thread_hander, (void *)sockfd);
    //要将id分配出去。
    pthread_detach(id);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}