
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
#include "hylinkReport.h"

int hylinkReportSingle(char *type, char *devId, char *modelId, char *key, char *value)
{
    if (type == NULL || devId == NULL || key == NULL)
        return -1;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, STR_COMMAND, STR_REPORT);

    cJSON_AddStringToObject(root, STR_FRAMENUMBER, "00");

    cJSON_AddStringToObject(root, STR_TYPE, type);

    cJSON *DataArray = cJSON_AddArrayToObject(root, STR_DATA);

    cJSON *arrayItem = cJSON_CreateObject();
    cJSON_AddItemToArray(DataArray, arrayItem);

    if (devId != NULL)
        cJSON_AddStringToObject(arrayItem, STR_DEVICEID, devId);
    if (modelId != NULL)
        cJSON_AddStringToObject(arrayItem, STR_MODELID, modelId);
    if (key != NULL)
        cJSON_AddStringToObject(arrayItem, STR_KEY, key);
    if (value != NULL)
        cJSON_AddStringToObject(arrayItem, STR_VALUE, value);

    char *json = cJSON_PrintUnformatted(root);
    logInfo("hylinkReportSingle json:%s\n", json);

    char *reportBuf = (char *)getHylinkReportBuf();
    int jsonLen = strlen(json);
    reportBuf[0] = 0x02;
    strncpy(&reportBuf[1], json, jsonLen);
    reportBuf[jsonLen + 1] = 0x03;
    int ret = runTransferCb(reportBuf, jsonLen + 2, TRANSFER_SERVER_HYLINK_WRITE);

    cJSON_free(json);
    cJSON_Delete(root);
    return ret;
}
int hylinkReportFunc(HylinkReport *hylinkReport)
{
    if (hylinkReport == NULL || hylinkReport->Data == NULL || hylinkReport->DataSize == 0)
        goto fail;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, STR_COMMAND, STR_REPORT);

    cJSON_AddStringToObject(root, STR_FRAMENUMBER, "00");

    cJSON_AddStringToObject(root, STR_TYPE, hylinkReport->Type);

    cJSON *DataArray = cJSON_AddArrayToObject(root, STR_DATA);

    for (int i = 0; i < hylinkReport->DataSize; ++i)
    {
        cJSON *arrayItem = cJSON_CreateObject();
        cJSON_AddItemToArray(DataArray, arrayItem);

        if (strlen(hylinkReport->Data[i].DeviceId))
            cJSON_AddStringToObject(arrayItem, STR_DEVICEID, hylinkReport->Data[i].DeviceId);
        if (strlen(hylinkReport->Data[i].ModelId))
            cJSON_AddStringToObject(arrayItem, STR_MODELID, hylinkReport->Data[i].ModelId);
        if (strlen(hylinkReport->Data[i].Key))
            cJSON_AddStringToObject(arrayItem, STR_KEY, hylinkReport->Data[i].Key);
        if (strlen(hylinkReport->Data[i].Value))
            cJSON_AddStringToObject(arrayItem, STR_VALUE, hylinkReport->Data[i].Value);
    }
    char *json = cJSON_PrintUnformatted(root);
    logInfo("hylink report json:%s\n", json);

    char *reportBuf = (char *)getHylinkReportBuf();
    int jsonLen = strlen(json);
    reportBuf[0] = 0x02;
    strncpy(&reportBuf[1], json, jsonLen);
    reportBuf[jsonLen + 1] = 0x03;
    int ret = runTransferCb(reportBuf, jsonLen + 2, TRANSFER_SERVER_HYLINK_WRITE);

    cJSON_free(json);
    cJSON_Delete(root);
    return ret;
fail:
    return -1;
}
