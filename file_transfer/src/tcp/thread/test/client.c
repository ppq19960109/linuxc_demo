
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "threadClient.h"

static ThreadTcp threadTcp;
int recv(char *data, unsigned int len)
{
    printf("---recv:%u,%s ...\n", len, data);

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
int client_main()
{
    printf("%d:%s\n", __LINE__, __func__);
    tcpEventSet(&threadTcp, "127.0.0.1", 7000, recv, disconnect, connect, 0);
    threadClientOpen(&threadTcp);
    while (1)
    {
        sleep(1);
    }
    threadClientClose(&threadTcp);
    return 0;
}