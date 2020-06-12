#ifndef GENERATE_QRCODE_H
#define GENERATE_QRCODE_H

#ifdef __cplusplus
extern "C"
{
#endif
    int create_qrcode(char* text,char* filePath,int w,int ecc);
    int qrcode_create(char *text, int w, int ecc, char *data);
#ifdef __cplusplus
}
#endif

#endif