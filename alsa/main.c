#include <stdio.h>
#include "alsa.h"

int main(int agrc, char *agrv[])
{
    printf("main start \n");

    audio_play("test.pcm");

    printf("main end\n");
    return 0;
}
