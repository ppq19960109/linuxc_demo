#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/reboot.h>

#include "logFunc.h"
#include "frameCb.h"
#include "hylink.h"
#include "cloudLinkReport.h"
#include "cloudLinkCtrl.h"
#include "cloudLink.h"

#include "linkkit_app_gateway.h"
#include "linkkit_subdev.h"

static CloudLinkControl g_CloudLinkControl;

void cloudLinkGatewayInit(void)
{
    g_CloudLinkControl.cloudLinkGateway = malloc(sizeof(CloudLinkDev));

    CloudLinkDev *cloudLinkDev = g_CloudLinkControl.cloudLinkGateway;
    memset(cloudLinkDev, 0, sizeof(CloudLinkDev));

    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    cloudLinkDev->devInfo.id = user_example_ctx->master_devid;

    cloudLinkDev->devSvcNum = g_GatewaAttr.attrLen;
    cloudLinkDev->devSvc = malloc(sizeof(DevSvc) * cloudLinkDev->devSvcNum);
    memset(cloudLinkDev->devSvc, 0, sizeof(DevSvc) * cloudLinkDev->devSvcNum);
    for (int i = 0; i < cloudLinkDev->devSvcNum; ++i)
        cloudLinkDev->devSvc[i].svcId = g_GatewaAttr.attr[i];
}

void cloudLinkInit(void)
{
    pthread_mutex_init(&g_CloudLinkControl.mutex, NULL);
    cloudLinkListInit(&g_CloudLinkControl.head);
    cloudLinkGatewayInit();
}

void cloudLinkDestory(void)
{
    cloudLinkListEmpty();
    cloudLinkListDestroy();
    cloudLinkDevFree(g_CloudLinkControl.cloudLinkGateway);
    pthread_mutex_destroy(&g_CloudLinkControl.mutex);
}

CloudLinkControl *cloudLinkControlGet(void)
{
    return &g_CloudLinkControl;
}
//-----------------------------------------------------
int reportGateWayInfo(const char *deviceName, const char *productKey, const int result)
{
    int res;
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "Command", "Report");
    cJSON_AddStringToObject(root, "FrameNumber", "00");
    cJSON_AddStringToObject(root, "Type", "Register");
    cJSON_AddStringToObject(root, "DeviceNum", "0");
    cJSON *Data = cJSON_AddArrayToObject(root, "Data");
    cJSON *DataList = cJSON_CreateObject();
    cJSON_AddItemToArray(Data, DataList);

    cJSON_AddStringToObject(DataList, "DeviceName", deviceName);
    cJSON_AddStringToObject(DataList, "ProductKey", productKey);
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", result);
    cJSON_AddStringToObject(DataList, "Result", buf);
    char *json = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);

    cJSON *ReportInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(ReportInfo, "ReportGateWayInfo", json);
    char *info = cJSON_PrintUnformatted(ReportInfo);
    // EXAMPLE_TRACE("info:%s", info);
    user_post_event(g_CloudLinkControl.cloudLinkGateway->devInfo.id, "GateWayInfo", info);

    cJSON_Delete(ReportInfo);

    free(info);
    free(json);

    return res;
}
static int cloudSubDevStatus(CloudLinkDev *cloudLinkDev, int status)
{

    int res = -1;
    if (cloudLinkDev == NULL)
        return res;
    HylinkDev *hylinkDev = hylinkListGetById(cloudLinkDev->devInfo.sn);
    logWarn("cloudSubDevStatus:id:%s,%d,%d\n", cloudLinkDev->devInfo.sn, hylinkDev->Online, status);
    if (hylinkDev->Online == SUBDEV_OFFLINE && status != SUBDEV_RESTORE)
        return res;

    res = linkkit_subdev_status(&cloudLinkDev->devInfo.meta_info, &cloudLinkDev->devInfo.id, status);
    if (res < 0)
    {
        logError("linkkit_subdev_status id:%s error:%d\n", cloudLinkDev->devInfo.sn, res);
        return res;
    }

    if (status == SUBDEV_RESTORE)
    {
        cloudLinkListDelDev(cloudLinkDev);
    }
    else if (status == SUBDEV_ONLINE)
    {
        cloudAttrReport(cloudLinkDev, ATTR_REPORT_ALL);
        reportGateWayInfo(cloudLinkDev->devInfo.meta_info.device_name, cloudLinkDev->devInfo.meta_info.product_key, res);
    }
    else
    {
        /* code */
    }
}

static int cloudSubDevLink(char *sn, int status)
{
    pthread_mutex_lock(&g_CloudLinkControl.mutex);
    int res = 0;
    CloudLinkDev *cloudLinkDev;
    if (sn == NULL)
    {
#ifndef CLOUD_USER_KHASH
        CloudLinkDev *next;
        cloudLinkListEachEntrySafe(cloudLinkDev, next, &g_CloudLinkControl.head, cloudLinkNode)
        {
            cloudSubDevStatus(cloudLinkDev, status);
        }
#else
        cloudLink_kh_foreach_value(cloudLinkDev)
        {
            logWarn("cloudLink_kh_foreach_value");
            cloudSubDevStatus(cloudLinkDev, status);
        }
#endif
    }
    else
    {
        cloudLinkDev = cloudLinkListGetById(sn);
        if (cloudLinkDev == NULL)
        {
            logError("cloudSubDevLink sn:%s not exist", sn);
            goto fail;
        }

        res=cloudSubDevStatus(cloudLinkDev, status);
    }
    pthread_mutex_unlock(&g_CloudLinkControl.mutex);
    return res;
fail:
    pthread_mutex_unlock(&g_CloudLinkControl.mutex);
    return -1;
}

void cloudLinkClose(void)
{
    // runTransferCb(NULL, SUBDEV_OFFLINE, TRANSFER_SUBDEV_LINE);
    runSystemCb(HYLINK_CLOSE);

    cloudLinkDestory();
    // sleep(2);
}

void cloudLinkReset(void)
{
    runTransferCb(NULL, SUBDEV_RESTORE, TRANSFER_SUBDEV_LINE);
    runSystemCb(HYLINK_RESET);

    system("sh /userdata/app/restore.sh alink &");
}

void cloudLinkMain(void)
{
    registerSystemCb(cloudLinkReset, SYSTEM_RESET);

    registerTransferCb(cloudSubDevLink, TRANSFER_SUBDEV_LINE);
    registerTransferCb(cloudUpdate, TRANSFER_CLOUD_REPORT);

    cloudLinkInit();
    hylinkMain();

    runSystemCb(HYLINK_OPEN);
}
