#include "client.h"

int net_client_srart()
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;                     //簇
    server.sin_port = htons(SERVER_PORT);            //端口
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); //ip地址

    int sockfd = Socket(AF_INET, SOCK_STREAM);
    // Bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    Connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    return sockfd;
}

void net_client()
{
    char buf[255] = {0};
    int readLen;

    int sockfd = net_client_srart();
    while (1)
    {
        scanf("%s", buf);
        // gets(buf);
        readLen = strlen(buf);
        log_debug("Write:%s len:%d\n", buf, readLen);
        Write(sockfd, buf, readLen);
        memset(buf, 0, sizeof(buf));
        readLen = Read(sockfd, buf, sizeof(buf));
        log_debug("Read:%s len:%d\n", buf, readLen);
    }
    Close(sockfd);
}