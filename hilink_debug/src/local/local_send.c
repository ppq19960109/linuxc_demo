#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "local_send.h"
#include "local_device.h"
#include "cloud_send.h"
#include "local_list.h"
#include "local_receive.h"

#include "socket.h"
#include "tool.h"

#include "uv_main.h"
#include "event_main.h"

static char *s_hanyarCmd[] = {STR_ADD, STR_DEVSINFO, STR_DEVATTRI, STR_REFACTORY};
const SAttrInfo g_SHamyarCmd = {
    .attr = s_hanyarCmd,
    .attrLen = sizeof(s_hanyarCmd) / sizeof(s_hanyarCmd[0])};

int write_hanyar_cmd(char *cmd, char *DeviceId, char *Value)
{
    local_dev_t local_cmd = {0};
    int index_cmd = str_search(cmd, g_SHamyarCmd.attr, g_SHamyarCmd.attrLen);
    switch (index_cmd)
    {
    case 0:
    {
        local_cmd.FrameNumber = 0;
        strcpy(local_cmd.Type, STR_ADD);
        strcpy(local_cmd.Data.DeviceId, STR_HOST_GATEWAYID);
        strcpy(local_cmd.Data.Key, "Time");
        strcpy(local_cmd.Data.Value, Value);
    }
    break;
    case 1:
    {
        local_cmd.FrameNumber = 0;
        strcpy(local_cmd.Type, STR_DEVSINFO);
        strcpy(local_cmd.Data.DeviceId, STR_HOST_GATEWAYID);
        strcpy(local_cmd.Data.Key, STR_DEVSINFO);
    }
    break;
    case 2:
    {
        local_cmd.FrameNumber = 0;
        strcpy(local_cmd.Type, STR_DEVATTRI);
        strcpy(local_cmd.Data.DeviceId, DeviceId);
        strcpy(local_cmd.Data.Key, "All");
    }
    break;
    case 3:
    {
        local_cmd.FrameNumber = 0;
        strcpy(local_cmd.Type, STR_REFACTORY);
    }
    break;
    default:
        return -1;
    }

    int ret = write_to_local(&local_cmd, &g_SLocalControl);
    if (ret < 0)
    {
        log_error("write_hanyar_cmd error\n");
    }
    return ret;
}

int write_haryan(const char *data, int dataLen)
{
    int ret=0;
    char *sendBuf = g_SLocalControl.sendData;
    if (dataLen + 3 <= SENDTOLOCAL_SIZE)
    {
        sendBuf[0] = 0x02;
        strcpy(&sendBuf[1], data);
        sendBuf[dataLen + 1] = 0x03;
        sendBuf[dataLen + 2] = 0x00;

#if USE_LIBEVENT
        ret =event_client_write(sendBuf, dataLen + 3);
#elif USE_LIBUV
        ret = client_write(sendBuf, dataLen + 3);
#else
        if (g_SLocalControl.socketfd == 0)
        {
            log_error("socketfd is null\n");
            return -1;
        }
        ret = Write(g_SLocalControl.socketfd, sendBuf, dataLen + 3);
#endif

        // for (int i = 0; i < dataLen + 3; ++i)
        // {
        //     printf("%x ", sendBuf[i]);
        // }
        // printf("\n");
        log_info("write ret:%d\n", ret);
        return ret;
    }
    return -1;
}

int write_to_local(void *ptr, LocalControl_t *localControl)
{
    if (NULL == ptr)
    {
        return -1;
    }
    local_dev_t *local_dev = (local_dev_t *)ptr;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, STR_COMMAND, STR_DISPATCH);

    char str[3];
    sprintf(str, "%d", local_dev->FrameNumber);
    cJSON_AddStringToObject(root, STR_FRAMENUMBER, str);

    cJSON_AddStringToObject(root, STR_TYPE, local_dev->Type);

    cJSON *DataArray = cJSON_CreateArray();
    cJSON_AddItemToObject(root, STR_DATA, DataArray);

    cJSON *arrayItem = cJSON_CreateObject();
    if (strlen(local_dev->Data.DeviceId))
        cJSON_AddStringToObject(arrayItem, STR_DEVICEID, local_dev->Data.DeviceId);
    if (strlen(local_dev->Data.ModelId))
        cJSON_AddStringToObject(arrayItem, STR_MODELID, local_dev->Data.ModelId);
    if (strlen(local_dev->Data.Key))
        cJSON_AddStringToObject(arrayItem, STR_KEY, local_dev->Data.Key);
    if (strlen(local_dev->Data.Value))
        cJSON_AddStringToObject(arrayItem, STR_VALUE, local_dev->Data.Value);
    cJSON_AddItemToArray(DataArray, arrayItem);

    char *json = cJSON_PrintUnformatted(root);
    log_info("send json:%s\n", json);

    int ret = write_haryan(json, strlen(json));

    free(json);
    free(root);
    return ret;
fail:
    free(root);
    return -1;
}