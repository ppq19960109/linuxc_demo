
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "threadServer.h"

int ota_state = 1;
static FILE *file = NULL;

static ThreadTcpServer threadTcpServer;
static ThreadTcpServer ota_tcpServer;
static int recv(char *data, unsigned int len)
{
    printf("---recv:%u ...\n", len);
    if (file)
        fwrite(data, 1, len, file);
    return 0;
}
static int disconnect(void)
{
    printf("---disconnect ...\n");
    if (file)
    {
        fclose(file);
        file = NULL;
        ota_state = 0;
    }
    return 0;
}
static int connect(void)
{
    printf("---connect ...\n");
    if (ota_state)
        file = fopen("server.bin", "wb");
    return 0;
}

static int m_recv(char *data, unsigned int len)
{
    printf("%d:%s\n", __LINE__, __func__);
    printf("---recv:%u ...\n", len);
    return 0;
}
static int m_disconnect(void)
{
    printf("%d:%s\n", __LINE__, __func__);
    return 0;
}
static int m_connect(void)
{
    printf("%d:%s\n", __LINE__, __func__);
    return 0;
}

int server_main()
{
    printf("%d:%s\n", __LINE__, __func__);
    tcpEventServerSet(&threadTcpServer, "0.0.0.0", 16666, m_recv, m_disconnect, m_connect, 2);
    tcpEventServerSet(&ota_tcpServer, "0.0.0.0", 16665, recv, disconnect, connect, 1);
    threadServerOpen();
    threadServerClose();
    return 0;
}