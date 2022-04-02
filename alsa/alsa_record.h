#ifndef _ALSA_RECORD_H_
#define _ALSA_RECORD_H_

#include "alsa_conf.h"

typedef struct _record_format
{
    unsigned int sample_rate;
    unsigned short channels;
    unsigned short sample_bytes;
    snd_pcm_stream_t snd_pcm_stream;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t period_size;
    char *buffer;
    int buffer_size;
    int (*record_cb)(char *data, int len);
    int running;
} record_format_t;

int audio_init(record_format_t *record_format, unsigned int sample_rate, unsigned short channels, snd_pcm_format_t sample_format, snd_pcm_stream_t snd_pcm_stream);
void audio_record_start(record_format_t *record_format, int (*record_cb)(char *data, int len));
void audio_record_pause(record_format_t *record_format);
void audio_record_resume(record_format_t *record_format);
void audio_deinit(record_format_t *record_format);
int audio_play_data(record_format_t *record_format, void *data, int len);
#endif