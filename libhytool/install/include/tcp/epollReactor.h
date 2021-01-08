#ifndef _EPOLLREACTOR_H_
#define _EPOLLREACTOR_H_

#include <pthread.h>
#include <sys/epoll.h>
#include "list.h"

#define CLIENT_MAX_EVENTS 8 /*监听上限*/
#define SERVER_MAX_EVENTS 4
#define BUFLEN 4096 /*缓存区大小*/
/*描述就绪文件描述符的相关信息*/

typedef int (*Epoll_cb)(int fd, int events, void *arg);
typedef int (*Recv_cb)(char *data, unsigned int len);
typedef int (*Disconnect_cb)(void);
typedef int (*Connect_cb)(void);

extern struct list_head epollClientList;
extern struct list_head epollServerList;

struct EpollTcpEvent
{
    int fd; //要监听的文件描述符
    int epollfd;
    int events;        //对应的监听事件，EPOLLIN和EPLLOUT
    void *arg;         //指向自己结构体指针
    Epoll_cb epoll_cb; //回调函数
    int status;        //是否在监听:1->在红黑树上(监听), 0->不在(不监听)
    long last_active;  //记录每次加入红黑树 g_efd 的时间值

    char recv_buf[BUFLEN + 1];
    int recv_len;
    char send_buf[BUFLEN + 1];
    int send_len;

    char addr[18];
    unsigned short port;
    Recv_cb recv_cb;             //回调函数
    Disconnect_cb disconnect_cb; //回调函数
    Connect_cb connect_cb;
    pthread_mutex_t mutex;
    struct list_head node;
};

void epollTcpEventSet(struct EpollTcpEvent *ev, const char *addr, const short port, Recv_cb recv_cb, Disconnect_cb disconnect_cb, Connect_cb connect_cb, const int isServer);
void eventadd(int efd, int events, struct EpollTcpEvent *ev);
void eventmod(int efd, int events, struct EpollTcpEvent *ev);
void eventdel(int efd, struct EpollTcpEvent *ev);

void epoll_list_add(struct EpollTcpEvent *, struct list_head *);
void epoll_list_del(struct EpollTcpEvent *);
void epoll_list_for_each(void (*call_back)(struct EpollTcpEvent *), struct list_head *);
#endif