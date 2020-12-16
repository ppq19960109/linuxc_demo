#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "logFunc.h"
#include "commonFunc.h"
#include "frameCb.h"
#include "hylink.h"

#include "hylinkRecv.h"

#include "hylinkSubDev.h"
#include "hylinkListFunc.h"

static char *s_typeReport[] = {
    "Register",
    "UnRegister",
    "OnOff",
    "Attribute",
    "Event",
    "DevAttri",
    "DevsInfo",
    "ReFactory",
    "Ack",
};

static const SAttrInfo s_StypeReport = {
    .attr = s_typeReport,
    .attrLen = sizeof(s_typeReport) / sizeof(s_typeReport[0])};

void hylinkAnalyDevInfo(cJSON *root, cJSON *Data, const char *Params)
{
    HylinkDev *hylinkDevBuf;

    cJSON *devidJson = cJSON_GetObjectItem(Data, STR_DEVICEID);
    if (devidJson == NULL)
        return;

    cJSON *params = cJSON_GetObjectItem(Data, Params);

    if (strcmp(STR_HOST_GATEWAYID, devidJson->valuestring) == 0)
    {
        return;
    }

    hylinkDevBuf = hylinkListGetById(devidJson->valuestring);

    if (hylinkDevBuf == NULL)
    {
        hylinkDevBuf = (HylinkDev *)malloc(sizeof(HylinkDev));
        memset(hylinkDevBuf, 0, sizeof(HylinkDev));
        getStrForJson(root, STR_GATEWAYID, hylinkDevBuf->GatewayId);
        getStrForJson(Data, STR_DEVICEID, hylinkDevBuf->DeviceId);
        getStrForJson(Data, STR_MODELID, hylinkDevBuf->ModelId);
        getStrForJson(Data, STR_VERSION, hylinkDevBuf->Version);
        getByteForJson(Data, STR_ONLINE, &hylinkDevBuf->Online);

        hylinkListAdd(&hylinkDevBuf->hylinkNode);
    }
    else
    {
        getStrForJson(Data, STR_VERSION, hylinkDevBuf->Version);
        getByteForJson(Data, STR_ONLINE, &hylinkDevBuf->Online);
    }
    int res;
    cJSON *array_sub;
    int array_size = cJSON_GetArraySize(params);
    for (int i = 0; i < array_size; i++)
    {
        array_sub = cJSON_GetArrayItem(params, i);
        res = hylinkSubDevAttrUpdate(hylinkDevBuf, array_sub);
        if (res == -2)
        {
            logError("hylinkSubDevAttrUpdate");
            goto fail;
        }
    }
    runTransferCb(hylinkDevBuf, ATTR_REPORT_ALL, TRANSFER_CLOUD_REPORT);
    return;
fail:
    logError("hylinkAnalyDevInfo fail");
    hylinkListDel(hylinkDevBuf);
}

int hylinkRecvAnaly(const char *json)
{
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        logError("root is NULL\n");
        goto fail;
    }
    // char *rjson = cJSON_Print(root);
    // logDebug("%s\n", rjson);
    // free(rjson);

    //command字段
    cJSON *Command = cJSON_GetObjectItem(root, STR_COMMAND);
    if (Command == NULL)
    {
        logError("Command is NULL\n");
        goto fail;
    }
    if (strcmp(STR_REPORT, Command->valuestring) == 0)
    {
        logDebug("Command is Report\n");
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
    int type = findStrIndex(Type->valuestring, s_StypeReport.attr, s_StypeReport.attrLen);
    if (type == -1)
    {
        logError("Type is no exist\n");
        goto fail;
    }

    HylinkDev hylinkDevData;
    HylinkDev *hylinkDevBuf = NULL;
    cJSON *array_sub = NULL;
    int array_size = cJSON_GetArraySize(Data);
    for (int i = 0; i < array_size; i++)
    {
        array_sub = cJSON_GetArrayItem(Data, i);
        if (array_sub == NULL)
        {
            continue;
        }
        getStrForJson(array_sub, STR_DEVICEID, hylinkDevData.DeviceId);
        switch (type)
        {
        case 0: //设备注册上报：”Register”；
        {
            hylinkDevBuf = hylinkListGetById(hylinkDevData.DeviceId);
            if (hylinkDevBuf == NULL)
            {
                hylinkDevBuf = (HylinkDev *)malloc(sizeof(HylinkDev));
                memset(hylinkDevBuf, 0, sizeof(HylinkDev));
                getStrForJson(root, STR_GATEWAYID, hylinkDevBuf->GatewayId);
                getStrForJson(array_sub, STR_DEVICEID, hylinkDevBuf->DeviceId);
                getStrForJson(array_sub, STR_MODELID, hylinkDevBuf->ModelId);
                hylinkDevBuf->Online = SUBDEV_ONLINE;
                if (hylinkSubDevAttrUpdate(hylinkDevBuf, NULL) != 0)
                {
                    logError("hylinkSubDevAttrUpdate error\n");
                    free(hylinkDevBuf);
                    break;
                }
                hylinkListAdd(&hylinkDevBuf->hylinkNode);
            }
        }
        break;
        case 1: //设备注销上报：”UnRegister”；
        {
            hylinkDevBuf = hylinkListGetById(hylinkDevData.DeviceId);
            if (hylinkDevBuf != NULL)
            {
                runTransferCb(hylinkDevBuf->DeviceId, SUBDEV_RESTORE, TRANSFER_SUBDEV_LINE);
                hylinkListDel(hylinkDevBuf);
            }
            else
            {
                logError("UnRegister but sub device not exist:%s\n", hylinkDevData.DeviceId);
            }
        }
        break;
        case 2: //设备在线状态上报, “OnOff”
        {
            hylinkDevBuf = hylinkListGetById(hylinkDevData.DeviceId);

            if (hylinkDevBuf != NULL)
            {
                getByteForJson(array_sub, STR_VALUE, &hylinkDevBuf->Online);
                runTransferCb(hylinkDevBuf->DeviceId, hylinkDevBuf->Online, TRANSFER_SUBDEV_LINE);
            }
        }
        break;
        case 3: //设备属性上报：”Attribute”；
        case 4: //设备事件上报：”Event”；
        case 5: //设备全部属性上报：”DevAttri”;
            if (strcmp(STR_HOST_GATEWAYID, hylinkDevData.DeviceId) == 0)
            {
                hylinkGateway(array_sub);
                break;
            }
            hylinkDevBuf = hylinkListGetById(hylinkDevData.DeviceId);
            if (hylinkDevBuf != NULL)
            {
                if (hylinkDevBuf->Online == SUBDEV_OFFLINE)
                {
                    hylinkDevBuf->Online = SUBDEV_ONLINE;
                    runTransferCb(hylinkDevBuf->DeviceId, hylinkDevBuf->Online, TRANSFER_SUBDEV_LINE);
                }
                int type = hylinkSubDevAttrUpdate(hylinkDevBuf, array_sub);
                if (type >= 0)
                    runTransferCb(hylinkDevBuf, type, TRANSFER_CLOUD_REPORT);
                else
                    logError("hylinkSubDevAttrUpdate error\n");
            }
            else
            {
                logError("sub device not exist:%s  -\n", hylinkDevData.DeviceId);
            }
            break;
        case 6: //获取设备列表详细信息(网关指令)DevsInfo
        {
            hylinkAnalyDevInfo(root, array_sub, STR_PARAMS);
        }
        break;
        case 7: //恢复出厂设置上报：”ReFactory”；
        {
            runSystemCb(SYSTEM_RESET);
        }
        break;
        case 8: //Ack
        {
            // logDebug("type:Ack\n");
        }
        break;
        default:

            break;
        }
    }
heart:
    cJSON_Delete(root);
    return 0;
fail:
    cJSON_Delete(root);
    logError("json error\n");
    return -1;
}

int hylinkRecv(void *recv, unsigned int len)
{
    char *data = (char *)recv;
    if (data == NULL)
        return -1;
    int ret = -1;

    // for (int i = 0; i < len; ++i)
    // {
    //     printf("%x,", data[i]);
    // }
    // printf("\n");
    // if (data[0] == 0x02)
    // {
    for (int i = 0; i < len; ++i)
    {
        if (data[i] == 0x02)
        {
            logDebug("hylinkRecv:%d,%s\n", len, &data[i + 1]);
            ret = hylinkRecvAnaly(&data[i + 1]);
            if (ret == 0)
            {
                i += 2;
            }
        }
    }
    return 0;
}
