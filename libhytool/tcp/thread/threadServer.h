#ifndef _THREADSERVER_H_
#define _THREADSERVER_H_

#include "threadClient.h"

typedef struct
{
    ThreadTcp threadTcp;
    ThreadTcp *clientList;
    int clientNumMax;
} ThreadTcpServer;

int threadServerOpen(ThreadTcpServer *threadTcpServer);
int threadServerClose(ThreadTcpServer *threadTcpServer);
int threadServerSend(ThreadTcpServer *threadTcpServer, void *send, unsigned int len);
void tcpEventServerSet(ThreadTcpServer *threadTcpServer, const char *addr, const short port, Recv_cb recv_cb, Disconnect_cb disconnect_cb, Connect_cb connect_cb, int clientNumMax);
#endif