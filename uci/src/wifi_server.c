#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "uci_wifi.h"
#include "POSIXTimer.h"
#include "networkFunc.h"

// #define USE_EPOLL
#ifdef USE_EPOLL
#include "epollServer.h"
static struct EpollTcpEvent hylink_myevents;
#else
#include "threadServer.h"
ThreadTcpServer hylink_threadTcpServer;
#endif

static int hylink_recv(char *data, unsigned int len)
{
    printf("---hylink_recv:%d,%s ...\n", len, data);
    wifi_config(data, len);
    return 0;
}

static int hylink_disconnect(void)
{
    printf("---disconnect ...\n");
    return 0;
}

static int hylink_connect(void)
{
    printf("---hylink_connect ...\n");

    return 0;
}

int wifi_server_send(void *data, unsigned int len)
{
#ifdef USE_EPOLL
    return epollServerSend(&hylink_myevents, data, len);
#else
    return threadServerSend(&hylink_threadTcpServer, data, len);
#endif
}

int wifi_serverClose(void)
{
#ifdef USE_EPOLL
    epollServerClose();
#else
    threadServerClose();
#endif
    return 0;
}

#define TCP_ADDR "0.0.0.0" //"127.0.0.1"
#define TCP_PORT 16666
int wifi_serverOpen(void)
{

#ifdef USE_EPOLL
    epollTcpEventSet(&hylink_myevents, TCP_ADDR, TCP_PORT, hylink_recv, hylink_disconnect, hylink_connect, 1);
    epollServerOpen(2000);
#else
    tcpEventServerSet(&hylink_threadTcpServer, TCP_ADDR, TCP_PORT, hylink_recv, hylink_disconnect, hylink_connect, 2);
    threadServerOpen();
#endif
    return 0;
}
//------------------------------
static timer_t wifi_timer_id;
static void wifi_client_close(void)
{
    for (int i = 0; i < hylink_threadTcpServer.clientNumMax; ++i)
    {
        threadClientClose(&hylink_threadTcpServer.clientList[i]);
    }
}

static void timerThreadHandler(union sigval v)
{
    printf("timerThreadHandler....\n");
    if (v.sival_int == 1)
    {
        printf("wifi_client_close....\n");
        wifi_client_close();
    }
    else
    {
        /* code */
    }
}

int wifi_client_open(void)
{
    if (wifi_timer_id == NULL)
    {
        return -1;
    }
    printf("wifi_client_open....\n");
    POSIXTimerSet(wifi_timer_id, 0, 10);
    return 0;
}

static void *threadHander(void *arg)
{
    wifi_serverOpen();
    wifi_serverClose();
    return NULL;
}

int thread_wifi_config(void)
{
    printf("getNetlinkEthtool:%d\n", getNetlinkEthtool("wlan0"));
    printf("getNetlink:%d\n", getNetlink("wlan0"));
    wifi_timer_id = POSIXTimerCreate(1, timerThreadHandler);

    pthread_t tid;
    //clientMethod为此线程客户端，要执行的程序。
    pthread_create(&tid, NULL, (void *)threadHander, NULL);
    //要将id分配出去。
    pthread_detach(tid);

    return 0;
}
