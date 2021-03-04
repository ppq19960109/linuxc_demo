#ifndef _PUSH_STREAM_H
#define _PUSH_STREAM_H

#include <libavcodec/avcodec.h>

#include <libavformat/avformat.h>

#include <libswresample/swresample.h>

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>

int push_stream_open(const char *in_filename, const char *out_url);
#endif