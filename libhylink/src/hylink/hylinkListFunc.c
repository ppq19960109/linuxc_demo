#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cJSON.h"
#include "frameCb.h"
#include "logFunc.h"
#include "hylinkListFunc.h"

char *generateCloudJson(const char *cloudKey, const char *hyValue, const unsigned char valueType)
{
    if (cloudKey == NULL || hyValue == NULL)
        return NULL;
    cJSON *root = cJSON_CreateObject();

    switch (valueType)
    {
    case LINK_VALUE_TYPE_ENUM:
        cJSON_AddNumberToObject(root, cloudKey, *hyValue);
        break;
    case LINK_VALUE_TYPE_NUM:
        cJSON_AddNumberToObject(root, cloudKey, *(int *)hyValue);
        break;
    case LINK_VALUE_TYPE_STRING:
        cJSON_AddStringToObject(root, cloudKey, hyValue);
        break;
    default:
        goto fail;
        break;
    }
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json;
fail:
    cJSON_Delete(root);
    return NULL;
}

int getHyLinkValueType(unsigned char dataType)
{
    int byteLen;
    switch (dataType)
    {
    case LINK_VALUE_TYPE_ENUM:
        byteLen = 1;
        break;
    case LINK_VALUE_TYPE_NUM:
        byteLen = 4;
        break;
    case LINK_VALUE_TYPE_STRING:
        byteLen = 18;
        break;
    default:
        byteLen = 0;
        break;
    }
    return byteLen;
}

void *hyLinkParseJson(const char *devId, const char *str)
{
    cJSON *root = cJSON_Parse(str);
    if (root == NULL)
    {
        return NULL;
    }

    cJSON *modelId = cJSON_GetObjectItem(root, "modelId");
    if (modelId == NULL)
    {
        logError("modelId is NULL\n");
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

    HyLinkDev *dev = (HyLinkDev *)malloc(sizeof(HyLinkDev));
    memset(dev, 0, sizeof(HyLinkDev));
    dev->attrLen = arraySize;
    dev->attr = (HyLinkDevAttr *)malloc(sizeof(HyLinkDevAttr) * dev->attrLen);
    memset(dev->attr, 0, sizeof(HyLinkDevAttr) * dev->attrLen);

    strcpy(dev->devId, devId);
    strcpy(dev->modelId, modelId->valuestring);
    dev->online = SUBDEV_ONLINE;

    cJSON *hyKey, *valueType, *repeat;
    for (int i = 0; i < arraySize; i++)
    {
        cJSON *arraySub = cJSON_GetArrayItem(attr, i);
        if (arraySub == NULL)
            continue;

        hyKey = cJSON_GetObjectItem(arraySub, "hyKey");
        strcpy(dev->attr[i].hyKey, hyKey->valuestring);
        valueType = cJSON_GetObjectItem(arraySub, "valueType");
        dev->attr[i].valueType = valueType->valueint;
        repeat = cJSON_GetObjectItem(arraySub, "repeat");
        dev->attr[i].repeat = repeat->valueint;

        int valueLen = getHyLinkValueType(dev->attr[i].valueType);
        if (valueLen < 0)
            continue;
        dev->attr[i].value = (char *)malloc(valueLen);
        memset(dev->attr[i].value, 0, valueLen);
    }
    cJSON_Delete(root);
    hylinkListAdd(dev);
    return dev;
fail:
    cJSON_Delete(root);
    return NULL;
}

void *addProfileDev(const char *path, const char *devId, const char *modelId, void *(*func)(const char *, const char *))
{
    char filePath[64] = {0};
    snprintf(filePath, sizeof(filePath), "%s/%s.json", path, modelId);
    struct stat statfile;
    if (stat(filePath, &statfile) < 0)
    {
        logError("stat %s error", filePath);
        return NULL;
    }
    if (statfile.st_size == 0)
    {
        logError("file size 0");
        return NULL;
    }
    logInfo("file size:%d", statfile.st_size);
    int fd = open(filePath, O_RDONLY);
    if (fd < 0)
    {
        perror("open file fail");
        return NULL;
    }
    void *buf = malloc(statfile.st_size);
    if (buf == NULL)
    {
        goto fail;
    }
    memset(buf, 0, statfile.st_size);

    int res = read(fd, buf, statfile.st_size);
    if (res != statfile.st_size)
    {
        logError("read file size :%d", res);
        goto fail;
    }
    close(fd);
    void *dev = func(devId, buf);

    free(buf);
    return dev;
fail:
    close(fd);
    if (buf != NULL)
    {
        free(buf);
    }
    return NULL;
}

//-----------------------------------------------------
void hyLinkDevFree(HyLinkDev *dev)
{
    if (dev == NULL)
        return;

    for (int i = 0; i < dev->attrLen; ++i)
    {
        if (dev->attr[i].value != NULL)
            free(dev->attr[i].value);
    }
    free(dev->attr);
    free(dev);
}

KHASH_MAP_INIT_STR(hyLink, HyLinkDev *)

khash_t(hyLink) * hyMap;
//-------------------------------
khint_t hyLink_kh_begin()
{
    return kh_begin(hyMap);
}

khint_t hyLink_kh_end()
{
    return kh_end(hyMap);
}
void *hyLink_kh_exist(int k)
{
    if (k < kh_end(hyMap) && kh_exist(hyMap, k))
        return kh_value(hyMap, k);

    return NULL;
}

//-------------------------------
void hylinkListInit(void)
{
    hyMap = kh_init(hyLink);
}

void hylinkListDestroy(void)
{
    kh_destroy(hyLink, hyMap);
}

void hylinkListAdd(void *node)
{
    if (node == NULL)
        return;
    HyLinkDev *dev = (HyLinkDev *)node;

    int ret;
    khint_t k = kh_put(hyLink, hyMap, dev->devId, &ret);
    if (ret < 0)
    {
        printf("kh_put error\n");
        return;
    }
    kh_value(hyMap, k) = dev;
}

void hylinkListDel(HyLinkDev *dev)
{
    if (dev == NULL)
        return;
    khint_t k = kh_get(hyLink, hyMap, dev->devId);
    kh_del(hyLink, hyMap, k);
    hyLinkDevFree(dev);
}

int hylinkListDelById(const char *id)
{
    if (id == NULL)
        return -1;
    HyLinkDev *dev = hylinkListGetById(id);
    if (dev == NULL)
    {
        return -1;
    }
    hylinkListDel(dev);
    return 0;
}

void hylinkListEmpty(void)
{
    if (hyMap == NULL)
    {
        return;
    }
    HyLinkDev *dev;

#define LISTEMPTY           \
    do                      \
    {                       \
        hyLinkDevFree(dev); \
    } while (0)

    kh_foreach_value(hyMap, dev, LISTEMPTY);
    kh_clear(hyLink, hyMap);
}

HyLinkDev *hylinkListGetById(const char *id)
{
    if (hyMap == NULL)
    {
        return NULL;
    }
    khint_t k = kh_get(hyLink, hyMap, id);
    if (k == kh_end(hyMap))
    {
        printf("hylinkListGetById id null:%d\n", k);
        return NULL;
    }
    return kh_value(hyMap, k);
}
