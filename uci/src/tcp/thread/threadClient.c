
#include "tcp.h"
#include "threadClient.h"

static void *threadHander(void *arg)
{
    int ret;
    ThreadTcp *threadTcp = ((ThreadTcp *)arg);
    threadTcp->status = 1;
    
    do
    {
        while (threadTcp->status && threadTcp->isServer == 0)
        {
            sleep(2);
            if (tcpClientConnect(&threadTcp->fd, threadTcp->addr, threadTcp->port) > 0)
                break;
        }

        if (threadTcp->connect_cb != NULL)
            threadTcp->connect_cb();

        while (threadTcp->status)
        {
            ret = Recv(threadTcp->fd, threadTcp->recv_buf, sizeof(threadTcp->recv_buf), 0);
            if (ret <= 0)
            {
                printf("thread client Recv ret:%d,error:%d,%s\n", ret, errno, strerror(errno));
                if (threadTcp->disconnect_cb != NULL)
                    threadTcp->disconnect_cb();
                break;
            }
            else
            {
                threadTcp->recv_len = ret;
                threadTcp->recv_buf[threadTcp->recv_len] = '\0';
                if (threadTcp->recv_cb != NULL)
                    threadTcp->recv_cb(threadTcp->recv_buf, threadTcp->recv_len);
            }
        }
        Close(threadTcp->fd);
        threadTcp->fd = 0;

    } while (threadTcp->status && threadTcp->isServer == 0);
    threadTcp->status = 0;
    pthread_exit(0);
}

int threadClientOpen(ThreadTcp *threadTcp)
{
    //clientMethod为此线程客户端，要执行的程序。
    pthread_create(&threadTcp->tid, NULL, (void *)threadHander, threadTcp->arg);
    //要将id分配出去。
    pthread_detach(threadTcp->tid);

    return 0;
}

int threadClientClose(ThreadTcp *threadTcp)
{
    if (threadTcp->status == 0)
        return -1;
    threadTcp->status = 0;
    if (threadTcp->tid != 0)
    {
        pthread_cancel(threadTcp->tid);
    }
    if (threadTcp->fd != 0)
    {
        Close(threadTcp->fd);
        threadTcp->fd = 0;
    }
    // sleep(1);

    return 0;
}

int threadClientSend(ThreadTcp *threadTcp, void *send, unsigned int len)
{
    if (send == NULL)
        return -1;
    if (threadTcp->fd == 0 || threadTcp->status == 0)
    {
        printf("socketfd is null\n");
        return -1;
    }
    int ret = Send(threadTcp->fd, send, len, 0);

    return ret;
}

void tcpEventSet(ThreadTcp *threadTcp, const char *addr, const short port, Recv_cb recv_cb, Disconnect_cb disconnect_cb, Connect_cb connect_cb, const int isServer)
{
    threadTcp->fd = 0;
    threadTcp->arg = threadTcp;
    threadTcp->status = 0;
    if (threadTcp->recv_len <= 0)
    {
        memset(threadTcp->recv_buf, 0, sizeof(threadTcp->recv_buf));
        threadTcp->recv_len = 0;
    }

    strcpy(threadTcp->addr, addr);
    threadTcp->port = port;
    threadTcp->recv_cb = recv_cb;
    threadTcp->disconnect_cb = disconnect_cb;
    threadTcp->connect_cb = connect_cb;
    threadTcp->isServer = isServer;
    return;
}
