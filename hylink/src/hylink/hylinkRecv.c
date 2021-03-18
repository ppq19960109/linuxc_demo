#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "frameCb.h"
#include "logFunc.h"
#include "cJSON.h"
#include "commonFunc.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "hylinkSend.h"
#include "hylinkListFunc.h"

#include "database.h"
#include "scene_adapter.h"

static char *hylink_recv_type[] = {
    "Add",
    "Register",
    "UnRegister",
    "OnOff",
    "Delete",
    "DevsInfo",
    "LocalScene",
};

enum Hylink_Type
{
    ADD = 0,
    REGISTER,
    UNREGISTER,
    ONOFF,
    DELETE,
    DEVSINFO,
    LOCAL_SCENE,
};

int hylinkRecvJson(char *data)
{
    int commandDir = 0;
    printf("hylinkRecvJson........\n");
    cJSON *root = cJSON_Parse(data);
    // char *json = cJSON_Print(root);
    // logDebug("hylinkRecv json:%s", json);
    // free(json);

    //command字段
    cJSON *Command = cJSON_GetObjectItem(root, STR_COMMAND);
    if (Command == NULL)
    {
        logError("Command is NULL\n");
        goto fail;
    }
    if (strcmp(STR_REPORT, Command->valuestring) == 0)
    {
        logDebug("Command is report\n");
        char *json = cJSON_PrintUnformatted(root);
        scene_adapter_report(json, strlen(json));
        cJSON_free(json);
    }
    else if (strcmp(STR_DISPATCH, Command->valuestring) == 0)
    {
        logDebug("Command is dispatch\n");
        commandDir = 1;
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
        logError("Type is NULL");
        goto fail;
    }
    //从type数组中查找type
    int type = findStrIndex(Type->valuestring, hylink_recv_type, sizeof(hylink_recv_type) / sizeof(hylink_recv_type[0]));
    if (type == -1)
    {
        logError("Type is no exist");
        goto fail;
    }
    //Data字段
    cJSON *Data = cJSON_GetObjectItem(root, STR_DATA);
    if (Data == NULL)
    {
        logError("Data is NULL");
        goto fail;
    }
    logDebug("hylinkRecvJson data:%s", data);

    int res = 0;
    char hyDevId[33] = {0};
    cJSON *array_sub = NULL;
    int array_size = cJSON_GetArraySize(Data);
    for (int i = 0; i < array_size; i++)
    {
        array_sub = cJSON_GetArrayItem(Data, i);
        if (array_sub == NULL)
        {
            continue;
        }
        getStrForJson(array_sub, STR_DEVICEID, hyDevId);
        switch (type)
        {
        case ADD:
        {
            cJSON *key = cJSON_GetObjectItem(array_sub, STR_KEY);
            cJSON *value = cJSON_GetObjectItem(array_sub, STR_VALUE);
            if (key == NULL || value == NULL)
                goto fail;
            if (commandDir)
            {
                long num;
                strToNum(value->valuestring, 10, &num);
                unsigned char cmd = num;

                runCmdCb(&cmd, CMD_NETWORK_ACCESS_TIME);
            }
        }
        break;
        case REGISTER:
        {
            char hyModelId[33] = {0};
            getStrForJson(array_sub, STR_MODELID, hyModelId);
            addDevToHyList(hyDevId, hyModelId);
            insertDatabse(hyDevId, hyModelId);
        }
        break;
        case ONOFF:
        {
            HylinkDev *hyDev = (HylinkDev *)hylinkListGet(hyDevId);
            if (hyDev == NULL)
            {
                logError("hyDev is null");
                break;
            }
            getByteForJson(array_sub, STR_ONLINE, &hyDev->online);
        }
        break;
        case UNREGISTER:
        case DELETE:
        {
            HylinkDev *hyDev = (HylinkDev *)hylinkListGet(hyDevId);
            if (hyDev == NULL)
            {
                logError("hyDev is null");
                break;
            }
            hylinkListDel(hyDevId);
            deleteDatabse(hyDevId);
        }
        break;
        case DEVSINFO:
        {

            int index = 0;
            HylinkDev *dev;
            HylinkSend hylinkSend = {0};
            hylinkSend.Command = !commandDir;
            strcpy(hylinkSend.Type, STR_DEVSINFO);
            hylinkSend.DataSize = hylinkListSize();
            HylinkSendData *hylinkSendData = malloc(sizeof(HylinkSendData) * hylinkSend.DataSize);
            memset(hylinkSendData, 0, sizeof(HylinkSendData) * hylinkSend.DataSize);
            hylinkSend.Data = hylinkSendData;

            hyLink_kh_foreach_value(dev)
            {
                logWarn("hyLink_kh_foreach_value:%d", index);
                if (dev != NULL)
                {
                    strcpy(hylinkSendData[index].DeviceId, dev->DeviceId);
                    strcpy(hylinkSendData[index].ModelId, dev->ModelId);
                    sprintf(hylinkSendData[index].Online, "%d", dev->online);
                    ++index;
                }
            }
            hylinkSendFunc(&hylinkSend);
        }
        break;
        case LOCAL_SCENE:
        {
            char *json = cJSON_PrintUnformatted(root);
            scene_adapter_cmd(json, strlen(json));
            cJSON_free(json);
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
