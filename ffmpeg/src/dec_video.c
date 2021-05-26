#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>

static int refcount = 0;
int open_codec_context(int *stream_idx,
                       AVCodecContext **dec_ctx, AVFormatContext *dec_fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(dec_fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find %s stream in input file\n",
                av_get_media_type_string(type));
        return ret;
    }
    else
    {
        stream_index = ret;
        st = dec_fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec)
        {
            fprintf(stderr, "Failed to find %s rtp_codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Allocate a rtp_codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx)
        {
            fprintf(stderr, "Failed to allocate the %s rtp_codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy rtp_codec parameters from input stream to output rtp_codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
        {
            fprintf(stderr, "Failed to copy %s rtp_codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return ret;
        }

        /* Init the decoders, with or without reference counting */
        av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
        if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0)
        {
            fprintf(stderr, "Failed to open %s rtp_codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }
    return 0;
}

static int yuv_rgb(AVFrame *frame, int width, int height, enum AVPixelFormat pix_fmt)
{
    int ret;
    uint8_t *rgb_data;
    const enum AVPixelFormat dst_pix_fmt = AV_PIX_FMT_BGR24;
    int rgb_size = avpicture_get_size(dst_pix_fmt, width, height);
    rgb_data = av_malloc(rgb_size);
    if (!rgb_data)
    {
        exit(1);
    }
    AVFrame *frm_rgb = av_frame_alloc();
    avpicture_fill((AVPicture *)frm_rgb, rgb_data, dst_pix_fmt, width, height);
    struct SwsContext *sws = sws_getContext(width, height, pix_fmt, width, height, dst_pix_fmt,
                                            SWS_BILINEAR, NULL, NULL, NULL);
    ret = sws_scale(sws, frame->data, frame->linesize, 0, 0, frm_rgb->data, frm_rgb->linesize);
    printf("sws_scale return:%d\n", ret);

    sws_freeContext(sws);

    av_frame_free(&frm_rgb);
    av_free(rgb_data);
    return 0;
}
static void decode(AVCodecContext *dec_ctx, int stream_idx, AVFrame *frame, AVPacket *pkt,
                   FILE *dst_file)
{
    int ret, i;
    if (pkt->stream_index == stream_idx)
    {
        ret = avcodec_send_packet(dec_ctx, pkt);
        if (ret < 0)
        {
            fprintf(stderr, "Error sending a packet for decoding:%d,:%s\n", ret, av_err2str(ret));
            exit(1);
        }

        while (ret >= 0)
        {
            ret = avcodec_receive_frame(dec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return;
            else if (ret < 0)
            {
                fprintf(stderr, "Error during decoding\n");
                exit(1);
            }
            printf("saving frame %d\n", dec_ctx->frame_number);

            printf("frame width:%d,height:%d,pix fmt:%d\n", frame->width, frame->height, dec_ctx->pix_fmt);

            if (dec_ctx->pix_fmt == AV_PIX_FMT_YUV420P || dec_ctx->pix_fmt == AV_PIX_FMT_YUVJ420P)
            {
                if (dst_file != NULL)
                {
                    // fwrite(frame->data[0], (frame->linesize[0]) * (frame->height), 1, dst_file);
                    // fwrite(frame->data[1], (frame->linesize[1]) * (frame->height) / 4, 1, dst_file);
                    // fwrite(frame->data[2], (frame->linesize[2]) * (frame->height) / 4, 1, dst_file);

                    for (i = 0; i < frame->height; ++i)
                    {
                        fwrite(frame->data[0] + i * frame->linesize[0], frame->width, 1, dst_file);
                    }
                    for (i = 0; i < frame->height / 2; ++i)
                    {
                        fwrite(frame->data[1] + i * frame->linesize[1], frame->width / 2, 1, dst_file);
                    }
                    for (i = 0; i < frame->height / 2; ++i)
                    {
                        fwrite(frame->data[2] + i * frame->linesize[2], frame->width / 2, 1, dst_file);
                    }
                }
            }

            if (refcount)
                av_frame_unref(frame);
        }
    }
}
//------------------------------
static AVFormatContext *dec_fmt_ctx = NULL;
static AVCodecContext *dec_ctx = NULL;
static AVStream *dec_video_stream = NULL;
static int dec_video_stream_idx = -1;

static AVFrame *frame = NULL;
static AVPacket pkt;

int main(int argc, char **argv)
{
    int width, height;
    enum AVPixelFormat pix_fmt;
    enum AVCodecID codec_id;

    FILE *video_dst_file = NULL;

    printf("%s start\n", __FILE__);

    const char *src_filename = argv[1];
    const char *video_dst_filename = argv[2];
    avformat_network_init();
    /* open input file, and allocate format context */
    if (avformat_open_input(&dec_fmt_ctx, src_filename, NULL, NULL) < 0)
    {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(dec_fmt_ctx, NULL) < 0)
    {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

    if (open_codec_context(&dec_video_stream_idx, &dec_ctx, dec_fmt_ctx, AVMEDIA_TYPE_VIDEO) < 0)
    {
        fprintf(stderr, "open_codec_context error\n");
        exit(1);
    }
    dec_video_stream = dec_fmt_ctx->streams[dec_video_stream_idx];
    if (!dec_video_stream)
    {
        fprintf(stderr, "Could not find video stream in the input, aborting\n");
        exit(1);
    }

    printf("dec video width:%d,height:%d,pix fmt:%d,codec_id:%d\n", dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt, dec_video_stream->codecpar->codec_id);
    printf("framerate den:%d,num:%d\n", dec_ctx->framerate.den, dec_ctx->framerate.num);
    printf("time_base den:%d,num:%d\n", dec_ctx->time_base.den, dec_ctx->time_base.num);

    width = dec_ctx->width;
    height = dec_ctx->height;
    pix_fmt = dec_ctx->pix_fmt;
    codec_id = dec_video_stream->codecpar->codec_id;
    /* dump input information to stderr */
    av_dump_format(dec_fmt_ctx, 0, src_filename, 0);

    frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate frame\n");
        // ret = AVERROR(ENOMEM);
        exit(1);
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    if (video_dst_filename != NULL)
    {
        video_dst_file = fopen(video_dst_filename, "wb+");
        if (!video_dst_file)
        {
            fprintf(stderr, "Could not open destination file %s\n", video_dst_filename);
            exit(1);
        }
    }

    while (av_read_frame(dec_fmt_ctx, &pkt) >= 0)
    {
        AVPacket orig_pkt = pkt;
        decode(dec_ctx, dec_video_stream_idx, frame, &pkt, video_dst_file);
        av_packet_unref(&orig_pkt);
    }

    if (video_dst_file)
        fclose(video_dst_file);

    av_frame_free(&frame);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&dec_fmt_ctx);

    printf("%s end\n", __FILE__);
    return 0;
}
