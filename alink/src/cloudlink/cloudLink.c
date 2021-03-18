#include "cloudLinkListFunc.h"
#include "cloudLinkReport.h"
#include "cloudLinkCtrl.h"
#include "cloudLink.h"

#include "scene.h"

static pthread_mutex_t cloudLink_mutex;

//-----------------------------------------------------
int reportGateWayInfo(const char *deviceName, const char *productKey, const int result)
{
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
    CloudLinkDev *cloudLinkDev = cloudLinkListGetById(STR_GATEWAY_DEVID);
    if (cloudLinkDev == NULL)
        goto fail;
    user_post_event(cloudLinkDev->id, "GateWayInfo", info);

    cJSON_Delete(ReportInfo);
    free(info);
    free(json);

    return 0;
fail:
    cJSON_Delete(ReportInfo);
    free(info);
    free(json);

    return -1;
}
static int cloudSubDevStatus(CloudLinkDev *cloudLinkDev, unsigned int status)
{

    int res = -1;
    if (cloudLinkDev == NULL)
        return res;

    HyLinkDev *hylinkDev = hylinkListGetById(cloudLinkDev->alinkInfo.device_name);
    logWarn("cloudSubDevStatus:id:%s,%d,%d\n", cloudLinkDev->alinkInfo.device_name, hylinkDev->online, status);

    res = linkkit_subdev_status(&cloudLinkDev->alinkInfo, &cloudLinkDev->id, status);
    if (res < 0)
    {
        logError("linkkit_subdev_status id:%s error:%d\n", cloudLinkDev->alinkInfo.device_name, res);
        return res;
    }

    if (status == SUBDEV_RESTORE)
    {
        cloudLinkListDelDev(cloudLinkDev);
    }
    else if (status == SUBDEV_ONLINE)
    {
        logWarn("cloudLinkDev:%s", cloudLinkDev->alinkInfo.device_name);
        cloudAttrReport(cloudLinkDev, ATTR_REPORT_ALL);
        reportGateWayInfo(cloudLinkDev->alinkInfo.device_name, cloudLinkDev->alinkInfo.product_key, res);
    }
    else
    {
        /* code */
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
            logError("cloudSubDevLink devId:%s not exist", devId);
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

void cloudLinkClose(void)
{
    runTransferCb(NULL, SUBDEV_OFFLINE, TRANSFER_SUBDEV_LINE);
    hylinkClose();

    databaseClose();

    cloudLinkListEmpty();
    cloudLinkListDestroy();
    pthread_mutex_destroy(&cloudLink_mutex);
}

static int systemReset(void)
{
    runTransferCb(NULL, SUBDEV_RESTORE, TRANSFER_SUBDEV_LINE);
    databseReset();
    cloudLinkClose();
    exit(0);
    return 0;
}

void cloudLinkOpen(void)
{
    registerSystemCb(systemReset, SYSTEM_RESET);

    registerTransferCb(cloudSubDevLink, TRANSFER_SUBDEV_LINE);
    registerTransferCb(cloudReport, TRANSFER_CLOUD_REPORT);
    registerTransferCb(sceneReport, TRANSFER_SCENE_REPORT);

    pthread_mutex_init(&cloudLink_mutex, NULL);
    cloudLinkListInit();

    databaseInit();
}
