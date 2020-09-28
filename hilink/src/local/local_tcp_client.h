#ifndef _LOCAL_TCP_CLIENT_H_
#define _LOCAL_TCP_CLIENT_H_

#if USE_LIBUV == 0 && USE_LIBEVENT == 0
#include <pthread.h>
#include <time.h>
#include <signal.h>

typedef void (*timer_function)(union sigval);

void tcp_client_open();
void tcp_client_close();
int tcp_client_write(char *data, unsigned int len);

timer_t start_timer(int sival, timer_function fun, int interval_sec, int sec);
#endif
#endif