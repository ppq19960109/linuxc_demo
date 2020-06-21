#include <string.h>
#include <stdlib.h>
#include "cJSON.h"
#include "sqlite3.h"
#include "log.h"
#include "list.h"

const char *Command[] = {"Dispatch", "Report"};
// 下发”Dispatch”：
//  设备控制：”Ctrl”；
// 允许设备入网：”Add”；
// 设备删除：”Delete”；
// 查询设备属性：”Attribute”；
// 查询设备全部属性:”DevAttri”;
// 获取设备列表：”DevList”;
// 查询邻居信息：”NeighborInfo”;
// 子节点信息：”ChildrenInfo”；
// 恢复出厂设置:”ReFactory”；
// 允许重复上报参数:”RepeatReport”；
// 设置签名指令:”SetSig”；
// 查询签名指令:”GetSig”；
// 修改/查询子设备名称：SubName
// 上报”Report”：
// 设备注册上报：”Register”；
// 设备注销上报：”UnRegister”；
// 设备在线状态上报, “OnOff”；
// 设备属性上报：”Attribute”；
// 设备全部属性上报：”DevAttri”;
// 设备列表上报：”DevList”；
// 设备事件上报：”Event”；
// 恢复出厂设置上报：”ReFactory”；
// COO网络信息上报：”CooInfo”；
// 邻居信息上报：”NeighborInfo”;
// 子节点信息上报：”ChildrenInfo”；
// 设置签名结果上报: “SetSig”；
// 查询签名结果上报: “GetSig”

const char *TYPE_Report[] = {"Register", "UnRegister", "OnOff", "Attribute", "DevAttri", "DevList", "Event", "ReFactory", "CooInfo", "NeighborInfo", "ChildrenInfo", "SetSig", "GetSig"};
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

const char *TYPE_Dispatch[] = {"Ctrl", "Add", "Delete", "Attribute", "DevAttri", "DevList", "NeighborInfo", "ChildrenInfo", "ReFactory", "RepeatReport", "SetSig", "GetSig", "SubName"};

#define ENUM_TYPE_CASE(x) \
    case x:               \
        return (#x);

enum types_enum
{
    Register = 0,
    UnRegister,
    OnOff,
    Attribute,
    DevAttri,
    DevList,
    Event,
    ReFactory,
    CooInfo,
    NeighborInfo,
    ChildrenInfo,
    SetSig,
    GetSig
};

static inline const char *type_to_string(enum types_enum type)
{
    switch (type)
    {
        ENUM_TYPE_CASE(Register)
        ENUM_TYPE_CASE(UnRegister)
        ENUM_TYPE_CASE(OnOff)
        ENUM_TYPE_CASE(Attribute)
        ENUM_TYPE_CASE(DevAttri)
        ENUM_TYPE_CASE(DevList)
        ENUM_TYPE_CASE(Event)
        ENUM_TYPE_CASE(ReFactory)
        ENUM_TYPE_CASE(CooInfo)
        ENUM_TYPE_CASE(NeighborInfo)
        ENUM_TYPE_CASE(ChildrenInfo)
        ENUM_TYPE_CASE(SetSig)
        ENUM_TYPE_CASE(GetSig)
    }
    return "NULL";
}

const char test_json[] = {"{\
                            \"Command\" : \"Dispatch\", \
                            \"FrameNumber\" : \"00\",\
                            \"Type\" : \"Ctrl\",\
                            \"Data\" : [\
                                {\
                                    \"DeviceId\" : \"123456787654310\",\
                                    \"Key\" : \"Switch\",\
                                    \"Value\" : \"1\"\
                                }\
                            ]} "};

struct local_data_t
{
    char DeviceId[16];
    char ModelId[16];
    char Key[16];
    char Value[16];
};
typedef struct
{
    char Command;
    int FrameNumber;
    char Type;
    char GatewayId[16];
    struct local_data_t Data;

} local_dev_t;

typedef struct
{
    char GatewayId[16];
    char DeviceType[16];
    char DeviceId[16];
    char ModelId[16];
    char Version[16];
    char Secret[16];
    char Online[2];
    char RegisterStatus[2];
    char State[16];
    struct list_head node;
} dev_data_t;

LIST_HEAD(dev_list);

void list_del_all()
{
    dev_data_t *ptr, *next;

    list_for_each_entry_safe(ptr, next, &dev_list, node)
    {
        list_del(&ptr->node);
        free(ptr);
    }
}

void protlcol_init()
{
}

int read_from_bottom(const char *json)
{
    cJSON *root = cJSON_Parse(test_json);
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
    if (strcmp("Report", Command->valuestring) == 0)
    {
        log_error("Command is value invaild\n");
        goto fail;
    }
    //GatewayId字段
    cJSON *GatewayId = cJSON_GetObjectItem(root, "GatewayId");
    if (GatewayId == NULL)
    {
        log_error("GatewayId is value NULL\n");
        goto fail;
    }

    cJSON *Type = cJSON_GetObjectItem(root, "Type");
    if (Type == NULL)
    {
        log_error("Type is NULL\n");
        goto fail;
    }

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

    dev_data_t *dev_data = (dev_data_t *)malloc(sizeof(dev_data_t));
    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);

    cJSON *DeviceId = cJSON_GetObjectItem(array_sub, "DeviceId");
    if (DeviceId != NULL)
    {
        strcpy(dev_data->DeviceId, DeviceId->valuestring);
        log_info("DeviceId is %s\n", DeviceId->valuestring);
    }
    cJSON *ModelId = cJSON_GetObjectItem(array_sub, "ModelId");
    if (ModelId != NULL)
    {
        strcpy(dev_data->ModelId, ModelId->valuestring);
        log_info("ModelId is %s\n", ModelId->valuestring);
    }
    cJSON *Key = cJSON_GetObjectItem(array_sub, "Key");
    if (Key != NULL)
    {
        log_info("Key is %s\n", Key->valuestring);
    }
    cJSON *Value = cJSON_GetObjectItem(array_sub, "Value");
    if (Value != NULL)
    {
        log_info("Value is %s\n", Value->valuestring);
    }

    switch (type)
    {
    case 0:
    {
        cJSON *DeviceType = cJSON_GetObjectItem(array_sub, "DeviceType");
        if (DeviceType != NULL)
        {
            strcpy(dev_data->DeviceType, DeviceType->valuestring);
        }
        cJSON *Secret = cJSON_GetObjectItem(array_sub, "Secret");
        if (Secret != NULL)
        {
            strcpy(dev_data->DeviceId, Secret->valuestring);
        }
        list_add(&dev_data->node, &dev_list);
    }
    break;
    case 1:
    {
    }
    break;
    case 2:
    {
    }
    break;
    case 3:
    {
    }
    break;
    case 4:
    {
    }
    break;
    case 5:
    {
        cJSON *DeviceType = cJSON_GetObjectItem(array_sub, "DeviceType");
        if (DeviceType != NULL)
        {
            strcpy(dev_data->DeviceType, DeviceType->valuestring);
        }
        cJSON *Version = cJSON_GetObjectItem(array_sub, "Version");
        if (Version != NULL)
        {
            strcpy(dev_data->Version, Version->valuestring);
        }
        cJSON *Online = cJSON_GetObjectItem(array_sub, "Online");
        if (Online != NULL)
        {
            strcpy(dev_data->Online, Online->valuestring);
        }
        cJSON *RegisterStatus = cJSON_GetObjectItem(array_sub, "RegisterStatus");
        if (RegisterStatus != NULL)
        {
            strcpy(dev_data->RegisterStatus, RegisterStatus->valuestring);
        }
        list_add(&dev_data->node, &dev_list);
    }
    break;
    case 6:
    {
    }
    break;
    case 7:
    {
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
    default:
        free(dev_data);
        break;
    }

    // int array_size = cJSON_GetArraySize(Data);
    // for (int cnt = 0; cnt < array_size; cnt++)
    // {
    //     cJSON *array_sub = cJSON_GetArrayItem(Data, cnt);
    // }

    free(root);
    return 0;
fail:
    free(root);
    log_error("json error\n");
    return -1;
}

int write_to_bottom(void *ptr)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "Command", "Dispatch");
    cJSON_AddStringToObject(root, "Type", "DevAttri");

    cJSON *DataArr = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "Data", DataArr);

    cJSON *arrayItem = cJSON_CreateObject();
    cJSON_AddStringToObject(arrayItem, "DeviceId", "123456787654310");
    cJSON_AddStringToObject(arrayItem, "Key", "All");
    cJSON_AddItemToArray(DataArr, arrayItem);

    char *json = cJSON_Print(root);
    log_debug("%s\n", json);
    return 0;
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