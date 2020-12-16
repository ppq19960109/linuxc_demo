#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logFunc.h"
#include "cJSON.h"
#include "commonFunc.h"
#include "frameCb.h"
#include "hylinkListFunc.h"
#include "hylinkSubDev.h"
#include "hylinkSend.h"
#include "hylinkRecv.h"

#include "cloudLink.h"
#include "cloudLinkReport.h"
#include "cloudLinkCtrl.h"

static void getValueForJson(cJSON *val, char *dst)
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

int cloudLinkGatewayCtrl(void *payload)
{
    cJSON *root = cJSON_Parse(payload);
    if (root == NULL)
        return -1;
    for (int i = 0; i < g_GatewaAttr.attrLen; i++)
    {
        if (cJSON_HasObjectItem(root, g_GatewaAttr.attr[i]))
        {
            cJSON *val;
            switch (i)
            {
            case 0:
                val = cJSON_GetObjectItem(root, g_GatewaAttr.attr[i]);
                if (val == NULL)
                    break;
                if (val->valueint > 0)
                {
                    runSystemCb(CMD_NETWORK);
                }
                break;
            }
        }
    }
    return 0;
}

//cloud转换成hanyar的json格式
int cloudLinkCtrl(void *sn, const char *payload)
{

    CloudLinkDev *cloudLinkDev = cloudLinkControlGet()->cloudLinkGateway;
    logInfo("cloudLinkCtrl:%d,%d", cloudLinkDev->devInfo.id, (int)sn);
    if (cloudLinkDev->devInfo.id == (int)sn)
    {
        return cloudLinkGatewayCtrl(payload);
    }
    cloudLinkDev = cloudLinkListGetBySn((int)sn);
    if (cloudLinkDev == NULL)
        return -1;

    HylinkDevSendData hylinkDevSend = {0};
    int i;
    hylinkDevSend.FrameNumber = 0;

    strcpy(hylinkDevSend.Data.DeviceId, cloudLinkDev->devInfo.sn);

    int devType = cloudLinkDev->devType;
    strcpy(hylinkDevSend.Data.ModelId, g_SLocalModel.attr[devType]);

    cJSON *root = cJSON_Parse(payload);
    cJSON *val;

    strcpy(hylinkDevSend.Type, STR_CTRL);
    for (i = 0; i < g_SCloudAttr[devType].attrLen; ++i)
    {
        if (cJSON_HasObjectItem(root, g_SCloudAttr[devType].attr[i]))
        {
            int attrType = findStrIndex(g_SCloudAttr[devType].attr[i], g_SCloudAttr[devType].attr, g_SCloudAttr[devType].attrLen);
            if (attrType < 0)
            {
                // if (devType == TOUCHPANEL02U2)
                // {
                // }
                // else
                // {
                    goto fail;
                // }
            }
            strcpy(hylinkDevSend.Data.Key, g_SLocalAttr[devType].attr[i]);

            val = cJSON_GetObjectItem(root, g_SCloudAttr[devType].attr[i]);
            break;
        }
    }
    getValueForJson(val, hylinkDevSend.Data.Value);
    hylinkSend(&hylinkDevSend);
    cJSON_Delete(root);
    return 0;
fail:
    cJSON_Delete(root);
    logError("cloudLinkCtrl fail\n");
    return -1;
}

int cloudLinkServicCtrl(const int devid, const char *serviceid, const int serviceid_len, const char *request, char **response, int *response_len)
{
    int res = -1;
    /* Parse Root */
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        logError("JSON Parse Error");
        return -1;
    }
    CloudLinkDev *cloudLinkDev = cloudLinkListGetBySn(devid);
    if (cloudLinkDev == NULL)
    {
        logError("cloudLinkDev Null Error");
        return -1;
    }
    int devType = cloudLinkDev->devType;
    if (g_SCloudAttr[devType].attrCtrl == NULL)
    {
        logError("g_SCloudAttr Null Error");
        return -1;
    }
    int attrType = findStrnIndex(serviceid, serviceid_len, g_SCloudAttr[devType].attrCtrl, g_SCloudAttr[devType].attrCtrlLen);
    if (attrType < 0)
    {
        logError("attrType Null Error,devType:%d,%*.s,%d", devType, serviceid_len, serviceid, g_SCloudAttr[devType].attrCtrlLen);
        goto fail;
    }

    HylinkDevSendData hylinkDevSend = {0};
    hylinkDevSend.FrameNumber = 0;
    strcpy(hylinkDevSend.Data.DeviceId, cloudLinkDev->devInfo.sn);
    strcpy(hylinkDevSend.Data.ModelId, g_SLocalModel.attr[devType]);
    strcpy(hylinkDevSend.Type, STR_CTRL);

    if (g_SLocalAttr[devType].attrCtrl == NULL || attrType >= g_SLocalAttr[devType].attrCtrlLen)
    {
        logError("g_SLocalAttr Null Error");
        if (devType == TOUCHPANEL02U2)
        {
            cJSON *Key = cJSON_GetObjectItem(root, "Key");

            cJSON *Words = cJSON_GetObjectItem(root, "Words");
            cJSON *PictureCode = cJSON_GetObjectItem(root, "PictureCode");
            if (Words != NULL)
            {
                sprintf(hylinkDevSend.Data.Key, "SceName_%d", Key->valueint);
                getValueForJson(Words, hylinkDevSend.Data.Value);
            }
            else if (PictureCode != NULL)
            {
                sprintf(hylinkDevSend.Data.Key, "ScePhoto_%d", Key->valueint);
                getValueForJson(PictureCode, hylinkDevSend.Data.Value);
            }
            else
            {
                goto fail;
            }
        }
        else
            goto fail;
    }
    else
    {
        strcpy(hylinkDevSend.Data.Key, g_SLocalAttr[devType].attrCtrl[attrType]);
        cJSON *val = cJSON_GetObjectItem(root, g_SLocalAttr[devType].attrCtrl[attrType]);
        getValueForJson(val, hylinkDevSend.Data.Value);
    }
    res = hylinkSend(&hylinkDevSend);

    /* Send Service Response To Cloud */
    // *response_len = 0;
    // *response = (char *)HAL_Malloc(*response_len);
    // if (*response == NULL)
    // {
    //     EXAMPLE_TRACE("Memory Not Enough");
    //     return -1;
    // }
    // memset(*response, 0, *response_len);
    // *response_len = strlen(*response);

    cJSON_Delete(root);
    return res;
fail:
    cJSON_Delete(root);
    return -1;
}

int cloudLinkDevDel(void *sn)
{
    logWarn("cloudLinkDevDel:%s", sn);
    CloudLinkDev *cloudLinkDev = cloudLinkListGetById(sn);
    runTransferCb(cloudLinkDev->devInfo.sn, SUBDEV_RESTORE, TRANSFER_SUBDEV_LINE);

    return hylinkDelDev(sn);
}
