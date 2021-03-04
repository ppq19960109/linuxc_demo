#ifndef _ENC_VIDEO_H
#define _ENC_VIDEO_H

#include <libavcodec/avcodec.h>

#include <libavformat/avformat.h>

#include <libswresample/swresample.h>

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/opt.h>
void encode_video(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
                  int (*enc_cb)(AVPacket *pkt));
int encode_video_open(const char *dst_filename, int width, int height, enum AVPixelFormat pix_fmt, enum AVCodecID codec_id);
int encode_video_run(AVFrame *enc_frame);
int encode_video_close();
#endif