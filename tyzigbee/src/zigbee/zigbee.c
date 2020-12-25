#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tuya_gw_infra_api.h"
#include "tuya_gw_z3_api.h"
#include "tuya_gw_dp_api.h"

#include "tyZigbee.h"
#include "tyZigbeeZcl3.h"

#include "frameCb.h"
#include "logFunc.h"
#include "commonFunc.h"
#include "cJSON.h"

#include "zigbee.h"
#include "zigbeeManage.h"
#include "zigbeeListFunc.h"

static int zigbeeCmdNetAccess(void *cmd)
{
    logDebug("zigbeeCmdNetAccess");
    if (cmd == NULL)
        return -1;

    bool permit;
    if (*(char *)cmd)
        permit = true;
    else
        permit = false;

    int ret = 0;

    ret = tuya_user_iot_permit_join(permit);
    if (ret != 0)
    {
        logError("tuya_user_iot_permit_join error, ret: %d", ret);
    }

    return ret;
}

static int zigbeeCmdDelDev(void *id)
{
    logDebug("zigbeeCmdDelDev");
    int ret = 0;

    ret = tuya_user_iot_z3_dev_del((const char *)id);
    if (ret != 0)
    {
        logError("tuya_user_iot_z3_dev_del error, ret: %d", ret);
    }
    return ret;
}

static void zigbeeParseJson(char *str)
{
    cJSON *root = cJSON_Parse(str);
    if (root == NULL)
    {
        return;
    }

    cJSON *modelId = cJSON_GetObjectItem(root, "modelId");
    if (modelId == NULL)
    {
        logError("modelId is NULL\n");
        goto fail;
    }
    cJSON *manuName = cJSON_GetObjectItem(root, "manuName");
    if (manuName == NULL)
    {
        logError("manuName is NULL\n");
        goto fail;
    }
    cJSON *attr = cJSON_GetObjectItem(root, "attr");
    if (attr == NULL)
    {
        logError("attr is NULL\n");
        goto fail;
    }

    int arraySize = cJSON_GetArraySize(attr);
    if (arraySize == 0)
    {
        logError("attr arraySize is 0\n");
        goto fail;
    }

    zigbeeDev *zDev = (zigbeeDev *)malloc(sizeof(zigbeeDev));
    memset(zDev, 0, sizeof(zigbeeDev));
    zDev->attrLen = arraySize;
    zDev->attr = (ZigbeeAttr *)malloc(sizeof(ZigbeeAttr) * zDev->attrLen);
    memset(zDev->attr, 0, sizeof(ZigbeeAttr) * zDev->attrLen);

    strcpy(zDev->modelId, modelId->valuestring);
    strcpy(zDev->manuName, manuName->valuestring);

    cJSON *arraySub, *hyKey, *hyKeyPrivate, *srcEndpoint, *dstEndpoint, *ClusterId, *AttributeId, *z3CmdType, *z3CmdId, *dataType;
    for (int i = 0; i < arraySize; i++)
    {
        arraySub = cJSON_GetArrayItem(attr, i);
        if (arraySub == NULL)
            continue;

        hyKey = cJSON_GetObjectItem(arraySub, "hyKey");
        strcpy(zDev->attr[i].hyKey, hyKey->valuestring);
        hyKeyPrivate = cJSON_GetObjectItem(arraySub, "hyKeyPrivate");
        zDev->attr[i].hyKeyPrivate = hyKeyPrivate->valueint;
        srcEndpoint = cJSON_GetObjectItem(arraySub, "srcEndpoint");
        zDev->attr[i].srcEndpoint = srcEndpoint->valueint;
        dstEndpoint = cJSON_GetObjectItem(arraySub, "dstEndpoint");
        zDev->attr[i].dstEndpoint = dstEndpoint->valueint;
        ClusterId = cJSON_GetObjectItem(arraySub, "ClusterId");
        zDev->attr[i].ClusterId = ClusterId->valueint;
        AttributeId = cJSON_GetObjectItem(arraySub, "AttributeId");
        zDev->attr[i].AttributeId = AttributeId->valueint;
        if (cJSON_HasObjectItem(arraySub, "z3CmdType"))
        {
            z3CmdType = cJSON_GetObjectItem(arraySub, "z3CmdType");
            zDev->attr[i].z3CmdType = z3CmdType->valueint;
        }
        if (cJSON_HasObjectItem(arraySub, "z3CmdId"))
        {
            z3CmdId = cJSON_GetObjectItem(arraySub, "z3CmdId");
            zDev->attr[i].z3CmdId = z3CmdId->valueint;
        }
        if (cJSON_HasObjectItem(arraySub, "dataType"))
        {
            dataType = cJSON_GetObjectItem(arraySub, "dataType");
            zDev->attr[i].dataType = dataType->valueint;
        }
    }
    cJSON_Delete(root);
    zigbeeListAdd(zDev);
    return;
fail:
    cJSON_Delete(root);
}
static int readProfileFile(const char *path)
{
    struct stat statfile;
    if (stat(path, &statfile) < 0)
    {
        return -1;
    }
    if (statfile.st_size == 0)
    {
        logError("file size 0");
        return -1;
    }
    logInfo("file size:%d", statfile.st_size);
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror("open file fail");
        return -1;
    }
    void *buf = malloc(statfile.st_size);
    memset(buf, 0, statfile.st_size);
    if (buf == NULL)
    {
        goto fail;
    }
    int res = read(fd, buf, statfile.st_size);
    if (res != statfile.st_size)
    {
        logError("read file size :%d", res);
        goto fail;
    }
    close(fd);
    zigbeeParseJson(buf);

    free(buf);
    return 0;
fail:
    close(fd);
    if (buf != NULL)
    {
        free(buf);
    }
    return -1;
}

int zigbeeClose()
{
    zigbeeListEmpty();
    return 0;
}

void zigbeeMain(void)
{
    registerSystemCb(zigbeeClose, ZIGBEE_CLOSE);
    registerCmdCb(zigbeeCmdNetAccess, CMD_NETWORK_ACCESS);
    registerCmdCb(zigbeeCmdDelDev, CMD_DELETE_DEV);
    registerCmdCb(zigbeeDevZclReport, CMD_DEV_REPORT);
    registerZigbeeCb(zigbeeDevZclDispatch, ZIGBEE_DEV_DISPATCH);
    zigbeeListInit();

    readFileList(ZIGBEE_PROFILE_PATH, readProfileFile);
    // zigbeeListPrintf();
    tyZigbeeInit();
    tuya_user_iot_permit_join(true);
}
