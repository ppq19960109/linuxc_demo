// 接收端
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
 #include<netinet/tcp.h>
#define SERVER_PORT 9999


int tcp_is_connected(int fd)
{
    struct tcp_info info;
    int len = sizeof(info);
 
    if (fd <= 0) return 0;
 
    getsockopt(fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    /*if (info.tcpi_state == TCP_ESTABLISHED) */
    if (info.tcpi_state == 1)  // Is connected
        return 1;
    else  // Losed
        return 0;
}

int net_server_srart()
{
    struct sockaddr_in server;
    bzero(&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;                     //簇
    server.sin_port = htons(SERVER_PORT);            //端口
    server.sin_addr.s_addr = htonl(INADDR_ANY); //ip地址

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int ret;
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
    {
        return -1;
    }
    ret = bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    if (ret < 0)
    {
        perror("bind error\n");
    }
    ret = listen(sockfd, 9);
    if (ret < 0)
    {
        perror("listen error\n");
    }
    return sockfd;
}

void net_server_input()
{
    char buf[1024];
    int readLen;
    struct sockaddr_in server_client;
    socklen_t addrlen=1;

    int sockfd = net_server_srart();
    printf("net_server_srart %d\n",sockfd);
    while (1)
    {
        int clientfd = accept(sockfd, (struct sockaddr *)&server_client, &addrlen);
        printf("clientfd:%d\n",clientfd);

        if (clientfd != -1)
        {
            printf("client ip: %s,port: %d\n", inet_ntoa(server_client.sin_addr), ntohs(server_client.sin_port));
            while (1)
            {
                memset(buf, 0, sizeof(buf));

                scanf("%s", buf);
                if(tcp_is_connected(clientfd)==0)
                    break;
                readLen = strlen(buf);
                
                readLen =write(clientfd, buf, readLen);
                printf("Write:%s len:%d\n", buf, readLen);

                

                // readLen = read(clientfd, buf, sizeof(buf));
                // printf("Read:%s len:%d\n", buf, readLen);
                // if (readLen == 0)
                // {
                //     printf("client close");
                //     break;
                // }
                // else if (readLen < 0)
                // {
                // }
                // else
                // {
                //     write(clientfd, buf, readLen);
                // }
            }
        }
        else
        {
            perror("accept error:");
        }
        
    }
    close(sockfd);
}

int main()
{
    net_server_input();
    return 0;
}