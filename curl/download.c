#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#include <curl/curl.h>

int (*download_cb)(char *, int);
void register_download_cb(int (*cb)(char *, int))
{
    download_cb = cb;
}

int http_progress_cb(void *clientp,
                     double dltotal,
                     double dlnow,
                     double ultotal,
                     double ulnow)
{
    printf("dltotal:%f,dlnow:%f,ultotal:%f,ulnow:%f\n", dltotal, dlnow, ultotal, ulnow);
    return 0;
}
size_t http_download_cb(void *ptr, size_t size, size_t nmemb, void *stream)
{
    printf("size:%ld,nmemb:%ld\n", size, nmemb);
    if (download_cb != NULL)
        download_cb(ptr, size * nmemb);
    return size * nmemb;
}
int curl_http_download(const char *url)
{
    CURLcode res;
    CURL *curl = curl_easy_init();
    if (curl)
    {
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        // set the URL with GET request
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // write response msg into strResponse
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_download_cb);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, http_progress_cb);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
        // curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        // perform the request, res will get the return code
        res = curl_easy_perform(curl);
        // check for errors
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else
        {
            fprintf(stderr, "curl_easy_perform() success.\n");
        }

        curl_easy_cleanup(curl);
    }

    return 0;
}
static void *http_download_thread_proc(void *para)
{
    curl_http_download((const char *)para);
    return NULL;
}
void curl_http_download_thread(const char *url)
{
    pthread_t tid;
    pthread_create(&tid, NULL, http_download_thread_proc, (void *)url);
    pthread_detach(tid);
}

int download_init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    return 0;
}

void download_deinit()
{
    curl_global_cleanup();
}