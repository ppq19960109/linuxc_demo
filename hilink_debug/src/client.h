#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "socket.h"
int net_client_srart();
void net_client(int* sockfd);
#endif 