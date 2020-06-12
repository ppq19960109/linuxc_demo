#include <stdio.h>
#include "qrcode.h"

int main(int agrc, char *agrv[])
{
    printf("main start \n");

    qrencodeString("hello123");

    printf("main end\n");
    return 0;
}
