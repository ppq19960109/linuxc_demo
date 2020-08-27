#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "local_receive.h"
#include "local_list.h"
#include "local_tcp_client.h"
#include "cloud_list.h"

#include "socket.h"

#include "hilink.h"

static char *s_typeReport[] = {
    "Register",
    "UnRegister",
    "OnOff",
    "Attribute",
    "DevAttri",
    "DevList",
    "DevsInfo",
    "Event",
    "ReFactory",
    "CooInfo",
    "NeighborInfo",
    "ChildrenInfo",
    "SetSig",
    "GetSig",
    "Ack",
};

static const SAttrInfo s_StypeReport = {
    .attr = s_typeReport,
    .attrLen = sizeof(s_typeReport) / sizeof(s_typeReport[0])};

const char *report_json[] = {
    // "{\
    //    \"Command\":\"Report\",\
    //    \"FrameNumber\":\"00\",\
    //    \"GatewayId\" :\"0006D12345678909\",\
    //    \"Type\":\"Register\",\
    //    \"Data\":[\
    //      {\
    //           \"DeviceId\":\"1234567876543670\",\
    //           \"ModelId\":\"HY0097\",\
    //           \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
    //   }\
    // ]\
    // }",
    "{\
       \"Command\":\"Report\",\
       \"FrameNumber\":\"00\",\
       \"GatewayId\" :\"0006D12345678909\",\
       \"Type\":\"Register\",\
       \"Data\":[\
         {\
              \"DeviceId\":\"2234567876543671\",\
              \"ModelId\":\"09223f\",\
              \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
      }\
    ]\
    }",
    "{\
       \"Command\":\"Report\",\
       \"FrameNumber\":\"00\",\
       \"GatewayId\" :\"0006D12345678909\",\
       \"Type\":\"Register\",\
       \"Data\":[\
         {\
              \"DeviceId\":\"3234567876543673\",\
              \"ModelId\":\"HY0107\",\
              \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
      }\
    ]\
    }",
    "{\
       \"Command\":\"Report\",\
       \"FrameNumber\":\"00\",\
       \"GatewayId\" :\"0006D12345678909\",\
       \"Type\":\"Register\",\
       \"Data\":[\
         {\
              \"DeviceId\":\"4234567876543674\",\
              \"ModelId\":\"HY0093\",\
              \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
      }\
    ]\
    }",
    "{\
       \"Command\":\"Report\",\
       \"FrameNumber\":\"00\",\
       \"GatewayId\" :\"0006D12345678909\",\
       \"Type\":\"Register\",\
       \"Data\":[\
         {\
              \"DeviceId\":\"5234567876543675\",\
              \"ModelId\":\"HY0134\",\
              \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
      }\
    ]\
    }",
    // "{\
    //    \"Command\":\"Report\",\
    //    \"FrameNumber\":\"00\",\
    //    \"GatewayId\" :\"0006D12345678909\",\
    //    \"Type\":\"Register\",\
    //    \"Data\":[\
    //      {\
    //           \"DeviceId\":\"4574567876543675\",\
    //           \"ModelId\":\"HY0095\",\
    //           \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
    //   }\
    // ]\
    // }",
    // "{\
    //    \"Command\":\"Report\",\
    //    \"FrameNumber\":\"00\",\
    //    \"GatewayId\" :\"0006D12345678909\",\
    //    \"Type\":\"Register\",\
    //    \"Data\":[\
    //      {\
    //           \"DeviceId\":\"5234567876543432\",\
    //           \"ModelId\":\"HY0096\",\
    //           \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
    //   }\
    // ]\
    // }@",
    "{\
       \"Command\":\"Report\",\
       \"FrameNumber\":\"00\",\
       \"GatewayId\" :\"0006D12345678909\",\
       \"Type\":\"Register\",\
       \"Data\":[\
         {\
              \"DeviceId\":\"5234564376543432\",\
              \"ModelId\":\"HY0121\",\
              \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
      }\
    ]\
    }4",
    "{\
       \"Command\":\"Report\",\
       \"FrameNumber\":\"00\",\
       \"GatewayId\" :\"0006D12345678909\",\
       \"Type\":\"Register\",\
       \"Data\":[\
         {\
              \"DeviceId\":\"523455676543432\",\
              \"ModelId\":\"HY0122\",\
              \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
      }\
    ]\
    }12",
};

LocalControl_t g_SLocalControl;

void local_control_init(LocalControl_t *localControl)
{
    localControl->discoverMode = 0;
    INIT_LIST_HEAD(&localControl->head);
    localControl->pid = net_client(localControl);
}

void local_control_destory(LocalControl_t *localControl)
{
    pthread_cancel(localControl->pid);
    localControl->pid = 0;

    if (localControl->socketfd != 0)
    {
        Close(localControl->socketfd);
        localControl->socketfd = 0;
    }
    //------------------------------
    list_del_all(&localControl->head);
}

struct list_head *local_get_list_head(LocalControl_t *localControl)
{
    return &localControl->head;
}

void local_hilink_upload_int(const char *svcId, const char *key, int value)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, key, value);
    char *json = cJSON_PrintUnformatted(root);
    hilink_upload_char_state(svcId, json, strlen(json) + 1);
    free(json);
    free(root);
}

void local_load_device_info(cJSON *root, cJSON *Data, const char *Params, struct list_head *localNode)
{
    dev_data_t *dev_buf;
    cJSON *array_sub;

    int array_size = cJSON_GetArraySize(Data);

    for (int cnt = 0; cnt < array_size; cnt++)
    {
        array_sub = cJSON_GetArrayItem(Data, cnt);

        dev_buf = list_get_by_id(cJSON_GetObjectItem(array_sub, STR_DEVICEID)->valuestring, localNode);
        if (dev_buf == NULL)
        {
            dev_buf = (dev_data_t *)malloc(sizeof(dev_data_t));
            memset(dev_buf, 0, sizeof(dev_data_t));
            str_copy_from_json(root, STR_GATEWAYID, dev_buf->GatewayId);
            str_copy_from_json(array_sub, STR_DEVICEID, dev_buf->DeviceId);
            str_copy_from_json(array_sub, STR_MODELID, dev_buf->ModelId);
            str_copy_from_json(array_sub, STR_VERSION, dev_buf->Version);
            str_copy_from_json(array_sub, STR_ONLINE, &dev_buf->Online);
            char_copy_from_json(array_sub, STR_REGISTERSTATUS, &dev_buf->RegisterStatus);

            if (local_attribute_update(dev_buf, cJSON_GetObjectItem(array_sub, Params)) != 0)
            {
                free(dev_buf);
                continue;
            }
            list_add(&dev_buf->node, localNode);
        }
        else
        {
            if (strcmp(cJSON_GetObjectItem(array_sub, STR_MODELID)->valuestring, dev_buf->ModelId))
            {
                log_error("DeviceId identical,but ModelId inequality");
                break;
            }
            str_copy_from_json(array_sub, STR_VERSION, dev_buf->Version);
            str_copy_from_json(array_sub, STR_ONLINE, &dev_buf->Online);

            char_copy_from_json(array_sub, STR_REGISTERSTATUS, &dev_buf->RegisterStatus);
            local_attribute_update(dev_buf, cJSON_GetObjectItem(array_sub, Params));

            HilinkSyncBrgDevStatus(dev_buf->DeviceId, dev_buf->Online);
        }
    }
}

int read_from_local(const char *json, struct list_head *localNode)
{
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        log_error("root is NULL\n");
        goto fail;
    }
    // log_debug("%s\n", cJSON_Print(root));

    //command字段
    cJSON *Command = cJSON_GetObjectItem(root, STR_COMMAND);
    if (Command == NULL)
    {
        log_error("Command is NULL\n");
        goto fail;
    }
    if (strcmp(STR_REPORT, Command->valuestring) == 0)
    {
        log_debug("Command is Report\n");
    }
    else if (strcmp(STR_BEATHEARTRESPONSE, Command->valuestring) == 0)
    {
        log_debug("Command is BeatHeartResponse\n");
        goto heart;
    }
    else
    {
        log_error("Command is value invaild:%s\n", Command->valuestring);
    }

    //Type字段
    cJSON *Type = cJSON_GetObjectItem(root, STR_TYPE);
    if (Type == NULL)
    {
        log_error("Type is NULL\n");
        goto fail;
    }
    //Data字段
    cJSON *Data = cJSON_GetObjectItem(root, STR_DATA);
    if (Data == NULL)
    {
        log_error("Data is NULL\n");
        goto fail;
    }
    //从type数组中查找type
    int type = str_search(Type->valuestring, s_StypeReport.attr, s_StypeReport.attrLen);
    if (type == -1)
    {
        log_error("Type is no exist\n");
        goto fail;
    }

    dev_data_t dev_data;
    dev_data_t *dev_buf;
    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);
    str_copy_from_json(array_sub, STR_DEVICEID, dev_data.DeviceId);

    switch (type)
    {
    case 0: //设备注册上报：”Register”；
    {
        dev_buf = list_get_by_id(dev_data.DeviceId, localNode);
        if (dev_buf == NULL)
        {
            dev_buf = (dev_data_t *)malloc(sizeof(dev_data_t));
            memset(dev_buf, 0, sizeof(dev_data_t));
            str_copy_from_json(root, STR_GATEWAYID, dev_buf->GatewayId);
            str_copy_from_json(array_sub, STR_DEVICEID, dev_buf->DeviceId);
            str_copy_from_json(array_sub, STR_MODELID, dev_buf->ModelId);

            if (local_attribute_update(dev_buf, NULL) != 0)
            {
                free(dev_buf);
                break;
            }

            list_add(&dev_buf->node, localNode);
        }
        str_copy_from_json(array_sub, STR_DEVICETYPE, dev_buf->DeviceType);
        str_copy_from_json(array_sub, STR_SECRET, dev_buf->Secret);
    }
    break;
    case 1: //设备注销上报：”UnRegister”；
    {
        if (list_del_by_id(dev_data.DeviceId, localNode) == 0)
        {
            list_del_by_id_hilink(dev_data.DeviceId, cloud_get_list_head(&g_SCloudControl));
            HilinkSyncBrgDevStatus(dev_data.DeviceId, DEV_RESTORE);
        }
        else
        {
            log_error("UnRegister device not exist");
        }
    }
    break;
    case 2: //设备在线状态上报, “OnOff”
    {
        cJSON *Key = cJSON_GetObjectItem(array_sub, STR_KEY);
        dev_buf = list_get_by_id(dev_data.DeviceId, localNode);
        if (dev_buf != NULL && Key != NULL && strcmp(Key->valuestring, STR_ONLINE) == 0)
        {
            char_copy_from_json(array_sub, STR_VALUE, &dev_buf->Online);
            HilinkSyncBrgDevStatus(dev_data.DeviceId, dev_buf->Online);
        }
    }
    break;
    case 3: //设备属性上报：”Attribute”；
    {
        // log_debug("设备属性上报：”Attribute”；");

        dev_buf = list_get_by_id(dev_data.DeviceId, localNode);
        if (dev_buf != NULL)
        {
            cJSON *Key = cJSON_GetObjectItem(array_sub, STR_KEY);
            if (Key != NULL && strcmp(Key->valuestring, STR_VERSION) == 0)
            {
                str_copy_from_json(array_sub, STR_VALUE, dev_buf->Version);
            }

            local_attribute_update(dev_buf, Data);
        }
        else
        {
            if (strcmp(STR_HOST_GATEWAYID, dev_data.DeviceId) == 0)
            {
                cJSON *Key = cJSON_GetObjectItem(array_sub, STR_KEY);
                if (Key != NULL && strcmp(Key->valuestring, STR_PERMITJOINING) == 0)
                {
                    char_copy_from_json(array_sub, STR_VALUE, &g_SLocalControl.discoverMode);

                    local_hilink_upload_int("switch", STR_ON, g_SLocalControl.discoverMode);
                }
            }
            log_error("sub device not exist");
        }
    }
    break;
    case 4: //设备全部属性上报：”DevAttri”;
    {
    }
    break;
    case 5: //设备列表上报：”DevList”
    {
        local_load_device_info(root, Data, STR_PARAMS, NULL);
    }
    break;
    case 6: //获取设备列表详细信息(网关指令)DevsInfo
    {
        local_load_device_info(root, Data, STR_PARAMS, localNode);
    }
    break;
    case 7: //设备事件上报：”Event”；恢复出厂设置上报：”
    {
        dev_buf = list_get_by_id(dev_data.DeviceId, localNode);
        if (dev_buf != NULL)
        {
            local_attribute_update(dev_buf, Data);
        }
    }
    break;
    case 8: //恢复出厂设置上报：”ReFactory”；
    {
        list_del_all(localNode);
        list_del_all_hilink(cloud_get_list_head(&g_SCloudControl));
        hilink_restore_factory_settings();
    }
    break;
    case 9: //COO网络信息上报：”CooInfo”；
    {
    }
    break;
    case 10: //邻居信息上报：”NeighborInfo”;
    {
    }
    break;
    case 11: //子节点信息上报：”ChildrenInfo”；
    {
    }
    break;
    case 12: //设置签名结果上报: “SetSig”；
    {
    }
    break;
    case 13: //查询签名结果上报: “GetSig”
    {
    }
    break;
    case 14: //Ack
    {
        log_debug("type:Ack");
    }
    break;
    default:

        break;
    }
heart:
    free(root);
    // list_print_all(localNode);
    return 0;
fail:
    free(root);
    log_error("json error\n");
    return -1;
}
