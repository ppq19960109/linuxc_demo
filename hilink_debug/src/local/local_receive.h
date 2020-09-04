#ifndef _LOCAL_RECEIVE_H_
#define _LOCAL_RECEIVE_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <pthread.h>
#include "list.h"
#include "tool.h"

// #define USE_LIBUV        //enable libuv
#define SERVER_PORT 7000 //7000
#define HY_HEART "{\"Command\":\"TcpBeatHeart\",\"Period\":\"60\"}"

#define STR_KEY "Key"
#define STR_VALUE "Value"
#define STR_COMMAND "Command"
#define STR_REPORT "Report"
#define STR_DISPATCH "Dispatch"
#define STR_BEATHEARTRESPONSE "BeatHeartResponse"
#define STR_FRAMENUMBER "FrameNumber"
#define STR_TYPE "Type"
#define STR_DATA "Data"
#define STR_DEVICEID "DeviceId"
#define STR_MODELID "ModelId"
#define STR_GATEWAYID "GatewayId"
#define STR_DEVICETYPE "DeviceType"
#define STR_SECRET "Secret"
#define STR_ONLINE "Online"
#define STR_VERSION "Version"
#define STR_REGISTERSTATUS "RegisterStatus"
#define STR_PARAMS "Params"

#define STR_CTRL "Ctrl"
#define STR_ATTRIBUTE "Attribute"
#define STR_DELETE "Delete"

#define STR_HOST_GATEWAYID "0000000000000000"
#define STR_PERMITJOINING "PermitJoining"

    typedef struct
    {
        char GatewayId[20];
        char DeviceType[16];
        char DeviceId[20];
        char ModelId[24];
        char Version[16];
        char Secret[40];
        char Online;
        char RegisterStatus;
        void *private;
        struct list_head node;
    } dev_data_t;

    struct local_data_t
    {
        char DeviceId[20];
        char ModelId[20];
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
        struct local_data_t Data;
        // void *private;
    } local_dev_t;
    //-----------------------------------------------
    typedef struct
    {
        char PermitJoining;
        char FirmwareVersion[8];
        char SoftVersion[8];
    } DevGateway_t;

    typedef struct
    {
#define SENDTOLOCAL_SIZE 1024
        char sendData[SENDTOLOCAL_SIZE];
#define RECVLEN 16384
        char tcpBuf[RECVLEN + 1];
#ifndef USE_LIBUV
        int socketfd;
        pthread_t pid;
#endif
        DevGateway_t devGateway;
        struct list_head head;
    }LocalControl_t;

    extern LocalControl_t g_SLocalControl;

    void local_control_init(LocalControl_t *localControl);
    void local_control_destory(LocalControl_t *localControl);
    struct list_head *local_get_list_head(LocalControl_t *localControl);

    int read_from_local(const char *json, struct list_head *node);
    void recv_toLocal(char *data, int len);
#ifdef __cplusplus
}
#endif
#endif