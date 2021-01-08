
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "threadServer.h"

ThreadTcpServer threadTcpServer;
int recv(char *data, unsigned int len)
{
    printf("---recv:%d,%s ...\n", len, data);
    threadServerSend(&threadTcpServer, data, len);
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
    tcpEventServerSet(&threadTcpServer, "127.0.0.1", 7000, recv, disconnect, connect, 8);
    threadServerOpen(&threadTcpServer);
    threadServerClose(&threadTcpServer);
    return 0;
}