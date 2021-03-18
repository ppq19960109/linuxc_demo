#include <sys/select.h>
#include <limits.h>
#include "tcp.h"
#include "threadServer.h"

LIST_HEAD(threadServerList);

void thread_list_add(ThreadTcpServer *threadTcpServer, struct list_head *head)
{
    list_add(&threadTcpServer->node, head);
}

void thread_list_del(ThreadTcpServer *threadTcpServer)
{
    list_del(&threadTcpServer->node);
}

void thread_list_for_each(void (*call_back)(ThreadTcpServer *), struct list_head *head)
{
    ThreadTcpServer *threadTcpServer;
    list_for_each_entry(threadTcpServer, head, node)
    {
        call_back(threadTcpServer);
    }
    return;
}

ThreadTcpServer *thread_list_get_by_fd(const int fd, struct list_head *head)
{
    ThreadTcpServer *threadTcpServer;
    list_for_each_entry(threadTcpServer, head, node)
    {
        if (threadTcpServer->threadTcp.fd == fd)
            return threadTcpServer;
    }
    return NULL;
}
//-------------------------------

int threadServerAccept(ThreadTcpServer *threadTcpServer)
{
    if (threadTcpServer == NULL)
    {
        printf("threadTcpServer is null!!!!!!!\n");
        return -1;
    }
    int cfd, i;
    struct sockaddr_in cin;
    socklen_t len = sizeof(cin);
    if ((cfd = accept(threadTcpServer->threadTcp.fd, (struct sockaddr *)&cin, &len)) == -1)
    {
        if (errno != EAGAIN && errno != EINTR)
        {
            sleep(1);
        }
        printf("%s:accept,%s\n", __func__, strerror(errno));
        return -1;
    }
    do
    {
        for (i = 0; i < threadTcpServer->clientNumMax; ++i) //从全局数组g_events中找一个空闲元素，类似于select中找值为-1的元素
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
        threadTcpServer->clientList[i].fd = cfd;
        threadClientOpen(&threadTcpServer->clientList[i]);
        printf("new connect[%s:%d]\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
    } while (0);
    return 0;
}

int threadServerOpen(void)
{
    int maxfd = INT_MIN, minfd = INT_MAX;
    fd_set rfds, copy_fds;
    FD_ZERO(&rfds);

    ThreadTcpServer *threadTcpServer;
    list_for_each_entry(threadTcpServer, &threadServerList, node)
    {
        FD_SET(threadTcpServer->threadTcp.fd, &rfds);

        if (threadTcpServer->threadTcp.fd > maxfd)
            maxfd = threadTcpServer->threadTcp.fd;
        if (threadTcpServer->threadTcp.fd < minfd)
            minfd = threadTcpServer->threadTcp.fd;
    }
    copy_fds = rfds;

    int i, n;

    while (1)
    {
        rfds = copy_fds;
        n = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (n < 0)
        {
            perror("select error");
            continue;
        }
        else if (n == 0) // 没有准备就绪的文件描述符  就进入下一次循环
        {
            printf("select timeout\n");
            continue;
        }
        else
        {
            for (i = minfd; i < maxfd + 1; i++)
            {
                if (FD_ISSET(i, &rfds))
                {
                    threadServerAccept(thread_list_get_by_fd(i, &threadServerList));
                }
            }
        }
    }
    return 0;
}

static void eachThreadServerClose(ThreadTcpServer *threadTcpServer)
{
    threadClientClose(&threadTcpServer->threadTcp);
    for (int i = 0; i < threadTcpServer->clientNumMax; ++i)
    {
        threadClientClose(&threadTcpServer->clientList[i]);
    }
    // sleep(1);
    free(threadTcpServer->clientList);
    // return 0;
}
void threadServerClose(void)
{
    thread_list_for_each(eachThreadServerClose, &threadServerList);
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

int tcpEventServerSet(ThreadTcpServer *threadTcpServer, const char *addr, const short port, Recv_cb recv_cb, Disconnect_cb disconnect_cb, Connect_cb connect_cb, int clientNumMax)
{
    threadTcpServer->clientNumMax = clientNumMax;
    threadTcpServer->clientList = malloc(sizeof(ThreadTcp) * clientNumMax);
    memset(threadTcpServer->clientList, 0, sizeof(ThreadTcp) * clientNumMax);
    tcpEventSet(&threadTcpServer->threadTcp, addr, port, recv_cb, disconnect_cb, connect_cb, 1);

    if (tcpServerListen(&threadTcpServer->threadTcp.fd, threadTcpServer->threadTcp.addr, threadTcpServer->threadTcp.port, threadTcpServer->clientNumMax) < 0)
        return -1;
    thread_list_add(threadTcpServer, &threadServerList);
    return 0;
}
