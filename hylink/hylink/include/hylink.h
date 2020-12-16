#ifndef _HYLINKCONTROL_H_
#define _HYLINKCONTROL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include "cJSON.h"
#include "hylinkListFunc.h"

#define GATEWAYTYPE 0xff

#define STR_HOST_GATEWAYID "0000000000000000"
#define STR_PERMITJOINING "PermitJoining"

#define STR_KEY "Key"
#define STR_VALUE "Value"

    typedef struct
    {
        char *const *attr;
        unsigned short attrLen;
        char *const *attrCtrl;
        unsigned short attrCtrlLen;
    } SAttrInfo;

    typedef struct
    {
        struct list_head head;
        pthread_mutex_t mutex;
        HylinkDev *gateway;
    } HylinkController;

    typedef struct
    {
        char PermitJoining;
        char FirmwareVersion[8];
        char SoftVersion[8];
    } HylinkDevGateway;

    void hylinkMain(void);
    pthread_mutex_t *hylinkGetMutex(void);
    struct list_head *hylinkGetListHead(void);

    int hylinkGateway(cJSON *Data);
#ifdef __cplusplus
}
#endif
#endif