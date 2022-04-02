#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include "alsa_conf.h"

int alsa_conf_set(snd_pcm_t **pcm, snd_pcm_hw_params_t *params, snd_pcm_stream_t stream,
                  snd_pcm_format_t format,
                  snd_pcm_access_t access,
                  unsigned int channels,
                  unsigned int rate,
                  int soft_resample,
                  unsigned int latency)
{
    int rc, err, dir;
    //以播放模式打开设备
    rc = snd_pcm_open(pcm, "default", stream, 0); //SND_PCM_ASYNC
    if (rc < 0)
    {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        return -1;
    }
    snd_pcm_t *handle = *pcm;
#if 1
    //使用pcm设备初始化hwparams
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0)
    {
        fprintf(stderr, "Can not configure this PCM device: %s\n", snd_strerror(err));
        return -1;
    }

    //设置多路数据在buffer中的存储方式
    err = snd_pcm_hw_params_set_access(handle, params, access);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to interleaved: %s\n", snd_strerror(err));
        return -1;
    }

    //设置16位采样格式，S16为有符号16位,LE是小端模式
    err = snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to 16-bit signed PCM: %s\n", snd_strerror(err));
        return -1;
    }

    //设置声道数,双声道
    err = snd_pcm_hw_params_set_channels(handle, params, channels);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to mono: %s\n", snd_strerror(err));
        return -1;
    }

    //采样率
    unsigned int rrate = rate;
    //设置采样率,如果采样率不支持，会用硬件支持最接近的采样率
    err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, &dir);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to sample rate =%d: %s\n", rrate, snd_strerror(err));
        return -1;
    }

    //设置缓冲时间
    err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &latency, 0);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to buffer time =%d: %s\n", latency, snd_strerror(err));
        return -1;
    }

    unsigned int period_time = latency / 4;
    //设置周期时间
    err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to period time =%d: %s\n", period_time, snd_strerror(err));
        return -1;
    }

    //让这些参数作用于PCM设备
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0)
    {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        return -1;
    }
#else
    if ((err = snd_pcm_set_params(handle,
                                  format,
                                  access,
                                  channels,
                                  rate,
                                  soft_resample,
                                  latency)) < 0)
    {
        printf("snd_pcm_set_params error: %s\n", snd_strerror(err));
        return -1;
    }
#endif
    return 0;
}
