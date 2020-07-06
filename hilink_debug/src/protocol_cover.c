#include "protocol_cover.h"
#include "dev_private.h"

const char *Command[] = {"Dispatch", "Report"};

const char *TYPE_Report[] = {"Register", "UnRegister", "OnOff", "Attribute", "DevAttri", "DevList", "Event", "ReFactory", "CooInfo", "NeighborInfo", "ChildrenInfo", "SetSig", "GetSig"};

const char *TYPE_Dispatch[] = {"Ctrl", "Add", "Delete", "Attribute", "DevAttri", "DevList", "NeighborInfo", "ChildrenInfo", "ReFactory", "RepeatReport", "SetSig", "GetSig", "SubName"};

// #define ENUM_TYPE_CASE(x) \
//     case x:               \
//         return (#x);

// enum types_enum
// {
//     Register = 0,
//     UnRegister,
//     OnOff,
//     Attribute,
//     DevAttri,
//     DevList,
//     Event,
//     ReFactory,
//     CooInfo,
//     NeighborInfo,
//     ChildrenInfo,
//     SetSig,
//     GetSig
// };

// static inline const char *type_to_string(enum types_enum type)
// {
//     switch (type)
//     {
//         ENUM_TYPE_CASE(Register)
//         ENUM_TYPE_CASE(UnRegister)
//         ENUM_TYPE_CASE(OnOff)
//         ENUM_TYPE_CASE(Attribute)
//         ENUM_TYPE_CASE(DevAttri)
//         ENUM_TYPE_CASE(DevList)
//         ENUM_TYPE_CASE(Event)
//         ENUM_TYPE_CASE(ReFactory)
//         ENUM_TYPE_CASE(CooInfo)
//         ENUM_TYPE_CASE(NeighborInfo)
//         ENUM_TYPE_CASE(ChildrenInfo)
//         ENUM_TYPE_CASE(SetSig)
//         ENUM_TYPE_CASE(GetSig)
//     }
//     return "NULL";
// }

const char report_test_json[] = {"{\
      \"Command\":\"Report\",\
      \"FrameNumber\":\"00\",\
      \"GatewayId\":\"0006D12345678909\",\
      \"Type\":\"DevList\",\
      \"TotalNumber\":\"21\",\
      \"AlreadyReportNumber\":\"21\",\
      \"Data\":[\
                {\
                \"DeviceId\":\"1234567876543210\",\
                \"ModelId\":\"0a0c3c\",\
                \"Name\":\"三位开关\",\
                \"Version\":\"20180201\",\
                \"Online\": \"1\",\
                \"RegisterStatus\": \"1\"\
                }\
                ]\
}"};

static int str_search(const char *key, const char **pstr, int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        // 指针数组  p首先是个指针  然后指向类型是地址 所以是二级指针
        if (strcmp(*pstr++, key) == 0)
        {
            return i;
        }
    }

    return -1;
}

char char_copy_from_json(cJSON *json, char *src, char *dst)
{
    cJSON *obj = cJSON_GetObjectItem(json, src);
    if (obj != NULL)
    {
        *dst = atoi(obj->valuestring);
        return *dst;
    }
    return -1;
}

int int_copy_from_json(cJSON *json, char *src, int *dst)
{
    cJSON *obj = cJSON_GetObjectItem(json, src);
    if (obj != NULL)
    {
        *dst = atoi(obj->valuestring);
        return *dst;
    }
    return -1;
}

int str_copy_from_json(cJSON *json, char *src, char *dst)
{
    cJSON *obj = cJSON_GetObjectItem(json, src);
    if (obj != NULL)
    {
        strcpy(dst, obj->valuestring);
        return 0;
    }
    return -1;
}

int isStrNotNull(const char *str)
{
    return strlen(str);
}
//-------------------------------------------------------------------------
protocol_data_t protocol_data;

void protlcol_init()
{
    INIT_LIST_HEAD(&protocol_data.dev_list);
    // net_client(&protocol_data.socketfd);
}

void protlcol_destory()
{
    Close(protocol_data.socketfd);
    list_del_all(&protocol_data.dev_list);
}

int read_from_local(const char *json)
{
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        log_error("root is NULL\n");
        goto fail;
    }
    // log_debug("%s\n",cJSON_Print(root));

    //command字段
    cJSON *Command = cJSON_GetObjectItem(root, "Command");
    if (Command == NULL)
    {
        log_error("Command is NULL\n");
        goto fail;
    }
    if (strcmp("Report", Command->valuestring) != 0)
    {
        log_error("Command is value invaild\n");
        goto fail;
    }

    //Type字段
    cJSON *Type = cJSON_GetObjectItem(root, "Type");
    if (Type == NULL)
    {
        log_error("Type is NULL\n");
        goto fail;
    }
    //Data字段
    cJSON *Data = cJSON_GetObjectItem(root, "Data");
    if (Data == NULL)
    {
        log_error("Data is NULL\n");
        goto fail;
    }
    //
    int type = str_search(Type->valuestring, TYPE_Report, sizeof(TYPE_Report));
    if (type == -1)
    {
        log_error("Type is no exist\n");
        goto fail;
    }

    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);

    dev_data_t *dev_data = (dev_data_t *)malloc(sizeof(dev_data_t));
    memset(dev_data, 0, sizeof(dev_data_t));
    //GatewayId字段
    str_copy_from_json(root, "GatewayId", dev_data->GatewayId);
    str_copy_from_json(array_sub, "DeviceId", dev_data->DeviceId);
    str_copy_from_json(array_sub, "ModelId", dev_data->ModelId);
    cJSON *Key = cJSON_GetObjectItem(array_sub, "Key");
    if (Key != NULL)
    {
        log_info("Key is %s\n", Key->valuestring);
    }
    // cJSON *Value = cJSON_GetObjectItem(array_sub, "Value");
    // if (Value != NULL)
    // {
    //     log_info("Value is %s\n", Value->valuestring);
    // }

    switch (type)
    {
    case 0: //设备注册上报：”Register”；
    {
        dev_data_t *dev_buf = list_get_by_id(dev_data->DeviceId, &protocol_data.dev_list);
        str_copy_from_json(array_sub, "DeviceType", dev_buf->DeviceType);
        str_copy_from_json(array_sub, "Secret", dev_buf->Secret);
        if (dev_buf == NULL)
        {
            list_add(&dev_data->node, &protocol_data.dev_list);
            goto add;
        }
    }
    break;
    case 1: //设备注销上报：”UnRegister”；
    {
        dev_data_t *dev_buf = list_get_by_id(dev_data->DeviceId, &protocol_data.dev_list);
        if (dev_buf != NULL)
        {
            list_del(&dev_buf->node);
        }
    }
    break;
    case 2: //设备在线状态上报, “OnOff”
    {
        if (Key != NULL && strcmp(Key->valuestring, "Online") == 0)
        {
            dev_data_t *dev_buf = list_get_by_id(dev_data->DeviceId, &protocol_data.dev_list);
            char_copy_from_json(array_sub, "Value", &dev_buf->Online);
        }
    }
    break;
    case 3: //设备属性上报：”Attribute”；
    {
        dev_data_t *dev_buf = list_get_by_id(dev_data->DeviceId, &protocol_data.dev_list);
        if (Key != NULL && strcmp(Key->valuestring, "Version") == 0)
        {
            str_copy_from_json(array_sub, "Value", dev_buf->Version);
        }

        dev_private_attribute(dev_buf, Data);
    }
    break;
    case 4:
    {
    }
    break;
    case 5:
    {
        str_copy_from_json(array_sub, "DeviceType", dev_data->DeviceType);
        str_copy_from_json(array_sub, "Version", dev_data->Version);
        str_copy_from_json(array_sub, "Online", &dev_data->Online);
        char_copy_from_json(array_sub, "RegisterStatus", &dev_data->RegisterStatus);
        list_add(&dev_data->node, &protocol_data.dev_list);
        goto add;
    }
    break;
    case 6:
    {
        dev_data_t *dev_buf = list_get_by_id(dev_data->DeviceId, &protocol_data.dev_list);
        dev_private_event(dev_buf, Data);
    }
    break;
    case 7:
    {
        //reboot
        list_del_all(&protocol_data.dev_list);
    }
    break;
    case 8:
    {
    }
    break;
    case 9:
    {
    }
    break;
    case 10:
    {
    }
    break;
    case 11:
    {
    }
    break;
    case 12:
    {
    }
    break;
    default:

        break;
    }

    // int array_size = cJSON_GetArraySize(Data);
    // for (int cnt = 0; cnt < array_size; cnt++)
    // {
    //     cJSON *array_sub = cJSON_GetArrayItem(Data, cnt);
    // }
    free(dev_data);
add:
    free(root);
    list_print_all(&protocol_data.dev_list);
    return 0;
fail:
    free(root);
    log_error("json error\n");
    return -1;
}

int write_to_local(void *ptr)
{
    if (NULL == ptr)
    {
        return -1;
    }
    local_dev_t *local_dev = (local_dev_t *)ptr;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "Command", "Dispatch");
    char str[3];
    sprintf(str, "%d", local_dev->FrameNumber);
    cJSON_AddStringToObject(root, "FrameNumber", str);
    cJSON_AddStringToObject(root, "Type", local_dev->Type);

    cJSON *DataArray = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "Data", DataArray);

    cJSON *arrayItem = cJSON_CreateObject();
    if (isStrNotNull(local_dev->Data.DeviceId))
        cJSON_AddStringToObject(arrayItem, "DeviceId", local_dev->Data.DeviceId);
    if (isStrNotNull(local_dev->Data.ModelId))
        cJSON_AddStringToObject(arrayItem, "ModelId", local_dev->Data.ModelId);
    if (isStrNotNull(local_dev->Data.Key))
        cJSON_AddStringToObject(arrayItem, "Key", local_dev->Data.Key);
    if (isStrNotNull(local_dev->Data.Value))
        cJSON_AddStringToObject(arrayItem, "Value", local_dev->Data.Value);
    cJSON_AddItemToArray(DataArray, arrayItem);

    char *json = cJSON_Print(root);
    log_debug("%s\n", json);

    free(json);

    free(root);
    return 0;
fail:
    free(root);
    return -1;
}

int read_from_cloud(void *ptr)
{

    return 0;
}

int write_to_cloud(void *ptr)
{

    return 0;
}

int cloud_to_bottom()
{
    return 0;
}

int bottom_to_cloud()
{
    return 0;
}