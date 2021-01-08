#ifndef _HYLINKSEND_H_
#define _HYLINKSEND_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        char DeviceId[24];
        char ModelId[24];
        char Key[24];
        char Value[16];
        // void *private;
    } HylinkSendData;
    typedef struct
    {
        char Command;
        int FrameNumber;
        char Type[12];
        char GatewayId[16];
        HylinkSendData *Data;
        unsigned char DataSize;
    } HylinkSend;

    int hylinkSendFunc(HylinkSend *hylinkSend);
#ifdef __cplusplus
}
#endif
#endif