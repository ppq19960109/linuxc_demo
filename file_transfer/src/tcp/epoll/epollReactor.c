#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "epollReactor.h"

LIST_HEAD(epollClientList);
LIST_HEAD(epollServerList);
/*
 * 封装一个自定义事件，包括fd，这个fd的回调函数，还有一个额外的参数项
 * 注意：在封装这个事件的时候，为这个事件指明了回调函数，一般来说，一个fd只对一个特定的事件
 * 感兴趣，当这个事件发生的时候，就调用这个回调函数
 */
void epollTcpEventSet(struct EpollTcpEvent *ev, const char *addr, const short port, Recv_cb recv_cb, Disconnect_cb disconnect_cb, Connect_cb connect_cb, const int isServer)
{
    ev->fd = 0;
    ev->epoll_cb = NULL;
    ev->events = 0;
    ev->arg = ev;
    ev->status = 0;
    if (ev->recv_len <= 0)
    {
        memset(ev->recv_buf, 0, sizeof(ev->recv_buf));
        ev->recv_len = 0;
    }
    if (ev->send_len <= 0)
    {
        memset(ev->send_buf, 0, sizeof(ev->send_buf));
        ev->send_len = 0;
    }
    ev->last_active = time(NULL); //调用eventset函数的时间

    strcpy(ev->addr, addr);
    ev->port = port;
    ev->recv_cb = recv_cb;
    ev->disconnect_cb = disconnect_cb;
    ev->connect_cb = connect_cb;
    if (isServer > 0)
    {
        epoll_list_add(ev, &epollServerList);
    }
    else if (isServer == 0)
    {
        epoll_list_add(ev, &epollClientList);
    }
    else
    {
        /* code */
    }

    return;
}

/* 向 epoll监听的红黑树 添加一个文件描述符 */
void eventadd(int efd, int events, struct EpollTcpEvent *ev)
{
    struct epoll_event epv = {0, {0}};
    epv.data.ptr = ev;                // ptr指向一个结构体（之前的epoll模型红黑树上挂载的是文件描述符cfd和lfd，现在是ptr指针）
    epv.events = ev->events = events; //EPOLLIN 或 EPOLLOUT
    if (ev->status != 0)              //status 说明文件描述符是否在红黑树上 0不在，1 在
        return;
    ev->status = 1;
    int op = EPOLL_CTL_ADD; //将其加入红黑树 g_efd, 并将status置1

    if (epoll_ctl(efd, op, ev->fd, &epv) < 0) // 添加一个节点
        printf("event add failed [fd=%d],events[%d]\n", ev->fd, events);
    else
        printf("event add OK [fd=%d],events[%0X]\n", ev->fd, events);
    return;
}

void eventmod(int efd, int events, struct EpollTcpEvent *ev)
{
    struct epoll_event epv = {0, {0}};
    epv.events = ev->events = events; //EPOLLIN 或 EPOLLOUT
    if (ev->status != 1)              //status 说明文件描述符是否在红黑树上 0不在，1 在
        return;
    int op = EPOLL_CTL_MOD;

    if (epoll_ctl(efd, op, ev->fd, &epv) < 0) // 添加一个节点
        printf("event mod failed [fd=%d],events[%d]\n", ev->fd, events);
    else
        printf("event mod OK [fd=%d],events[%0X]\n", ev->fd, events);
    return;
}

/* 从epoll 监听的 红黑树中删除一个文件描述符*/
void eventdel(int efd, struct EpollTcpEvent *ev)
{
    struct epoll_event epv = {0, {0}};
    if (ev->status != 1) //如果fd没有添加到监听树上，就不用删除，直接返回
        return;
    epv.data.ptr = NULL;
    ev->status = 0;
    epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);
    return;
}

//-------------------------------
void epoll_list_add(struct EpollTcpEvent *epollTcpEvent, struct list_head *head)
{
    list_add(&epollTcpEvent->node, head);
}

void epoll_list_del(struct EpollTcpEvent *epollTcpEvent)
{
    list_del(&epollTcpEvent->node);
}

void epoll_list_for_each(void (*call_back)(struct EpollTcpEvent *), struct list_head *head)
{
    struct EpollTcpEvent *epollTcpEvent = NULL;
    struct EpollTcpEvent *epollTcpEvent_temp = NULL;
    list_for_each_entry_safe(epollTcpEvent, epollTcpEvent_temp, head, node)
    {
        call_back(epollTcpEvent);
    }
    return;
}
