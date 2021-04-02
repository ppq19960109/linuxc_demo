#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
// #include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "enc_video.h"

void encode_video(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
                  int (*enc_cb)(AVPacket *pkt))
{
    int ret;

    /* send the frame to the encoder */
    if (frame)
        printf("encode Send frame %ld\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0)
    {
        fprintf(stderr, "Error sending a frame for encoding:%d,:%s\n", ret, av_err2str(ret));
        exit(1);
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        printf("Write packet %ld (size=%5d)\n", pkt->pts, pkt->size);

        enc_cb(pkt);
        av_packet_unref(pkt);
    }
}
//------------------------------
static AVCodec *enc_codec = NULL;
static AVCodecContext *enc_codec_ctx = NULL;
static FILE *enc_file = NULL;

static AVFrame *frame;
static AVPacket *pkt;

// static unsigned int enc_frame_pts = 0;

static int enc_cb(AVPacket *pkt)
{
    if (enc_file != NULL)
        fwrite(pkt->data, 1, pkt->size, enc_file);
    return 0;
}

int encode_video_run(AVFrame *enc_frame)
{
    if (av_frame_make_writable(frame) < 0)
    {
        fprintf(stderr, "enc frame not write\n");
        return -1;
    }
    // enc_frame->pts = enc_frame_pts++;
    /* encode the image */
    encode_video(enc_codec_ctx, enc_frame, pkt, enc_cb);
    return 0;
}

int encode_video_close()
{
    /* flush the encoder */
    encode_video(enc_codec_ctx, NULL, pkt, enc_cb);

    if (enc_file)
        fclose(enc_file);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    avcodec_free_context(&enc_codec_ctx);

    printf("ffmpeg encode end!!!!!!!\n");
    return 0;
}

int encode_video_open(const char *dst_filename, int width, int height, enum AVPixelFormat pix_fmt, enum AVCodecID codec_id)
{
    int ret = -1;
    printf("ffmpeg encode video start...\n");
    printf("encode video width:%d,height:%d,pix fmt:%d rtp_codec id:%d\n", width, height, pix_fmt, codec_id);
    //------------------------------
    enc_codec = avcodec_find_encoder(codec_id);
    if (!enc_codec)
    {
        fprintf(stderr, "Codec id not found\n");
        exit(1);
    }
    enc_codec_ctx = avcodec_alloc_context3(enc_codec);
    if (!enc_codec_ctx)
    {
        fprintf(stderr, "Could not allocate video enc_codec context\n");
        exit(1);
    }

    /* put sample parameters */
    enc_codec_ctx->bit_rate = 400000;
    /* resolution must be a multiple of two */
    enc_codec_ctx->width = width;
    enc_codec_ctx->height = height;
    /* frames per second */
    enc_codec_ctx->time_base = (AVRational){1, 25};
    enc_codec_ctx->framerate = (AVRational){25, 1};
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    enc_codec_ctx->gop_size = 10;
    enc_codec_ctx->max_b_frames = 1;
    enc_codec_ctx->pix_fmt = pix_fmt;

    if (enc_codec->id == AV_CODEC_ID_H264)
        av_opt_set(enc_codec_ctx->priv_data, "preset", "slow", 0);

    /* open it */
    ret = avcodec_open2(enc_codec_ctx, enc_codec, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open enc_codec: %s\n", av_err2str(ret));
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = enc_codec_ctx->pix_fmt;
    frame->width = enc_codec_ctx->width;
    frame->height = enc_codec_ctx->height;

    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0)
    {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }

    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    if (dst_filename != NULL)
    {
        enc_file = fopen(dst_filename, "wb");
        if (!enc_file)
        {
            fprintf(stderr, "Could not open %s\n", dst_filename);
            exit(1);
        }
    }

    return 0;
}
