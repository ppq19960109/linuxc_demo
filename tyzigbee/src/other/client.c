
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "threadClient.h"
#include "signalQuit.h"
#include "POSIXTimer.h"

#include "frameCb.h"

static ThreadTcp threadTcp;
static timer_t heartTimerid;

static int recv(char *data, unsigned int len)
{
    printf("---recv:%d,%s ...\n", len, data);
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
//------------------------------
static int systemQuit(void)
{
    runSystemCb(SYSTEM_CLOSE);
    return 0;
}
static void timerThreadHandler(union sigval v)
{
    runSystemCb(SYSTEM_HEARTBEAT);
}
//------------------------------
static int clientClose()
{
    POSIXTimerDelete(heartTimerid);
    threadClientClose(&threadTcp);
    return 0;
}

static int clientSend(void *data, unsigned int len)
{
    return threadClientSend(&threadTcp, data, len);
}

int clientOpen(void)
{
    registerSystemCb(clientClose, LAN_CLOSE);
    registerTransferCb(clientSend, TRANSFER_CLIENT_WRITE);

    registerQuitCb(systemQuit);
    signalQuit();
    tcpEventSet(&threadTcp, "127.0.0.1", 12580, recv, disconnect, connect, 0);
    threadClientOpen(&threadTcp);

    heartTimerid = POSIXTimerCreate(1, timerThreadHandler);
    POSIXTimerSet(heartTimerid, 300, 300);

    return 0;
}
