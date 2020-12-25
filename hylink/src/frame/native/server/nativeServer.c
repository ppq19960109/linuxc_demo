#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#include "nativeFrame.h"
#include "nativeTimer.h"
#include "nativeSignal.h"
#include "frameCb.h"
#include "socketFunc.h"
#include "epollReactor.h"

typedef struct
{
#define HYLINK_SERVER_PORT 7000
#define ZIGBEE_SERVER_PORT 12580
#define LISTEN_SOCKET_NUM 2

    int efd;                                                 //全局变量，作为红黑树根
    struct myevent_s events[MAX_EVENTS + LISTEN_SOCKET_NUM]; //自定义结构体类型数组. +2-->listen fd

    pthread_mutex_t mutex;

} nativeNet_t;

static nativeNet_t nativeNet;

//-------------------------------------------

/*读取客户端发过来的数据的函数*/
static void recvdata(int fd, int events, void *arg)
{
    int epollfd = nativeNet.efd;

    struct myevent_s *ev = (struct myevent_s *)arg;
    int len;

    len = recv(fd, ev->buf, sizeof(ev->buf), 0); //读取客户端发过来的数据

    if (len <= 0)
    {
        eventdel(epollfd, ev); //将该节点从红黑树上摘除
        close(ev->fd);
        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
    }
    else
    {
        ev->len = len;
        ev->buf[len] = '\0'; //手动添加字符串结束标记
        // printf("C[%d]:%s\n", fd, ev->buf);
        if (ev->listenPort == HYLINK_SERVER_PORT)
        {
            runTransferCb(ev->buf, ev->len, TRANSFER_SERVER_HYLINK_READ);
        }
        else if (ev->listenPort == ZIGBEE_SERVER_PORT)
        {
        }
        else
        {
            printf("server port error\n");
            return;
        }
    }

    return;
}

/*  当有文件描述符就绪, epoll返回, 调用该函数与客户端建立链接 */
static void acceptconn(int lfd, int events, void *arg)
{
    int epollfd = nativeNet.efd;
    struct myevent_s *myevents = nativeNet.events;

    struct sockaddr_in cin;
    socklen_t len = sizeof(cin);

    int cfd, i;
    if ((cfd = accept(lfd, (struct sockaddr *)&cin, &len)) == -1)
    {
        if (errno != EAGAIN && errno != EINTR)
        {
            sleep(1);
        }
        printf("%s:accept,%s\n", __func__, strerror(errno));
        return;
    }
    do
    {
        for (i = 0; i < MAX_EVENTS; i++) //从全局数组g_events中找一个空闲元素，类似于select中找值为-1的元素
        {
            if (myevents[i].status == 0)
                break;
        }
        if (i == MAX_EVENTS) // 超出连接数上限
        {
            printf("%s: max connect limit[%d]\n", __func__, MAX_EVENTS);
            break;
        }
        if (setNonBlock(cfd) < 0) //将cfd也设置为非阻塞
        {
            printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
            break;
        }

        getsockname(lfd, (struct sockaddr *)&cin, &len);
        myevents[i].listenPort = ntohs(cin.sin_port);

        eventset(&myevents[i], cfd, recvdata, &myevents[i]); //找到合适的节点之后，将其添加到监听树中，并监听读事件
        eventadd(epollfd, EPOLLIN, &myevents[i]);
    } while (0);

    printf("new connect[%s:%d],[time:%ld],pos[%d]\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), myevents[i].last_active, i);
    return;
}

/*创建 socket, 初始化lfd */
static void initListenSocket(int efd, struct myevent_s *event, short port, void (*call_back)(int fd, int events, void *arg))
{
    struct sockaddr_in sin;

    int lfd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
    {
        return;
    }

    memset(&sin, 0, sizeof(sin)); //bzero(&sin, sizeof(sin))
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("127.0.0.1");

    sin.sin_port = htons(port);

    bind(lfd, (struct sockaddr *)&sin, sizeof(sin));

    listen(lfd, MAX_EVENTS);

    /* void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg);  */
    eventset(event, lfd, call_back, event);

    /* void eventadd(int efd, int events, struct myevent_s *ev) */
    eventadd(efd, EPOLLIN, event); //将lfd添加到监听树上，监听读事件

    return;
}

int nativeServerEpollClose()
{
    pthread_mutex_destroy(&nativeNet.mutex);
    int epollfd = nativeNet.efd;
    struct myevent_s *myevents = nativeNet.events;
    for (int i = 0; i < MAX_EVENTS + LISTEN_SOCKET_NUM; ++i)
    {
        if (myevents[i].status == 0)
            continue;
        eventdel(epollfd, &myevents[i]);
        close(myevents[i].fd);
    }
    close(epollfd);

    return 0;
}

int nativeServerEpollOpen()
{
    pthread_mutex_init(&nativeNet.mutex, NULL);
    int i;
    nativeNet.efd = epoll_create1(0); //创建红黑树,返回给全局 g_efd
    int epollfd = nativeNet.efd;
    struct myevent_s *myevents = nativeNet.events;

    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    initListenSocket(epollfd, &myevents[MAX_EVENTS], HYLINK_SERVER_PORT, acceptconn); //初始化监听socket
    initListenSocket(epollfd, &myevents[MAX_EVENTS + 1], ZIGBEE_SERVER_PORT, acceptconn);

    struct epoll_event events[MAX_EVENTS]; //定义这个结构体数组，用来接收epoll_wait传出的满足监听事件的fd结构体
#ifdef EPOLLCHECK
    int checkpos = 0;
#endif

    while (1)
    {
#ifdef EPOLLCHECK
        long now = time(NULL);
        for (i = 0; i < MAX_EVENTS; ++i, ++checkpos)
        {
            if (checkpos == MAX_EVENTS)
                checkpos = 0;
            if (myevents[checkpos].status != 1)
                continue;
            long duration = now - myevents[checkpos].last_active;
            if (duration >= 60)
            {
                close(myevents[checkpos].fd);
                printf("[fd=%d] timeout\n", myevents[checkpos].fd);
                eventdel(epollfd, &myevents[checkpos]);
            }
        }
#endif
        //调用eppoll_wait等待接入的客户端事件,epoll_wait传出的是满足监听条件的那些fd的struct epoll_event类型
        int nfd = epoll_wait(epollfd, events, MAX_EVENTS + LISTEN_SOCKET_NUM, -1);
        if (nfd < 0)
        {
            printf("epoll_wait error, exit\n");
            exit(-1);
        }
        for (i = 0; i < nfd; ++i)
        {
            //evtAdd()函数中，添加到监听树中监听事件的时候将myevents_t结构体类型给了ptr指针
            //这里epoll_wait返回的时候，同样会返回对应fd的myevents_t类型的指针
            struct myevent_s *ev = (struct myevent_s *)events[i].data.ptr;
            //如果监听的是读事件，并返回的是读事件
            if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
            {
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
            //如果监听的是写事件，并返回的是写事件
            if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
            {
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
        }
    }
    nativeServerEpollClose();

    return 0;
}

//---------------------------------------------------------------

int nativeHylinkWrite(void *recv, unsigned int dataLen)
{
    const char *data = (const char *)recv;
    if (data == NULL)
        return -1;
    // printf("nativeFrameWrite %s\n",data);
    pthread_mutex_lock(&nativeNet.mutex);
    int ret;
    struct myevent_s *myevents = nativeNet.events;
    for (int i = 0; i < MAX_EVENTS; ++i)
    {
        if (myevents[i].status == 0)
            continue;

        if (myevents[i].listenPort != HYLINK_SERVER_PORT)
        {
            continue;
        }

        ret = Write(myevents[i].fd, data, dataLen);
        if (ret < 0)
        {
            printf("Hylink Write error\n");
        }
    }
    pthread_mutex_unlock(&nativeNet.mutex);
    return 0;
}

int nativeZigbeeWrite(void *recv, unsigned int dataLen)
{
    const char *data = (const char *)recv;
    if (data == NULL)
        return -1;
    int ret;
    struct myevent_s *myevents = nativeNet.events;
    for (int i = 0; i < MAX_EVENTS; ++i)
    {
        if (myevents[i].status == 0)
            continue;

        if (myevents[i].listenPort != ZIGBEE_SERVER_PORT)
        {
            continue;
        }

        ret = Write(myevents[i].fd, data, dataLen);
        if (ret < 0)
        {
            printf("Write error\n");
        }
    }
    return 0;
}

int nativeServerCLose(void)
{
    nativeServerEpollClose();
    return 0;
}

int nativeServerOpen(void)
{
    registerTransferCb(nativeHylinkWrite, TRANSFER_SERVER_HYLINK_WRITE);
    nativeServerEpollOpen();
    return 0;
}
