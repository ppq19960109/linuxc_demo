
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

#include "nativeFrame.h"

#include "frameCb.h"
#include "socketFunc.h"

typedef struct
{
#define SERVER_PORT 7000
#define RECV_LEN 16384 //10k
    char readBuf[RECV_LEN + 1];
    int socketfd;
    pthread_mutex_t mutex;
    pthread_t tid;
} nativeClientNet_t;

static nativeClientNet_t nativeClientNet;

static int nativeClientStart(void)
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;                     //簇
    server.sin_port = htons(SERVER_PORT);            //端口
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); //ip地址

    int sockfd = Socket(AF_INET, SOCK_STREAM);
    // Bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
connect:
    sleep(2);
    if (Connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) != 0)
        goto connect;

    return sockfd;
}

static void *nativeThreadHander(void *arg)
{
    int readLen = 0;
    nativeClientNet_t *pdata = ((nativeClientNet_t *)arg);
    do
    {
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
                    readLen = Recv(pdata->socketfd, pdata->readBuf, RECV_LEN, 0);
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
                        pdata->readBuf[readLen] = '\0';
                        runTransferCb(pdata->readBuf, readLen, TRANSFER_CLIENT_READ);
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

        runTransferCb(NULL, SUBDEV_OFFLINE, TRANSFER_SUBDEV_LINE);
    } while (1);
    pthread_exit(0);
}

#if USE_EPOLL
void nativeClientHander(void)
{
    nativeThreadHander(&nativeClientNet);
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
//-----------------------------------------------------
static int nativeClientWrite(void *recv, unsigned int dataLen)
{
    if (recv == NULL)
        return -1;
    if (nativeClientNet.socketfd == 0)
    {
        printf("socketfd is null\n");
        return -1;
    }
    // printf("nativeFrameWrite %s\n",data);
    pthread_mutex_lock(&nativeClientNet.mutex);
    int ret = Write(nativeClientNet.socketfd, recv, dataLen);
    pthread_mutex_unlock(&nativeClientNet.mutex);
    return ret;
}

int nativeClientClose(void)
{
    if (nativeClientNet.tid != 0)
    {
        pthread_cancel(nativeClientNet.tid);
    }
    pthread_join(nativeClientNet.tid, NULL);
    printf("pthread_join close\n");
    nativeClientNet.tid = 0;

    if (nativeClientNet.socketfd != 0)
    {
        Close(nativeClientNet.socketfd);
        nativeClientNet.socketfd = 0;
    }
    pthread_mutex_destroy(&nativeClientNet.mutex);
    return 0;
}

int nativeClientOpen(void)
{
    registerSystemCb(nativeClientClose, LAN_CLOSE);
    registerTransferCb(nativeClientWrite, TRANSFER_CLIENT_WRITE);

    pthread_mutex_init(&nativeClientNet.mutex, NULL);

#if USE_EPOLL

#else
    nativeClientNet.tid = nativeClient(&nativeClientNet);
#endif
    return 0;
}
