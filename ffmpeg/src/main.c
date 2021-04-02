#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
// #include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "dec_video.h"
#include "enc_video.h"

#include "push_stream.h"
int main(int argc, char **argv)
{
    // int ret = -1;
    printf("ffmpeg main start...\n");
    const char *src_filename = argv[1];
    const char *dec_filename = argv[2];
    const char *enc_filename = argv[3];
#if 0
    int width, height;
    enum AVPixelFormat pix_fmt;
    enum AVCodecID codec_id;
    decode_video_open(src_filename, dec_filename, &width, &height, &pix_fmt, &codec_id);

    encode_video_open(enc_filename, width, height, pix_fmt, codec_id);
    decode_video_run(encode_video_run);
    encode_video_close();

    decode_video_close();
#else
    push_stream_open(src_filename, dec_filename);
#endif
    printf("ffmpeg main end!!!!!!!\n");
    return 0;
}
