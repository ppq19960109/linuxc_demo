#include <libavcodec/avcodec.h>

#include <libavformat/avformat.h>

#include <libswresample/swresample.h>

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

static AVFormatContext *ofmt_ctx = NULL;

void push_stream_hisi_close()
{
    // 3.5 写输出文件尾
    av_write_trailer(ofmt_ctx);

    /* close output */
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);
}

int push_stream_hisi_write(AVPacket *pkt)
{
    printf("pts:%ld,dts:%ld\n",pkt->pts,pkt->dts);
    av_usleep((int64_t)(1.0 / 25.0 * AV_TIME_BASE));
    pkt->pos = -1;
    // 3.4 将packet写入输出
    int ret = av_interleaved_write_frame(ofmt_ctx, pkt);
    if (ret < 0)
    {
        printf("Error av_interleaved_write_frame packet\n");
    }
    return ret;
}

int push_stream_hisi_write2(unsigned char *data, int len)
{
    av_usleep((int64_t)(1.0 / 25.0 * AV_TIME_BASE));
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = data;
    pkt.size = len;
    pkt.pos = -1;
    // 3.4 将packet写入输出
    int ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
    if (ret < 0)
    {
        printf("Error av_interleaved_write_frame packet\n");
    }
    return ret;
}

int push_stream_hisi_open(const char *out_url)
{
    printf("out_url:%s\n", out_url);
    int ret;

    // 2. 打开输出
    // 2.1 分配输出ctx
    char *ofmt_name = NULL;
    if (strstr(out_url, "rtmp://") != NULL)
    {
        ofmt_name = "flv";
    }
    else if (strstr(out_url, "udp://") != NULL)
    {
        ofmt_name = "mpegts";
    }
    else if (strstr(out_url, "rtp://") != NULL)
    {
        ofmt_name = "rtp";
    }
    else if (strstr(out_url, "rtsp://") != NULL)
    {
        ofmt_name = "rtsp";
    }
    else
    {
        goto end;
    }

    avformat_alloc_output_context2(&ofmt_ctx, NULL, ofmt_name, out_url);
    if (!ofmt_ctx)
    {
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    AVStream *out_stream;

    // 2.2 将一个新流(out_stream)添加到输出文件(ofmt_ctx)
    out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if (!out_stream)
    {
        printf("Failed allocating output stream\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    // out_stream->codecpar->bit_rate = 0;
    out_stream->codecpar->codec_id = AV_CODEC_ID_H264;
    out_stream->codecpar->width = 2304;
    out_stream->codecpar->height = 1296;
    out_stream->codecpar->format = AV_PIX_FMT_YUV420P;
    out_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;

    out_stream->codecpar->codec_tag = 0;

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        // TODO: 研究AVFMT_NOFILE标志
        // 2.4 创建并初始化一个AVIOContext，用以访问URL(out_url)指定的资源
        ret = avio_open(&ofmt_ctx->pb, out_url, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            printf("Could not open output file '%s',%s\n", out_url, av_err2str(ret));
            goto end;
        }
    }
    else
    {
        printf("%s not avio_open\n", out_url);
    }

    // 3. 数据处理
    // 3.1 写输出文件头
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0)
    {
        printf("Error occurred when opening output file,err2str:%s\n", av_err2str(ret));
        goto end;
    }
    av_dump_format(ofmt_ctx, 0, out_url, 1);

    return 0;
end:
    push_stream_hisi_close();
    return -1;
}

int main(int argc, char **argv)
{
    printf("ffmpeg push stream start...\n");
    if (argc <= 2)
    {
        fprintf(stderr, "Usage: %s <input file> <output url>\n", argv[0]);
        exit(0);
    }
    push_stream_hisi_open(argv[2]);
    int ret;
    const char *in_filename = argv[1];
    AVFormatContext *ifmt_ctx = NULL;

    // 1. 打开输入
    // 1.1 读取文件头，获取封装格式相关信息
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0)
    {
        printf("Could not open input file '%s'", in_filename);
        exit(1);
    }
    // 1.2 解码一段数据，获取流相关信息
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
    {
        printf("Failed to retrieve input stream information");
        exit(1);
    }
    av_dump_format(ifmt_ctx, 0, in_filename, 0);
    AVPacket pkt;
    int frame = 0;
    while (1)
    {
        // 3.2 从输出流读取一个packet
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
        {
            break;
        }

        push_stream_hisi_write(&pkt);
        printf("write frame:%d\n", frame++);
        av_packet_unref(&pkt);
    }
    avformat_close_input(&ifmt_ctx);
    push_stream_hisi_close();
    printf("ffmpeg push stream end...\n");
    return 0;
}
