#ifndef _LOCAL_RECEIVE_H_
#define _LOCAL_RECEIVE_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <pthread.h>
#include "list.h"
#include "tool.h"

#define SERVER_PORT 7000

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
#define STR_ONLINE "Online"
#define STR_VERSION "Version"
#define STR_PARAMS "Params"

#define STR_CTRL "Ctrl"
#define STR_ATTRIBUTE "Attribute"
#define STR_DELETE "Delete"

#define STR_HOST_GATEWAYID "0000000000000000"
#define STR_PERMITJOINING "PermitJoining"

#define INT_ONLINK 1
#define INT_OFFLINK 0

    typedef struct
    {
        char DeviceId[24];
        char ModelId[24];
        char GatewayId[20];
        char Version[16];
        char Online;
        void *private;
        struct list_head node;
    } dev_local_t;
    //---------------------------
    struct local_data_t
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
        struct local_data_t Data;
    } local_dev_t;
    //-----------------------------------------------

    typedef struct
    {
#define SENDTOLOCAL_SIZE 1024
        char sendData[SENDTOLOCAL_SIZE];
        dev_local_t gateway;
        pthread_mutex_t mutex;
        struct list_head head;
    } LocalControl_t;

    void local_control_init();
    void local_control_destory();
    struct list_head *local_get_list_head();
    char *local_get_sendData();
    pthread_mutex_t *local_get_mutex();
    dev_local_t *local_get_gateway();

    void recv_toLocal(char *data, int len);
#ifdef __cplusplus
}
#endif
#endif