#include "main.h"
#include "cloudLinkReport.h"
#include "cloudLink.h"
#include "linkkit_subdev.h"

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
            json = generateCloudJson(cloudLinkDev->eventAttr[i].key, hyLinkDev->attr[hyAttr].value, hyLinkDev->attr[hyAttr].valueType);
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
    logInfo("attr report cloudKey %s", cloudLinkDev->attr[i].cloudKey);
    char *json = NULL;
    if (strcmp(cloudLinkDev->attr[i].cloudKey, "version") == 0)
    {
        char value[64] = {0};
        sprintf(value, "TY_%s", hyLinkDev->attr[hyAttr].value);
        json = generateCloudJson(cloudLinkDev->attr[i].cloudKey, value, hyLinkDev->attr[hyAttr].valueType);
    }
    else
    {
        json = generateCloudJson(cloudLinkDev->attr[i].cloudKey, hyLinkDev->attr[hyAttr].value, hyLinkDev->attr[hyAttr].valueType);
    }

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
            res = runTransferCb(cloudLinkDev->alinkInfo.device_name, hyLinkDev->online, TRANSFER_SUBDEV_LINE);
            if (res < 0)
                logError("cloudReport TRANSFER_SUBDEV_LINE onlink error:%d", res);
            else
                logInfo("cloudReport TRANSFER_SUBDEV_LINE onlink success:%d", res);
        }
        return res;
    }

    if (hyLinkDev->online == SUBDEV_ONLINE)
    {
        res = cloudAttrReport(cloudLinkDev, hyAttr);
    }

    return res;
}
