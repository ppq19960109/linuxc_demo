#ifndef _HTTP_TEST_H_
#define _HTTP_TEST_H_

int http_init(struct ev_app_t* ev_app);
void http_destroy(struct ev_app_t* ev_app);
#endif