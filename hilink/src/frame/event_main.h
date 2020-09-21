#if USE_LIBEVENT
#ifndef _EVENT_MAIN_H_
#define _EVENT_MAIN_H_


int event_net_client_open();
void event_net_client_close();
int event_main();
void event_main_close();

int event_client_write(char *data, unsigned int len);
#endif
#endif