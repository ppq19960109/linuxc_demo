#if USE_LIBEVENT
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>

#include "event_main.h"
#include "local_send.h"
#include "local_callback.h"
#include "local_device.h"

struct ev_app_t
{
    struct event_base *base;
    struct event *timer_event;
    struct event *signal_event;
    struct event *connect_event;
    struct bufferevent *client_bufev;
#define RECVLEN 16384
    char tcpBuf[RECVLEN + 1];
    int read_total;
};

struct ev_app_t ev_app;

static int event_connect_timer();
static int event_timer_open();
static void event_timer_close();

//------------------------------------------
static int event_client_write(char *data, unsigned int len)
{
    // printf("event_client_write:%s\n", data);
    return bufferevent_write(ev_app.client_bufev, data, len);
}
static void client_write_cb(struct bufferevent *bev, void *ctx)
{
    printf("bufferevent write\n");
}
//------------------------------------------
static void client_read_cb(struct bufferevent *bev, void *ctx)
{
    printf("bufferevent read\n");

    // struct evbuffer* input = bufferevent_get_input(bev);
    struct evbuffer *input = evbuffer_new();
    int single = 0;

    bufferevent_read_buffer(bev, input);
    single = evbuffer_get_length(input);
    // char *buf = malloc(read_total + 1);
    evbuffer_remove(input, &ev_app.tcpBuf[ev_app.read_total], single);
    ev_app.read_total += single;

    if (single >= 4096 && ev_app.tcpBuf[single] != 0x03)
    {
        return;
    }

    ev_app.tcpBuf[ev_app.read_total] = 0;
    recv_toLocal(ev_app.tcpBuf, ev_app.read_total);
    ev_app.read_total = 0;
    // printf("client_read:%s,%d\n", buf, total);
    evbuffer_free(input);
    // free(buf);
    return;
}
static void client_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    printf("bufferevent event: %x\n", events);
    // #define BEV_EVENT_READING	0x01	/**< error encountered while reading */
    // #define BEV_EVENT_WRITING	0x02	/**< error encountered while writing */
    // #define BEV_EVENT_EOF		0x10	/**< eof file reached */
    // #define BEV_EVENT_ERROR		0x20	/**< unrecoverable error encountered */
    // #define BEV_EVENT_TIMEOUT	0x40	/**< user-specified timeout reached */
    // #define BEV_EVENT_CONNECTED	0x80	/**< connect operation finished. */
    if (events & BEV_EVENT_EOF)
    {
        printf("BEV_EVENT_EOF:eof file reached\n");
    }
    if (events & BEV_EVENT_ERROR)
    {
        printf("BEV_EVENT_ERROR:unrecoverable error encountered\n");
    }

    if (events & BEV_EVENT_CONNECTED)
    {
        printf("BEV_EVENT_CONNECTED:connect operation finished.\n");
        // struct evbuffer *evbuf = bufferevent_get_input(bev);
        // size_t evlen = evbuffer_get_length(evbuf);
        // char *evdata = malloc(evlen);
        // evbuffer_remove(evbuf, evdata, evlen);
        // log_debug("evlen:%d,evdata:%s\n", evlen, evdata);
        // free(evdata);
        event_timer_open();
        event_del(ev_app.connect_event);
        write_hanyar_cmd(STR_ADD, NULL, STR_NET_CLOSE);
        write_hanyar_cmd(STR_DEVSINFO, NULL, NULL);
    }
    else
    {
        event_timer_close();
        event_connect_timer();
    }
}
static void event_client_connect_cb(evutil_socket_t fd, short event, void *arg)
{
    printf("event_client_connect_cb\n");
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(SERVER_PORT); //SERVER_PORT

    if (bufferevent_socket_connect(ev_app.client_bufev, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0)
    {
        printf("bufferevent_socket_connect fail\n");
    }
}

static int event_connect_timer()
{
    if (event_pending(ev_app.connect_event, EV_PERSIST, NULL) != 0)
    {
        printf("event_pending EV_PERSIST\n");
        return -1;
    }
    struct timeval timeout = {2, 0};
    int rc = event_add(ev_app.connect_event, &timeout);
    if (0 != rc)
    {
        event_del(ev_app.connect_event);
        return -1;
    }
    return 0;
}

static void event_socket_client_open()
{
    ev_app.client_bufev = bufferevent_socket_new(ev_app.base, -1, BEV_OPT_CLOSE_ON_FREE);

    // ev_ssize_t n = bufferevent_get_read_limit(ev_app.client_bufev);
    // ev_ssize_t n2 = bufferevent_get_max_to_read(ev_app.client_bufev);
    // ev_ssize_t n3 = bufferevent_get_max_single_read(ev_app.client_bufev);
    // log_error("bufferevent_get_read_limit %d,%d,%d\n", n, n2, n3);

    // evutil_make_socket_nonblocking(fd);

    bufferevent_setcb(ev_app.client_bufev, client_read_cb, client_write_cb, client_event_cb, NULL);
    bufferevent_enable(ev_app.client_bufev, EV_WRITE | EV_READ);
    // bufferevent_set_max_single_read(ev_app.client_bufev, 16384);
    ev_app.connect_event = event_new(ev_app.base, -1, EV_TIMEOUT | EV_PERSIST, event_client_connect_cb, NULL);
    event_connect_timer();
}

static void event_socket_client_close()
{
    bufferevent_free(ev_app.client_bufev);
    event_del(ev_app.connect_event);
    event_free(ev_app.connect_event);
}
//------------------------------

static void event_timer_cb(evutil_socket_t fd, short event, void *arg)
{
    // printf("event_timer_cb\n");
    write_heart();
}

static int event_timer_open()
{
    ev_app.timer_event = event_new(ev_app.base, -1, EV_TIMEOUT | EV_PERSIST, event_timer_cb, NULL);
    struct timeval timeout = {60, 0};
    int rc = event_add(ev_app.timer_event, &timeout);
    if (0 != rc)
    {
        event_del(ev_app.timer_event);
        return -1;
    }
    return 0;
}

static void event_timer_close()
{
    if (ev_app.timer_event)
    {
        event_del(ev_app.timer_event);
        event_free(ev_app.timer_event);
    }
}
//---------------------------------------------
static void event_signal_cb(evutil_socket_t fd, short event, void *arg)
{
    printf("event_signal_cb:%d\n", fd);
    if (fd == SIGINT || fd == SIGQUIT || fd == SIGKILL)
    {
        local_system_restartOrReFactory(INT_OFFLINE);
    }
}
static void event_signal_open()
{
    ev_app.signal_event = evsignal_new(ev_app.base, SIGINT, event_signal_cb, NULL);
    evsignal_add(ev_app.signal_event, NULL);
}
static void event_signal_close()
{
    evsignal_del(ev_app.signal_event);
    event_free(ev_app.signal_event);
}
//--------------------------------------------------
void event_main_close()
{
    event_signal_close();
    event_timer_close();
    event_socket_client_close();
}

void event_main_open()
{
    register_closeCallback(event_main_close);
    register_writeCallback(event_client_write);

    evthread_use_pthreads();
    ev_app.base = event_base_new();

    event_signal_open();
    event_socket_client_open();

    event_base_loop(ev_app.base, EVLOOP_NO_EXIT_ON_EMPTY);
    event_base_free(ev_app.base);
    event_main_close();
}
#endif