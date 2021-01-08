#include "tcp.h"
#include "epollClient.h"


typedef struct
{
    int epollfd;

} EpollClient;

static EpollClient epollClient;

int epollClientSend(struct EpollTcpEvent *myevents, void *send, unsigned int len)
{
    if (send == NULL)
        return -1;

    if (myevents->fd == 0)
    {
        printf("send socketfd is null\n");
        return -1;
    }

    pthread_mutex_lock(&myevents->mutex);

    myevents->events |= EPOLLOUT;
    eventmod(myevents->epollfd, myevents->events, myevents);

    int send_max_len = sizeof(myevents->send_buf);
    myevents->send_len = len > send_max_len ? send_max_len : len;
    memcpy(myevents->send_buf, recv, myevents->send_len);

    pthread_mutex_unlock(&myevents->mutex);

    return myevents->send_len;
}

int epollClientCb(int fd, int events, void *arg)
{
    struct EpollTcpEvent *myevent = (struct EpollTcpEvent *)arg;
    int len;
    if (events & EPOLLIN)
    {
        len = Recv(fd, myevent->recv_buf, sizeof(myevent->recv_buf), 0); //读取客户端发过来的数据

        if (len <= 0)
        {
            eventdel(myevent->epollfd, myevent); //将该节点从红黑树上摘除
            close(myevent->fd);
            myevent->fd = 0;
            pthread_mutex_destroy(&myevent->mutex);
            printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
            if (myevent->disconnect_cb != NULL)
                myevent->disconnect_cb();
        }
        else
        {
            myevent->recv_len = len;
            myevent->recv_buf[len] = '\0'; //手动添加字符串结束标记
            // printf("socket[%d]: %d,%s\n", fd, myevent->recv_len , myevent->recv_buf);
            if (myevent->recv_cb != NULL)
                myevent->recv_cb(myevent->recv_buf, myevent->recv_len);
        }
    }
    if (events & EPOLLOUT)
    {
        pthread_mutex_lock(&myevent->mutex);
        len = Send(fd, myevent->send_buf, myevent->send_len, 0);
        if (len < 0)
            goto sendFail;

        myevent->send_len = 0;

        myevent->events &= ~EPOLLOUT;
        eventmod(myevent->epollfd, myevent->events, myevent);
    sendFail:
        pthread_mutex_unlock(&myevent->mutex);
    }

    return 0;
}

void epollClientInit(int efd, struct EpollTcpEvent *event, Epoll_cb epoll_cb)
{
    event->epoll_cb = epoll_cb;
    event->epollfd = efd;
    /* void eventadd(int efd, int events, struct EpollTcpEvent *ev) */
    eventadd(efd, EPOLLIN, event); //将lfd添加到监听树上，监听读事件
}

static void epollListConnect(struct EpollTcpEvent *event)
{
    if (event->status != 0)
        return;
    if (tcpClientConnect(&event->fd, event->addr, event->port) < 0)
        return;
    pthread_mutex_init(&event->mutex, NULL);
    epollClientInit(epollClient.epollfd, event, epollClientCb);
}

static void epollListClose(struct EpollTcpEvent *event)
{
    if (event->fd == 0)
        return;
    Close(event->fd);

    pthread_mutex_destroy(&event->mutex);
}
#ifdef EPOLLCHECK
void epollListCheck(struct EpollTcpEvent *event)
{

    long now = time(NULL);

    if (checkpos == CLIENT_MAX_EVENTS)
        checkpos = 0;
    if (event->status != 1)
        continue;
    long duration = now - event->last_active;
    if (duration >= 60)
    {
        close(event->fd);
        printf("[fd=%d] timeout\n", event->fd);
        eventdel(epollfd, event);
    }
}
#endif
int epollClientClose(void)
{
    if (epollClient.epollfd != 0)
    {
        Close(epollClient.epollfd);
        epollClient.epollfd = 0;
    }
    epoll_list_for_each(epollListClose, &epollClientList);

    return 0;
}

int epollHandle(struct epoll_event *events, int nfd)
{
    int i, res = 0;
    for (i = 0; i < nfd; ++i)
    {
        //evtAdd()函数中，添加到监听树中监听事件的时候将myevents_t结构体类型给了ptr指针
        //这里epoll_wait返回的时候，同样会返回对应fd的myevents_t类型的指针
        struct EpollTcpEvent *ev = (struct EpollTcpEvent *)events[i].data.ptr;
        //如果监听的是读事件，并返回的是读事件
        if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
        {
            res = ev->epoll_cb(ev->fd, events[i].events, ev->arg);
        }
        //如果监听的是写事件，并返回的是写事件
        if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
        {
            res = ev->epoll_cb(ev->fd, events[i].events, ev->arg);
        }
    }
    return res;
}

int epollClientOpen(int timeout)
{

    int epollfd = epollClient.epollfd = epoll_create1(0); //创建红黑树,返回给全局 g_efd
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event events[CLIENT_MAX_EVENTS];

    while (1)
    {
        epoll_list_for_each(epollListConnect, &epollClientList);

        //调用eppoll_wait等待接入的客户端事件,epoll_wait传出的是满足监听条件的那些fd的struct epoll_event类型
        int nfd = epoll_wait(epollfd, events, CLIENT_MAX_EVENTS, timeout);
        if (nfd < 0)
        {
            printf("epoll_wait error...errno:%d,%s\n", errno, strerror(errno));
            continue;
        }
        epollHandle(events, nfd);
    }
    return epollClientClose();
}
