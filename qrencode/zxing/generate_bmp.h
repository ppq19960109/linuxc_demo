#pragma once

namespace BmpUtils
{
    int saveBitmap(const char *bmpName, void *bmpData, int width, int height, int bodySize, const unsigned char bmpBitCount);
    int saveQrcode(const char *qrcodeName, unsigned char *bmpData, int width, int height, const unsigned char bmpBitCount);
};
