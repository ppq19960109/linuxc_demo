#ifndef _PAEX_PLAY_H_
#define _PAEX_PLAY_H_
#include "portaudio.h"
int paex_play(unsigned short channel, PaSampleFormat sampleFormat, unsigned int sample_rate, unsigned long framesPerBuffer);
#endif