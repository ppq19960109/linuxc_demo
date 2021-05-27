#ifndef DEC_VIDEO_H
#define DEC_VIDEO_H
#ifdef __cplusplus
extern "C"
{
#endif
int dec_open(const char *src_filename,const char *video_dst_filename,int* w,int* h);
int dec_run(unsigned char*out_data);
void dec_close();
#ifdef __cplusplus
}
#endif
#endif // DEC_VIDEO_H

