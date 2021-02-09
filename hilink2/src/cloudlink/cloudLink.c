#include "cloudLinkListFunc.h"
#include "cloudLinkReport.h"
#include "cloudLinkCtrl.h"
#include "cloudLink.h"

static CloudLinkControl g_CloudLinkControl;

void cloudLinkInit(void)
{
    pthread_mutex_init(&g_CloudLinkControl.mutex, NULL);
    cloudLinkListInit();
}

void cloudLinkDestory(void)
{
    cloudLinkListEmpty();
    cloudLinkListDestroy();
    pthread_mutex_destroy(&g_CloudLinkControl.mutex);
}

static int cloudSubDevStatus(CloudLinkDev *cloudLinkDev, unsigned int status)
{
    int res = -1, i;
    if (cloudLinkDev == NULL)
        return res;
    if (strcmp(STR_GATEWAY_MODELID, cloudLinkDev->modelId) == 0)
        return 0;

    HyLinkDev *hylinkDev = hylinkListGetById(cloudLinkDev->brgDevInfo.sn);
    logWarn("cloudSubDevStatus:id:%s,%d,%d\n", cloudLinkDev->brgDevInfo.sn, hylinkDev->online, status);

    res = HilinkSyncBrgDevStatus(cloudLinkDev->brgDevInfo.sn, status);
    if (cloudLinkDev->cloudLinkSubDevLen != 0)
    {
        for (i = 0; i < cloudLinkDev->cloudLinkSubDevLen; ++i)
        {
            res = HilinkSyncBrgDevStatus(cloudLinkDev->cloudLinkSubDev[i].brgDevInfo.sn, status);
        }
    }
    if (res < 0)
    {
        logError("HilinkSyncBrgDevStatus id:%s error:%d\n", cloudLinkDev->brgDevInfo.sn, res);
        return res;
    }
    return 0;
}

static int cloudSubDevLink(void *id, unsigned int status)
{
    const char *devId = (const char *)id;
    pthread_mutex_lock(&g_CloudLinkControl.mutex);
    int res = 0;
    CloudLinkDev *cloudLinkDev = NULL;
    if (devId == NULL)
    {

        cloudLink_kh_foreach_value(cloudLinkDev)
        {
            logWarn("cloudLink_kh_foreach_value");
            cloudSubDevStatus(cloudLinkDev, status);
        }
    }
    else
    {
        cloudLinkDev = cloudLinkListGetById(devId);
        if (cloudLinkDev == NULL)
        {
            logError("cloudSubDevLink devId:%s not exist", devId);
            goto fail;
        }

        res = cloudSubDevStatus(cloudLinkDev, status);
    }
    pthread_mutex_unlock(&g_CloudLinkControl.mutex);
    return res;
fail:
    pthread_mutex_unlock(&g_CloudLinkControl.mutex);
    return -1;
}

void cloudLinkClose(void)
{
    runTransferCb(NULL, SUBDEV_OFFLINE, TRANSFER_SUBDEV_LINE);
    runSystemCb(HYLINK_CLOSE);

    cloudLinkDestory();
}

static int systemReset(void)
{
    runTransferCb(NULL, SUBDEV_RESTORE, TRANSFER_SUBDEV_LINE);
    cloudLinkClose();
    exit(0);
    return 0;
}

void cloudLinkMain(void)
{
    registerSystemCb(systemReset, SYSTEM_RESET);

    registerTransferCb(cloudSubDevLink, TRANSFER_SUBDEV_LINE);
    registerTransferCb(cloudReport, TRANSFER_CLOUD_REPORT);
    cloudLinkInit();
}
