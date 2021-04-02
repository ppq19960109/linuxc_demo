#ifndef _EPOLLCLIENT_H_
#define _EPOLLCLIENT_H_

#include "epollReactor.h"

int epollClientOpen(int timeout);
int epollClientClose(void);
int epollClientSend(struct EpollTcpEvent *myevents, void *send, unsigned int len);
int epollClientCb(int fd, int events, void *arg);
void epollClientInit(int efd, struct EpollTcpEvent *event, Epoll_cb epoll_cb);

int epollHandle(struct epoll_event *events, int nfd);
#endif