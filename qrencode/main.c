#include <stdio.h>
#include "qrcode.h"

int main(int agrc, char *agrv[])
{
    printf("main start \n");

    qrencodeString("http://club.marssenger.com/hxr/download.html?pk=a1n3oZED0Y8&dn=X5B-ChengWei-01");

    printf("main end\n");
    return 0;
}
