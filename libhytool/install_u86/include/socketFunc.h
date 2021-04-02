#ifndef _SOCKETFUNC_H_
#define _SOCKETFUNC_H_

#include <netinet/ip.h>

int Socket(int domain, int type);
int Bind(int sockfd, const struct sockaddr *net_addr, socklen_t addrlen);
int Listen(int sockfd, int listenNum);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int Close(int fd);

ssize_t Read(int fd, void *ptr, size_t nbytes);
ssize_t Recv(int fd, void *ptr, size_t nbytes, int flag);
ssize_t Write(int fd, const void *ptr, size_t nbytes);
ssize_t Send(int fd, const void *ptr, size_t nbytes, int flag);
ssize_t Readn(int fd, void *vptr, size_t n);
ssize_t Writen(int fd, const void *vptr, size_t n);
int setNonBlock(int sockfd);

#endif
