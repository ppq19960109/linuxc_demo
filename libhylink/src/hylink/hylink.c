#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commonFunc.h"
#include "frameCb.h"
#include "rkDriver.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "hylinkSend.h"
#include "hylinkSubDev.h"
#include "logFunc.h"

#include "hytool.h"

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
    return 0;
}

static int hylinkDestory(void)
{
    hytoolClose();

    hylinkListEmpty();
    runSystemCb(RK_DRIVER_CLOSE);

    pthread_mutex_destroy(&s_hylink.mutex);
    return 0;
}

void hylinkMain(void)
{
    registerSystemCb(hylinkDestory, HYLINK_CLOSE);

    registerTransferCb(hylinkRecv, TRANSFER_CLIENT_READ);
    registerTransferCb(hylinkSendDevAttr, TRANSFER_DEVATTR);
    registerSystemCb(hylinkHeart, CMD_HEART);

    registerSystemCb(hylinkSendDevInfo, CMD_DEVSINFO);

    hylinkInit();
    hytoolOpen();
}
