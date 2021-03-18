#ifndef _THREADSERVER_H_
#define _THREADSERVER_H_

#include "threadClient.h"
#include "list.h"
typedef struct
{
    ThreadTcp threadTcp;
    ThreadTcp *clientList;
    int clientNumMax;
    struct list_head node;
} ThreadTcpServer;

int threadServerOpen(void);
void threadServerClose(void);
int threadServerSend(ThreadTcpServer *threadTcpServer, void *send, unsigned int len);
int tcpEventServerSet(ThreadTcpServer *threadTcpServer, const char *addr, const short port, Recv_cb recv_cb, Disconnect_cb disconnect_cb, Connect_cb connect_cb, int clientNumMax);
#endif