#include "cloudLinkListFunc.h"
#include "cloudLinkReport.h"
#include "cloudLink.h"

static pthread_mutex_t cloudLink_mutex;

static int cloudSubDevStatus(CloudLinkDev *cloudLinkDev, unsigned int status)
{
    int res = -1, i;
    if (cloudLinkDev == NULL)
    {
        logError("cloudLinkDev is NULL");
        return res;
    }
    if (strcmp(STR_GATEWAY_MODELID, cloudLinkDev->modelId) == 0)
        return 0;

    HyLinkDev *hylinkDev = hylinkListGetById(cloudLinkDev->brgDevInfo.sn);
    logWarn("cloudSubDevStatus:id:%s,online:%d,status:%d", cloudLinkDev->brgDevInfo.sn, hylinkDev->online, status);

    res = HilinkSyncBrgDevStatus(cloudLinkDev->brgDevInfo.sn, status);
    if (cloudLinkDev->cloudLinkSubDevLen != 0)
    {
        for (i = 0; i < cloudLinkDev->cloudLinkSubDevLen; ++i)
        {
            logWarn("cloudSubDevStatus: sub id:%s,status:%d", cloudLinkDev->cloudLinkSubDev[i].brgDevInfo.sn, status);
            res = HilinkSyncBrgDevStatus(cloudLinkDev->cloudLinkSubDev[i].brgDevInfo.sn, status);
        }
    }
    if (res < 0)
    {
        logError("HilinkSyncBrgDevStatus id:%s error:%d\n", cloudLinkDev->brgDevInfo.sn, res);
        return res;
    }
    if (status == SUBDEV_RESTORE)
    {
        cloudLinkListDelDev(cloudLinkDev);
    }
    else if (status == SUBDEV_OFFLINE)
    {
#ifdef HILINK_REPORT_SYNC
        cloudLinkDev->hilink_now_online = SUBDEV_OFFLINE;
#endif
    }
    return 0;
}

static int cloudSubDevLink(void *id, unsigned int status)
{
    const char *devId = (const char *)id;
    pthread_mutex_lock(&cloudLink_mutex);
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
            logError("cloudSubDevLink id:%s not exist", devId);
            goto fail;
        }

        res = cloudSubDevStatus(cloudLinkDev, status);
    }
    pthread_mutex_unlock(&cloudLink_mutex);
    return res;
fail:
    pthread_mutex_unlock(&cloudLink_mutex);
    return -1;
}

static int systemReset(void)
{
    runTransferCb(NULL, SUBDEV_RESTORE, TRANSFER_SUBDEV_LINE);
    cloudLinkClose();
    exit(0);
    return 0;
}

static int systemRestart(void)
{
    cloudLinkClose();
    sync();
    reboot(RB_AUTOBOOT);
    return 0;
}

void cloudLinkClose(void)
{
    runTransferCb(NULL, SUBDEV_OFFLINE, TRANSFER_SUBDEV_LINE);
    hylinkClose();

    cloudLinkListEmpty();
    cloudLinkListDestroy();
    pthread_mutex_destroy(&cloudLink_mutex);
}

void cloudLinkOpen(void)
{
    registerSystemCb(systemRestart, SYSTEM_RESTART);
    registerSystemCb(systemReset, SYSTEM_RESET);

    registerTransferCb(cloudSubDevLink, TRANSFER_SUBDEV_LINE);
    registerTransferCb(cloudReport, TRANSFER_CLOUD_REPORT);

    pthread_mutex_init(&cloudLink_mutex, NULL);
    cloudLinkListInit();
}
