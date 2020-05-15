#include "event_app.h"
#include "event_test.h"
#include "commom.h"

int GetTime(char* psTime) {
    time_t nSeconds;
    struct tm* pTM;

    time(&nSeconds);
    pTM = localtime(&nSeconds);

    /* 系统时间，格式: HHMMSS */
    sprintf(psTime, "%02d:%02d:%02d", pTM->tm_hour, pTM->tm_min, pTM->tm_sec);

    return 0;
}

void event_callback(evutil_socket_t fd, short flags, void* args) {
    char* psTime = malloc(16);
    GetTime(psTime);
    log_debug("event_callback time: %s\n", psTime);
    free(psTime);
}

static void bev_read_cb(struct bufferevent* bev, void* ctx) {
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
    log_debug("body:%s\n", buf);
    evbuffer_free(input);
    free(buf);
    return;
end:
    free(buf);
}

static void bev_write_cb(struct bufferevent* bev, void* ctx) { log_debug("bufferevent write\n"); }

static void bev_event_cb(struct bufferevent* bev, short events, void* ctx) {
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

int event_test_init(struct ev_app_t* ev_app) {
    int rc = -1;
    ev_app->event = event_new(ev_app->base, -1, EV_TIMEOUT, event_callback, ev_app);
    // ev_app->event = malloc(sizeof(struct event));
    // event_assign(ev_app->event, ev_app->base, -1, EV_TIMEOUT, event_callback, ev_app);
    struct timeval five_seconds = {3, 0};
    rc = event_add(ev_app->event, &five_seconds);
    if (0 != rc) {
        event_del(ev_app->event);
        goto fail;
    }

    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1"); /* 127.0.0.1 */
    sin.sin_port = htons(7070);                   /* Port 8080 */

    ev_app->bev = bufferevent_socket_new(ev_app->base, -1, BEV_OPT_CLOSE_ON_FREE);
    // evutil_make_socket_nonblocking(fd);

    bufferevent_enable(ev_app->bev, EV_WRITE | EV_READ);
    bufferevent_setcb(ev_app->bev, bev_read_cb, bev_write_cb, bev_event_cb, ev_app);

    if (bufferevent_socket_connect(ev_app->bev, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        log_debug("bufferevent_socket_connect fail\n");
        bufferevent_free(ev_app->bev);
        goto fail;
    }
    return 0;

fail:

    return -1;
}

void event_test_destory(struct ev_app_t* ev_app) {
    event_del(ev_app->event);
    bufferevent_free(ev_app->bev);
}
