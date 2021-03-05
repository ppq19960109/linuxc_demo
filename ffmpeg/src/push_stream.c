#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#include "push_stream.h"

int push_stream_open(const char *in_filename, const char *out_url)
{
    printf("in_filename:%s,out_url:%s\n", in_filename, out_url);
    int ret, i;

    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;

    // 1. 打开输入
    // 1.1 读取文件头，获取封装格式相关信息
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0)
    {
        printf("Could not open input file '%s'", in_filename);
        goto end;
    }
    // 1.2 解码一段数据，获取流相关信息
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
    {
        printf("Failed to retrieve input stream information");
        goto end;
    }
    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    // 2. 打开输出
    // 2.1 分配输出ctx
    int rtp_stream = 0;
    char *ofmt_name = NULL;
    if (strstr(out_url, "rtmp://") != NULL)
    {
        // ofmt_name = "hls";
        ofmt_name = "flv";
    }
    else if (strstr(out_url, "udp://") != NULL)
    {
        ofmt_name = "mpegts";
    }
    else if (strstr(out_url, "rtp://") != NULL)
    {
        ofmt_name = "rtp";
        rtp_stream = 1;
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

    int stream_mapping_size = ifmt_ctx->nb_streams;
    int *stream_mapping = av_mallocz_array(stream_mapping_size, sizeof(*stream_mapping));
    if (!stream_mapping)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    int out_stream_index = 0;
    AVRational frame_rate = {0};
    double duration = 0.0;

    for (i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        AVStream *out_stream;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            frame_rate = av_guess_frame_rate(ifmt_ctx, in_stream, NULL);

            duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);
            printf("input stream frame num:%d,den:%d,duration:%f\n", frame_rate.num, frame_rate.den, duration);
            if (rtp_stream)
                rtp_stream = 2;
        }
        else
        {
            if (rtp_stream)
            {
                stream_mapping[i] = -1;
                printf("rtp only stream\n");
                continue;
            }
        }

        stream_mapping[i] = out_stream_index++;
        // 2.2 将一个新流(out_stream)添加到输出文件(ofmt_ctx)
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream)
        {
            printf("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        // 2.3 将当前输入流中的参数拷贝到输出流中
        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0)
        {
            printf("Failed to copy codec parameters\n");
            goto end;
        }
        out_stream->codecpar->codec_tag = 0;
        if (rtp_stream == 2)
            break;
    }
    av_dump_format(ofmt_ctx, 0, out_url, 1);
    // av_opt_set(ofmt_ctx->priv_data, "hls_time", "5", AV_OPT_SEARCH_CHILDREN);
    // av_opt_set(ofmt_ctx->priv_data, "hls_list_size" ,"10" , AV_OPT_SEARCH_CHILDREN);
    // av_opt_set(ofmt_ctx->priv_data, "hls_wrap", "5", AV_OPT_SEARCH_CHILDREN);
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
        // goto end;
    }
    if (rtp_stream)
    {
        char sdp[1024];
        //打印SDP信息, 该信息可用于Rtp流接收解码
        av_sdp_create(&ofmt_ctx, 1, sdp, sizeof(sdp));
        printf("------------------\n");
        printf("%s\n", sdp);
        printf("------------------\n");
    }
    // 3. 数据处理
    // 3.1 写输出文件头
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0)
    {
        printf("Error occurred when opening output file,err2str:%s\n", av_err2str(ret));
        goto end;
    }
    AVPacket pkt;
    AVStream *in_stream, *out_stream;
    int frame = 0;
    // int start_time = av_gettime();
    while (1)
    {
        // 3.2 从输出流读取一个packet
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
        {
            break;
        }

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        if (in_stream == NULL)
        {
            printf("in_stream is NULL\n");
            av_packet_unref(&pkt);
            continue;
        }

        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            av_usleep((int64_t)(duration * AV_TIME_BASE));

            // int64_t pts_time = av_rescale_q(pkt.pts, in_stream->time_base, AV_TIME_BASE_Q);
            // int64_t now_time = av_gettime() - start_time;
            // if (pts_time > now_time)
            //     av_usleep(pts_time - now_time);
        }
        else
        {
            if (rtp_stream)
            {
                printf("rtp only stream\n");
                continue;
            }
        }

        pkt.stream_index = stream_mapping[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        /* copy packet */
        // 3.3 更新packet中的pts和dts
        // 关于AVStream.time_base(容器中的time_base)的说明：
        // 输入：输入流中含有time_base，在avformat_find_stream_info()中可取到每个流中的time_base
        // 输出：avformat_write_header()会根据输出的封装格式确定每个流的time_base并写入文件中
        // AVPacket.pts和AVPacket.dts的单位是AVStream.time_base，不同的封装格式AVStream.time_base不同
        // 所以输出文件中，每个packet需要根据输出封装格式重新计算pts和dts

        printf("pkt1 pts:%ld,dts:%ld,duration:%ld\n", pkt.pts, pkt.dts, pkt.duration);
        av_packet_rescale_ts(&pkt, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        printf("pkt2 pts:%ld,dts:%ld,duration:%ld\n", pkt.pts, pkt.dts, pkt.duration);
        printf("input stream time num:%d,den:%d\n", in_stream->time_base.num, in_stream->time_base.den);
        printf("output stream time num:%d,den:%d\n", out_stream->time_base.num, out_stream->time_base.den);
        // 3.4 将packet写入输出
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0)
        {
            printf("Error muxing packet\n");
            break;
        }
        printf("write frame:%d\n", frame++);
        av_packet_unref(&pkt);
    }

    // 3.5 写输出文件尾
    av_write_trailer(ofmt_ctx);

end:
    avformat_close_input(&ifmt_ctx);

    /* close output */
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);

    if (ret < 0 && ret != AVERROR_EOF)
    {
        printf("Error occurred: %s\n", av_err2str(ret));
        return -1;
    }

    return 0;
}