#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/opt.h>

void encode_video(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile)
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

        printf("Write packet %3" PRId64 " (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}
//------------------------------
static AVCodec *enc_codec = NULL;
static AVCodecContext *enc_codec_ctx = NULL;
static FILE *enc_file = NULL;

static AVFrame *frame;
static AVPacket *pkt;

int main(int argc, char **argv)
{
    int ret = -1;
    printf("%s start\n", __FILE__);

    const char *src_filename = argv[1];
    const char *dst_filename = argv[2];
    const char *codec_name = argv[3];

    int width = 2304;
    int height = 1296;
    enum AVPixelFormat pix_fmt = AV_PIX_FMT_YUVJ420P; // AV_PIX_FMT_YUV420P;
    printf("encode video width:%d,height:%d,pix fmt:%d\n", width, height, pix_fmt);

    // enc_codec = avcodec_find_encoder(codec_id);
    // if (!enc_codec)
    // {
    //     fprintf(stderr, "Codec id not found\n");
    //     exit(1);
    // }
    enc_codec = avcodec_find_encoder_by_name(codec_name);
    if (!enc_codec)
    {
        fprintf(stderr, "Codec '%s' not found\n", codec_name);
        exit(1);
    }
    printf("codec id:%d\n", enc_codec->id);
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
    FILE *src_file = fopen(src_filename, "rb");
    if (!src_file)
    {
        fprintf(stderr, "Could not open %s\n", src_filename);
        exit(1);
    }
    int enc_frame_pts = 0;
    while (1)
    {
        if (av_frame_make_writable(frame) < 0)
        {
            fprintf(stderr, "enc frame not write\n");
            return -1;
        }

        if ((ret = fread(frame->data[0], width * height, 1, src_file)) < 1)
        {
            printf("fread end:%d\n", ret);
            break;
        }
        if (fread(frame->data[1], width * height / 4, 1, src_file) < 1)
        {
            break;
        }
        if (fread(frame->data[2], width * height / 4, 1, src_file) < 1)
        {
            break;
        }

        frame->pts = enc_frame_pts++;
        /* encode the image */
        encode_video(enc_codec_ctx, frame, pkt, enc_file);
    }
    /* flush the encoder */
    encode_video(enc_codec_ctx, NULL, pkt, enc_file);

    fclose(src_file);
    if (enc_file)
        fclose(enc_file);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&enc_codec_ctx);
    printf("%s end\n", __FILE__);
    return 0;
}
