#ifndef _WAV_H_
#define _WAV_H_

typedef struct WAV_RIFF
{
    /* chunk "riff" */
    char ChunkID[4]; /* "RIFF" */
    /* sub-chunk-size */
    unsigned int ChunkSize; /* 36 + Subchunk2Size */
    /* sub-chunk-data */
    char Format[4]; /* "WAVE" */
} RIFF_t;

typedef struct WAV_FMT
{
    /* sub-chunk "fmt" */
    char Subchunk1ID[4]; /* "fmt " */
    /* sub-chunk-size */
    unsigned int Subchunk1Size; /* 16 for PCM */
    /* sub-chunk-data */
    unsigned short AudioFormat;   /* PCM = 1*/
    unsigned short NumChannels;   /* Mono = 1, Stereo = 2, etc. */
    unsigned int SampleRate;      /* 8000, 44100, etc. */
    unsigned int ByteRate;        /* = SampleRate * NumChannels * BitsPerSample/8 */
    unsigned short BlockAlign;    /* = NumChannels * BitsPerSample/8 */
    unsigned short BitsPerSample; /* 8bits, 16bits, etc. */
} FMT_t;

typedef struct WAV_data
{
    /* sub-chunk "data" */
    char Subchunk2ID[4]; /* "data" */
    /* sub-chunk-size */
    unsigned int Subchunk2Size; /* data size */
    /* sub-chunk-data */
    //    Data_block_t block;
} Data_t;

//typedef struct WAV_data_block {
//} Data_block_t;

typedef struct WAV_fotmat
{
    RIFF_t riff;
    FMT_t fmt;
    Data_t data;
} Wav;

#endif