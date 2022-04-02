#include <stdio.h>

#include "download.h"
//https://dds-ack.dui.ai/runtime/v1/longtext/3ed848ddf3df43c1a0db137053ee5966?productId=279608534
int main(int agrc, char *agrv[])
{
    printf("main start \n");
    if (agrc <= 1)
        return -1;
    printf("main agrv:%s\n", agrv[1]);
    download_init();
    curl_http_download(agrv[1]);
    getchar();
    download_deinit();
    printf("main end\n");
    return 0;
}
