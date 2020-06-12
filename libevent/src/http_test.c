#include "event_app.h"
#include "http_test.h"
#include "commom.h"

void http_handler(struct evhttp_request* req, void* args) {
    struct ev_app_t* ev_app = (struct ev_app_t*)args;

    const char* temp = evhttp_request_get_uri(req);
    log_debug("uri:%s\n", temp);
    // head
    struct evkeyvalq* req_val = evhttp_request_get_input_headers(req);
    temp = evhttp_find_header(req_val, "content-type");
    log_debug("content-type:%s\n", temp);


    // bufffer
    struct evbuffer* evbuf = evhttp_request_get_input_buffer(req);
    int evbuf_total = evbuffer_get_length(evbuf);
    if (evbuf_total == 0) {
        goto fail;
    }
    char* buf = malloc(evbuf_total);

    evbuffer_remove(evbuf, buf, evbuf_total);
    log_debug("body:%s\n", buf);
    cJSON* req_json = cJSON_Parse(buf);
    log_debug("body json:%s\n", cJSON_Print(req_json));

    // buffer
    bufferevent_write(ev_app->bepair[0], buf, evbuf_total);
    free(buf);

    // response
    cJSON* rsp = cJSON_CreateObject();
    cJSON_AddNumberToObject(rsp, "retcode", 12);
    char* rspbuf = cJSON_Print(rsp);
    log_debug("rsp:%s\n", rspbuf);

    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type",
                      "application/json;charset=UTF-8");
    evbuffer_add(evbuf, rspbuf, strlen(rspbuf));
    evhttp_send_reply(req, HTTP_OK, "OK", evbuf);

fail:
    evhttp_send_reply(req, HTTP_NOCONTENT, "NOCONTENT", NULL);
}

int http_init(struct ev_app_t* ev_app) {
    int rc = -1;
    ev_app->http = evhttp_new(ev_app->base);

    rc = evhttp_bind_socket(ev_app->http, "0.0.0.0", 8080);
    if (0 != rc) {
        goto fail;
    }
    evhttp_set_gencb(ev_app->http, http_handler, ev_app);

    return 0;
fail:
    evhttp_free(ev_app->http);
    return -1;
}

void http_destroy(struct ev_app_t* ev_app) {
    evhttp_free(ev_app->http);
    event_base_free(ev_app->base);
}