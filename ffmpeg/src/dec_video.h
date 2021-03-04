#ifndef _DEC_VIDEO_H
#define _DEC_VIDEO_H

#include <libavcodec/avcodec.h>

#include <libavformat/avformat.h>

#include <libswresample/swresample.h>

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/opt.h>
int open_codec_context(int *stream_idx,
                       AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
int decode_video_open(const char *src_filename, const char *dst_filename, int *width, int *height, enum AVPixelFormat *pix_fmt, enum AVCodecID *codec_id);
int decode_video_run(int (*cb)(AVFrame *frame));
int decode_video_close();
#endif