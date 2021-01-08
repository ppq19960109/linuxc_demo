
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "epollServer.h"

struct EpollTcpEvent myevents;
int recv(char *data, unsigned int len)
{
    printf("---recv:%d,%s ...\n", len, data);
    epollServerSend(&myevents, data, len);
    return 0;
}
int disconnect(void)
{
    printf("---disconnect ...\n");
    return 0;
}

int main()
{
    epollTcpEventSet(&myevents, "127.0.0.1", 7000, recv, disconnect, NULL, 1);
    epollServerOpen(3000);
    epollServerClose();
    return 0;
}