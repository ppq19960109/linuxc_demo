#include <stdio.h>
#include "alsa.h"

int main(int agrc, char *agrv[])
{
    printf("main start \n");
    if (agrc <= 1)
        return -1;
    printf("audio_play:%s\n",agrv[1]);
    audio_play(agrv[1]);

    printf("main end\n");
    return 0;
}
