#ifndef _GENERATE_BITMAP_H
#define _GENERATE_BITMAP_H
#ifdef __cplusplus
extern "C"
{
#endif
enum YUV420_FORMAT
{
    YUV420SP_NV12,
    YUV420SP_NV21
};
void YUV420SPToRGBByte(unsigned char *src, unsigned char *dst, int width, int height,enum YUV420_FORMAT format);
int save_rgb_bmp(const char *bmpName, void *bmpData, int width, int height, const unsigned char bmpBitCount);
#ifdef __cplusplus
}
#endif
#endif