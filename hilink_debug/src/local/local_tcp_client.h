#ifndef _LOCAL_TCP_CLIENT_H_
#define _LOCAL_TCP_CLIENT_H_

#if USE_LIBUV == 0 && USE_LIBEVENT == 0
#include <pthread.h>

pthread_t net_client(void *arg);
void main_thread_set_signal();
#endif
#endif