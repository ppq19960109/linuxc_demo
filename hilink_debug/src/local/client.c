#include "client.h"
#include "protocol_cover.h"

#define SERVER_PORT 9090

int net_client_srart()
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;                     //簇
    server.sin_port = htons(SERVER_PORT);            //端口
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); //ip地址

    int sockfd = Socket(AF_INET, SOCK_STREAM);
    // Bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    while (Connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) != 0)
    {
        sleep(2);
    }
    return sockfd;
}

void *thread_hander(void *arg)
{
    int *fd = ((int *)arg);
    while (1)
    {
        *fd = net_client_srart();
        int sockfd = *fd;
        char buf[256];
        int readLen, pos, step = 0;
        while (1)
        {
            // memset(buf, 0, sizeof(buf));
            // while (step < 2)
            // {
            //     switch (step)
            //     {
            //     case 0:
            //         readLen = Recv(sockfd, buf, 1, 0);
            //         if (readLen == 1)
            //         {
            //             if (buf[0] == '{')
            //             {
            //                 step = 1;
            //                 pos = 1;
            //             }
            //         }
            //         break;
            //     case 1:
            //         readLen = Recv(sockfd, buf + pos, sizeof(buf) - pos, 0);
            //         if (readLen < 0)
            //         {
            //             return;
            //         }
            //         pos += readLen;
            //         if (pos >= 256)
            //         {
            //             step = 3;
            //             break;
            //         }
            //         if (buf[pos - 1] == '}')
            //         {
            //             step = 2;
            //         }
            //         break;
            //     }
            // }

            readLen = Recv(sockfd, buf, sizeof(buf), 0);
            // log_debug("Read:%s len:%d\n", buf, readLen);
            if (readLen == 0)
            {
                log_debug("client close");
                break;
            }
            else if (readLen < 0)
            {
                break;
            }
            else
            {
                printf("%s\n", buf);
                // Write(STDOUT_FILENO, buf, readLen);

                read_from_local(buf);
                step = 0;
            }
        }
        close(sockfd);
    }
    pthread_exit(0);
}

void net_client(int *sockfd)
{

    pthread_t id;
    //clientMethod为此线程客户端，要执行的程序。
    pthread_create(&id, NULL, (void *)thread_hander, (void *)sockfd);
    //要将id分配出去。
    pthread_detach(id);
}