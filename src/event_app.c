#include "event_app.h"
#include "commom.h"
#include "event_buffer_test.h"
#include "event_test.h"
#include "event_server.h"
#include "http_test.h"
#include "sqlite_test.h"

struct ev_app_t ev_app;

int event_app_init() {
    int rc = -1;

    evthread_use_pthreads();

    ev_app.base = event_base_new();

    rc = sqlite_test_init();
    if (0 != rc) {
        log_debug("sqlite_test_init\n");
        goto fail;
    }
    rc = event_server_init(&ev_app);
    if (0 != rc) {
        log_debug("event_server_init\n");
        goto fail;
    }
    // rc = event_test_init(&ev_app);
    // if (0 != rc) {
    //     log_debug("event_test_init\n");
    //     goto fail;
    // }
    rc = event_buffer_init(&ev_app);
    if (0 != rc) {
        log_debug("event_buffer_init\n");
        goto fail;
    }
    rc = http_init(&ev_app);
    if (0 != rc) {
        log_debug("http_init\n");
        goto fail;
    }
    event_base_loop(ev_app.base, EVLOOP_NO_EXIT_ON_EMPTY);
    return 0;
fail:
    event_base_free(ev_app.base);
    return -1;
}

void event_app_destroy() {
    event_base_loopbreak(ev_app.base);
    event_test_destory(&ev_app);
    event_buffer_destory(&ev_app);
    http_destroy(&ev_app);
    event_base_free(ev_app.base);
}
