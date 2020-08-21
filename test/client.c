#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int net_client_srart()
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;                     //簇
    server.sin_port = htons(2233);            //端口
    server.sin_addr.s_addr = inet_addr("192.168.1.22"); //ip地址

        struct sockaddr_in client;
    client.sin_family = AF_INET;                     //簇
    client.sin_port = htons(7777);            //端口
    client.sin_addr.s_addr = inet_addr("192.168.1.166"); //ip地址
    int ret;
    int sockfd = socket(AF_INET, SOCK_STREAM,0);
    if (sockfd < 0)
    {
        perror("socket error\n");
    }
    ret=bind(sockfd, (struct sockaddr *)&client, sizeof(struct sockaddr));
        if (ret != 0)
    {
        perror("bind error\n");
    }
    ret=connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    if (ret != 0)
    {
        perror("connect error\n");
    }
    return sockfd;
}

void net_client()
{
    char buf[255] = {"0123"};
    int readLen;

    int sockfd = net_client_srart();
    printf("net_client_srart %d\n",sockfd);
    while (1)
    {
                        memset(buf, 0, sizeof(buf));
                scanf("%s", buf);

                readLen = strlen(buf);
                printf("Write:%s len:%d\n", buf, readLen);

       readLen= write(sockfd, buf, readLen);
        // memset(buf, 0, sizeof(buf));
        // readLen = read(sockfd, buf, sizeof(buf));
        // // if(readLen>0)
        // printf("Read:%s len:%d\n", buf, readLen);
    }
    close(sockfd);
}
int main()
{
    net_client();
    return 0;
}