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
    "LocalScene",
};

static const SAttrInfo s_StypeReport = {
    .attr = s_typeReport,
    .attrLen = sizeof(s_typeReport) / sizeof(s_typeReport[0])};

void hylinkAnalyDevInfo(cJSON *root, cJSON *Data, const char *Params)
{
    HyLinkDev *hyLinkDevBuf;

    cJSON *devidJson = cJSON_GetObjectItem(Data, STR_DEVICEID);
    if (devidJson == NULL)
        return;
    char online = 0;
    getByteForJson(Data, STR_ONLINE, &online);

    cJSON *params = cJSON_GetObjectItem(Data, Params);

    hyLinkDevBuf = hylinkListGetById(devidJson->valuestring);

    if (hyLinkDevBuf == NULL)
    {
        if (online == SUBDEV_ONLINE)
        {
            char hyModelId[33] = {0};
            getStrForJson(Data, STR_MODELID, hyModelId);
            hyLinkDevBuf = addProfileDev(HYLINK_PROFILE_PATH, devidJson->valuestring, hyModelId, hyLinkParseJson);
            if (hyLinkDevBuf == NULL)
                return;
        }
    }
    else
    {
        if (online != hyLinkDevBuf->online)
        {
            hyLinkDevBuf->online = online;
            runTransferCb(hyLinkDevBuf->devId, hyLinkDevBuf->online, TRANSFER_SUBDEV_LINE);
        }
    }
    int res;
    cJSON *array_sub;
    int array_size = cJSON_GetArraySize(params);
    for (int i = 0; i < array_size; i++)
    {
        array_sub = cJSON_GetArrayItem(params, i);
        res = hylinkSubDevAttrUpdate(hyLinkDevBuf, array_sub);
        if (res < 0)
        {
            logError("hylinkSubDevAttrUpdate error");
            goto fail;
        }
    }
    runTransferCb(hyLinkDevBuf, ATTR_REPORT_ALL, TRANSFER_CLOUD_REPORT);
    return;
fail:
    logError("hylinkAnalyDevInfo fail");
    hylinkListDel(hyLinkDevBuf);
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

    char hyDevId[33] = {0};

    HyLinkDev *hyLinkDevBuf = NULL;
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
        case 0: //设备注册上报：”Register”；
        {
            hyLinkDevBuf = hylinkListGetById(hyDevId);
            if (hyLinkDevBuf == NULL)
            {
                char hyModelId[33] = {0};
                getStrForJson(array_sub, STR_MODELID, hyModelId);
                hyLinkDevBuf = addProfileDev(HYLINK_PROFILE_PATH, hyDevId, hyModelId, hyLinkParseJson);
                if (hyLinkDevBuf != NULL)
                    runTransferCb(hyLinkDevBuf, ATTR_REPORT_ALL, TRANSFER_CLOUD_REPORT);
            }
            else
            {
                if (hyLinkDevBuf->online == SUBDEV_OFFLINE)
                {
                    hyLinkDevBuf->online = SUBDEV_ONLINE;
                    runTransferCb(hyLinkDevBuf->devId, hyLinkDevBuf->online, TRANSFER_SUBDEV_LINE);
                }
            }
        }
        break;
        case 1: //设备注销上报：”UnRegister”；
        {
            hyLinkDevBuf = hylinkListGetById(hyDevId);
            if (hyLinkDevBuf != NULL)
            {
                runTransferCb(hyLinkDevBuf->devId, SUBDEV_RESTORE, TRANSFER_SUBDEV_LINE);
                hylinkListDel(hyLinkDevBuf);
            }
            else
            {
                logError("UnRegister but sub device not exist:%s\n", hyDevId);
            }
        }
        break;
        case 2: //设备在线状态上报, “OnOff”
        {
            hyLinkDevBuf = hylinkListGetById(hyDevId);
            char online = 0;
            getByteForJson(array_sub, STR_VALUE, &online);
            if (hyLinkDevBuf == NULL)
            {
                if (online)
                {
                    if (cJSON_HasObjectItem(array_sub, STR_MODELID))
                    {
                        char hyModelId[33] = {0};
                        getStrForJson(array_sub, STR_MODELID, hyModelId);
                        hyLinkDevBuf = addProfileDev(HYLINK_PROFILE_PATH, hyDevId, hyModelId, hyLinkParseJson);
                        if (hyLinkDevBuf != NULL)
                        {
                            runTransferCb(hyLinkDevBuf, ATTR_REPORT_ALL, TRANSFER_CLOUD_REPORT);
                            runTransferCb(hyLinkDevBuf->devId, strlen(hyLinkDevBuf->devId), TRANSFER_DEVATTR);
                        }
                    }
                }
            }
            else
            {
                if (online != hyLinkDevBuf->online)
                {
                    getByteForJson(array_sub, STR_VALUE, &hyLinkDevBuf->online);
                    runTransferCb(hyLinkDevBuf->devId, hyLinkDevBuf->online, TRANSFER_SUBDEV_LINE);
                }
            }
        }
        break;
        case 3: //设备属性上报：”Attribute”；
        case 4: //设备事件上报：”Event”；
        case 5: //设备全部属性上报：”DevAttri”;
            hyLinkDevBuf = hylinkListGetById(hyDevId);
            if (hyLinkDevBuf != NULL)
            {
                if (hyLinkDevBuf->online == SUBDEV_OFFLINE)
                {
                    hyLinkDevBuf->online = SUBDEV_ONLINE;
                    runTransferCb(hyLinkDevBuf->devId, hyLinkDevBuf->online, TRANSFER_SUBDEV_LINE);
                    runTransferCb(hyLinkDevBuf->devId, strlen(hyLinkDevBuf->devId), TRANSFER_DEVATTR);
                }
                int type = hylinkSubDevAttrUpdate(hyLinkDevBuf, array_sub);
                if (type < 0)
                {
                    if (type == -2)
                        logWarn("hylinkSubDevAttrUpdate repeat\n");
                    else
                        logError("hylinkSubDevAttrUpdate error\n");
                }
                else
                    runTransferCb(hyLinkDevBuf, type, TRANSFER_CLOUD_REPORT);
            }
            else
            {
                logError("sub device not exist:%s \n", hyDevId);
                if (cJSON_HasObjectItem(array_sub, STR_MODELID))
                {
                    char hyModelId[33] = {0};
                    getStrForJson(array_sub, STR_MODELID, hyModelId);
                    hyLinkDevBuf = addProfileDev(HYLINK_PROFILE_PATH, hyDevId, hyModelId, hyLinkParseJson);
                    if (hyLinkDevBuf != NULL)
                    {
                        runTransferCb(hyLinkDevBuf, ATTR_REPORT_ALL, TRANSFER_CLOUD_REPORT);
                        runTransferCb(hyLinkDevBuf->devId, strlen(hyLinkDevBuf->devId), TRANSFER_DEVATTR);
                    }
                }
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
        case 9:
        {
            cJSON *Op = cJSON_GetObjectItem(array_sub, "Op");
            if (strcmp("ExecScene", Op->valuestring) == 0)
            {
                cJSON *Id = cJSON_GetObjectItem(array_sub, "Id");
                if (Id != NULL)
                    runTransferCb(Id->valuestring, strlen(Id->valuestring), TRANSFER_SCENE_REPORT);
            }
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
