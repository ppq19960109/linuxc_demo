#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"
#include "cJSON.h"
#include "commonFunc.h"

#include "hylinkRecv.h"
#include "hylinkSend.h"
#include "hylinkListFunc.h"

static char *s_dispatchTypeAttr[] = {
    "Add",
    "Delete",
    "ReFactory",
    "Restart",
    "DevsInfo",
    "DevAttri",
    "Ctrl",
    "Attribute",
};

static const AttrDesc s_dispatchType = {
    .attr = s_dispatchTypeAttr,
    .attrLen = sizeof(s_dispatchTypeAttr) / sizeof(s_dispatchTypeAttr[0])};

enum DispatchType
{
    ADD = 0,
    DELETE,
    REFACTORY,
    RESTART,
    DEVSINFO,
    DEVATTRI,
    CTRL,
    ATTRIBUTE,
};

int hylinkRecvJson(char *data)
{
    printf("hylinkRecvJson........\n");
    cJSON *root = cJSON_Parse(data);
    char *json = cJSON_Print(root);
    logDebug("hylinkRecv json:%s", json);
    free(json);

    //command字段
    cJSON *Command = cJSON_GetObjectItem(root, STR_COMMAND);
    if (Command == NULL)
    {
        logError("Command is NULL\n");
        goto fail;
    }
    if (strcmp(STR_DISPATCH, Command->valuestring) == 0)
    {
        logDebug("Command is dispatch\n");
    }
    else if (strcmp(STR_BEATHEARTRESPONSE, Command->valuestring) == 0)
    {
        logDebug("Command is BeatHeartResponse\n");
        goto heart;
    }
    else
    {
        logError("Command is value invaild:%s\n", Command->valuestring);
    }

    //Type字段
    cJSON *Type = cJSON_GetObjectItem(root, STR_TYPE);
    if (Type == NULL)
    {
        logError("Type is NULL\n");
        goto fail;
    }
    //Data字段
    cJSON *Data = cJSON_GetObjectItem(root, STR_DATA);
    if (Data == NULL)
    {
        logError("Data is NULL\n");
        goto fail;
    }
    //从type数组中查找type
    int type = findStrIndex(Type->valuestring, s_dispatchType.attr, s_dispatchType.attrLen);
    if (type == -1)
    {
        logError("Type is no exist\n");
        goto fail;
    }
    int res;
    cJSON *array_sub = NULL;
    int array_size = cJSON_GetArraySize(Data);
    for (int i = 0; i < array_size; i++)
    {
        array_sub = cJSON_GetArrayItem(Data, i);
        if (array_sub == NULL)
        {
            continue;
        }
        switch (type)
        {
        case ADD:
        {
            cJSON *key = cJSON_GetObjectItem(array_sub, STR_KEY);
            cJSON *value = cJSON_GetObjectItem(array_sub, STR_VALUE);
            if (key == NULL || value == NULL)
                goto fail;
            long num;
            strToNum(value->valuestring, 10, &num);
            unsigned char cmd = num;

            res = runCmdCb(&cmd, CMD_NETWORK_ACCESS);
            runSystemCb(HYLINK_ZB_CHANNEL);
        }
        break;
        case DELETE:
        {
            cJSON *DeviceId = cJSON_GetObjectItem(array_sub, STR_DEVICEID);
            if (DeviceId == NULL)
                goto fail;
            res = runZigbeeCb((void *)DeviceId->valuestring, NULL, NULL, NULL, ZIGBEE_DEV_LEAVE);
        }
        break;
        case REFACTORY:
            break;
        case RESTART:
            break;
        case DEVSINFO:
        {
            if (!cJSON_HasObjectItem(array_sub, STR_MODELID))
            {
                logError("model id is NULL");
                break;
            }

            cJSON *devidJson = cJSON_GetObjectItem(array_sub, STR_DEVICEID);
            if (devidJson == NULL)
            {
                logError("dev id is NULL");
                break;
            }
            cJSON *modelidJson = cJSON_GetObjectItem(array_sub, STR_MODELID);
            if (modelidJson == NULL)
            {
                logError("model id is NULL");
                break;
            }
            HylinkDev *hyDev = (HylinkDev *)malloc(sizeof(HylinkDev));
            memset(hyDev, 0, sizeof(HylinkDev));
            strcpy(hyDev->DeviceId, devidJson->valuestring);
            strcpy(hyDev->ModelId, modelidJson->valuestring);
            hylinkListAdd(hyDev);
            runZigbeeCb((void *)hyDev->DeviceId, (void *)hyDev->ModelId, NULL, NULL, ZIGBEE_DEV_DISPATCH);
        }
        break;
        case DEVATTRI:
        {
            cJSON *DeviceId = cJSON_GetObjectItem(array_sub, STR_DEVICEID);
            if (DeviceId == NULL)
                goto fail;
            res = runZigbeeCb((void *)DeviceId->valuestring, NULL, NULL, NULL, ZIGBEE_DEV_DISPATCH);
        }
        break;
        case CTRL:
        {
            cJSON *DeviceId = cJSON_GetObjectItem(array_sub, STR_DEVICEID);
            cJSON *key = cJSON_GetObjectItem(array_sub, STR_KEY);
            cJSON *value = cJSON_GetObjectItem(array_sub, STR_VALUE);
            if (DeviceId == NULL || key == NULL || value == NULL)
                goto fail;
            res = runZigbeeCb((void *)DeviceId->valuestring, NULL, (void *)key->valuestring, (void *)value->valuestring, ZIGBEE_DEV_DISPATCH);
        }
        break;
        case ATTRIBUTE:
        {
            cJSON *DeviceId = cJSON_GetObjectItem(array_sub, STR_DEVICEID);
            cJSON *key = cJSON_GetObjectItem(array_sub, STR_KEY);
            if (DeviceId == NULL || key == NULL)
                goto fail;
            res = runZigbeeCb((void *)DeviceId->valuestring, NULL, (void *)key->valuestring, NULL, ZIGBEE_DEV_DISPATCH);
        }
        break;
        default:
            break;
        }
    }
    cJSON_Delete(root);
    return res;
heart:
fail:
    cJSON_Delete(root);
    return -1;
}

int hylinkRecvManage(void *recv, unsigned int len)
{
    printf("hylinkRecvManage........\n");
    char *data = (char *)recv;
    int ret = 0;
    for (int i = 0; i < len; ++i)
    {
        if (data[i] == 0x02)
        {
            ret = hylinkRecvJson(&data[i + 1]);
        }
    }
    return ret;
}
