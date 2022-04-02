#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "paex_record.h"

#define PA_SAMPLE_TYPE paInt16
typedef short SAMPLE;
static int time;

static int recordCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData)
{
    const SAMPLE *rptr = (const SAMPLE *)inputBuffer;
    int fd = (int)userData;
    printf("recordCallback framesPerBuffer:%ld\n", framesPerBuffer);
    if (inputBuffer == NULL)
    {
        printf("recordCallback inputBuffer null\n");
    }
    else
    {
        int rc = write(fd, inputBuffer, sizeof(SAMPLE) * framesPerBuffer);
        if (rc < 0)
        {
            fprintf(stderr, "write error\n");
        }
    }
    time -= framesPerBuffer;
    if (time <= 0)
    {
        return paComplete;
    }
    return paContinue; //paContinue
}

int paex_record(unsigned short channel, PaSampleFormat sampleFormat, unsigned int sample_rate, unsigned long framesPerBuffer)
{
    PaStreamParameters inputParameters;
    PaStream *stream;
    PaError err = paNoError;
    time = sample_rate * 5;
    int fd = open("record.pcm", O_CREAT | O_RDWR, 0777);

    err = Pa_Initialize();
    if (err != paNoError)
        goto done;

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice)
    {
        fprintf(stderr, "Error: No default input device.\n");
        goto done;
    }
    inputParameters.channelCount = channel; /* stereo input */
    inputParameters.sampleFormat = sampleFormat;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;//defaultHighInputLatency defaultLowInputLatency
    inputParameters.hostApiSpecificStreamInfo = NULL;

    /* Record some audio. -------------------------------------------- */
    err = Pa_OpenStream(
        &stream,
        &inputParameters,
        NULL, /* &outputParameters, */
        sample_rate,
        framesPerBuffer,
        paClipOff, /* we won't output out of range samples so don't bother clipping them */
        recordCallback,
        (void *)fd);
    if (err != paNoError)
        goto done;

    err = Pa_StartStream(stream);
    if (err != paNoError)
        goto done;
    printf("\n=== Now recording!! Please speak into the microphone. ===\n");

    while ((err = Pa_IsStreamActive(stream)) == 1)
    {
        Pa_Sleep(1000);
    }
    if (err < 0)
        goto done;

    err = Pa_CloseStream(stream);
    if (err != paNoError)
        goto done;

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
