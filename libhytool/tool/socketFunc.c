#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include "socketFunc.h"

void perrPrintf(const char *str)
{
    perror(str);
}

int Socket(int domain, int type)
{
    int sockfd = 0;
    sockfd = socket(domain, type, 0);
    if (sockfd < 0)
    {
        perrPrintf("socket error\n");
    }

    return sockfd;
}

int Bind(int sockfd, const struct sockaddr *net_addr, socklen_t addrlen)
{
    if (!net_addr)
    {
        perrPrintf("when binding, net_addr is NULL\n");
        return -1;
    }

    if (bind(sockfd, net_addr, addrlen) < 0)
    {
        perrPrintf("fail to bind ipaddr\n");
        return -1;
    }

    return 0;
}

int Listen(int sockfd, int listenNum)
{
    if (listen(sockfd, listenNum) < 0)
    {
        perrPrintf("fail to listen socket\n");
        return -1;
    }

    return 0;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (connect(sockfd, addr, addrlen) < 0)
    {
        perrPrintf("connect error\n");
        return -1;
    }

    return 0;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int ret;

again:
    ret = accept(sockfd, addr, addrlen);
    if (ret < 0)
    {
        if (errno == EINTR || errno == ECONNABORTED)
            goto again;
        else
            perrPrintf("accept error\n");
    }
    return ret;
}

int Close(int fd)
{
    if (fd == 0)
    {
        return -1;
    }
    int n;
    if ((n = close(fd)) == -1)
        perrPrintf("close error:");

    return n;
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
    ssize_t n;

again:
    if ((n = read(fd, ptr, nbytes)) == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            goto again;
        else
            return -1;
    }

    return n;
}

ssize_t Recv(int fd, void *ptr, size_t nbytes, int flag)
{
    ssize_t n;

again:
    if ((n = recv(fd, ptr, nbytes, flag)) == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            goto again;
        else
            return -1;
    }

    return n;
}

ssize_t Write(int fd, const void *ptr, size_t nbytes)
{
    ssize_t n;
    char repeat = 0;
    int outQueueSize = -1;
again:
    if ((n = write(fd, ptr, nbytes)) == -1)
    {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            goto again;
        else
            return -1;
    }
    ioctl(fd, SIOCOUTQ, &outQueueSize);
    if (outQueueSize != 0 && repeat > 0)
    {
        printf("out queue size:%d ........\n", outQueueSize);
        --repeat;
        goto again;
    }
    return n;
}

ssize_t Send(int fd, const void *ptr, size_t nbytes, int flag)
{
    ssize_t n;
    char repeat = 0;
    int outQueueSize = -1;
again:
    if ((n = send(fd, ptr, nbytes, flag)) == -1)
    {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            goto again;
        else
            return -1;
    }
    ioctl(fd, SIOCOUTQ, &outQueueSize);
    if (outQueueSize != 0 && repeat > 0)
    {
        printf("out queue size:%d ........\n", outQueueSize);
        --repeat;
        goto again;
    }
    return n;
}

ssize_t Readn(int fd, void *vptr, size_t n)
{
    size_t nleft; //usigned int 剩余未读取的字节数
    char *ptr;

    ptr = (char *)vptr;
    nleft = n; //n 未读取字节数

    while (nleft > 0)
    {
        ssize_t nread = read(fd, ptr, nleft); //int 实际读到的字节数
        if (nread < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) //被中断，一个都没有读
                nread = 0;
            else
                return -1; //出错 返回 -1
        }
        else if (nread == 0) //读0个字节 读取完毕
            break;
        /*
         fd 要读取的文件、vptr当前读指针位置、以及要读取的字节
         返回实际读取的字节
         */
        nleft -= nread; //nleft = nleft - nread 还要读多少个字节
        ptr += nread;   //已经写到的缓存位置
    }
    return n - nleft;
}

ssize_t Writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    const char *ptr;

    ptr = (char *)vptr;
    nleft = n;
    while (nleft > 0)
    {
        ssize_t nwritten = write(fd, ptr, nleft);
        if (nwritten <= 0)
        {
            if (nwritten < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

int setNonBlock(int sockfd)
{
    int opt = 0;
    opt = fcntl(sockfd, F_GETFL);
    if (opt < 0)
    {
        printf("fcntl(sock,GETFL) failed\n");
        return -1;
    }

    opt = opt | O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, opt) < 0)
    {
        printf("fcntl(sock,SETFL,opts) failed\n");
        return -1;
    }

    return 0;
}
