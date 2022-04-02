#ifndef _DOWNLOAD_H_
#define _DOWNLOAD_H_

int curl_http_download(const char *url);
void curl_http_download_thread(const char *url);
int download_init();
void download_deinit();
void register_download_cb(int (*cb)(char *, int));
#endif