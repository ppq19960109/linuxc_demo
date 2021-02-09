#include "main.h"
#include "cloudLink.h"
#include "cloudLinkReport.h"
#include "cloudLinkCtrl.h"

void getValueForJson(cJSON *val, char *dst)
{
    if (val->valuestring != NULL)
    {
        strcpy(dst, val->valuestring);
    }
    else
    {
        sprintf(dst, "%d", val->valueint);
    }
}
static int cloudLinkCtrlSend(const char *sn, const char *modelId, const char *hyType, const char *hyKey, cJSON *root, const char *cloudKey)
{
    HylinkDevSendData hylinkDevSend = {0};

    hylinkDevSend.FrameNumber = 0;

    strcpy(hylinkDevSend.Data.DeviceId, sn);
    strcpy(hylinkDevSend.Data.ModelId, modelId);

    strcpy(hylinkDevSend.Type, STR_CTRL);

    if (strlen(hyType) != 0)
        strcpy(hylinkDevSend.Type, hyType);

    strcpy(hylinkDevSend.Data.Key, hyKey);

    cJSON *val = cJSON_GetObjectItem(root, cloudKey);

    getValueForJson(val, hylinkDevSend.Data.Value);
    return hylinkSend(&hylinkDevSend);
}
static int cloudLinkSubDevCtrl(cJSON *root, CloudLinkDev *cloudLinkDev, const char *svcId)
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
            if (strcmp(cloudLinkDev->attr[i].cloudSid, svcId) == 0)
            {
                break;
            }
        }
        if (j == subDev[i].attrLen)
            return -1;
    }
    if (i == subDevLen)
        return -1;
    return cloudLinkCtrlSend(cloudLinkDev->brgDevInfo.sn, cloudLinkDev->modelId, subDev[i].attr[j].hyType, subDev[i].attr[j].hyKey, root, subDev[i].attr[j].cloudKey);
}
/*********************************************************************************
  *Function:  cloudLinkCtrl
  * Description： receive cloud control information
  *Input:  
    sn:cloud device id
    payload:cloud control information(json)
  *Return:  0:success otherwise:fail
**********************************************************************************/
int cloudLinkCtrl(void *sn, const char *svcId, const char *payload)
{
    int i, res;
    CloudLinkDev *cloudLinkDev = cloudLinkListGetById(sn);
    if (cloudLinkDev == NULL)
        return -1;

    cJSON *root = cJSON_Parse(payload);

    for (i = 0; i < cloudLinkDev->attrLen; ++i)
    {
        if (strcmp(cloudLinkDev->attr[i].cloudSid, svcId) == 0)
        {
            break;
        }
    }
    if (i == cloudLinkDev->attrLen)
    {
        res = cloudLinkSubDevCtrl(root, cloudLinkDev, svcId);
    }
    else
    {
        res = cloudLinkCtrlSend(cloudLinkDev->brgDevInfo.sn, cloudLinkDev->modelId, cloudLinkDev->attr[i].hyType, cloudLinkDev->attr[i].hyKey, root, cloudLinkDev->attr[i].cloudKey);
    }
    cJSON_Delete(root);
    return res;
}

int cloudLinkDevDel(const char *sn)
{
    logWarn("cloudLinkDevDel:%s", sn);
    CloudLinkDev *cloudLinkDev = cloudLinkListGetById(sn);
    if (cloudLinkDev != NULL)
        runTransferCb(cloudLinkDev->brgDevInfo.sn, SUBDEV_RESTORE, TRANSFER_SUBDEV_LINE);

    return hylinkDelDev(sn);
}
