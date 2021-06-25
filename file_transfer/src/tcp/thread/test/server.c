
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "threadServer.h"

static ThreadTcpServer threadTcpServer;
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

int server_main()
{
    printf("%d:%s\n", __LINE__, __func__);
    tcpEventServerSet(&threadTcpServer, "127.0.0.1", 7000, recv, disconnect, connect, 1);
    threadServerOpen();
    threadServerClose();
    return 0;
}