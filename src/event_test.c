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

int event_test_init(struct ev_app_t* ev_app) {
    int rc = -1;
    ev_app->event =
        event_new(ev_app->base, -1, EV_TIMEOUT, event_callback, ev_app);
    struct timeval five_seconds = {3, 0};
    rc=event_add(ev_app->event, &five_seconds);
    if (0 != rc) {
        goto fail;
    }
    return 0;

fail:
    event_del(ev_app->event);
    return -1;
}

void event_test_destory(struct ev_app_t* ev_app) { event_del(ev_app->event); }