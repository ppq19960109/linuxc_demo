#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"

#include "epollServer.h"
static int zigbee_connecting_num = 0;
static struct EpollTcpEvent hylink_myevents;
static struct EpollTcpEvent zigbee_myevents;

static int hylink_recv(char *data, unsigned int len)
{
    printf("---hylink_recv:%d,%s ...\n", len, data);
    runTransferCb(data, len, TRANSFER_SERVER_ZIGBEE_WRITE);
    runTransferCb(data, len, TRANSFER_SERVER_HYLINK_READ);
    return 0;
}

static int zigbee_recv(char *data, unsigned int len)
{
    printf("---zigbee_recv:%d,%s ...\n", len, data);
    runTransferCb(data, len, TRANSFER_SERVER_HYLINK_WRITE);
    runTransferCb(data, len, TRANSFER_SERVER_ZIGBEE_READ);

    return 0;
}

static int hylink_disconnect(void)
{
    printf("---disconnect ...\n");
    return 0;
}

static int zigbee_disconnect(void)
{
    printf("---disconnect ...\n");
    if (zigbee_connecting_num > 0 && --zigbee_connecting_num == 0)
        runCmdCb((void *)0, LED_DRIVER_LINE);
    return 0;
}

static int hylink_connect(void)
{
    printf("---hylink_connect ...\n");
    return 0;
}

static int zigbee_connect(void)
{
    printf("---zigbee_connect ...\n");
    if (++zigbee_connecting_num == 1)
        runCmdCb((void *)1, LED_DRIVER_LINE);
    return 0;
}

static int hylink_send(void *data, unsigned int len)
{
    return epollServerSend(&hylink_myevents, data, len);
}

static int zigbee_send(void *data, unsigned int len)
{
    return epollServerSend(&zigbee_myevents, data, len);
}

int clientClose(void)
{
    epollServerClose();
    return 0;
}
#define TCP_ADDR "127.0.0.1" //"127.0.0.1" //"192.168.1.2"
int clientOpen(void)
{
    registerTransferCb(hylink_send, TRANSFER_SERVER_HYLINK_WRITE);
    registerTransferCb(zigbee_send, TRANSFER_SERVER_ZIGBEE_WRITE);
    //"127.0.0.1"
    epollTcpEventSet(&hylink_myevents, TCP_ADDR, 7000, hylink_recv, hylink_disconnect, hylink_connect, 1);
    epollTcpEventSet(&zigbee_myevents, TCP_ADDR, 12580, zigbee_recv, zigbee_disconnect, zigbee_connect, 1);
    epollServerOpen(3000);
    epollServerClose();

    return 0;
}
