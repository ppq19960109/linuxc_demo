#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include <sys/un.h>

#define UNIX_DOMAIN "/tmp/unix_server.domain"
#define CLI_UNIX_DOMAIN "/tmp/unix_client.domain"

int local_tcp_server()
{
    unlink(UNIX_DOMAIN);
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, UNIX_DOMAIN);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        printf("bind error:%s\n", strerror(errno));
        exit(1);
    }
    listen(sockfd, 5);

    int client_sockfd;
    struct sockaddr_un client_addr;
    socklen_t len = sizeof(client_addr);
    char buffer[1024];
    size_t read_size;
    while (1)
    {
        printf("server waiting:\n");
        //接收客户端连接
        client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &len);
        printf("server accept:%s\n", client_addr.sun_path);
        while (client_sockfd > 0)
        {
            //读取数据
            read_size = read(client_sockfd, buffer, sizeof(buffer));
            if (read_size <= 0)
                break;
            buffer[read_size] = 0;
            printf("read:%s\n", buffer);
            //发送数据
            write(client_sockfd, buffer, read_size);
        }
        close(client_sockfd);
    }
    return 0;
}

int local_tcp_client()
{
    unlink(CLI_UNIX_DOMAIN);
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, CLI_UNIX_DOMAIN);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        printf("bind error:%s\n", strerror(errno));
        exit(1);
    }

    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, UNIX_DOMAIN);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect failed: ");
        exit(1);
    }

    char buffer[128];
    size_t read_size;
    while (1)
    {

        fgets(buffer, sizeof(buffer), stdin);
        //发送数据
        write(sockfd, buffer, strlen(buffer));
        //读取数据
        read_size = read(sockfd, buffer, sizeof(buffer));
        buffer[read_size] = 0;
        printf("read:%s\n", buffer);
    }
    close(sockfd);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        local_tcp_client();
    }
    local_tcp_server();
    return 0;
}