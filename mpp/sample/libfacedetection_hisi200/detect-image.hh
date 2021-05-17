#ifndef _DETECT_IMAGE_HH
#define _DETECT_IMAGE_HH
#ifdef __cplusplus
extern "C"
{
#endif
    typedef int (*facedetection_callback)(char *);
    int yuv420_rgb(unsigned char *yuv, int width, int height);
    int facedetection(unsigned char *ptr, int width, int height);

#ifdef __cplusplus
}
#endif
#endif
