#ifndef _LOCAL_TCP_CLIENT_H_
#define _LOCAL_TCP_CLIENT_H_

#if USE_LIBUV == 0 && USE_LIBEVENT == 0
#include <pthread.h>

void tcp_client_open();
void tcp_client_close();
int tcp_client_write(char *data, unsigned int len);
#endif
#endif