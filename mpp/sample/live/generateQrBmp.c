#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "generateQrBmp.h"

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

static int saveBitmap(const char *bmpName, void *bmpData, int width, int height, int bodySize, const unsigned char bmpBitCount)
{
    FILE *pf = fopen(bmpName, "wb");

    if (NULL == pf)
    {
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
    if (fwrite(&bitMapFileHeader, sizeof(BITMAPFILEHEADER), 1, pf) != 1)
    {
        printf("fwrite bitMapFileHeader fail.\n");
        goto fail;
    }
    //写位图信息头进文件
    if (fwrite(&bitMapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pf) != 1)
    {
        printf("fwrite bitMapInfoHeader fail.\n");
        goto fail;
    }

    //写数据进文件
    if (fwrite(bmpData, bodySize, 1, pf) != 1)
    {
        printf("fwrite bmpData fail.\n");
        goto fail;
    }

    fclose(pf);

    return 0;
fail:
    fclose(pf);
    return -1;
}

int saveQrcodeBmp(const char *bmpName, void *bmpData, int width, int height, const unsigned char bmpBitCount)
{
    int widthByte = (width * bmpBitCount / 8 + 3) / 4 * 4; //每line字节数必须为4的倍数
    int bodySize = widthByte * height;
    unsigned char bmpByteCount = bmpBitCount / 8;

    unsigned char *dstBmpData = malloc(bodySize);
    if (dstBmpData == NULL || bmpData == NULL)
    {
        return -1;
    }
    // memset(dstBmpData, 255, bodySize);

    unsigned char *srcBmpData = bmpData;
    int i, j;
    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; j += 3)
        {
            dstBmpData[i * widthByte + j + 0] = srcBmpData[i * widthByte + j + 0];
            dstBmpData[i * widthByte + j + 1] = srcBmpData[i * widthByte + j + 1];
            dstBmpData[i * widthByte + j + 2] = srcBmpData[i * widthByte + j + 2];
        }
    }
    unsigned char error = saveBitmap(bmpName, dstBmpData, width, height, bodySize, bmpBitCount);
    if (error != 0)
    {
        return -1;
    }

    free(dstBmpData);

    return 0;
}

#define YUV420SP_NV12 0
#define YUV420SP_NV21 0
void YUV420SPToRGBAByte(unsigned char *src, unsigned char *dst, int width, int height, int format)
{
    if (format == YUV420SP_NV12 || format == YUV420SP_NV21)
    {
        int pos = 0;
        unsigned char *pRGBA = dst;
        unsigned char *pY = src;
        unsigned char *pUV = src + width * height;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int uvOffset = (y >> 1) * width + 2 * (x >> 1);
                int uOffset;
                int vOffset;
                if (format == YUV420SP_NV12)
                {
                    uOffset = uvOffset;
                    vOffset = uvOffset + 1;
                }
                else
                {
                    uOffset = uvOffset + 1;
                    vOffset = uvOffset;
                }
                // rgbaIntToBytes(YUV2RGBA(*pY++, pUV[uOffset], pUV[vOffset]), pRGBA);
                unsigned char Y = *pY++;
                unsigned char U = pUV[uOffset];
                unsigned char V = pUV[vOffset];
                pRGBA[pos++] = Y + 1.779 * (U - 128);
                pRGBA[pos++] = Y - 0.3455 * (U - 128) - 0.7169 * (V - 128);
                pRGBA[pos++] = Y + 1.4075 * (V - 128);
            }
        }
    }
}