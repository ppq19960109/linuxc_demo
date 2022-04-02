#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#include "alsa_record.h"

int audio_init(record_format_t *record_format, unsigned int sample_rate, unsigned short channels, snd_pcm_format_t sample_format, snd_pcm_stream_t snd_pcm_stream)
{
    int dir;
    memset(record_format, 0, sizeof(record_format_t));
    record_format->channels = channels;
    record_format->sample_rate = sample_rate;
    record_format->sample_bytes = snd_pcm_format_physical_width(sample_format) / 8;
    record_format->snd_pcm_stream = snd_pcm_stream;

    snd_pcm_hw_params_alloca(&record_format->params);

    alsa_conf_set(&record_format->handle, record_format->params, record_format->snd_pcm_stream,
                  sample_format,
                  SND_PCM_ACCESS_RW_INTERLEAVED,
                  record_format->channels,
                  record_format->sample_rate,
                  0,
                  5 * 100000);
    /* get the current hwparams */
    snd_pcm_hw_params_current(record_format->handle, record_format->params);
    snd_pcm_hw_params_get_period_size(record_format->params, &record_format->period_size, &dir);

    record_format->buffer_size = record_format->period_size * record_format->channels * record_format->sample_bytes; /* 2 bytes/sample, 1 channels */
    record_format->buffer = (char *)malloc(record_format->buffer_size);
    return 0;
}

static void *record_thread_proc(void *para)
{
    snd_pcm_sframes_t frames;
    record_format_t *record_format = (record_format_t *)para;
    while (record_format->running)
    {
        if (record_format->running > 1)
        {
            usleep(100000);
            continue;
        }
        frames = snd_pcm_readi(record_format->handle, record_format->buffer, record_format->period_size); //rc / FRAME_SIZE
        if (frames < 0)
            frames = snd_pcm_recover(record_format->handle, frames, 0);
        if (frames < 0)
        {
            printf("snd_pcm_readi failed: %s\n", snd_strerror(frames));
            continue;
            // break;
        }
        else if (frames != record_format->period_size)
            printf("Short read (expected %li, wrote %li)\n", record_format->period_size, frames);

        if (record_format->record_cb != NULL)
            record_format->record_cb(record_format->buffer, record_format->buffer_size);
    }
    return NULL;
}

int audio_write_data(record_format_t *record_format, const void *data, snd_pcm_uframes_t period_size)
{
    snd_pcm_sframes_t frames;
    frames = snd_pcm_writei(record_format->handle, data, period_size); //rc / FRAME_SIZE
    if (frames < 0)
        frames = snd_pcm_recover(record_format->handle, frames, 0);
    if (frames < 0)
    {
        printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
        return frames;
    }
    else if (frames != period_size)
        printf("Short write (expected %li, wrote %li)\n", period_size, frames);

    usleep(50 * 1000);
    return frames;
}

int audio_play_data(record_format_t *record_format, void *data, int len)
{
    static int buffer_remain_len = 0;
    unsigned char period_unit = record_format->channels * record_format->sample_bytes;

    if (buffer_remain_len != 0)
    {
        int remain_len = record_format->buffer_size - buffer_remain_len;
        if (remain_len > len)
        {
            return -1;
        }

        memcpy(&record_format->buffer[buffer_remain_len], data, remain_len);
        audio_write_data(record_format, record_format->buffer, record_format->period_size);
        len -= remain_len;
        data += remain_len;
        buffer_remain_len = 0;
    }

    snd_pcm_uframes_t total_period_size = len / period_unit;
    int total_remain_len = len % period_unit;
    while (total_period_size > 0)
    {
        if (total_period_size >= record_format->period_size)
        {
            audio_write_data(record_format, data, record_format->period_size);
            total_period_size -= record_format->period_size;
            data += record_format->period_size * period_unit;
        }
        else
        {
            buffer_remain_len = total_period_size * period_unit;
            memcpy(record_format->buffer, data, buffer_remain_len);
            total_period_size = 0;
        }
    }
    return 0;
}

void audio_record_start(record_format_t *record_format, int (*record_cb)(char *data, int len))
{
    if (record_format->running > 0)
        return;

    record_format->record_cb = record_cb;
    record_format->running = 1;
    pthread_t tid;
    pthread_create(&tid, NULL, record_thread_proc, (void *)record_format);
    pthread_detach(tid);
}

void audio_record_pause(record_format_t *record_format)
{
    record_format->running = 2;
    snd_pcm_drain(record_format->handle);
}
void audio_record_resume(record_format_t *record_format)
{
    record_format->running = 1;
    snd_pcm_start(record_format->handle);
}

void audio_deinit(record_format_t *record_format)
{
    if (SND_PCM_STREAM_CAPTURE == record_format->snd_pcm_stream)
    {
        record_format->running = 0;
    }
    snd_pcm_drain(record_format->handle);
    snd_pcm_close(record_format->handle);
    free(record_format->buffer);
}