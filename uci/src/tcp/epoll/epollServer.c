#include "tcp.h"
#include "epollServer.h"
#include <signal.h>
typedef struct
{
    int epollfd;
    int listenNum;                                                      //全局变量，作为红黑树根
    struct EpollTcpEvent events[CLIENT_MAX_EVENTS + SERVER_MAX_EVENTS]; //自定义结构体类型数组. +2-->listen fd
    int run_flag;
} EpollServer;

static EpollServer epollServer = {0};

//---------------------------------------------------------------

int epollServerSend(struct EpollTcpEvent *myevents, void *send, unsigned int len)
{
    if (send == NULL)
        return -1;

    struct EpollTcpEvent *clientEvents = epollServer.events;
    if (myevents->fd == 0)
    {
        printf("send socketfd is null\n");
        return -1;
    }
    for (int i = 0; i < CLIENT_MAX_EVENTS; ++i)
    {
        if (clientEvents[i].status != 0 && myevents->port == clientEvents[i].port)
        {
            epollClientSend(&clientEvents[i], send, len);
        }
    }

    return 0;
}
/*  当有文件描述符就绪, epoll返回, 调用该函数与客户端建立链接 */
static int epollAccetpCb(int lfd, int events, void *arg)
{
    int epollfd = epollServer.epollfd;
    struct EpollTcpEvent *clientEvents = epollServer.events;
    struct EpollTcpEvent *serverEvent = (struct EpollTcpEvent *)arg;

    struct sockaddr_in cin;
    socklen_t len = sizeof(cin);

    int cfd;
    if ((cfd = accept(lfd, (struct sockaddr *)&cin, &len)) == -1)
    {
        if (errno != EAGAIN && errno != EINTR)
        {
            sleep(1);
        }
        printf("%s:accept,%s\n", __func__, strerror(errno));
        return -1;
    }
    do
    {
        int i;
        for (i = 0; i < CLIENT_MAX_EVENTS; i++) //从全局数组g_events中找一个空闲元素，类似于select中找值为-1的元素
        {
            if (clientEvents[i].status == 0)
                break;
        }
        if (i == CLIENT_MAX_EVENTS) // 超出连接数上限
        {
            printf("%s: max connect limit[%d]\n", __func__, CLIENT_MAX_EVENTS);
            close(cfd);
            break;
        }
        if (setNonBlock(cfd) < 0) //将cfd也设置为非阻塞
        {
            printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
            break;
        }
        printf("new connect[%s:%d]\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
        getsockname(lfd, (struct sockaddr *)&cin, &len);

        epollTcpEventSet(&clientEvents[i], inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), serverEvent->recv_cb, serverEvent->disconnect_cb, serverEvent->connect_cb, -1);
        clientEvents[i].fd = cfd;
        epollClientInit(epollfd, &clientEvents[i], epollClientCb);

        printf("new connect[%s:%d],fd:%d\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), cfd);
    } while (0);

    return 0;
}

static void epollServerInit(int efd, struct EpollTcpEvent *event, Epoll_cb epoll_cb)
{
    if (tcpServerListen(&event->fd, event->addr, event->port, CLIENT_MAX_EVENTS) < 0)
        return;
    printf("tcpServerListen fd:%d\n", event->fd);
    ++epollServer.listenNum;
    event->epoll_cb = epoll_cb;
    /* void eventadd(int efd, int events, struct EpollTcpEvent *ev) */
    eventadd(efd, EPOLLIN | EPOLLHUP | EPOLLERR, event); //将lfd添加到监听树上，监听读事件

    return;
}
static void epollListListen(struct EpollTcpEvent *event)
{
    if (event->status != 0 && epollServer.listenNum >= SERVER_MAX_EVENTS)
        return;
    epollServerInit(epollServer.epollfd, event, epollAccetpCb);
}

static void epollListClose(struct EpollTcpEvent *event)
{
    if (event == NULL)
        return;
    if (event->status > 0 && event->fd != 0)
    {
        printf("test:%s,port:%d,fd:%d\n", event->addr, event->port, event->fd);
        Close(event->fd);
        event->fd = 0;
        eventdel(epollServer.epollfd, event);

        epoll_list_del(event);
    }
}

int epollServerClose(void)
{
    printf("epollServerClose\n");
    if (epollServer.run_flag == 0)
        return -1;
    for (int i = 0; i < CLIENT_MAX_EVENTS; ++i)
    {
        if (epollServer.events[i].status > 0 && epollServer.events[i].fd != 0)
        {
            Close(epollServer.events[i].fd);
            epollServer.events[i].fd = 0;
            eventdel(epollServer.epollfd, &epollServer.events[i]);
        }
    }
    epoll_list_for_each(epollListClose, &epollServerList);

    if (epollServer.epollfd != 0)
    {
        Close(epollServer.epollfd);
        epollServer.epollfd = 0;
    }
    epollServer.run_flag = 0;
    return 0;
}

int epollServerOpen(int timeout)
{
    epollServer.run_flag = 1;
    int epollfd = epollServer.epollfd = epoll_create1(0); //创建红黑树,返回给全局 g_efd
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event events[CLIENT_MAX_EVENTS + SERVER_MAX_EVENTS]; //定义这个结构体数组，用来接收epoll_wait传出的满足监听事件的fd结构体
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);

    epoll_list_for_each(epollListListen, &epollServerList);
    while (epollServer.run_flag)
    {
        //调用eppoll_wait等待接入的客户端事件,epoll_wait传出的是满足监听条件的那些fd的struct epoll_event类型
        int nfd = epoll_pwait(epollfd, events, CLIENT_MAX_EVENTS + SERVER_MAX_EVENTS, timeout, &mask);
        if (nfd < 0)
        {
            printf("epoll_wait error, exit:%d,errno:%d,strerror(errno):%s\n", nfd, errno, strerror(errno));
            continue;
        }
        epollHandle(events, nfd);
    }

    epollServerClose();

    return 0;
}

// void epollTcpServerEventSet(struct EpollTcpServerEvent *epollTcpServerEvent, const char *addr, const short port, Recv_cb recv_cb, Disconnect_cb disconnect_cb, Connect_cb connect_cb, int epollClientNumMax)
// {
//     epollTcpServerEvent->epollClientList = malloc(sizeof(struct EpollTcpEvent) * epollClientNumMax);
//     memset(epollTcpServerEvent->epollClientList, 0, sizeof(struct EpollTcpEvent) * epollClientNumMax);
//     epollTcpEventSet(&epollTcpServerEvent->epollTcpEvent, addr, port, recv_cb, disconnect_cb, connect_cb, 1);
// }
