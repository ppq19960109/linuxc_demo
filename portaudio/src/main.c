#include <stdio.h>
#include "paex_record.h"
#include "paex_play.h"

int main(int agrc, char *agrv[])
{
    printf("main start \n");
    // if (agrc <= 1)
    //     return -1;
    // printf("audio_play:%s\n", agrv[1]);
    // paex_record(1, paInt16, 16000, 4096);
    paex_play(1, paInt16, 16000, 4096);
    printf("main end\n");
    return 0;
}
