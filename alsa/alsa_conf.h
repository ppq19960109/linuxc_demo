#ifndef _ALSA_CONF_H_
#define _ALSA_CONF_H_

#include <alsa/asoundlib.h>

int alsa_conf_set(snd_pcm_t **pcm, snd_pcm_hw_params_t *params, snd_pcm_stream_t stream,
                  snd_pcm_format_t format,
                  snd_pcm_access_t access,
                  unsigned int channels,
                  unsigned int rate,
                  int soft_resample,
                  unsigned int latency);

#endif