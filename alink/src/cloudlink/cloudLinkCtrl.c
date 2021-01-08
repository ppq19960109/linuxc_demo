#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logFunc.h"
#include "cJSON.h"
#include "base64.h"
#include "commonFunc.h"
#include "frameCb.h"
#include "hylinkListFunc.h"
#include "hylinkSubDev.h"
#include "hylinkSend.h"
#include "hylinkRecv.h"

#include "cloudLink.h"
#include "cloudLinkReport.h"
#include "cloudLinkCtrl.h"

#include "scene.h"

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

/*********************************************************************************
  *Function:  cloudLinkCtrl
  * Description： receive cloud control information
  *Input:  
    sn:cloud device id
    payload:cloud control information(json)
  *Return:  0:success otherwise:fail
**********************************************************************************/
int cloudLinkCtrl(void *sn, const char *payload)
{
    int i, res;
    CloudLinkDev *cloudLinkDev = cloudLinkListGetBySn((int)sn);
    if (cloudLinkDev == NULL)
        return -1;

    HylinkDevSendData hylinkDevSend = {0};

    hylinkDevSend.FrameNumber = 0;

    strcpy(hylinkDevSend.Data.DeviceId, cloudLinkDev->alinkInfo.device_name);
    strcpy(hylinkDevSend.Data.ModelId, cloudLinkDev->modelId);

    cJSON *root = cJSON_Parse(payload);
    cJSON *val;

    strcpy(hylinkDevSend.Type, STR_CTRL);

    for (i = 0; i < cloudLinkDev->attrLen; ++i)
    {
        if (cJSON_HasObjectItem(root, cloudLinkDev->attr[i].cloudKey))
        {
            break;
        }
    }
    if (i == cloudLinkDev->attrLen)
    {
        logError("cloudKey not exist");
        goto fail;
    }
    if (strlen(cloudLinkDev->attr[i].hyType) != 0)
        strcpy(hylinkDevSend.Type, cloudLinkDev->attr[i].hyType);

    strcpy(hylinkDevSend.Data.Key, cloudLinkDev->attr[i].hyKey);

    val = cJSON_GetObjectItem(root, cloudLinkDev->attr[i].cloudKey);

    getValueForJson(val, hylinkDevSend.Data.Value);
    res = hylinkSend(&hylinkDevSend);
    cJSON_Delete(root);
    return res;
fail:
    cJSON_Delete(root);
    return -1;
}

int cloudLinkServicCtrl(const int devid, const char *serviceid, const int serviceid_len, const char *request, char **response, int *response_len)
{
    int res = -1, i;

    if (strncmp(serviceid, "TriggerGatewayService", serviceid_len) == 0)
    {
        return localScene(devid, serviceid, serviceid_len, request, response, response_len);
    }
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
    logDebug("serviceid:%s", serviceid);
    for (i = 0; i < cloudLinkDev->serverAttrLen; ++i)
    {
        if (strncmp(serviceid, cloudLinkDev->serverAttr[i].serverId, serviceid_len) == 0)
        {
            break;
        }
    }
    if (i == cloudLinkDev->serverAttrLen)
    {
        logError("server cloudKey not exist:%s", serviceid);
        goto fail;
    }
    HylinkDevSendData hylinkDevSend = {0};
    hylinkDevSend.FrameNumber = 0;
    strcpy(hylinkDevSend.Data.DeviceId, cloudLinkDev->alinkInfo.device_name);
    strcpy(hylinkDevSend.Data.ModelId, cloudLinkDev->modelId);
    strcpy(hylinkDevSend.Type, STR_CTRL);

    strcpy(hylinkDevSend.Data.Key, cloudLinkDev->serverAttr[i].hyKey);
    cJSON *key = NULL;
    cJSON *value;
    if (cJSON_HasObjectItem(root, cloudLinkDev->serverAttr[i].key))
    {
        key = cJSON_GetObjectItem(root, cloudLinkDev->serverAttr[i].key);
    }
    //------------------------------------
    value = cJSON_GetObjectItem(root, cloudLinkDev->serverAttr[i].value);
    if (strcmp(cloudLinkDev->alinkInfo.product_key, "a1aqqjEXCWa") == 0)
    {
        int keyNum = key->valueint - 1;
        char buf[24] = {0};
        sprintf(buf, "%d", keyNum);
        strcat(hylinkDevSend.Data.Key, buf);

        if (strncmp(serviceid, "WordsCfg", serviceid_len) == 0)
        {
            getValueForJson(value, buf);
            int encLen = strlen(buf);
            void *decodeOut = malloc(BASE64_DECODE_OUT_SIZE(encLen));
            int decodeOutlen = base64_decode(buf, encLen, decodeOut);
            strncpy(hylinkDevSend.Data.Value, decodeOut, decodeOutlen);
            free(decodeOut);
        }
        else if (strncmp(serviceid, "PictureCfg", serviceid_len) == 0)
        {
            getValueForJson(value, hylinkDevSend.Data.Value);
        }
    }
    else
    {
        if (key != NULL)
        {
            char buf[24];
            getValueForJson(key, buf);
            strcat(hylinkDevSend.Data.Key, buf);
        }
        getValueForJson(value, hylinkDevSend.Data.Value);
    }
    //------------------------------------
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
    if (cloudLinkDev != NULL)
        runTransferCb(cloudLinkDev->alinkInfo.device_name, SUBDEV_RESTORE, TRANSFER_SUBDEV_LINE);

    return hylinkDelDev(sn);
}
