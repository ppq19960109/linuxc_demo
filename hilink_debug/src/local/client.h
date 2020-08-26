#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <pthread.h>


int net_client_srart();
pthread_t net_client(void *arg);
#endif 