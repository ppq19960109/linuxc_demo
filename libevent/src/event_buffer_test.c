#include "event_app.h"
#include "event_buffer_test.h"
#include "commom.h"
#include "file.h"

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
    log_debug("body:%s\n", buf);

    file_test(buf, total);

    free(buf);
    return;
end:
    free(buf);
}

static void write_cb(struct bufferevent* bev, void* ctx) {
    log_debug("bufferevent write\n");
}

static void event_cb(struct bufferevent* bev, short events, void* ctx) {
    log_debug("bufferevent event\n");
    log_debug("event: %d\n", events);
}

int event_buffer_init(struct ev_app_t* ev_app) {
    int rc = -1;
    rc = bufferevent_pair_new(ev_app->base, BEV_OPT_THREADSAFE, ev_app->bepair);
    if (0 != rc) {
        log_debug("bufferevent_pair_new fail:%d\n", rc);
        goto fail;
    }
    bufferevent_enable(ev_app->bepair[1], EV_READ);
    bufferevent_enable(ev_app->bepair[0], EV_WRITE);
    bufferevent_setcb(ev_app->bepair[0], NULL, write_cb, event_cb, ev_app);
    bufferevent_setcb(ev_app->bepair[1], read_cb, NULL, event_cb, ev_app);
    return 0;
fail:

    return -1;
}

void event_buffer_destory(struct ev_app_t* ev_app) {
    bufferevent_free(ev_app->bepair[0]);
    bufferevent_free(ev_app->bepair[1]);
}