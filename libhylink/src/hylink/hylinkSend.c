#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cJSON.h"
#include "logFunc.h"
#include "commonFunc.h"
#include "frameCb.h"

#include "hylink.h"
#include "hylinkListFunc.h"
#include "hylinkRecv.h"
#include "hylinkSend.h"

static char hyLinkSendBuf[1024];
int hylinkDispatch(const char *str, const int str_len)
{
    hyLinkSendBuf[0] = 0x02;
    strncpy(&hyLinkSendBuf[1], str, str_len);
    hyLinkSendBuf[str_len + 1] = 0x03;

    return runTransferCb(hyLinkSendBuf, str_len + 2, TRANSFER_CLIENT_WRITE);
}
int hylinkSend(void *ptr)
{
    if (NULL == ptr)
    {
        return -1;
    }
    HylinkDevSendData *hylinkDevSendData = (HylinkDevSendData *)ptr;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, STR_COMMAND, STR_DISPATCH);

    cJSON_AddStringToObject(root, STR_FRAMENUMBER, "0");

    cJSON_AddStringToObject(root, STR_TYPE, hylinkDevSendData->Type);

    cJSON *DataArray = cJSON_CreateArray();
    cJSON_AddItemToObject(root, STR_DATA, DataArray);

    cJSON *arrayItem = cJSON_CreateObject();
    if (strlen(hylinkDevSendData->Data.DeviceId))
        cJSON_AddStringToObject(arrayItem, STR_DEVICEID, hylinkDevSendData->Data.DeviceId);
    if (strlen(hylinkDevSendData->Data.ModelId))
        cJSON_AddStringToObject(arrayItem, STR_MODELID, hylinkDevSendData->Data.ModelId);
    if (strlen(hylinkDevSendData->Data.Key))
        cJSON_AddStringToObject(arrayItem, STR_KEY, hylinkDevSendData->Data.Key);
    if (strlen(hylinkDevSendData->Data.Value))
        cJSON_AddStringToObject(arrayItem, STR_VALUE, hylinkDevSendData->Data.Value);
    cJSON_AddItemToArray(DataArray, arrayItem);

    char *json = cJSON_PrintUnformatted(root);
    logInfo("send json:%s\n", json);

    int ret = hylinkDispatch(json, strlen(json));

    free(json);
    cJSON_Delete(root);
    return ret;
}

int hylinkHeart(void)
{
    return runTransferCb(HY_HEART, strlen(HY_HEART), TRANSFER_CLIENT_WRITE);
}

int hylinkSendDevInfo(void)
{
    HylinkDevSendData hylinkDevSendData = {0};
    hylinkDevSendData.FrameNumber = 0;
    strcpy(hylinkDevSendData.Type, STR_DEVSINFO);
    strcpy(hylinkDevSendData.Data.DeviceId, STR_GATEWAY_DEVID);
    strcpy(hylinkDevSendData.Data.Key, STR_DEVSINFO);
    return hylinkSend(&hylinkDevSendData);
}

int hylinkSendDevAttr(void *devId, unsigned int len)
{
    HylinkDevSendData hylinkDevSendData = {0};
    hylinkDevSendData.FrameNumber = 0;
    strcpy(hylinkDevSendData.Type, STR_DEVATTRI);
    strcpy(hylinkDevSendData.Data.DeviceId, devId);
    strcpy(hylinkDevSendData.Data.Key, "All");
    return hylinkSend(&hylinkDevSendData);
}

int hylinkDelDev(const char *sn)
{
    if (sn == NULL)
        return -1;
    HylinkDevSendData out = {0};
    out.FrameNumber = 0;
    strcpy(out.Type, STR_DELETE);
    strcpy(out.Data.DeviceId, sn);
    int ret = hylinkSend(&out);
    if (ret >= 0)
    {
        hylinkListDelById(sn);
    }

    return ret;
}

int hyCloudCtrlSend(const char *sn, const char *modelId, const char *hyType, const char *hyKey, cJSON *cloud_payload, const char *cloudKey)
{
    HylinkDevSendData hylinkDevSend = {0};

    hylinkDevSend.FrameNumber = 0;

    strcpy(hylinkDevSend.Data.DeviceId, sn);
    strcpy(hylinkDevSend.Data.ModelId, modelId);

    strcpy(hylinkDevSend.Type, STR_CTRL);

    if (strlen(hyType) != 0)
        strcpy(hylinkDevSend.Type, hyType);

    strcpy(hylinkDevSend.Data.Key, hyKey);

    cJSON *cloud_value = cJSON_GetObjectItem(cloud_payload, cloudKey);

    cjson_to_str(cloud_value, hylinkDevSend.Data.Value);
    return hylinkSend(&hylinkDevSend);
}
