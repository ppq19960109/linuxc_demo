#pragma once

namespace QrBmpUtils
{
    int saveBitmap(const char *bmpName, void *bmpData, int width, int height, int bodySize, const unsigned char bmpBitCount);
    int saveQrcodeBmp(const char *qrcodeName, void *bmpData, int width, int height, const unsigned char bmpBitCount);
};
