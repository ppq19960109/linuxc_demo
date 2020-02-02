#include <iostream>
#include <cstring>

#include "generate_bmp.h"

using namespace std;
// #pragma pack(1)

//下面两个结构是位图的结构
typedef struct tagBITMAPFILEHEADER
{
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
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

int BmpUtils::saveBitmap(const char *bmpName, void *bmpData, int width, int height, int bodySize, const unsigned char bmpBitCount)
{
    FILE *pf = fopen(bmpName, "wb");

    if (NULL == pf)
    {
        cout << bmpName << " file open fail." << endl;
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
    if (fwrite(&bitMapFileHeader, sizeof(BITMAPFILEHEADER), 1, pf) != 1)
    {
        cout << "fwrite bitMapFileHeader fail." << endl;
        goto fail;
    }
    //写位图信息头进文件
    if (fwrite(&bitMapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pf) != 1)
    {
        cout << "fwrite bitMapInfoHeader fail." << endl;
        goto fail;
    }

    //写数据进文件
    if (fwrite(bmpData, bodySize, 1, pf) != 1)
    {
        cout << "fwrite bmpData fail." << endl;
        goto fail;
    }

    fclose(pf);

    return 0;
fail:
    fclose(pf);
    return -1;
}

int BmpUtils::saveQrcode(const char *qrcodeName, unsigned char *bmpData, int width, int height, const unsigned char bmpBitCount)
{

    int widthByte = (width * bmpBitCount / 8 + 3) / 4 * 4; //每line字节数必须为4的倍数
    int bodySize = widthByte * height;
    unsigned char bmpByteCount = bmpBitCount / 8;

    unsigned char *qrBmpData = new unsigned char[bodySize];
    memset(qrBmpData, 255, bodySize);
    unsigned char *qrData = bmpData;
    int i, j, k;
    for (i = height - 1; i >= 0; i--)
    {
        for (j = 0; j < width; j++)
        {
            if (*(qrData) == 0)
            {
                //设置rgb颜色，可自定义设置，这里设为黑色。
                for (k = 0; k < bmpByteCount; ++k)
                {
                    *(qrBmpData + widthByte * i + bmpByteCount * j + k) = 0;
                }
            }
            qrData++;
        }
    }

    saveBitmap(qrcodeName, qrBmpData, width, height, bodySize, bmpBitCount);

    delete[] qrBmpData;

    return 0;
}