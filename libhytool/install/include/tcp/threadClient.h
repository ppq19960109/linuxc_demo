#ifndef _THREADCLIENT_H_
#define _THREADCLIENT_H_

#include <pthread.h>
#define BUF_LEN (16 * 1024)

typedef int (*Recv_cb)(char *data, unsigned int len);
typedef int (*Connect_cb)(void);
typedef int (*Disconnect_cb)(void);

typedef struct
{
    int status;

    char recv_buf[BUF_LEN + 1];
    int recv_len;

    int fd;
    pthread_t tid;
    void *arg;

    char addr[18];
    unsigned short port;
    Recv_cb recv_cb;             //回调函数
    Disconnect_cb disconnect_cb; //回调函数
    Connect_cb connect_cb;
    int isServer;
} ThreadTcp;

int threadClientOpen(ThreadTcp *threadTcp);
int threadClientClose(ThreadTcp *threadTcp);
int threadClientSend(ThreadTcp *threadTcp, void *send, unsigned int len);
void tcpEventSet(ThreadTcp *threadTcp, const char *addr, const short port, Recv_cb recv_cb, Disconnect_cb disconnect_cb, Connect_cb connect_cb, const int isServer);
#endif