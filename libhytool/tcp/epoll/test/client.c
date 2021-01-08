
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "epollClient.h"

struct EpollTcpEvent myevents;
int recv(char *data, unsigned int len)
{
    printf("---recv:%d,%s ...\n", len, data);
    epollClientSend(&myevents, data, len);
    return 0;
}
int disconnect(void)
{
    printf("---disconnect ...\n");
    return 0;
}
int connect(void)
{
    printf("---connect ...\n");
    return 0;
}
int main()
{
    epollTcpEventSet(&myevents, "127.0.0.1", 7000, recv, disconnect, connect, 0);
    epollClientOpen(3000);
    epollClientClose();
    return 0;
}