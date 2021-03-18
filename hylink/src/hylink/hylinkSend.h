#ifndef _HYLINKREPORT_H_
#define _HYLINKREPORT_H_

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
        char Online[4];
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
    int hylinkDispatch(const char *str, const int str_len, const char dir);
#ifdef __cplusplus
}
#endif
#endif