
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "paex_play.h"

#define PA_SAMPLE_TYPE paInt16
typedef short SAMPLE;
static int playCallback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData)
{
    printf("playCallback framesPerBuffer:%ld\n", framesPerBuffer);
    SAMPLE *wptr = (SAMPLE *)outputBuffer;
    int fd = (int)userData;
    if (outputBuffer != NULL)
    {
        int rc = read(fd, outputBuffer, sizeof(SAMPLE) * framesPerBuffer);
        if (rc < 0)
        {
            fprintf(stderr, "read error\n");
        }
        else if (rc == 0)
        {
            printf("read end\n");
            return paComplete;
        }
    }
    return paContinue;
}

int paex_play(unsigned short channel, PaSampleFormat sampleFormat, unsigned int sample_rate, unsigned long framesPerBuffer)
{
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err = paNoError;
    int fd = open("record.pcm", O_RDWR);
    err = Pa_Initialize();
    if (err != paNoError)
        goto done;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice)
    {
        fprintf(stderr, "Error: No default output device.\n");
        goto done;
    }
    outputParameters.channelCount = channel; /* stereo output */
    outputParameters.sampleFormat = sampleFormat;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    printf("\n=== Now playing back. ===\n");
    fflush(stdout);
    err = Pa_OpenStream(
        &stream,
        NULL, /* no input */
        &outputParameters,
        sample_rate,
        framesPerBuffer,
        paClipOff, /* we won't output out of range samples so don't bother clipping them */
        playCallback,
        (void *)fd);
    if (err != paNoError)
        goto done;

    if (stream)
    {
        err = Pa_StartStream(stream);
        if (err != paNoError)
            goto done;

        printf("Waiting for playback to finish.\n");

        while ((err = Pa_IsStreamActive(stream)) == 1)
            Pa_Sleep(100);
        if (err < 0)
            goto done;

        err = Pa_CloseStream(stream);
        if (err != paNoError)
            goto done;
    }

done:
    Pa_Terminate();
    close(fd);
    if (err != paNoError)
    {
        fprintf(stderr, "An error occurred while using the portaudio stream\n");
        fprintf(stderr, "Error number: %d\n", err);
        fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
        err = 1; /* Always return 0 or 1, but no other return codes. */
    }
    return err;
}
