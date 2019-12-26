#include "http.h"
#include "commom.h"
#include "event2/buffer.h"
#include "event2/event.h"
#include "event2/event_struct.h"
#include "event2/http.h"

#define NORMAL_COLOR  "\033[0m"
#define YELLOW "\033[1;33m"

#define log_color(color,fmt,...)  log_printf(color "%s-[%s-%d]: " fmt NORMAL_COLOR,__FUNCTION__,__FILE__,__LINE__,##__VA_ARGS__)
#define log_debug(fmt,...)  log_color(YELLOW,fmt,##__VA_ARGS__)


void log_printf(char *format,...)
{
   va_list args;
   
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
}

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

    temp = evhttp_request_get_uri(req);
    log_debug("uri:%s",temp);

    req_val = evhttp_request_get_input_headers(req);
    temp = evhttp_find_header(req_val, "content-type");
    printf("content-type:%s\n", temp);
    temp = evhttp_find_header(req_val, "Content-Length");
    printf("Content-Length:%s\n", temp);

    evbuf = evhttp_request_get_input_buffer(req);
    evbuf_total = evbuffer_get_length(evbuf);
    if (evbuf_total == 0) {
        goto fail;
    }
    buf = malloc(evbuf_total);

    evbuffer_remove(evbuf, buf, evbuf_total);
    printf("body:%s\n", buf);
    req_json = cJSON_Parse(buf);
    printf("body json:%s\n", cJSON_Print(req_json));

    cJSON* rsp = cJSON_CreateObject();
    cJSON_AddNumberToObject(rsp, "retcode", 12);
    char* rspbuf = cJSON_Print(rsp);
    printf("rsp:%s\n", rspbuf);

    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type",
                      "application/json;charset=UTF-8");
    evbuffer_add(evbuf, rspbuf, strlen(rspbuf));
    evhttp_send_reply(req, HTTP_OK, "OK", evbuf);

fail:
    evhttp_send_reply(req, HTTP_NOCONTENT, "NOCONTENT", NULL);
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