#include "http.h"
#include "commom.h"
#include "event2/buffer.h"
#include "event2/event.h"
#include "event2/event_struct.h"
#include "event2/http.h"

struct ev_http_t {
    pthread_t thread;

    struct event_base* base;
    struct event* event;
    struct evhttp* http;
    struct bufferevent* bepair[2];
};

struct ev_http_t ev_http;

void http_handler(struct evhttp_request* req, void* args) {
    struct evkeyvalq* req_val;
    struct evbuffer* evbuf;
    int evbuf_total;
    char* buf;
    const char* temp;
    cJSON* req_json;

    cJSON* rsp = cJSON_CreateObject();
    cJSON_AddStringToObject(rsp, "pwd", "hello");
    printf("rsp:%s\n", cJSON_Print(rsp));

    temp = evhttp_request_get_uri(req);
    printf("uri:%s\n", temp);
    req_val = evhttp_request_get_input_headers(req);
    temp = evhttp_find_header(req_val, "content-type");
    printf("head:%s\n", temp);
    temp = evhttp_find_header(req_val, "Content-Length");
    printf("head:%s\n", temp);

    evbuf = evhttp_request_get_input_buffer(req);
    evbuf_total = evbuffer_get_length(evbuf);
    buf = malloc(evbuf_total);

    evbuffer_remove(evbuf, buf, evbuf_total);
    printf("buffer:%s\n", buf);
    req_json = cJSON_Parse(buf);
    printf("json:%s\n", cJSON_Print(req_json));
    int has = cJSON_HasObjectItem(req_json, "name");
    printf("has:%d\n", has);
    cJSON* name = cJSON_GetObjectItem(req_json, "name");
    printf("name:%s\n", cJSON_Print(name));

    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type",
                      "application/json;charset=utf-8");
    evbuffer_add_printf(evbuf, "{\"retcode\"=%d}\n", 123);
    evhttp_send_reply(req, HTTP_OK, "OK", evbuf);
}

void http_init() {
    ev_http.base = event_base_new();
    ev_http.http = evhttp_new(ev_http.base);

    evhttp_bind_socket(ev_http.http, "0.0.0.0", 8080);

    evhttp_set_gencb(ev_http.http, http_handler, NULL);

    event_base_loop(ev_http.base, EVLOOP_NO_EXIT_ON_EMPTY);

    evhttp_free(ev_http.http);
    event_base_free(ev_http.base);
    printf("http exit\n");
}

void http_destroy() {
    event_base_loopbreak(ev_http.base);
    evhttp_free(ev_http.http);
    event_base_free(ev_http.base);
}