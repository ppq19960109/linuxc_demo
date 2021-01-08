
#include "tcp.h"
#include "threadServer.h"

int threadServerOpen(ThreadTcpServer *threadTcpServer)
{
    if (tcpServerListen(&threadTcpServer->threadTcp.fd, threadTcpServer->threadTcp.addr, threadTcpServer->threadTcp.port, threadTcpServer->clientNumMax) < 0)
        return -1;

    int cfd;
    struct sockaddr_in cin;

    while (1)
    {
        socklen_t len = sizeof(cin);
        if ((cfd = accept(threadTcpServer->threadTcp.fd, (struct sockaddr *)&cin, &len)) == -1)
        {
            if (errno != EAGAIN && errno != EINTR)
            {
                sleep(1);
            }
            printf("%s:accept,%s\n", __func__, strerror(errno));
            continue;
        }
        do
        {
            int i;
            for (i = 0; i < threadTcpServer->clientNumMax; i++) //从全局数组g_events中找一个空闲元素，类似于select中找值为-1的元素
            {
                if (threadTcpServer->clientList[i].status == 0)
                    break;
            }
            if (i == threadTcpServer->clientNumMax) // 超出连接数上限
            {
                printf("%s: max connect limit[%d]\n", __func__, threadTcpServer->clientNumMax);
                close(cfd);
                break;
            }

            printf("new connect[%s:%d]\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
            tcpEventSet(&threadTcpServer->clientList[i], inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), threadTcpServer->threadTcp.recv_cb, threadTcpServer->threadTcp.disconnect_cb, threadTcpServer->threadTcp.connect_cb, 1);
            threadClientOpen(&threadTcpServer->clientList[i]);
            printf("new connect[%s:%d]\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
        } while (0);
    }
    return 0;
}

int threadServerClose(ThreadTcpServer *threadTcpServer)
{
    threadClientClose(&threadTcpServer->threadTcp);
    for (int i = 0; i < threadTcpServer->clientNumMax; ++i)
    {
        threadClientClose(&threadTcpServer->clientList[i]);
    }
    sleep(1);
    free(threadTcpServer->clientList);
    return 0;
}

int threadServerSend(ThreadTcpServer *threadTcpServer, void *send, unsigned int len)
{
    if (send == NULL)
        return -1;
    if (threadTcpServer->threadTcp.fd == 0)
    {
        printf("socketfd is null\n");
        return -1;
    }

    for (int i = 0; i < threadTcpServer->clientNumMax; ++i)
    {
        threadClientSend(&threadTcpServer->clientList[i], send, len);
    }

    return 0;
}

void tcpEventServerSet(ThreadTcpServer *threadTcpServer, const char *addr, const short port, Recv_cb recv_cb, Disconnect_cb disconnect_cb, Connect_cb connect_cb, int clientNumMax)
{
    threadTcpServer->clientList = malloc(sizeof(ThreadTcp) * clientNumMax);
    memset(threadTcpServer->clientList, 0, sizeof(ThreadTcp) * clientNumMax);
    tcpEventSet(&threadTcpServer->threadTcp, addr, port, recv_cb, disconnect_cb, connect_cb, 1);
    return;
}
