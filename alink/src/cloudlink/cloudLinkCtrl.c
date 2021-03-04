#include "main.h"

#include "cloudLink.h"
#include "cloudLinkReport.h"
#include "cloudLinkCtrl.h"

#include "scene.h"

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
    int i, res = -1;
    CloudLinkDev *cloudLinkDev = cloudLinkListGetBySn((int)sn);
    if (cloudLinkDev == NULL)
        return -1;

    cJSON *root = cJSON_Parse(payload);

    for (i = 0; i < cloudLinkDev->attrLen; ++i)
    {
        if (cJSON_HasObjectItem(root, cloudLinkDev->attr[i].cloudKey))
        {
            res = hyCloudCtrlSend(cloudLinkDev->alinkInfo.device_name, cloudLinkDev->modelId, cloudLinkDev->attr[i].hyType, cloudLinkDev->attr[i].hyKey, root, cloudLinkDev->attr[i].cloudKey);
            goto end;
        }
    }
end:
    cJSON_Delete(root);
    return res;
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
    cJSON *key = NULL, *cloud_value;
    if (cJSON_HasObjectItem(root, cloudLinkDev->serverAttr[i].key))
    {
        key = cJSON_GetObjectItem(root, cloudLinkDev->serverAttr[i].key);
    }
    cloud_value = cJSON_GetObjectItem(root, cloudLinkDev->serverAttr[i].value);
    //------------------------------------
    HylinkDevSendData hylinkDevSend = {0};
    hylinkDevSend.FrameNumber = 0;
    strcpy(hylinkDevSend.Data.DeviceId, cloudLinkDev->alinkInfo.device_name);
    strcpy(hylinkDevSend.Data.ModelId, cloudLinkDev->modelId);
    strcpy(hylinkDevSend.Type, STR_CTRL);
    strcpy(hylinkDevSend.Data.Key, cloudLinkDev->serverAttr[i].hyKey);
    if (strcmp(cloudLinkDev->alinkInfo.product_key, "a1aqqjEXCWa") == 0)
    {
        int keyNum = key->valueint - 1;
        char buf[24] = {0};
        sprintf(buf, "%d", keyNum);
        strcat(hylinkDevSend.Data.Key, buf);

        if (strncmp(serviceid, "WordsCfg", serviceid_len) == 0)
        {
            cjson_to_str(cloud_value, buf);
            int encLen = strlen(buf);
            void *decodeOut = malloc(BASE64_DECODE_OUT_SIZE(encLen));
            int decodeOutlen = base64_decode(buf, encLen, decodeOut);
            strncpy(hylinkDevSend.Data.Value, decodeOut, decodeOutlen);
            free(decodeOut);
        }
        else if (strncmp(serviceid, "PictureCfg", serviceid_len) == 0)
        {
            cjson_to_str(cloud_value, hylinkDevSend.Data.Value);
        }
    }
    else
    {
        if (key != NULL)
        {
            char buf[24];
            cjson_to_str(key, buf);
            strcat(hylinkDevSend.Data.Key, buf);
        }
        cjson_to_str(cloud_value, hylinkDevSend.Data.Value);
    }
    res = hylinkSend(&hylinkDevSend);
    //------------------------------------
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
    if (cloudLinkDev == NULL)
    {
        logWarn("cloudLink Dev not exits:%s", sn);
        return -1;
    }
    runTransferCb(cloudLinkDev->alinkInfo.device_name, SUBDEV_RESTORE, TRANSFER_SUBDEV_LINE);
    return hylinkDelDev(sn);
}
