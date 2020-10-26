#if USE_LIBUV == 0 && USE_LIBEVENT == 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "local_tcp_client.h"
#include "local_send.h"
#include "local_callback.h"
#include "local_device.h"

#include "socket.h"
#include "rk_driver.h"


typedef struct
{
    int socketfd;
    pthread_t pid;
#define RECVLEN 16384
    char tcpBuf[RECVLEN + 1];
} tcp_app_t;

static tcp_app_t tcp_app;

static void main_thread_signal_handler(int signal)
{
    printf("signal is %d\n", signal);
    if (signal == SIGHUP || signal == SIGINT || signal == SIGQUIT || signal == SIGKILL || signal == SIGTERM)
    {
        if (signal == SIGQUIT)
            local_system_restartOrReFactory(INT_REBOOT);
        else
            local_system_restartOrReFactory(INT_OFFLINE);
    }
}

static void main_thread_signal_open()
{
    signal(SIGQUIT, main_thread_signal_handler);
    signal(SIGKILL, main_thread_signal_handler);
    signal(SIGTERM, main_thread_signal_handler);

    struct sigaction act, oldact;
    act.sa_handler = main_thread_signal_handler;
    sigemptyset(&act.sa_mask);

    sigaddset(&act.sa_mask, SIGQUIT); //见注(1)
    // act.sa_flags = SA_NODEFER; //| SA_NODEFER 见注(2)SA_RESETHAND
    act.sa_flags = 0; //见注(3)

    sigaction(SIGINT, &act, &oldact);
    sigaction(SIGHUP, &act, &oldact);
}
//---------------------------------------------------------------
static void timer_thread_handler(union sigval v)
{
    if (v.sival_int == 1)
    {
        write_haryan(HY_HEART, strlen(HY_HEART));
    }
}

static void timer_signal_handler(int signal)
{
    // printf("timer_signal_handler function! %s\n", HY_HEART);
    write_haryan(HY_HEART, strlen(HY_HEART));
}

timer_t start_timer(int sival, timer_function fun, int interval_sec, int sec)
{
    // struct sigaction act;
    // act.sa_handler = timer_signal_handler;
    // sigemptyset(&act.sa_mask);
    // act.sa_flags = SA_RESTART;
    // sigaction(SIGUSR1, &act, NULL);

    timer_t timerid;
    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent)); //清零初始化

    evp.sigev_value.sival_int = sival; //也是标识定时器的，回调函数可以获得
    evp.sigev_notify = SIGEV_THREAD;   //线程通知的方式，派驻新线程
    evp.sigev_notify_function = fun;   //线程函数地址

    // evp.sigev_signo = SIGUSR1;
    // evp.sigev_notify = SIGEV_SIGNAL;

    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)
    {
        perror("fail to timer_create");
        return NULL;
    }

    /* 第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长,就是说it.it_value变0的时候会>装载it.it_interval的值 */
    struct itimerspec it;
    it.it_interval.tv_sec = interval_sec; // 回调函数执行频率为1s运行1次
    it.it_interval.tv_nsec = 0;
    it.it_value.tv_sec = sec; // 倒计时3秒开始调用回调函数
    it.it_value.tv_nsec = 0;

    if (timer_settime(timerid, 0, &it, NULL) == -1)
    {
        perror("fail to timer_settime");
        return NULL;
    }

    return timerid;
}

static int net_client_srart()
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
//---------------------------------------------------------------
extern void hlink_online_ledStatus(void);
static void *thread_hander(void *arg)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGIO);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    int readLen = 0;
    timer_t timerid = NULL;
    tcp_app_t *pdata = ((tcp_app_t *)arg);
    do
    {
        pdata->socketfd = net_client_srart();
        hlink_online_ledStatus();

        timerid = start_timer(1, timer_thread_handler, 60, 60);
        readLen = write_hanyar_cmd(STR_ADD, NULL, STR_NET_CLOSE);
        if (readLen < 0)
            goto fail_write;
        readLen = write_hanyar_cmd(STR_DEVSINFO, NULL, NULL);
        if (readLen < 0)
            goto fail_write;
        while (1)
        {
            readLen = Recv(pdata->socketfd, pdata->tcpBuf, RECVLEN, 0);
            if (readLen == 0)
            {
                printf("client close\n");
                break;
            }
            else if (readLen < 0)
            {
                printf("Recv error:%d\n", readLen);
                break;
            }
            else
            {
                pdata->tcpBuf[readLen] = '\0';
                recv_toLocal(pdata->tcpBuf, readLen);
            }
        }
    fail_write:
        printf("thread_hander close\n");
        Close(pdata->socketfd);
        pdata->socketfd = 0;

        timer_delete(timerid);
        local_allDevice_onlineStatus(0, DEV_OFFLINE);
        driver_deviceCloudOffline();
    } while (1);
    pthread_exit(0);
}

static pthread_t net_client(void *arg)
{

    pthread_t id;
    //clientMethod为此线程客户端，要执行的程序。
    pthread_create(&id, NULL, (void *)thread_hander, arg);
    //要将id分配出去。
    pthread_detach(id);

    // sigset_t set;
    // sigemptyset(&set);
    // sigaddset(&set, SIGALRM);
    // sigaddset(&set, SIGUSR1);
    // pthread_sigmask(SIG_BLOCK, &set, NULL);

    return id;
}
//---------------------------------------------------------------
static int tcp_client_write(char *data, unsigned int len)
{
    if (tcp_app.socketfd == 0)
    {
        log_error("socketfd is null\n");
        return -1;
    }
    return Write(tcp_app.socketfd, data, len);
}
//---------------------------------------------------------------
void tcp_client_open()
{
    printf("tcp_client_open\n");
    register_closeCallback(tcp_client_close);
    register_writeCallback(tcp_client_write);

    main_thread_signal_open();
    tcp_app.pid = net_client(&tcp_app);
}

void tcp_client_close()
{
    printf("tcp_client_close\n");
    if (tcp_app.pid != 0)
    {
        pthread_cancel(tcp_app.pid);
        tcp_app.pid = 0;
    }
    if (tcp_app.socketfd != 0)
    {
        Close(tcp_app.socketfd);
        tcp_app.socketfd = 0;
    }
}
#endif