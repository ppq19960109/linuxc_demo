#ifndef _LOCAL_SEND_H_
#define _LOCAL_SEND_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define HY_HEART "{\"Command\":\"TcpBeatHeart\",\"Period\":\"60\"}"

#define STR_ADD "Add"
#define STR_DEVSINFO "DevsInfo"
#define STR_DEVATTRI "DevAttri"
#define STR_REFACTORY "ReFactory"

#define STR_NET_TIME "120"
#define STR_NET_CLOSE "0"

    struct HylinkSendData
    {
        char DeviceId[24];
        char ModelId[24];
        char Key[24];
        char Value[16];
        void *private;
    };
    typedef struct
    {
        char Command;
        int FrameNumber;
        char Type[12];
        // char GatewayId[16];
        struct HylinkSendData Data;
    } HylinkDevSendData;
    int hylinkDispatch(const char *str);
    int hylinkSend(void *ptr);
    int hylinkDelDev(const char *sn);

    int hylinkHeart(void);

    int hylinkSendDevInfo(void);
    int hylinkSendDevAttr(void *devId, unsigned int len);
    int hylinkSendReset(void);
#ifdef __cplusplus
}
#endif
#endif