#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <alsa/asoundlib.h>

#define SAMPLE_RATE (44100)
#define CHANNELS (2)
#define FRAME_SIZE (2 * CHANNELS)
// snd_pcm_hw_params_alloca snd_pcm_hw_params_set_format
int audio_play(const char *filename)
{
    int rc;
    int size;

    unsigned int val;

    int dir = 0;
    snd_pcm_uframes_t frames;
    char *buffer;

    int fd;
    int err;

    fd = open(filename, O_RDONLY);

    snd_pcm_t *handle;
    //以播放模式打开设备
    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0)
    {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }
    //配置硬件参数结构体
    snd_pcm_hw_params_t *params;

    //params申请内存
    snd_pcm_hw_params_malloc(&params);

    //使用pcm设备初始化hwparams
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0)
    {
        fprintf(stderr, "Can not configure this PCM device: %s\n", snd_strerror(err));
        exit(1);
    }

    //设置多路数据在buffer中的存储方式
    //SND_PCM_ACCESS_RW_INTERLEAVED每个周期(period)左右声道的数据交叉存放
    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to interleaved: %s\n", snd_strerror(err));
        exit(1);
    }

    //设置16位采样格式，S16为有符号16位,LE是小端模式
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE
        err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to 16-bit signed PCM: %s\n", snd_strerror(err));
        exit(1);
    }

    //设置声道数,双声道
    err = snd_pcm_hw_params_set_channels(handle, params, CHANNELS);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to mono: %s\n", snd_strerror(err));
        exit(1);
    }

    //采样率44.1KHZ
    val = SAMPLE_RATE;
    //设置采样率,如果采样率不支持，会用硬件支持最接近的采样率
    err = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to sample rate =%d: %s\n", val, snd_strerror(err));
        exit(1);
    }

    unsigned int buffer_time, period_time;
    //获取最大的缓冲时间,buffer_time单位为us,500000us=0.5s
    snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0);
    if (buffer_time > 500000)
        buffer_time = 500000;

    //设置缓冲时间
    err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, 0);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to buffer time =%d: %s\n", buffer_time, snd_strerror(err));
        exit(1);
    }

    period_time = buffer_time / 2;
    //设置周期时间
    err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to period time =%d: %s\n", period_time, snd_strerror(err));
        exit(1);
    }

    snd_pcm_uframes_t buffer_size;
    snd_pcm_hw_params_get_buffer_size_max(params, &buffer_size);
    snd_pcm_hw_params_set_buffer_size_near(handle, params, &buffer_size);

    snd_pcm_uframes_t period_size = buffer_size / 4;
    snd_pcm_hw_params_set_period_size_near(handle, params, &period_size, 0);

    //让这些参数作用于PCM设备
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0)
    {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(1);
    }

    int bit = snd_pcm_format_physical_width(format);
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    // 1 frame = channels * sample_size.
    size = frames * FRAME_SIZE; /* 2 bytes/sample, 1 channels */
    buffer = (char *)malloc(size);

    while (1)
    {
        rc = read(fd, buffer, size);
        if (rc == 0)
        {
            fprintf(stderr, "end of file on input\n");
            break;
        }
        else if (rc != size)
        {
            fprintf(stderr, "short read: read %d bytes\n", rc);
        }
        rc = snd_pcm_writei(handle, buffer, rc / FRAME_SIZE);
        if (rc == -EPIPE)
        {
            fprintf(stderr, "underrun occurred\n");
            err = snd_pcm_prepare(handle);
            if (err < 0)
            {
                fprintf(stderr, "can not recover from underrun: %s\n", snd_strerror(err));
            }
        }
        else if (rc < 0)
        {
            fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
        }
        else if (rc != (int)frames)
        {
            fprintf(stderr, "short write, write %d frames\n", rc);
        }
    }
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
    close(fd);
    return 0;
}
