#ifndef _PAEX_RECORD_H_
#define _PAEX_RECORD_H_
#include "portaudio.h"
int paex_record(unsigned short channel, PaSampleFormat sampleFormat, unsigned int sample_rate, unsigned long framesPerBuffer);
#endif