#ifndef _EVENT_APP_H_
#define _EVENT_APP_H_

#include "event2/event.h"
#include "event2/event_struct.h"
#include "event2/http.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include "event2/thread.h"


#include <event2/listener.h>

struct ev_app_t {
    pthread_t thread;

    struct event_base* base;
    struct event* event;
    struct evhttp* http;
    struct bufferevent* bepair[2];
    struct bufferevent* ev_app.client_bufevbev;
};

int event_app_init();
void event_app_destroy();
#endif
