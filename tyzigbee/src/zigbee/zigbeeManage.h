#ifndef _ZIGBEEMANAGE_H_
#define _ZIGBEEMANAGE_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        char *modelId;
        char hyKey[33];
        char hyKeyPrivate;
        char *dir;
        unsigned char *in;
        int inLen;
        unsigned char *out;
        int outLen;
    } ConverDesc;

    int hylinkValueConversion(ConverDesc *converDesc);
#ifdef __cplusplus
}
#endif
#endif