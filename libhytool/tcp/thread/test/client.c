
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "threadClient.h"

ThreadTcp threadTcp;
int recv(char *data, unsigned int len)
{
    printf("---recv:%d,%s ...\n", len, data);
    threadClientSend(&threadTcp, data, len);
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
    tcpEventSet(&threadTcp, "127.0.0.1", 7000, recv, disconnect, connect, 0);
    threadClientOpen(&threadTcp);
    threadClientClose(&threadTcp);
    return 0;
}