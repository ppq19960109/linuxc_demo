#ifndef _UV_MAIN_H_
#define _UV_MAIN_H_

#include "local_receive.h"
#ifdef USE_LIBUV
int net_client_open();
void net_client_close();
int main_open();
void main_close();

int client_write(char *data, unsigned int len);
#endif
#endif