#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include "alsa.h"

#include "wav.h"

int audio_play(const char *filename, sound_format_t snd_format, unsigned int sample_rate, unsigned short channels, snd_pcm_format_t sample_format)
{
    int rc, dir;
    int size, sample_format_bits;
    snd_pcm_uframes_t period_size;
    char *buffer;

    int fd = open(filename, O_RDONLY);
    if (SND_FORMAT_WAV == snd_format)
    {
        Wav wav_format;
        printf("WAV_fotmat size:%ld\n", sizeof(Wav));
        rc = read(fd, &wav_format, sizeof(Wav));
        if (rc != sizeof(Wav))
        {
            fprintf(stderr, "read wav erroe\n");
            exit(1);
        }
        printf("ChunkSize:%d\n", wav_format.riff.ChunkSize);
        printf("AudioFormat:%d\n", wav_format.fmt.AudioFormat);
        printf("NumChannels:%d\n", wav_format.fmt.NumChannels);
        printf("SampleRate:%d\n", wav_format.fmt.SampleRate);
        printf("ByteRate:%d\n", wav_format.fmt.ByteRate);
        printf("BlockAlign:%d\n", wav_format.fmt.BlockAlign);
        printf("BitsPerSample:%d\n", wav_format.fmt.BitsPerSample);
        printf("Subchunk2Size:%d\n", wav_format.data.Subchunk2Size);

        sample_rate = wav_format.fmt.SampleRate;
        channels = wav_format.fmt.NumChannels;
        sample_format_bits = wav_format.fmt.BitsPerSample;
    }
    else
    {
        sample_format_bits = snd_pcm_format_physical_width(sample_format);
    }

    snd_pcm_t *handle;
    //配置硬件参数结构体
    snd_pcm_hw_params_t *params;

    snd_pcm_hw_params_alloca(&params);

    alsa_conf_set(&handle, params, SND_PCM_STREAM_PLAYBACK,
                  sample_format,
                  SND_PCM_ACCESS_RW_INTERLEAVED,
                  channels,
                  sample_rate,
                  0,
                  5 * 100000);
    /* get the current hwparams */
    snd_pcm_hw_params_current(handle, params);
    snd_pcm_hw_params_get_period_size(params, &period_size, &dir);

    size = period_size * channels * sample_format_bits / 8; /* 2 bytes/sample, 1 channels */
    buffer = (char *)malloc(size);
    snd_pcm_sframes_t frames;
    while (1)
    {
        rc = read(fd, buffer, size);
        if (rc == 0)
        {
            fprintf(stderr, "end of file on input\n");
            break;
        }
        else if (rc != size)
        {
            fprintf(stderr, "short read: read %d bytes\n", rc);
        }

        frames = snd_pcm_writei(handle, buffer, period_size); //rc / FRAME_SIZE
        if (frames < 0)
            frames = snd_pcm_recover(handle, frames, 0);
        if (frames < 0)
        {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
            break;
        }
        else if (frames != period_size)
            printf("Short write (expected %li, wrote %li)\n", period_size, frames);
        usleep(50 * 1000);
    }
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
    close(fd);
    return 0;
}

void wav_format_init(Wav *wav_format, unsigned short channels, unsigned int sample_rate, unsigned short bits_sample)
{
    wav_format->riff.ChunkID[0] = 'R';
    wav_format->riff.ChunkID[1] = 'I';
    wav_format->riff.ChunkID[2] = 'F';
    wav_format->riff.ChunkID[3] = 'F';
    wav_format->riff.ChunkSize = 36;
    wav_format->riff.Format[0] = 'W';
    wav_format->riff.Format[1] = 'A';
    wav_format->riff.Format[2] = 'V';
    wav_format->riff.Format[3] = 'E';

    wav_format->fmt.Subchunk1ID[0] = 'f';
    wav_format->fmt.Subchunk1ID[1] = 'm';
    wav_format->fmt.Subchunk1ID[2] = 't';
    wav_format->fmt.Subchunk1ID[3] = ' ';
    wav_format->fmt.Subchunk1Size = 16;
    wav_format->fmt.AudioFormat = 1;
    wav_format->fmt.NumChannels = channels;
    wav_format->fmt.SampleRate = sample_rate;
    wav_format->fmt.ByteRate = sample_rate * channels * bits_sample / 8;
    wav_format->fmt.BlockAlign = channels * bits_sample / 8;
    wav_format->fmt.BitsPerSample = bits_sample;

    wav_format->data.Subchunk2ID[0] = 'd';
    wav_format->data.Subchunk2ID[1] = 'a';
    wav_format->data.Subchunk2ID[2] = 't';
    wav_format->data.Subchunk2ID[3] = 'a';
    wav_format->data.Subchunk2Size = 0;
}

void wav_format_full(Wav *wav_format, unsigned int data_size)
{
    wav_format->riff.ChunkSize += data_size;
    wav_format->data.Subchunk2Size = data_size;
}
int audio_capture(const char *filename, sound_format_t snd_format, unsigned int sample_rate, unsigned short channels, snd_pcm_format_t sample_format)
{
    int rc, dir;
    int size;
    snd_pcm_uframes_t period_size;
    char *buffer;

    int fd = open(filename, O_CREAT | O_RDWR);
    if (SND_FORMAT_WAV == snd_format)
    {
        lseek(fd, sizeof(Wav), SEEK_SET);
    }

    snd_pcm_t *handle;
    //配置硬件参数结构体
    snd_pcm_hw_params_t *params;

    snd_pcm_hw_params_alloca(&params);

    alsa_conf_set(&handle, params, SND_PCM_STREAM_CAPTURE,
                  sample_format,
                  SND_PCM_ACCESS_RW_INTERLEAVED,
                  channels,
                  sample_rate,
                  0,
                  5 * 100000);
    /* get the current hwparams */
    snd_pcm_hw_params_current(handle, params);
    snd_pcm_hw_params_get_period_size(params, &period_size, &dir);

    size = period_size * channels * snd_pcm_format_physical_width(sample_format) / 8; /* 2 bytes/sample, 1 channels */
    buffer = (char *)malloc(size);
    snd_pcm_sframes_t frames;

    long loops = 500000 / sample_rate;
    long file_data_size = 0;
    while (loops--)
    {
        frames = snd_pcm_readi(handle, buffer, period_size); //rc / FRAME_SIZE
        if (frames < 0)
            frames = snd_pcm_recover(handle, frames, 0);
        if (frames < 0)
        {
            printf("snd_pcm_readi failed: %s\n", snd_strerror(frames));
            break;
        }
        else if (frames != period_size)
            printf("Short read (expected %li, wrote %li)\n", period_size, frames);

        file_data_size += frames;
        rc = write(fd, buffer, size);
        if (rc < 0)
        {
            fprintf(stderr, "write error\n");
            break;
        }
        else if (rc != size)
        {
            fprintf(stderr, "short read: read %d bytes\n", rc);
        }
        printf("file_data_size: %ld\n", file_data_size);
    }
    file_data_size *= channels * snd_pcm_format_physical_width(sample_format) / 8;
    if (SND_FORMAT_WAV == snd_format)
    {
        lseek(fd, 0, SEEK_SET);
        Wav wav_format;
        wav_format_init(&wav_format, channels, sample_rate, snd_pcm_format_physical_width(sample_format));
        wav_format_full(&wav_format, file_data_size);

        printf("WAV_fotmat data size:%ld\n", file_data_size);
        rc = write(fd, &wav_format, sizeof(Wav));
        if (rc != sizeof(Wav))
        {
            fprintf(stderr, "write wav error;%d\n", rc);
            return -1;
        }
        printf("ChunkSize:%d\n", wav_format.riff.ChunkSize);
        printf("AudioFormat:%d\n", wav_format.fmt.AudioFormat);
        printf("NumChannels:%d\n", wav_format.fmt.NumChannels);
        printf("SampleRate:%d\n", wav_format.fmt.SampleRate);
        printf("ByteRate:%d\n", wav_format.fmt.ByteRate);
        printf("BlockAlign:%d\n", wav_format.fmt.BlockAlign);
        printf("BitsPerSample:%d\n", wav_format.fmt.BitsPerSample);
        printf("Subchunk2Size:%d\n", wav_format.data.Subchunk2Size);
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
    close(fd);
    return 0;
}
