#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

int main(int agrc, char *agrv[]) {
    printf("main start \n");
    create_qrcode("1s224dweFGF2331","qrcode.bmp",400,4);

    return 0;
}