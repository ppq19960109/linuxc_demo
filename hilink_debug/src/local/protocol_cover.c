#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "socket.h"
#include "protocol_cover.h"
#include "dev_private.h"
#include "list_tool.h"

#include "list_hilink.h"
#include "hilink.h"
// char *Command[] = {"Dispatch", "Report"};

char *TYPE_Report[] = {
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

// char *TYPE_Dispatch[] = {"Ctrl", "Add", "Delete", "Attribute", "DevAttri", "DevList", "NeighborInfo", "ChildrenInfo", "ReFactory", "RepeatReport", "SetSig", "GetSig", "SubName"};

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

int str_search(const char *key, char **pstr, int num)
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

int strn_search(const char *key, char **pstr, int num, int n)
{
    int i;

    for (i = 0; i < num; i++)
    {
        // 指针数组  p首先是个指针  然后指向类型是地址 所以是二级指针
        if (strncmp(*pstr++, key, n) == 0)
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
    protocol_data.discoverMode = 0;
    INIT_LIST_HEAD(&protocol_data.dev_list);
    protocol_data.pid = net_client(&protocol_data);
}

void protlcol_destory()
{
    pthread_cancel(protocol_data.pid);
    protocol_data.pid = 0;

    if (protocol_data.socketfd != 0)
    {
        Close(protocol_data.socketfd);
        protocol_data.socketfd = 0;
    }
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
    // log_debug("%s\n", cJSON_Print(root));

    //command字段
    cJSON *Command = cJSON_GetObjectItem(root, "Command");
    if (Command == NULL)
    {
        log_error("Command is NULL\n");
        goto fail;
    }
    if (strcmp("Report", Command->valuestring) == 0)
    {
        log_debug("Command is Report\n");
    }
    else if (strcmp("BeatHeartResponse", Command->valuestring) == 0)
    {
        log_debug("Command is BeatHeartResponse\n");
        goto heart;
    }
    else
    {
        log_error("Command is value invaild:%s\n", Command->valuestring);
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
    //从type数组中查找type
    int type = str_search(Type->valuestring, TYPE_Report, sizeof(TYPE_Report) / POINTER_SIZE);
    if (type == -1)
    {
        log_error("Type is no exist\n");
        goto fail;
    }

    dev_data_t dev_data;
    //GatewayId字段
    // str_copy_from_json(root, "GatewayId", dev_data.GatewayId);

    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);
    str_copy_from_json(array_sub, "DeviceId", dev_data.DeviceId);

    dev_data_t *dev_buf;
    switch (type)
    {
    case 0: //设备注册上报：”Register”；
    {
        // dev_data_t *dev_add;
        dev_buf = list_get_by_id(dev_data.DeviceId, &protocol_data.dev_list);
        if (dev_buf == NULL)
        {
            dev_buf = (dev_data_t *)malloc(sizeof(dev_data_t));
            memset(dev_buf, 0, sizeof(dev_data_t));
            str_copy_from_json(root, "GatewayId", dev_buf->GatewayId);
            str_copy_from_json(array_sub, "DeviceId", dev_buf->DeviceId);
            str_copy_from_json(array_sub, "ModelId", dev_buf->ModelId);

            if (dev_private_attribute(dev_buf, NULL) != 0)
            {
                free(dev_buf);
                break;
            }

            list_add(&dev_buf->node, &protocol_data.dev_list);
        }
        str_copy_from_json(array_sub, "DeviceType", dev_buf->DeviceType);
        str_copy_from_json(array_sub, "Secret", dev_buf->Secret);
    }
    break;
    case 1: //设备注销上报：”UnRegister”；
    {
        log_debug("设备注销上报：”UnRegister”；");
        dev_buf = list_get_by_id(dev_data.DeviceId, &protocol_data.dev_list);
        dev_hilink_t *dev_hilink = list_get_by_id_hilink(dev_data.DeviceId, &hilink_handle.node);
        if (dev_hilink != NULL)
        {
            HilinkSyncBrgDevStatus(dev_data.DeviceId, DEV_RESTORE);
            list_del_dev_hilink(dev_hilink);
            // list_del(&dev_hilink->node);
            list_del_dev(dev_buf);
            // list_del(&dev_buf->node);
        }
        else
        {
            log_error("UnRegister device not exist");
        }
    }
    break;
    case 2: //设备在线状态上报, “OnOff”
    {
        cJSON *Key = cJSON_GetObjectItem(array_sub, "Key");
        dev_buf = list_get_by_id(dev_data.DeviceId, &protocol_data.dev_list);
        if (dev_buf != NULL && Key != NULL && strcmp(Key->valuestring, "Online") == 0)
        {
            char_copy_from_json(array_sub, "Value", &dev_buf->Online);
            HilinkSyncBrgDevStatus(dev_data.DeviceId, dev_buf->Online);
        }
    }
    break;
    case 3: //设备属性上报：”Attribute”；
    {
        // log_debug("设备属性上报：”Attribute”；");

        dev_buf = list_get_by_id(dev_data.DeviceId, &protocol_data.dev_list);
        if (dev_buf != NULL)
        {
            cJSON *Key = cJSON_GetObjectItem(array_sub, "Key");
            if (Key != NULL && strcmp(Key->valuestring, "Version") == 0)
            {
                str_copy_from_json(array_sub, "Value", dev_buf->Version);
            }

            dev_private_attribute(dev_buf, Data);
        }
        else
        {
            if (strcmp(GATEWAYID, dev_data.DeviceId) == 0)
            {
                cJSON *Key = cJSON_GetObjectItem(array_sub, "Key");
                if (Key != NULL && strcmp(Key->valuestring, "PermitJoining") == 0)
                {
                    char_copy_from_json(array_sub, "Value", &protocol_data.discoverMode);

                    cJSON *root = cJSON_CreateObject();
                    cJSON_AddNumberToObject(root, "on", protocol_data.discoverMode);
                    char *json = cJSON_PrintUnformatted(root);
                    hilink_upload_char_state("switch", json, strlen(json) + 1);
                    free(root);
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
        int array_size = cJSON_GetArraySize(Data);
        // dev_data_t *dev_add;
        for (int cnt = 0; cnt < array_size; cnt++)
        {
            array_sub = cJSON_GetArrayItem(Data, cnt);
            dev_buf = list_get_by_id(cJSON_GetObjectItem(array_sub, "DeviceId")->valuestring, &protocol_data.dev_list);
            if (dev_buf == NULL)
            {
                dev_buf = (dev_data_t *)malloc(sizeof(dev_data_t));
                memset(dev_buf, 0, sizeof(dev_data_t));
                str_copy_from_json(root, "GatewayId", dev_buf->GatewayId);
                str_copy_from_json(array_sub, "DeviceId", dev_buf->DeviceId);
                str_copy_from_json(array_sub, "ModelId", dev_buf->ModelId);
                str_copy_from_json(array_sub, "Version", dev_buf->Version);
                str_copy_from_json(array_sub, "Online", &dev_buf->Online);
                char_copy_from_json(array_sub, "RegisterStatus", &dev_buf->RegisterStatus);

                if (dev_private_attribute(dev_buf, NULL) != 0)
                {
                    free(dev_buf);
                    continue;
                }
                list_add(&dev_buf->node, &protocol_data.dev_list);
            }
            else
            {
                if (strcmp(cJSON_GetObjectItem(array_sub, "ModelId")->valuestring, dev_buf->ModelId))
                {
                    log_error("DeviceId identical,but ModelId inequality");
                    break;
                }
                str_copy_from_json(array_sub, "Version", dev_buf->Version);
                str_copy_from_json(array_sub, "Online", &dev_buf->Online);

                char_copy_from_json(array_sub, "RegisterStatus", &dev_buf->RegisterStatus);
                dev_private_attribute(dev_buf, NULL);

                HilinkSyncBrgDevStatus(dev_buf->DeviceId, dev_buf->Online);
            }
        }
    }
    break;
    case 6: //获取设备列表详细信息(网关指令)DevsInfo
    {
        // log_debug("DevsInfo");
        int array_size = cJSON_GetArraySize(Data);

        for (int cnt = 0; cnt < array_size; cnt++)
        {
            array_sub = cJSON_GetArrayItem(Data, cnt);
            // str_copy_from_json(array_sub, "DeviceId", dev_data.DeviceId);
            // str_copy_from_json(array_sub, "ModelId", dev_data->ModelId);
            dev_buf = list_get_by_id(cJSON_GetObjectItem(array_sub, "DeviceId")->valuestring, &protocol_data.dev_list);
            if (dev_buf == NULL)
            {
                dev_buf = (dev_data_t *)malloc(sizeof(dev_data_t));
                memset(dev_buf, 0, sizeof(dev_data_t));
                str_copy_from_json(root, "GatewayId", dev_buf->GatewayId);
                str_copy_from_json(array_sub, "DeviceId", dev_buf->DeviceId);
                str_copy_from_json(array_sub, "ModelId", dev_buf->ModelId);
                str_copy_from_json(array_sub, "Version", dev_buf->Version);
                str_copy_from_json(array_sub, "Online", &dev_buf->Online);
                char_copy_from_json(array_sub, "RegisterStatus", &dev_buf->RegisterStatus);

                if (dev_private_attribute(dev_buf, cJSON_GetObjectItem(array_sub, "Params")) != 0)
                {
                    free(dev_buf);
                    continue;
                }
                list_add(&dev_buf->node, &protocol_data.dev_list);
            }
            else
            {
                if (strcmp(cJSON_GetObjectItem(array_sub, "ModelId")->valuestring, dev_buf->ModelId))
                {
                    log_error("DeviceId identical,but ModelId inequality");
                    break;
                }
                str_copy_from_json(array_sub, "Version", dev_buf->Version);
                str_copy_from_json(array_sub, "Online", &dev_buf->Online);

                char_copy_from_json(array_sub, "RegisterStatus", &dev_buf->RegisterStatus);
                dev_private_attribute(dev_buf, cJSON_GetObjectItem(array_sub, "Params"));

                HilinkSyncBrgDevStatus(dev_buf->DeviceId, dev_buf->Online);
            }
        }
    }
    break;
    case 7: //设备事件上报：”Event”；恢复出厂设置上报：”
    {
        dev_buf = list_get_by_id(dev_data.DeviceId, &protocol_data.dev_list);
        if (dev_buf != NULL)
        {
            dev_private_attribute(dev_buf, Data);
        }
    }
    break;
    case 8: //恢复出厂设置上报：”ReFactory”；
    {
        list_del_all(&protocol_data.dev_list);
        list_del_all_hilink(&hilink_handle.node);
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
    // list_print_all(&protocol_data.dev_list);
    return 0;
fail:
    free(root);
    log_error("json error\n");
    return -1;
}

//-----------------------------------------------------
void local_reFactory()
{
    list_del_all(&protocol_data.dev_list);
    hilink_handle_destory();
    hilink_restore_factory_settings();
}

char *hanyar_cmd[] = {"Add", "DevsInfo", "DevAttri", "ReFactory"};

int write_cmd(char *cmd, char *DeviceId, char *Value)
{
    local_dev_t local_cmd = {0};
    int index_cmd = str_search(cmd, hanyar_cmd, POINTER_SIZE);
    switch (index_cmd)
    {
    case 0:
    {
        local_cmd.FrameNumber = 0;
        strcpy(local_cmd.Type, "Add");
        strcpy(local_cmd.Data.DeviceId, GATEWAYID);
        strcpy(local_cmd.Data.Key, "Time");
        strcpy(local_cmd.Data.Value, Value);
    }
    break;
    case 1:
    {
        local_cmd.FrameNumber = 0;
        strcpy(local_cmd.Type, "DevsInfo");
        strcpy(local_cmd.Data.DeviceId, GATEWAYID);
        strcpy(local_cmd.Data.Key, "DevsInfo");
    }
    break;
    case 2:
    {
        local_cmd.FrameNumber = 0;
        strcpy(local_cmd.Type, "DevAttri");
        strcpy(local_cmd.Data.DeviceId, DeviceId);
        strcpy(local_cmd.Data.Key, "All");
    }
    break;
    case 3:
    {
        local_cmd.FrameNumber = 0;
        strcpy(local_cmd.Type, "ReFactory");
    }
    break;
    default:
        return -1;
    }

    // HilinkGetDeviceSn(sizeof(net_access_cmd.Data.DeviceId),net_access_cmd.Data.DeviceId);
    // if(strlen(net_access_cmd.Data.DeviceId)==0)
    // {
    //     HILINK_GetMacAddr(net_access_cmd.Data.DeviceId, sizeof(net_access_cmd.Data.DeviceId));
    // }
    int ret = write_to_local(&local_cmd);
    if (ret < 0)
    {
        log_error("write_net_access error");
    }
    return ret;
}

int writeToHaryan(const char *data, int socketfd, char *sendBuf, int bufLen)
{
    if (socketfd == 0)
    {
        log_error("socketfd not exist");
        return -1;
    }
    int datalen = strlen(data);
    if (datalen + 3 <= bufLen)
    {
        sendBuf[0] = 0x02;
        strcpy(&sendBuf[1], data);
        sendBuf[datalen + 1] = 0x03;
        sendBuf[datalen + 2] = 0x00;
        if (socketfd != 0)
        {
            log_warn("%s \nprotocol_data.socketfd:%d\n", &sendBuf[1], socketfd);
            // for(int i=0;i<datalen + 3;++i)
            // {
            //     printf("%x,",sendBuf[i]);
            // }
            // printf("\n");
            return Write(socketfd, sendBuf, datalen + 3);
        }
    }
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

    char *json = cJSON_PrintUnformatted(root);

    int ret = writeToHaryan(json, protocol_data.socketfd, protocol_data.sendData, SENDTOLOCAL_SIZE);
    if (ret < 0)
    {
        log_error("writeToHaryan error ret:%d,%s", ret, strerror(errno));
    }
    free(json);

    free(root);
    return ret;
fail:
    free(root);
    return -1;
}
