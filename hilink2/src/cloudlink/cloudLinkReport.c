#include "cloudLinkReport.h"

static int hilink_report(const char *sn, const char *svcId, const char *key, const char *value, unsigned char valueType, const char hilink_now_online)
{
    char *json;
    if (strlen(svcId) == 0 || strlen(key) == 0)
    {
        logError("cloudSid cloudKey is len 0");
        return -1;
    }
    logInfo("hilink_report sn:%s,svcId:%s,key:%s", sn, svcId, key);
#ifdef HILINK_REPORT_SYNC
    goto sync;
#else
    if (strcmp(STR_GATEWAY_DEVID, sn) == 0)
        goto sync;
    else
        HilinkUploadBrgDevCharState(sn, svcId);
#endif
sync:
#ifdef HILINK_REPORT_SYNC
    if (!hilink_now_online)
    {
        logWarn("cloud dev offline");
        return 0;
    }
#endif
    json = generateCloudJson(key, value, valueType);
    if (json != NULL)
    {
        logInfo("sync hilink_report json:%s", json);
        HilinkReportBrgDevCharState(sn, svcId, json, strlen(json), getpid());
        cJSON_free(json);
    }

    return 0;
}

static int cloudSingleSubDevAttrReport(HyLinkDev *hyLinkDev, CloudLinkDev *cloudLinkDev, const int hyAttr)
{
    CloudLinkSubDev *subDev = cloudLinkDev->cloudLinkSubDev;
    char subDevLen = cloudLinkDev->cloudLinkSubDevLen;
    if (subDev == NULL || subDevLen == 0)
    {
        logError("subDev is NULL");
        return -1;
    }
    int report_num = 0;
    int i, j;
    for (i = 0; i < subDevLen; ++i)
    {
        for (j = 0; j < subDev[i].attrLen; ++j)
        {
            if (strcmp(hyLinkDev->attr[hyAttr].hyKey, subDev[i].attr[j].hyKey) == 0)
            {
                hilink_report(subDev[i].brgDevInfo.sn, subDev[i].attr[j].cloudSid, subDev[i].attr[j].cloudKey, hyLinkDev->attr[hyAttr].value, hyLinkDev->attr[hyAttr].valueType, cloudLinkDev->hilink_now_online);
                ++report_num;
            }
        }
    }
    if (report_num)
        return 0;
    logError("subDev is hyKey Not found");
    return -1;
}
static int cloudSingleAttrReport(HyLinkDev *hyLinkDev, CloudLinkDev *cloudLinkDev, const int hyAttr)
{
    int report_num = 0;
    int i;
    for (i = 0; i < cloudLinkDev->attrLen; ++i)
    {
        if (strcmp(hyLinkDev->attr[hyAttr].hyKey, cloudLinkDev->attr[i].hyKey) == 0)
        {
            hilink_report(cloudLinkDev->brgDevInfo.sn, cloudLinkDev->attr[i].cloudSid, cloudLinkDev->attr[i].cloudKey, hyLinkDev->attr[hyAttr].value, hyLinkDev->attr[hyAttr].valueType, cloudLinkDev->hilink_now_online);
        }
        ++report_num;
    }
    if (report_num)
        return 0;
    return cloudSingleSubDevAttrReport(hyLinkDev, cloudLinkDev, hyAttr);
}

int cloudAttrReport(HyLinkDev *hyLinkDev, CloudLinkDev *cloudLinkDev, const int hyAttr)
{
    int res = 0;
    if (hyAttr == ATTR_REPORT_ALL)
    {
        for (int i = 0; i < hyLinkDev->attrLen; ++i)
        {
            cloudSingleAttrReport(hyLinkDev, cloudLinkDev, i);
        }
    }
    else
    {
        res = cloudSingleAttrReport(hyLinkDev, cloudLinkDev, hyAttr);
    }
    return res;
}
/*********************************************************************************
  *Function:  cloudReport
  * Description： report hylink device information
  *Input:  
    hyLinkDev:hylink device information
    hyAttr:index of the attributes reported by hylink device,0xFF means report all attributes
  *Return:  0:success otherwise:fail
**********************************************************************************/
int cloudReport(void *hydev, unsigned int hyAttr)
{
    if (hydev == NULL)
    {
        logError("cloudReport error : hydev is NULL");
        return -1;
    }
    HyLinkDev *hyLinkDev = (HyLinkDev *)hydev;
    logInfo("cloudReport hyAttr:%d,devid:%s,modelid:%s", hyAttr, hyLinkDev->devId, hyLinkDev->modelId);

    int res;

    CloudLinkDev *cloudLinkDev = cloudLinkListGetById(hyLinkDev->devId);

    if (cloudLinkDev == NULL)
    {
        cloudLinkDev = (CloudLinkDev *)addProfileDev(PROFILE_PATH, hyLinkDev->devId, hyLinkDev->modelId, cloudLinkParseJson);
        if (cloudLinkDev == NULL)
        {
            logError("addProfileDev error : The corresponding file is missing");
            return -1;
        }
        if (hyLinkDev->online == SUBDEV_ONLINE)
        {
            res = runTransferCb(cloudLinkDev->brgDevInfo.sn, hyLinkDev->online, TRANSFER_SUBDEV_LINE);
            if (res < 0)
                logError("cloudReport online error:%d", res);
            else
                logInfo("cloudReport online success:%d", res);
        }
        return res;
    }
    // if (strcmp(STR_GATEWAY_MODELID, hyLinkDev->modelId) == 0)
    // {
    //     logInfo("gatway not cloudReport");
    //     return 0;
    // }
    if (hyLinkDev->online == SUBDEV_ONLINE)
    {
        res = cloudAttrReport(hyLinkDev, cloudLinkDev, hyAttr);
    }

    return res;
}
