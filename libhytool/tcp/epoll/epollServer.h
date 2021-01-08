#ifndef _EPOLLSERVER_H_
#define _EPOLLSERVER_H_

#include "epollClient.h"



int epollServerOpen(int timeout);
int epollServerClose(void);
int epollServerSend(struct EpollTcpEvent *myevents, void *send, unsigned int len);

// struct EpollTcpServerEvent
// {
//     struct EpollTcpEvent epollTcpEvent;
//     struct EpollTcpEvent *epollClientList;
//     int epollClientNumMax;
// };
// void epollTcpServerEventSet(struct EpollTcpServerEvent *epollTcpServerEvent, const char *addr, const short port, Recv_cb recv_cb, Disconnect_cb disconnect_cb, Connect_cb connect_cb, int epollClientNumMax);
#endif