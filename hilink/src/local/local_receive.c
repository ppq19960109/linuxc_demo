#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "local_send.h"
#include "local_receive.h"
#include "local_list.h"
#include "local_tcp_client.h"

#include "cloud_list.h"
#include "cloud_send.h"
#include "cloud_receive.h"

#include "socket.h"

static char *s_typeReport[] = {
    "Register",
    "UnRegister",
    "OnOff",
    "Attribute",
    "DevAttri",
    "Event",
    "DevList",
    "DevsInfo",
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

LocalControl_t g_SLocalControl;

dev_data_t *create_gateway(struct list_head *head)
{

    dev_data_t *dev_buf = list_get_by_id(STR_HOST_GATEWAYID, head);
    if (dev_buf != NULL)
    {
        return dev_buf;
    }
    dev_buf = (dev_data_t *)malloc(sizeof(dev_data_t));
    memset(dev_buf, 0, sizeof(dev_data_t));
    strcpy(dev_buf->GatewayId, "");
    strcpy(dev_buf->DeviceId, STR_HOST_GATEWAYID);
    strcpy(dev_buf->ModelId, "000000");
    dev_buf->Online = 1;

    if (local_attribute_update(dev_buf, NULL) != 0)
    {
        log_error("create_gateway error\n");
        free(dev_buf);
        return NULL;
    }

    list_add(&dev_buf->node, head);
    return dev_buf;
}

void local_control_init(LocalControl_t *localControl)
{
    INIT_LIST_HEAD(&localControl->head);
    create_gateway(&localControl->head);
}

void local_control_destory(LocalControl_t *localControl)
{
    list_del_all(&localControl->head);
}

struct list_head *local_get_list_head(LocalControl_t *localControl)
{
    return &localControl->head;
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
            char_copy_from_json(array_sub, STR_ONLINE, &dev_buf->Online);
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
                log_error("DeviceId identical,but ModelId inequality\n");
                continue;
            }
            str_copy_from_json(array_sub, STR_VERSION, dev_buf->Version);
            str_copy_from_json(array_sub, STR_ONLINE, &dev_buf->Online);

            char_copy_from_json(array_sub, STR_REGISTERSTATUS, &dev_buf->RegisterStatus);
            local_attribute_update(dev_buf, cJSON_GetObjectItem(array_sub, Params));

            HilinkSyncBrgDevStatus(dev_buf->DeviceId, dev_buf->Online);
        }
    }
}

void recv_toLocal(char *data, int len)
{
    if (data[0] == 0x02)
    {
        for (int i = 0; i < len; ++i)
        {
            if (data[i] == 0x02)
            {
                log_debug("recv_toLocal:%d,%s\n", len, &data[1]);
                read_from_local(&data[i + 1], local_get_list_head(&g_SLocalControl));
            }
        }
    }
    else
    {
        log_debug("recv_toLocal:%d,%s\n", len, data);
        read_from_local(data, local_get_list_head(&g_SLocalControl));
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
            dev_buf->Online = 1;
            if (local_attribute_update(dev_buf, NULL) != 0)
            {
                log_error("local_attribute_update error\n");
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
            log_error("UnRegister device not exist\n");
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
            hilink_online(dev_buf);
        }
    }
    break;
    case 3: //设备属性上报：”Attribute”；
    case 4: //设备全部属性上报：”DevAttri”;
    case 5: //设备事件上报：”Event”；恢复出厂设置上报：”
    {

        dev_buf = list_get_by_id(dev_data.DeviceId, localNode);
        if (dev_buf != NULL)
        {
            cJSON *Key = cJSON_GetObjectItem(array_sub, STR_KEY);
            if (Key != NULL)
            {
                if (strcmp(Key->valuestring, STR_VERSION) == 0)
                {
                    str_copy_from_json(array_sub, STR_VALUE, dev_buf->Version);
                }
            }
            local_attribute_update(dev_buf, Data);
        }
        else
        {
            log_error("sub device not exist:%s\n", dev_data.DeviceId);
        }
    }
    break;
    case 6: //设备列表上报：”DevList”
    {
        local_load_device_info(root, Data, STR_PARAMS, NULL);
    }
    break;
    case 7: //获取设备列表详细信息(网关指令)DevsInfo
    {
        local_load_device_info(root, Data, STR_PARAMS, localNode);
    }
    break;
    case 8: //恢复出厂设置上报：”ReFactory”；
    {
        cloud_restart_reFactory(INT_REFACTORY);
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
        log_debug("type:Ack\n");
    }
    break;
    default:

        break;
    }
heart:
    cJSON_Delete(root);
    // list_print_all(localNode);
    return 0;
fail:
    cJSON_Delete(root);
    log_error("json error\n");
    return -1;
}
