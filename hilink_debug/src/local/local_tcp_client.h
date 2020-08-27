#ifndef _LOCAL_TCP_CLIENT_H_
#define _LOCAL_TCP_CLIENT_H_

#include <pthread.h>

int net_client_srart();
pthread_t net_client(void *arg);
void main_thread_set_signal();
#endif 