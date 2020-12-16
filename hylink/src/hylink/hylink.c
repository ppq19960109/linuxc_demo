#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commonFunc.h"
#include "frameCb.h"
#include "nativeFrame.h"
#include "rkDriver.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "hylinkSend.h"
#include "logFunc.h"

static HylinkController s_hylinkler;

void hylinkGatewayInit(void)
{
    s_hylinkler.gateway = malloc(sizeof(HylinkDev));

    HylinkDev *hylinkDev = s_hylinkler.gateway;
    memset(hylinkDev, 0, sizeof(HylinkDev));
    strcpy(hylinkDev->GatewayId, "");
    strcpy(hylinkDev->DeviceId, STR_HOST_GATEWAYID);
    strcpy(hylinkDev->ModelId, "000000");
    hylinkDev->Online = SUBDEV_ONLINE;
    hylinkDev->devType = GATEWAYTYPE;
    hylinkDev->private = malloc(sizeof(HylinkDevGateway));
    memset(hylinkDev->private, 0, sizeof(HylinkDevGateway));
}

int hylinkInit(void)
{
    pthread_mutex_init(&s_hylinkler.mutex, NULL);
    hylinkListInit(&s_hylinkler.head);
    hylinkGatewayInit();
    hylinkGateway(NULL);
    rkDriverOpen();

    runSystemCb(LAN_OPEN);
    return 0;
}

int hylinkReset(void)
{
    HylinkDev *ptr;
#ifndef HY_USER_KHASH
    HylinkDev *next;
    hylinkListEachEntrySafe(ptr, next, &s_hylinkler.head, hylinkNode)
    {
        hylinkDelDev(ptr->DeviceId);
    }
#else
    hyLink_kh_foreach_value(ptr)
    {
        logWarn("hyLink_kh_foreach_value");
        hylinkDelDev(ptr->DeviceId);
    }
#endif
    hylinkSendReset();
    hylinkListEmpty();
    return 0;
}

int hylinkDestory(void)
{
    runSystemCb(LAN_CLOSE);

    hylinkListEmpty();
    rkDriverClose();

    hylinkDevFree(s_hylinkler.gateway);
    pthread_mutex_destroy(&s_hylinkler.mutex);
    return 0;
}

pthread_mutex_t *hylinkGetMutex(void)
{
    return &s_hylinkler.mutex;
}

struct list_head *hylinkGetListHead(void)
{
    return &s_hylinkler.head;
}

int hylinkGateway(cJSON *Data)
{
    HylinkDev *hylinkDev = s_hylinkler.gateway;
    int res = -1;
    if (Data == NULL)
        goto cloud;

    HylinkDevGateway *gatewayDev = (HylinkDevGateway *)hylinkDev->private;

    cJSON *Key = cJSON_GetObjectItem(Data, STR_KEY);
    if (Key == NULL)
        goto fail;
    if (strcmp(Key->valuestring, STR_PERMITJOINING) == 0)
    {
        getByteForJson(Data, STR_VALUE, &gatewayDev->PermitJoining);
    }
    else
    {
        goto fail;
    }

cloud:
    res = runTransferCb(hylinkDev, GATEWAYTYPE, TRANSFER_CLOUD_REPORT);
    return res;
fail:
    return -1;
}

void hylinkMain(void)
{
    registerSystemCb(hylinkInit, HYLINK_OPEN);
    registerSystemCb(hylinkDestory, HYLINK_CLOSE);
    registerSystemCb(hylinkReset, HYLINK_RESET);

    registerTransferCb(hylinkRecv, TRANSFER_READ);

    registerSystemCb(hylinkHeart, CMD_HEART);
    registerSystemCb(hylinkSendNetwork, CMD_NETWORK);
    registerSystemCb(hylinkSendDevInfo, CMD_DEVSINFO);

    registerSystemCb(nativeFrameOpen, LAN_OPEN);
}
