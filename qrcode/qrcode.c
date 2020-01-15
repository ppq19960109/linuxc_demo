#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <qrencode.h>
#include "qrcode.h"

// #pragma pack(1)

//下面两个结构是位图的结构
typedef struct tagBITMAPFILEHEADER {
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    unsigned int biSize;
    unsigned int biWidth;
    unsigned int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    unsigned int biXPelsPerMeter;
    unsigned int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} __attribute__((packed)) BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    unsigned char rgbBlue;      //蓝色的亮度（值范围为0-255)
    unsigned char rgbGreen;     //绿色的亮度（值范围为0-255)
    unsigned char rgbRed;       //红色的亮度（值范围为0-255)
    unsigned char rgbReserved;  //保留，必须为0
} __attribute__((packed)) RgbQuad;

int saveQrcode(const char* qrcodeName, QRcode* qrCode, const unsigned char bmpBitCount);
int rgb888to565(const char* srcBmpName,const char* dstBmpName,const unsigned char dstBitCount);
int resizeBmp(const char* srcBmpName,const char* dstBmpName,const unsigned char multiple);

int qrcode_test_init() {
    qrencodeString("hello123");

    return 0;
}

int qrencodeString(char* QRTEXT) {
    QRcode* qrCode = NULL;
    int version = 10;  //设置版本号，这里设为5，对应尺寸：37 * 37
    QRecLevel level = QR_ECLEVEL_H;
    QRencodeMode hint = QR_MODE_8;
    int casesensitive = 1;
    qrCode = QRcode_encodeString(QRTEXT, version, level, hint, casesensitive);
    if (NULL == qrCode) {
        printf("QRcode create fail\n");
        return -1;
    }

    printf("QRcode create success\n");

    saveQrcode("./qrcode16.bmp", qrCode, 16);
    saveQrcode("qrcode24.bmp", qrCode, 24);
    free(qrCode);
    resizeBmp("qrcode16.bmp", "qrcode16bit.bmp", 4);
    rgb888to565("qrcode24.bmp", "qrcode565.bmp", 16);
    return 0;
}

void* readBitmap(const char* bmpName, BITMAPFILEHEADER* bmpHeader, BITMAPINFOHEADER* bmpInfoHeader, int* bodySize) {
    FILE* fd = fopen(bmpName, "rb");
    if (NULL == fd) {
        printf("file open fail.\n");
        fclose(fd);
        return NULL;
    }

    if (fread(bmpHeader, 1, sizeof(BITMAPFILEHEADER), fd) != sizeof(BITMAPFILEHEADER)) {
        printf("fread fail.\n");
        return NULL;
    }

    if (fread(bmpInfoHeader, 1, sizeof(BITMAPINFOHEADER), fd) != sizeof(BITMAPINFOHEADER)) {
        printf("fread fail.\n");
        return NULL;
    }

    *bodySize = bmpHeader->bfSize - 54;
    void* body = malloc(*bodySize);

    fseek(fd, 54, SEEK_SET);
    if (fread(body, 1, *bodySize, fd) != *bodySize) {
        printf("fread fail.\n");
        return NULL;
    }

    fclose(fd);
    return body;
}

int saveBitmap(const char* bmpName, void* bmpData, int width, int height, int bodySize, const unsigned char bmpBitCount) {
    FILE* pf = fopen(bmpName, "wb");

    if (NULL == pf) {
        printf("%s file open fail.\n", bmpName);
        goto fail;
    }

    //位图文件头
    BITMAPFILEHEADER bitMapFileHeader;

    bitMapFileHeader.bfType = 0x4D42;
    bitMapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bodySize;
    bitMapFileHeader.bfReserved1 = 0;
    bitMapFileHeader.bfReserved2 = 0;
    bitMapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    //位图信息头
    BITMAPINFOHEADER bitMapInfoHeader;

    bitMapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitMapInfoHeader.biWidth = width;
    bitMapInfoHeader.biHeight = height;
    bitMapInfoHeader.biPlanes = 1;
    bitMapInfoHeader.biBitCount = bmpBitCount;
    bitMapInfoHeader.biCompression = 0;
    bitMapInfoHeader.biSizeImage = bodySize;
    bitMapInfoHeader.biXPelsPerMeter = 0;
    bitMapInfoHeader.biYPelsPerMeter = 0;
    bitMapInfoHeader.biClrUsed = 0;
    bitMapInfoHeader.biClrImportant = 0;

    //写文件头进文件
    if (fwrite(&bitMapFileHeader, sizeof(BITMAPFILEHEADER), 1, pf) != 1) {
        printf("fwrite bitMapFileHeader fail.\n");
        goto fail;
    }
    //写位图信息头进文件
    if (fwrite(&bitMapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pf) != 1) {
        printf("fwrite bitMapInfoHeader fail.\n");
        goto fail;
    }

    //写数据进文件
    if (fwrite(bmpData, bodySize, 1, pf) != 1) {
        printf("fwrite bmpData fail.\n");
        goto fail;
    }

    fclose(pf);

    return 0;
fail:
    fclose(pf);
    return -1;
}

int saveQrcode(const char* qrcodeName, QRcode* qrCode, const unsigned char bmpBitCount) {
    int width = qrCode->width;
    int height = qrCode->width;
    int widthByte = (width * bmpBitCount / 8 + 3) / 4 * 4;  //每line字节数必须为4的倍数
    int bodySize = widthByte * height;
    unsigned char bmpByteCount = bmpBitCount / 8;

    unsigned char* qrBmpData = malloc(bodySize);
    memset(qrBmpData, 255, bodySize);
    unsigned char* qrData = qrCode->data;
    int i, j, k;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (*(qrData)&1) {
                //设置rgb颜色，可自定义设置，这里设为黑色。
                for (k = 0; k < bmpByteCount; ++k) {
                    *(qrBmpData + widthByte * i + bmpByteCount * j + k) = 0;
                }
            }
            qrData++;
        }
    }

    saveBitmap(qrcodeName, qrBmpData, width, height, bodySize, bmpBitCount);
    free(qrBmpData);

    return 0;
}

unsigned short RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue) {
    // unsigned short B = (blue >> 3) & 0x001F;
    // unsigned short G = ((green >> 2) << 5) & 0x07E0;
    // unsigned short R = ((red >> 3) << 11) & 0xF800;
    unsigned short B = ((blue >> 3) & 0x1F) << 0;
    unsigned short G = ((green >> 2) & 0x3F) << 5;
    unsigned short R = ((red >> 3) & 0x1F) << 11;
    return (unsigned short)(R | G | B);
}

int rgb888to565(const char* srcBmpName,const char* dstBmpName,const unsigned char dstBitCount) {
    BITMAPFILEHEADER srcBmpHeader;
    BITMAPINFOHEADER srcBmpInfoHeader;

    unsigned char* srcBmpData = NULL;
    int srcBodySize = 0;
    srcBmpData = readBitmap(srcBmpName, &srcBmpHeader, &srcBmpInfoHeader, &srcBodySize);
    if (srcBmpData == NULL) {
        printf("readBitmap fail\n");
        return -1;
    }
    int srcWidthByte = (srcBmpInfoHeader.biWidth * srcBmpInfoHeader.biBitCount / 8 + 3) / 4 * 4;
    unsigned char srcBmpByteCount = srcBmpInfoHeader.biBitCount / 8;
    //  ----------------------------------
    int width = srcBmpInfoHeader.biWidth;
    int height = srcBmpInfoHeader.biHeight;
    int widthByte = (width * dstBitCount / 8 + 3) / 4 * 4;  //每line字节数必须为4的倍数
    int bodySize = widthByte * height;
    unsigned char bmpByteCount = dstBitCount / 8;

    unsigned char* dstBmpData = malloc(bodySize);

    int i, j;
    unsigned short color;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            color = RGB888toRGB565(srcBmpData[i * srcWidthByte + j * srcBmpByteCount + 2], srcBmpData[i * srcWidthByte + j * srcBmpByteCount + 1],
                                   srcBmpData[i * srcWidthByte + j * srcBmpByteCount]);
            dstBmpData[i * widthByte + bmpByteCount * j] = color;
            dstBmpData[i * widthByte + bmpByteCount * j + 1] = color >> 8;
        }
    }
    saveBitmap(dstBmpName, dstBmpData, width, height, bodySize, dstBitCount);
    free(dstBmpData);
    free(srcBmpData);
    return 0;
}

int resizeBmp(const char* srcBmpName,const char* dstBmpName,const unsigned char multiple) {
    BITMAPFILEHEADER srcBmpHeader;
    BITMAPINFOHEADER srcBmpInfoHeader;

    unsigned char* srcBmpData = NULL;
    int srcBodySize = 0;
    srcBmpData = readBitmap(srcBmpName, &srcBmpHeader, &srcBmpInfoHeader, &srcBodySize);
    if (srcBmpData == NULL) {
        printf("readBitmap fail\n");
        return -1;
    }
    unsigned char bmpByteCount = srcBmpInfoHeader.biBitCount / 8;
    int srcWidthByte = (srcBmpInfoHeader.biWidth * bmpByteCount + 3) / 4 * 4;

    //  ----------------------------------
    int width = srcBmpInfoHeader.biWidth * multiple;
    int height = srcBmpInfoHeader.biHeight * multiple;
    int dstWidthSize = (width * srcBmpInfoHeader.biBitCount / 8 + 3) / 4 * 4;
    int dstBodySize = dstWidthSize * height;

    unsigned char* dstBmpData = malloc(dstBodySize);
    memset(dstBmpData, 0, dstBodySize);

    double rateH = (double)1 / multiple;
    double rateW = (double)1 / multiple;

    int tSrcH, tSrcW;
    int i, j;

    for (i = 0; i < height; i++) {
        tSrcH = (int)(rateH * i);
        for (j = 0; j < width; j++) {
            tSrcW = (int)(rateW * j);
            memcpy(&dstBmpData[i * dstWidthSize] + j * bmpByteCount, &srcBmpData[tSrcH * srcWidthByte] + tSrcW * bmpByteCount, bmpByteCount);
        }
    }

    saveBitmap(dstBmpName, dstBmpData, width, height, dstBodySize, srcBmpInfoHeader.biBitCount);
    free(dstBmpData);
    free(srcBmpData);
    return 0;
}
