#include "tcp.h"

int tcpClientConnect(int *fd, const char *addr, const short port)
{
    if (*fd != 0)
        close(*fd);

    struct sockaddr_in server;
    server.sin_family = AF_INET;              //簇
    server.sin_port = htons(port);            //端口
    server.sin_addr.s_addr = inet_addr(addr); //ip地址

    *fd = Socket(AF_INET, SOCK_STREAM);

    int buf_size = 0;
    socklen_t optlen = sizeof(buf_size);
    if (getsockopt(*fd, SOL_SOCKET, SO_SNDBUF, &buf_size, &optlen) < 0)
    {
        printf("getsockopt error=%d(%s)!!!\n", errno, strerror(errno));
    }
    printf("getsockopt send success=%d\n", buf_size);
    if (getsockopt(*fd, SOL_SOCKET, SO_RCVBUF, &buf_size, &optlen) < 0)
    {
        printf("getsockopt error=%d(%s)!!!\n", errno, strerror(errno));
    }
    printf("getsockopt recv success=%d\n", buf_size);

    // Bind(*fd, (struct sockaddr *)&server, sizeof(struct sockaddr));

    if (Connect(*fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) != 0)
        return -1;

    return *fd;
}

int tcpServerListen(int *fd, const char *addr, const short port, int listenNum)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin)); //bzero(&sin, sizeof(sin))
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(addr);
    sin.sin_port = htons(port);

    int lfd = *fd = socket(AF_INET, SOCK_STREAM, 0);

    int buf_size = 0;
    socklen_t optlen = sizeof(buf_size);
    if (getsockopt(*fd, SOL_SOCKET, SO_SNDBUF, &buf_size, &optlen) < 0)
    {
        printf("getsockopt error=%d(%s)!!!\n", errno, strerror(errno));
    }
    printf("getsockopt send success=%d\n", buf_size);
    if (getsockopt(*fd, SOL_SOCKET, SO_RCVBUF, &buf_size, &optlen) < 0)
    {
        printf("getsockopt error=%d(%s)!!!\n", errno, strerror(errno));
    }
    printf("getsockopt recv success=%d\n", buf_size);

    int opt = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
    {
        printf("setsockopt error\n");
        return -1;
    }

    bind(lfd, (struct sockaddr *)&sin, sizeof(sin));

    listen(lfd, listenNum);

    return lfd;
}
