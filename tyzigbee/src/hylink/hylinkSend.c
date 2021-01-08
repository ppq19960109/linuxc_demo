
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"
#include "cJSON.h"
#include "commonFunc.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "hylinkSend.h"

int hylinkSendFunc(HylinkSend *hylinkSend)
{
    if (hylinkSend == NULL || hylinkSend->Data == NULL || hylinkSend->DataSize == 0)
        goto fail;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, STR_COMMAND, STR_REPORT);

    cJSON_AddStringToObject(root, STR_FRAMENUMBER, "00");

    cJSON_AddStringToObject(root, STR_TYPE, hylinkSend->Type);

    cJSON *DataArray = cJSON_AddArrayToObject(root, STR_DATA);

    for (int i = 0; i < hylinkSend->DataSize; ++i)
    {
        cJSON *arrayItem = cJSON_CreateObject();
        cJSON_AddItemToArray(DataArray, arrayItem);

        if (strlen(hylinkSend->Data[i].DeviceId))
            cJSON_AddStringToObject(arrayItem, STR_DEVICEID, hylinkSend->Data[i].DeviceId);
        if (strlen(hylinkSend->Data[i].ModelId))
            cJSON_AddStringToObject(arrayItem, STR_MODELID, hylinkSend->Data[i].ModelId);
        if (strlen(hylinkSend->Data[i].Key))
            cJSON_AddStringToObject(arrayItem, STR_KEY, hylinkSend->Data[i].Key);
        if (strlen(hylinkSend->Data[i].Value))
            cJSON_AddStringToObject(arrayItem, STR_VALUE, hylinkSend->Data[i].Value);
    }
    char *json = cJSON_PrintUnformatted(root);
    logInfo("hylink report json:%s\n", json);

    char *reportBuf = (char *)getHylinkReportBuf();
    int jsonLen = strlen(json);
    reportBuf[0] = 0x02;
    strncpy(&reportBuf[1], json, jsonLen);
    reportBuf[jsonLen + 1] = 0x03;
    int ret = runTransferCb(reportBuf, jsonLen + 2, TRANSFER_CLIENT_WRITE);

    cJSON_free(json);
    cJSON_Delete(root);
    return ret;
fail:
    return -1;
}
