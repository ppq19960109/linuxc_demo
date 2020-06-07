#ifndef _SERVER_H_
#define _SERVER_H_

#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "socket.h"
#include "log.h"
/* According to POSIX.1-2001, POSIX.1-2008 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <poll.h>
#include <sys/epoll.h>

#define SERVER_PORT 9090

int net_server_srart();
void net_server();
void net_server_fork();
void net_server_pthread();
void net_server_pthread_select();
void net_server_select();

void net_server_epoll();
#endif