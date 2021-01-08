#ifndef _EPOLLREACTOR_H_
#define _EPOLLREACTOR_H_

#define MAX_EVENTS 4 /*监听上限*/
#define BUFLEN 4096 /*缓存区大小*/
/*描述就绪文件描述符的相关信息*/
struct myevent_s
{
    int fd;                                           //要监听的文件描述符
    int events;                                       //对应的监听事件，EPOLLIN和EPLLOUT
    void *arg;                                        //指向自己结构体指针
    void (*call_back)(int fd, int events, void *arg); //回调函数
    int status;                                       //是否在监听:1->在红黑树上(监听), 0->不在(不监听)
    char buf[BUFLEN];
    int len;
    long last_active; //记录每次加入红黑树 g_efd 的时间值
    int listenPort;         //7000-hylink,12580-zigbee
};

void eventset(struct myevent_s *ev, int fd, void (*call_back)(int fd, int events, void *arg), void *arg);
void eventadd(int efd, int events, struct myevent_s *ev);
void eventdel(int efd, struct myevent_s *ev);
#endif