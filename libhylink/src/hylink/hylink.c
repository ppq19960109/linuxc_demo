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
#include "hylinkSubDev.h"
#include "logFunc.h"

typedef struct
{
    unsigned char dispatchBuf[1024];
    pthread_mutex_t mutex;
} HylinkController;

static HylinkController s_hylink;

pthread_mutex_t *hylinkGetMutex(void)
{
    return &s_hylink.mutex;
}

unsigned char *getHyDispatchBuf(void)
{
    return s_hylink.dispatchBuf;
}

int hylinkInit(void)
{
    pthread_mutex_init(&s_hylink.mutex, NULL);

    hylinkListInit();
    rkDriverOpen();

    HyLinkDev *gwHyLinkDev = addProfileDev(HYLINK_PROFILE_PATH, STR_GATEWAY_DEVID, STR_GATEWAY_MODELID, hyLinkParseJson);
    if (gwHyLinkDev != NULL)
        runTransferCb(gwHyLinkDev, ATTR_REPORT_ALL, TRANSFER_CLOUD_REPORT);

    runSystemCb(LAN_OPEN);
    return 0;
}

int hylinkReset(void)
{
    HyLinkDev *dev;

    hyLink_kh_foreach_value(dev)
    {
        logWarn("hyLink_kh_foreach_value");
        if (dev != NULL)
            hylinkDelDev(dev->devId);
    }

    hylinkListEmpty();
    return 0;
}

int hylinkDestory(void)
{
    runSystemCb(LAN_CLOSE);

    hylinkListEmpty();
    runSystemCb(RK_DRIVER_CLOSE);

    pthread_mutex_destroy(&s_hylink.mutex);
    return 0;
}

void hylinkMain(void)
{
    registerSystemCb(hylinkInit, HYLINK_OPEN);
    registerSystemCb(hylinkDestory, HYLINK_CLOSE);
    registerSystemCb(hylinkReset, HYLINK_RESET);

    registerTransferCb(hylinkRecv, TRANSFER_CLIENT_READ);
    registerTransferCb(hylinkSendDevAttr, TRANSFER_DEVATTR);
    registerSystemCb(hylinkHeart, CMD_HEART);

    registerSystemCb(hylinkSendDevInfo, CMD_DEVSINFO);

    registerSystemCb(nativeFrameOpen, LAN_OPEN);

    runSystemCb(HYLINK_OPEN);
}
