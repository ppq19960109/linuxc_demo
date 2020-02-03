#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "generateQrBmp.h"
int main(int agrc, char *agrv[])
{
    printf("main start \n");
    create_qrcode("1s224dweFGF2331", "qrcode.bmp", 400, 4);
    int width = 350;
    char *data = malloc(width * width);
    char err = qrcode_create("ADF1s224dweFGF2331", width, 8, data);
    if (err != 0)
    {
        return -1;
    }
    err = saveQrcodeBmp("qr.bmp", data, width, width, 16);
    if (err != 0)
    {
        return -1;
    }
    printf("main end \n");
    return 0;
}