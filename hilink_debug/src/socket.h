#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <sys/wait.h>

void perr_exit(const char *str);
int Socket(int domain, int type);
int Bind(int sockfd, const struct sockaddr *net_addr, socklen_t addrlen);
int Listen(int sockfd, int listenNum);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int Close(int fd);

ssize_t Read(int fd, void *ptr, size_t nbytes);
ssize_t Write(int fd, const void *ptr, size_t nbytes);
ssize_t Readn(int fd, void *vptr, size_t n);
ssize_t Writen(int fd, const void *vptr, size_t n);
ssize_t Recv(int fd, void *ptr, size_t nbytes,int flag);
#endif // DEBUG
