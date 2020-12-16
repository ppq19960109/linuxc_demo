#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "local_send.h"
#include "local_receive.h"
#include "local_list.h"
#include "local_tcp_client.h"
#include "local_callback.h"

#include "event_main.h"
#include "uv_main.h"

#include "cloud_list.h"
#include "local_device.h"

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

static LocalControl_t g_SLocalControl;

void local_init_gateway()
{
    dev_local_t *dev_buf = local_get_gateway();
    memset(dev_buf, 0, sizeof(dev_local_t));
    strcpy(dev_buf->GatewayId, "");
    strcpy(dev_buf->DeviceId, STR_HOST_GATEWAYID);
    strcpy(dev_buf->ModelId, "000000");
    dev_buf->Online = 1;
}

void local_control_init()
{
#if USE_LIBEVENT
    printf("libevent is start\n");
    register_openCallback(event_main_open);
#elif USE_LIBUV
    printf("libuv is start\n");
    register_openCallback(uv_main_open);
#else
    printf("tcp client is start\n");
    register_openCallback(tcp_client_open);
#endif
    pthread_mutex_init(&g_SLocalControl.mutex, NULL);
    list_init_local(&g_SLocalControl.head);
    local_init_gateway();
}

void local_control_destory()
{
    list_del_all_local();
    pthread_mutex_destroy(&g_SLocalControl.mutex);
}

struct list_head *local_get_list_head()
{
    return &g_SLocalControl.head;
}

char *local_get_sendData()
{
    return g_SLocalControl.sendData;
}

pthread_mutex_t *local_get_mutex()
{
    return &g_SLocalControl.mutex;
}

dev_local_t *local_get_gateway()
{
    return &g_SLocalControl.gateway;
}

int local_attr_gateway(dev_local_t *dev_data, cJSON *Data)
{
    if (strcmp(STR_HOST_GATEWAYID, dev_data->DeviceId))
        return -1;
    if (dev_data->private == NULL)
    {
        dev_data->private = malloc(sizeof(DevGateway_t));
        memset(dev_data->private, 0, sizeof(DevGateway_t));
    }

    if (Data == NULL)
        return 0;
    cJSON *Key, *array_sub;

    DevGateway_t *dev = (DevGateway_t *)dev_data->private;
    int array_size = cJSON_GetArraySize(Data);
    for (int i = 0; i < array_size; ++i)
    {
        array_sub = cJSON_GetArrayItem(Data, i);
        Key = cJSON_GetObjectItem(array_sub, STR_KEY);
        if (Key == NULL)
            continue;
        if (strcmp(Key->valuestring, STR_PERMITJOINING) == 0)
        {
            char_copy_from_json(array_sub, STR_VALUE, &dev->PermitJoining);
        }
    }
    return 0;
}

void local_load_device_info(cJSON *root, cJSON *Data, const char *Params)
{
    dev_local_t *dev_buf;
    cJSON *array_sub;

    int array_size = cJSON_GetArraySize(Data);

    for (int cnt = 0; cnt < array_size; cnt++)
    {
        array_sub = cJSON_GetArrayItem(Data, cnt);
        if (array_sub == NULL)
            continue;
        dev_buf = list_get_by_id_local(cJSON_GetObjectItem(array_sub, STR_DEVICEID)->valuestring);
        if (dev_buf == NULL)
        {
            dev_buf = (dev_local_t *)malloc(sizeof(dev_local_t));
            memset(dev_buf, 0, sizeof(dev_local_t));
            str_copy_from_json(root, STR_GATEWAYID, dev_buf->GatewayId);
            str_copy_from_json(array_sub, STR_DEVICEID, dev_buf->DeviceId);
            str_copy_from_json(array_sub, STR_MODELID, dev_buf->ModelId);
            str_copy_from_json(array_sub, STR_VERSION, dev_buf->Version);
            char_copy_from_json(array_sub, STR_ONLINE, &dev_buf->Online);

            if (local_attribute_update(dev_buf, cJSON_GetObjectItem(array_sub, Params)) != 0)
            {
                free(dev_buf);
                continue;
            }
            list_add_local(&dev_buf->node);
        }
        else
        {
            if (strcmp(cJSON_GetObjectItem(array_sub, STR_MODELID)->valuestring, dev_buf->ModelId))
            {
                log_error("DeviceId identical,but ModelId inequality\n");
                continue;
            }
            str_copy_from_json(array_sub, STR_VERSION, dev_buf->Version);
            char online = dev_buf->Online;
            char_copy_from_json(array_sub, STR_ONLINE, &dev_buf->Online);

            if (local_attr_gateway(dev_buf, cJSON_GetObjectItem(array_sub, Params)) != 0)
            {
                if (dev_buf->Online == 1 || online != dev_buf->Online)
                    hyLinkDevStatus(dev_buf, dev_buf->Online);
                local_attribute_update(dev_buf, cJSON_GetObjectItem(array_sub, Params));
            }
        }
    }
}

int read_from_local(const char *json)
{
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        log_error("root is NULL\n");
        goto fail;
    }
    // char *rjson = cJSON_Print(root);
    // log_debug("%s\n", rjson);
    // free(rjson);

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

    dev_local_t dev_data;
    dev_local_t *dev_buf = NULL;
    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);
    if (array_sub != NULL)
        str_copy_from_json(array_sub, STR_DEVICEID, dev_data.DeviceId);
    else
    {
        goto fail;
    }

    switch (type)
    {
    case 0: //设备注册上报：”Register”；
    {
        dev_buf = list_get_by_id_local(dev_data.DeviceId);
        if (dev_buf == NULL)
        {
            dev_buf = (dev_local_t *)malloc(sizeof(dev_local_t));
            memset(dev_buf, 0, sizeof(dev_local_t));
            str_copy_from_json(root, STR_GATEWAYID, dev_buf->GatewayId);
            str_copy_from_json(array_sub, STR_DEVICEID, dev_buf->DeviceId);
            str_copy_from_json(array_sub, STR_MODELID, dev_buf->ModelId);
            dev_buf->Online = INT_ONLINK;
            list_add_local(&dev_buf->node);
            if (local_attribute_update(dev_buf, NULL) != 0)
            {
                log_error("local_attribute_update error\n");
                list_del_dev_local(dev_buf);
                break;
            }
            
        }
    }
    break;
    case 1: //设备注销上报：”UnRegister”；
    {
        dev_buf = list_get_by_id_local(dev_data.DeviceId);
        if (dev_buf != NULL)
        {
            write_delete_dev(dev_buf->DeviceId);
            hyLinkDevStatus(dev_buf, DEV_RESTORE);
            list_del_by_id_cloud(dev_data.DeviceId);
            list_del_dev_local(dev_buf);
        }
        else
        {
            log_error("UnRegister device not exist\n");
        }
    }
    break;
    case 2: //设备在线状态上报, “OnOff”
    {

        dev_buf = list_get_by_id_local(dev_data.DeviceId);
        cJSON *Key = cJSON_GetObjectItem(array_sub, STR_KEY);

        if (dev_buf != NULL && Key != NULL && strcmp(Key->valuestring, STR_ONLINE) == 0)
        {
            char_copy_from_json(array_sub, STR_VALUE, &dev_buf->Online);
            hyLinkDevStatus(dev_buf, dev_buf->Online);

            // time_t now;
            // time(&now);
            // log_info("sub device OnOff:%s,%d,%s\n", dev_data.DeviceId, dev_buf->Online, ctime(&now));
        }
    }
    break;
    case 3: //设备属性上报：”Attribute”；
    case 4: //设备全部属性上报：”DevAttri”;
    case 5: //设备事件上报：”Event”；
    {
        dev_buf = list_get_by_id_local(dev_data.DeviceId);
        if (dev_buf != NULL)
        {
            if (dev_buf->Online == INT_OFFLINK)
            {
                dev_buf->Online = INT_ONLINK;
                hyLinkDevStatus(dev_buf, dev_buf->Online);
            }
            if (local_attr_gateway(dev_buf, Data) != 0)
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
        local_load_device_info(root, Data, NULL);
    }
    break;
    case 7: //获取设备列表详细信息(网关指令)DevsInfo
    {
        local_load_device_info(root, Data, STR_PARAMS);
    }
    break;
    case 8: //恢复出厂设置上报：”ReFactory”；
    {
        hyLinkSystem(INT_REFACTORY);
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
        // log_debug("type:Ack\n");
    }
    break;
    default:

        break;
    }

heart:
    cJSON_Delete(root);
    // list_print_all_local();
    return 0;
fail:
    cJSON_Delete(root);
    log_error("json error\n");
    return -1;
}
