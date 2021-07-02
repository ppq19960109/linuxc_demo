
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "threadClient.h"

#define OTA_BUF_LEN 4096
static int recv_buf[OTA_BUF_LEN];
static FILE *file = NULL;

static ThreadTcp threadTcp;
static int recv(char *data, unsigned int len)
{
    printf("---recv:%u,%s ...\n", len, data);

    return 0;
}
static int disconnect(void)
{
    printf("---disconnect ...\n");

    return 0;
}
static int connect(void)
{
    printf("---connect ...\n");
    file = fopen("install.bin", "rb+");
    return 0;
}
// #define ADDR "192.168.1.56"
// #define ADDR "127.0.0.1"
int client_main(char* remote_ip)
{
    size_t read_len;
    printf("%d:%s\n", __LINE__, __func__);
    tcpEventSet(&threadTcp, remote_ip, 16665, recv, disconnect, connect, 0);
    threadClientOpen(&threadTcp);
    while (1)
    {
        if (file == NULL)
        {
            sleep(1);
        }
        else
        {
            read_len = fread(recv_buf, 1, OTA_BUF_LEN, file);
            threadClientSend(&threadTcp, recv_buf, read_len);
            printf("threadClientSend:%d\n",read_len);
            if (read_len < OTA_BUF_LEN)
            {
                break;
            }
        }
    }
    if (file)
    {
        fclose(file);
        file = NULL;
    }
    threadClientClose(&threadTcp);
    return 0;
}