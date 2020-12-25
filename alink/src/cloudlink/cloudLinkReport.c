#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "logFunc.h"
#include "commonFunc.h"
#include "frameCb.h"
#include "hylinkListFunc.h"
#include "hylinkSubDev.h"
#include "cloudLinkReport.h"
#include "cloudLink.h"

#include "linkkit_subdev.h"

char *getCloudJson(char *cloudKey, char *hyValue, unsigned char valueType)
{
    if (strlen(cloudKey) == 0)
    {
        logError("cloudKey is len 0");
        return NULL;
    }
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

        char *json = getCloudJson(cloudLinkDev->eventAttr[i].key, hyLinkDev->attr[hyAttr].value, hyLinkDev->attr[hyAttr].valueType);

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
        user_post_event(cloudLinkDev->id, cloudLinkDev->eventAttr[i].eventId, json);
    quit:
        if (json != NULL)
        {
            cJSON_free(json);
        }
        return i;
    }
    return -1;
}
static int cloudSingleAttrReport(HyLinkDev *hyLinkDev, CloudLinkDev *cloudLinkDev, const int hyAttr)
{
    int i;

    for (i = 0; i < cloudLinkDev->attrLen; ++i)
    {
        if (strcmp(hyLinkDev->attr[hyAttr].hyKey, cloudLinkDev->attr[i].hyKey) == 0)
        {
            break;
        }
    }
    if (i == cloudLinkDev->attrLen)
        goto event;

    char *json = getCloudJson(cloudLinkDev->attr[i].cloudKey, hyLinkDev->attr[hyAttr].value, hyLinkDev->attr[hyAttr].valueType);
    if (json != NULL)
    {
        linkkit_user_post_property(cloudLinkDev->id, json);
        cJSON_free(json);
    }
    cloudSingleEventReport(hyLinkDev, cloudLinkDev, hyAttr);
    return i;
event:
    return cloudSingleEventReport(hyLinkDev, cloudLinkDev, hyAttr);
}

int cloudAttrReport(CloudLinkDev *cloudLinkDev, const int hyAttr)
{
    int res = 0;
    HyLinkDev *hyLinkDev = hylinkListGetById(cloudLinkDev->alinkInfo.device_name);
    if (hyLinkDev == NULL)
    {
        logError("hyLinkDev is null");
        return -1;
    }

    if (hyAttr == ATTR_REPORT_ALL)
    {
        for (int i = 0; i < cloudLinkDev->attrLen; ++i)
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

int cloudUpdate(HyLinkDev *hyLinkDev, const unsigned int hyAttr)
{

    logInfo("cloudUpdate hyAttr:%d,devid:%s,modelid:%s", hyAttr, hyLinkDev->devId, hyLinkDev->modelId);

    int res = 0;

    CloudLinkDev *cloudLinkDev = cloudLinkListGetById(hyLinkDev->devId);

    if (cloudLinkDev == NULL)
    {
        cloudLinkDev = (CloudLinkDev *)addProfileDev(ALILINK_PROFILE_PATH, hyLinkDev->devId, hyLinkDev->modelId, cloudLinkParseJson);
        if (cloudLinkDev == NULL)
        {
            logError("addProfileDev error : The corresponding file is missing");
            return -1;
        }
        if (hyLinkDev->online == SUBDEV_ONLINE)
        {
            res = runTransferCb(cloudLinkDev->alinkInfo.device_name, hyLinkDev->online, TRANSFER_SUBDEV_LINE);
            if (res < 0)
                logError("cloudUpdate TRANSFER_SUBDEV_LINE onlink error:%d", res);
            else
                logInfo("cloudUpdate TRANSFER_SUBDEV_LINE onlink success:%d", res);
        }
        return res;
    }

    if (hyLinkDev->online == SUBDEV_ONLINE)
    {
        res = cloudAttrReport(cloudLinkDev, hyAttr);
    }

    return res;
}
