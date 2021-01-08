#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "frameCb.h"
#include "logFunc.h"
#include "commonFunc.h"
#include "cJSON.h"

#include "zigbeeListFunc.h"

static void zigbeeParseJson(char *str)
{
    int i;
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
    cJSON *heartbeatTime = cJSON_GetObjectItem(root, "heartbeatTime");
    if (heartbeatTime == NULL)
    {
        logError("heartbeatTime is NULL\n");
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
    zDev->heartbeatTime = heartbeatTime->valueint;

    cJSON *arraySub, *hyKey, *hyKeyPrivate, *srcEndpoint, *dstEndpoint, *ClusterId, *AttributeId, *z3CmdType, *z3CmdId, *dataType;
    for (i = 0; i < arraySize; ++i)
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
    if (cJSON_HasObjectItem(root, "scene"))
    {
        attr = cJSON_GetObjectItem(root, "scene");
        if (attr == NULL)
        {
            logError("attr is NULL\n");
            goto fail;
        }
        arraySize = cJSON_GetArraySize(attr);
        zDev->sceneAttrLen = arraySize;
        zDev->sceneAttr = (SceneAttr *)malloc(sizeof(SceneAttr) * zDev->sceneAttrLen);
        memset(zDev->sceneAttr, 0, sizeof(SceneAttr) * zDev->sceneAttrLen);
        cJSON *key, *value, *valueArraySub;
        for (i = 0; i < arraySize; ++i)
        {
            arraySub = cJSON_GetArrayItem(attr, i);
            if (arraySub == NULL)
                continue;

            key = cJSON_GetObjectItem(arraySub, "key");
            strcpy(zDev->sceneAttr[i].key, key->valuestring);
            value = cJSON_GetObjectItem(arraySub, "value");

            int valueSize = cJSON_GetArraySize(value);
            zDev->sceneAttr[i].valueLen = valueSize;
            zDev->sceneAttr[i].value = (char(*)[33])malloc(sizeof(*(zDev->sceneAttr[i].value)) * zDev->sceneAttr[i].valueLen);
            memset(zDev->sceneAttr[i].value, 0, sizeof(*(zDev->sceneAttr[i].value)) * zDev->sceneAttr[i].valueLen);
            for (int j = 0; j < zDev->sceneAttr[i].valueLen; ++j)
            {
                valueArraySub = cJSON_GetArrayItem(value, j);
                if (valueArraySub == NULL)
                    continue;
                strcpy(zDev->sceneAttr[i].value[j], valueArraySub->valuestring);
            }
        }
    }
    cJSON_Delete(root);
    zigbeeListAdd(zDev);
    return;
fail:
    cJSON_Delete(root);
}

int readProfileFile(const char *path)
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