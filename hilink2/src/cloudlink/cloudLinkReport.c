#include "cloudLinkReport.h"

char *getCloudJson(const char *cloudKey, const char *hyValue, const unsigned char valueType)
{
    cJSON *root = cJSON_CreateObject();

    switch (valueType)
    {
    case LINK_VALUE_TYPE_ENUM:
        cJSON_AddNumberToObject(root, cloudKey, *hyValue);
        break;
    case LINK_VALUE_TYPE_NUM:
        cJSON_AddNumberToObject(root, cloudKey, *(int *)hyValue);
        break;
    case LINK_VALUE_TYPE_STRING:
        cJSON_AddStringToObject(root, cloudKey, hyValue);
        break;
    default:
        goto fail;
        break;
    }
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json;
fail:
    cJSON_Delete(root);
    return NULL;
}
static int hilink_report(const char *sn, const char *svcId, const char *key, const char *value, unsigned char valueType)
{
    if (strlen(svcId) == 0 || strlen(key) == 0)
    {
        logError("cloudSid cloudKey is len 0");
        return -1;
    }
#ifdef HILINK_REPORT_ASYNC
    HilinkUploadBrgDevCharState(sn, svcId);
#else
    logInfo("hilink_report key:%s", key);
    char *json = getCloudJson(key, value, valueType);
    if (json != NULL)
    {
        HilinkReportBrgDevCharState(sn, svcId, json, strlen(json), getpid());
        cJSON_free(json);
    }
#endif
    return 0;
}

static int cloudSingleEventReport(HyLinkDev *hyLinkDev, CloudLinkDev *cloudLinkDev, const int hyAttr)
{
    if (cloudLinkDev->eventAttr != NULL)
    {
        int i;
        for (i = 0; i < cloudLinkDev->eventAttrLen; ++i)
        {
            if (strcmp(hyLinkDev->attr[hyAttr].hyKey, cloudLinkDev->eventAttr[i].hyKey) == 0)
            {
                break;
            }
        }
        if (i == cloudLinkDev->eventAttrLen)
            return -1;
        char *json = NULL;
        if (strlen(cloudLinkDev->eventAttr[i].key) != 0)
        {
            json = getCloudJson(cloudLinkDev->eventAttr[i].key, hyLinkDev->attr[hyAttr].value, hyLinkDev->attr[hyAttr].valueType);
        }
        if (hyLinkDev->attr[hyAttr].valueType == LINK_VALUE_TYPE_ENUM)
        {
            if (*hyLinkDev->attr[hyAttr].value == 0)
                goto quit;
        }
        else if (hyLinkDev->attr[hyAttr].valueType == LINK_VALUE_TYPE_NUM)
        {
            if (*(int *)hyLinkDev->attr[hyAttr].value == 0)
                goto quit;
        }
        //event
    quit:
        if (json != NULL)
        {
            cJSON_free(json);
        }
        return i;
    }
    return -1;
}
static int cloudSingleSubDevAttrReport(HyLinkDev *hyLinkDev, CloudLinkDev *cloudLinkDev, const int hyAttr)
{
    CloudLinkSubDev *subDev = cloudLinkDev->cloudLinkSubDev;
    char subDevLen = cloudLinkDev->cloudLinkSubDevLen;
    if (subDev == NULL || subDevLen == 0)
        return -1;

    int i, j;
    for (i = 0; i < subDevLen; ++i)
    {
        for (j = 0; j < subDev[i].attrLen; ++j)
        {
            if (strcmp(hyLinkDev->attr[hyAttr].hyKey, cloudLinkDev->attr[i].hyKey) == 0)
            {
                break;
            }
        }

        hilink_report(subDev[i].brgDevInfo.sn, subDev[i].attr[j].cloudSid, subDev[i].attr[j].cloudKey, hyLinkDev->attr[hyAttr].value, hyLinkDev->attr[hyAttr].valueType);
    }
    return 0;
}
static int cloudSingleAttrReport(HyLinkDev *hyLinkDev, CloudLinkDev *cloudLinkDev, const int hyAttr)
{
    int i;

    for (i = 0; i < cloudLinkDev->attrLen; ++i)
    {
        if (strcmp(hyLinkDev->attr[hyAttr].hyKey, cloudLinkDev->attr[i].hyKey) == 0)
        {
            if (hyLinkDev->first_online_report == 1 && cloudLinkDev->attr[i].repeat == ONLINE_NON_REPORT_AND_NON_REPEAT_REPORT)
            {
                continue;
            }
            hilink_report(cloudLinkDev->brgDevInfo.sn, cloudLinkDev->attr[i].cloudSid, cloudLinkDev->attr[i].cloudKey, hyLinkDev->attr[hyAttr].value, hyLinkDev->attr[hyAttr].valueType);
        }
    }
    if (i == cloudLinkDev->attrLen)
    {
        cloudSingleSubDevAttrReport(hyLinkDev, cloudLinkDev, hyAttr);
        goto event;
    }

    cloudSingleEventReport(hyLinkDev, cloudLinkDev, hyAttr);
    return i;
event:
    return cloudSingleEventReport(hyLinkDev, cloudLinkDev, hyAttr);
}

int cloudAttrReport(CloudLinkDev *cloudLinkDev, const int hyAttr)
{
    int res = 0;
    HyLinkDev *hyLinkDev = hylinkListGetById(cloudLinkDev->brgDevInfo.sn);
    if (hyLinkDev == NULL)
    {
        logError("hyLinkDev is null");
        return -1;
    }

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
    hyLinkDev->first_online_report = 0;
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
    HyLinkDev *hyLinkDev = (HyLinkDev *)hydev;
    logInfo("cloudReport hyAttr:%d,devid:%s,modelid:%s", hyAttr, hyLinkDev->devId, hyLinkDev->modelId);

    int res = 0;

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
                logError("cloudReport TRANSFER_SUBDEV_LINE error:%d", res);
            else
                logInfo("cloudReport TRANSFER_SUBDEV_LINE success:%d", res);
        }
        return res;
    }

    if (hyLinkDev->online == SUBDEV_ONLINE)
    {
        res = cloudAttrReport(cloudLinkDev, hyAttr);
    }

    return res;
}
