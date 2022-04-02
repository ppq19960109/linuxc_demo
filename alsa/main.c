#include <stdio.h>
#include "alsa.h"

int main(int agrc, char *agrv[])
{
    printf("main start \n");
    // if (agrc <= 1)
    //     return -1;
    // printf("audio_play:%s\n", agrv[1]);
    audio_play(agrv[1], SND_FORMAT_PCM, 16000, 1, SND_PCM_FORMAT_S16);
    // audio_capture("test.wav",SND_FORMAT_WAV,16000,1,SND_PCM_FORMAT_S16);

    printf("main end\n");
    return 0;
}
