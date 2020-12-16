#if USE_SOCKET == 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <pthread.h>
#include <time.h>
#include <signal.h>

#include "nativeFrame.h"

#include "frameCb.h"
#include "socketFunc.h"

typedef void (*timer_function)(union sigval);

typedef struct
{
#define SERVER_PORT 7000
#define SEND_LEN 1024
    char sendbuf[SEND_LEN];
#define RECV_LEN 16384 //10k
    char readbuf[RECV_LEN + 1];
    int socketfd;
    unsigned int getDevListCount;
    pthread_mutex_t mutex;
    pthread_t tid;
#define TIMEER_INDEX 1
    timer_t timerid;
#define LED_TIMEER_INDEX 2
    timer_t ledTimerid;
} nativeNet_t;

static nativeNet_t nativeNet;

static void nativeSignalHandler(int signal)
{
    printf("nativeSignalHandler signal is %d\n", signal);
    if (signal == SIGHUP || signal == SIGINT || signal == SIGQUIT || signal == SIGKILL || signal == SIGTERM)
    {
        runSystemCb(SYSTEM_CLOSE);
        exit(0);
    }
}

static void setNativeSignal()
{
    signal(SIGQUIT, nativeSignalHandler);
    signal(SIGKILL, nativeSignalHandler);
    signal(SIGTERM, nativeSignalHandler);

    struct sigaction act, oldact;
    act.sa_handler = nativeSignalHandler;
    sigemptyset(&act.sa_mask);

    // sigaddset(&act.sa_mask, SIGINT); //见注(1)
    // act.sa_flags = SA_NODEFER; //| SA_NODEFER 见注(2)SA_RESETHAND
    act.sa_flags = 0; //见注(3)

    sigaction(SIGINT, &act, &oldact);
    sigaction(SIGHUP, &act, &oldact);
}
//---------------------------------------------------------------
static void POSIXTTimerThreadHandler(union sigval v)
{
    if (v.sival_int == TIMEER_INDEX)
    {
        ++nativeNet.getDevListCount;
        if (nativeNet.getDevListCount < 0)
        {
            runSystemCb(CMD_DEVSINFO);
        }
        else if (nativeNet.getDevListCount % 1440 == 0)
        {
            runSystemCb(CMD_DEVSINFO);
        }
        runSystemCb(CMD_HEART);
    }
    else if (v.sival_int == LED_TIMEER_INDEX)
    {
        runSystemCb(LED_DRIVER_TIMER_FILP);
    }
    else
    {
        /* code */
    }
}

// static void POSIXTTimerSignalHandler(int signal)
// {
// }

int setPOSIXTimer(timer_t timerid, int interval_sec, int sec)
{
    /* 第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长,就是说it.it_value变0的时候会>装载it.it_interval的值 */
    struct itimerspec it;
    it.it_interval.tv_sec = interval_sec; // 回调函数执行频率为1s运行1次
    it.it_interval.tv_nsec = 0;
    it.it_value.tv_sec = sec; // 倒计时3秒开始调用回调函数
    it.it_value.tv_nsec = 0;

    int ret = timer_settime(timerid, 0, &it, NULL);
    if (ret < 0)
    {
        perror("fail to timer_settime!");
    }
    return ret;
}

timer_t createPOSIXTimer(int sival, timer_function fun)
{

    timer_t timerid;
    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent)); //清零初始化
#if 1
    evp.sigev_value.sival_int = sival; //也是标识定时器的，回调函数可以获得
    evp.sigev_notify = SIGEV_THREAD;   //线程通知的方式，派驻新线程
    evp.sigev_notify_function = fun;   //线程函数地址
#else
    // evp.sigev_signo = SIGUSR1;
    // evp.sigev_notify = SIGEV_SIGNAL;

    // struct sigaction act;
    // act.sa_handler = fun;
    // sigemptyset(&act.sa_mask);
    // act.sa_flags = SA_RESTART;
    // sigaction(SIGUSR1, &act, NULL);
#endif

    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)
    {
        perror("fail to timer_create");
        return NULL;
    }
    return timerid;
}

void resetTimerStatus(void)
{
    printf("resetTimerStatus!\n");
    if (nativeNet.timerid != NULL)
    {
        setPOSIXTimer(nativeNet.timerid, 60, 60);
    }
    nativeNet.getDevListCount = 0;
}

static int nativeClientStart(void)
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;                     //簇
    server.sin_port = htons(SERVER_PORT);            //端口
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); //ip地址

    int sockfd = Socket(AF_INET, SOCK_STREAM);
    // Bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
connect:
    // sleep(10);
    if (Connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) != 0)
        goto connect;

    return sockfd;
}

static void *nativeThreadHander(void *arg)
{
    int readLen = 0;
    nativeNet_t *pdata = ((nativeNet_t *)arg);
    do
    {
        pdata->getDevListCount = 0;
        pdata->socketfd = nativeClientStart();

#if USE_EPOLL
        setNonBlock(pdata->socketfd);
#define MAX_EVENTS 10
        struct epoll_event ev, events[MAX_EVENTS];
        int nfds, n;
        int epollfd = epoll_create1(0);
        if (epollfd == -1)
        {
            perror("epoll_create1");
            exit(EXIT_FAILURE);
        }
        ev.events = EPOLLIN;
        ev.data.fd = pdata->socketfd;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, pdata->socketfd, &ev) == -1)
        {
            perror("epoll_ctl: listen_sock");
            exit(EXIT_FAILURE);
        }
#endif

        pdata->timerid = createPOSIXTimer(TIMEER_INDEX, POSIXTTimerThreadHandler);
        setPOSIXTimer(pdata->timerid, 60, 60);

        if (runSystemCb(CMD_DEVSINFO) < 0)
            goto fail;

        while (1)
        {
#if USE_EPOLL
            nfds = epoll_wait(epollfd, events, MAX_EVENTS, 0);
            if (nfds == -1)
            {
                perror("epoll_wait error:");
                break;
            }
            for (n = 0; n < nfds; ++n)
            {
                if (events[n].data.fd == pdata->socketfd)
                {
#endif
                    //----------------------------------------
                    readLen = Recv(pdata->socketfd, pdata->readbuf, RECV_LEN, 0);
                    if (readLen == 0)
                    {
                        printf("native client close\n");
                        break;
                    }
                    else if (readLen < 0)
                    {
                        printf("Recv error:%d\n", readLen);
                        break;
                    }
                    else
                    {
                        pdata->readbuf[readLen] = '\0';
                        runTransferCb(pdata->readbuf, readLen, TRANSFER_READ);
                    }
//----------------------------------------
#if USE_EPOLL
                }
                else
                {
                    /* code */
                }
            }
#endif
        }
    fail:
        printf("nativeThreadHander close\n");

        Close(pdata->socketfd);
        pdata->socketfd = 0;
        timer_delete(pdata->timerid);
        pdata->timerid = NULL;

        runTransferCb(NULL, SUBDEV_OFFLINE, TRANSFER_SUBDEV_LINE);
    } while (1);
    pthread_exit(0);
}

#if USE_EPOLL
void nativeClientHander(void)
{
    nativeThreadHander(&nativeNet);
}
#else
static pthread_t nativeClient(void *arg)
{
    pthread_t id;
    //clientMethod为此线程客户端，要执行的程序。
    pthread_create(&id, NULL, (void *)nativeThreadHander, arg);
    //要将id分配出去。
    // pthread_detach(id);

    return id;
}
#endif
//---------------------------------------------------------------
int ledTimerOpen(void)
{
    if (nativeNet.ledTimerid != NULL)
    {
        return -1;
    }
    nativeNet.ledTimerid = createPOSIXTimer(LED_TIMEER_INDEX, POSIXTTimerThreadHandler);
    setPOSIXTimer(nativeNet.ledTimerid, 1, 1);
    return 0;
}
int ledTimerClose(void)
{
    if (nativeNet.ledTimerid != NULL)
    {
        timer_delete(nativeNet.ledTimerid);
        nativeNet.ledTimerid = NULL;
    }
    return 0;
}
//-----------------------------------------------------
static int nativeFrameWrite(void *recv, unsigned int dataLen)
{
    const char *data = (const char *)recv;
    if (data == NULL)
        return -1;
    if (nativeNet.socketfd == 0)
    {
        printf("socketfd is null:%s\n", data);
        return -1;
    }
    // printf("nativeFrameWrite %s\n",data);
    pthread_mutex_lock(&nativeNet.mutex);
    char *send = nativeNet.sendbuf;

    send[0] = 0x02;
    strncpy(&send[1], data, SEND_LEN - 3);
    send[dataLen + 1] = 0x03;
    send[dataLen + 2] = 0x00;

    int ret = Write(nativeNet.socketfd, send, dataLen + 3);
    pthread_mutex_unlock(&nativeNet.mutex);
    return ret;
}

static int nativeFrameClose(void)
{
    printf("nativeFrameClose\n");
    timer_delete(nativeNet.timerid);
    if (nativeNet.tid != 0)
    {
        pthread_cancel(nativeNet.tid);
    }
    pthread_join(nativeNet.tid, NULL);
    printf("pthread_join close\n");
    nativeNet.tid = 0;

    if (nativeNet.socketfd != 0)
    {
        Close(nativeNet.socketfd);
        nativeNet.socketfd = 0;
    }
    pthread_mutex_destroy(&nativeNet.mutex);
    return 0;
}

int nativeFrameOpen(void)
{
    printf("nativeFrameOpen\n");
    registerSystemCb(ledTimerOpen, LED_DRIVER_TIMER_OPEN);
    registerSystemCb(ledTimerClose, LED_DRIVER_TIMER_CLOSE);

    registerSystemCb(nativeFrameClose, LAN_CLOSE);
    registerTransferCb(nativeFrameWrite, TRANSFER_WRITE);

    pthread_mutex_init(&nativeNet.mutex, NULL);
    setNativeSignal();
#if USE_EPOLL

#else
    nativeNet.tid = nativeClient(&nativeNet);
#endif
    return 0;
}
#endif