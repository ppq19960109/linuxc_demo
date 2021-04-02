#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"

// #include "epollClient.h"
// struct EpollTcpEvent myevents;

#include "threadClient.h"
static ThreadTcp threadTcp;

static int recv(char *data, unsigned int len)
{
    printf("---recv:%u,%s ...\n", len, data);
    runTransferCb(data, len, TRANSFER_CLIENT_READ);
    return 0;
}
static int disconnect(void)
{
    printf("---disconnect ...\n");
    return 0;
}
static int connect(void)
{
    printf("---connect ...\n");
    return 0;
}

int clientClose(void)
{
    threadClientClose(&threadTcp);
    // epollClientClose();
    return 0;
}

static int clientSend(void *data, unsigned int len)
{
    return threadClientSend(&threadTcp, data, len);
    // return epollClientSend(&myevents, data, len);
}

int clientOpen(void)
{
    registerTransferCb(clientSend, TRANSFER_CLIENT_WRITE);

    //"127.0.0.1"
    tcpEventSet(&threadTcp, "127.0.0.1", 12580, recv, disconnect, connect, 0);
    threadClientOpen(&threadTcp);

    // epollTcpEventSet(&myevents, "192.168.1.6", 12580, recv, disconnect, connect, 0);
    // epollClientOpen(3000);
    // epollClientClose();
    return 0;
}
