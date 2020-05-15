#include "event_app.h"
#include "event_server.h"
#include "commom.h"


static void read_cb(struct bufferevent* bev, void* ctx) {
    int rc;
    log_debug("bufferevent read\n");
    // struct evbuffer* input = bufferevent_get_input(bev);
    struct evbuffer* input = evbuffer_new();
    bufferevent_read_buffer(bev, input);
    int total = evbuffer_get_length(input);

    char* buf = malloc(total);
    rc = evbuffer_remove(input, buf, total);
    if (-1 == rc) {
        goto end;
    }
    log_debug("body:%s end\n", buf);
    log_debug("body\n");
    evbuffer_free(input);
    free(buf);
    return;
end:
    free(buf);
}

static void write_cb(struct bufferevent* bev, void* ctx) { log_debug("bufferevent write\n"); }

static void event_cb(struct bufferevent* bev, short events, void* ctx) {
    log_debug("bufferevent event\n");
    // #define BEV_EVENT_READING	0x01	/**< error encountered while reading */
    // #define BEV_EVENT_WRITING	0x02	/**< error encountered while writing */
    // #define BEV_EVENT_EOF		0x10	/**< eof file reached */
    // #define BEV_EVENT_ERROR		0x20	/**< unrecoverable error encountered */
    // #define BEV_EVENT_TIMEOUT	0x40	/**< user-specified timeout reached */
    // #define BEV_EVENT_CONNECTED	0x80	/**< connect operation finished. */
    log_debug("event_cb event: %x\n", events);
    if (events & BEV_EVENT_CONNECTED) {
        struct evbuffer* evbuf = bufferevent_get_input(bev);
        size_t evlen = evbuffer_get_length(evbuf);
        char* evdata = malloc(evlen);
        evbuffer_remove(evbuf, evdata, evlen);
        log_debug("evlen:%d,evdata:%s\n", evlen, evdata);
        free(evdata);
    }
}

void server_accept(struct evconnlistener* listener,evutil_socket_t fd,struct sockaddr *address,int socklen,void *arg)
{

    struct event_base *base = evconnlistener_get_base(listener);
	//下面代码是为这个fd创建一个bufferevent
    struct bufferevent *bev =  bufferevent_socket_new(base, fd,
                                               BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, read_cb,write_cb, event_cb, NULL);
    bufferevent_enable(bev, EV_WRITE | EV_READ | EV_PERSIST);

}
int event_server_init(struct ev_app_t* ev_app) {
    int rc = -1;
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1"); /* 127.0.0.1 */
    sin.sin_port = htons(7070);                   /* Port 8080 */

     struct evconnlistener* listener =evconnlistener_new_bind(ev_app->base,server_accept,
                                                                        NULL,LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,-1,
                                                                        (struct sockaddr*)&sin,sizeof(struct sockaddr));

    return 0;

fail:

    return -1;
}

void event_server_destory(struct ev_app_t* ev_app) {
    //  evconnlistener_free(listener);
}
