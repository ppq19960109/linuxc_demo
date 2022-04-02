#ifndef _ALSA_H_
#define _ALSA_H_

#include "alsa_conf.h"

typedef enum _sound_format
{
    SND_FORMAT_PCM = 0,
    SND_FORMAT_WAV
} sound_format_t;

int audio_play(const char *filename, sound_format_t snd_format, unsigned int sample_rate, unsigned short channels, snd_pcm_format_t sample_format);
int audio_capture(const char *filename, sound_format_t snd_format, unsigned int sample_rate, unsigned short channels, snd_pcm_format_t sample_formats);
#endif