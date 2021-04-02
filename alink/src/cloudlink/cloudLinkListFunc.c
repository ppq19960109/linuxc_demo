#include "cloudLinkListFunc.h"

void *cloudLinkParseJson(const char *devId, const char *str)
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

    cJSON *productKey = cJSON_GetObjectItem(root, "productKey");
    if (productKey == NULL)
    {
        logError("productKey is NULL\n");
        goto fail;
    }

    cJSON *productSecret = cJSON_GetObjectItem(root, "productSecret");
    if (productSecret == NULL)
    {
        logError("productSecret is NULL\n");
        goto fail;
    }

    cJSON *deviceSecret = cJSON_GetObjectItem(root, "deviceSecret");
    if (deviceSecret == NULL)
    {
        logError("deviceSecret is NULL\n");
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
    int i;
    CloudLinkDev *dev = (CloudLinkDev *)malloc(sizeof(CloudLinkDev));
    memset(dev, 0, sizeof(CloudLinkDev));
    dev->attrLen = arraySize;
    dev->attr = (CloudLinkDevAttr *)malloc(sizeof(CloudLinkDevAttr) * dev->attrLen);
    memset(dev->attr, 0, sizeof(CloudLinkDevAttr) * dev->attrLen);
    dev->id = -1;

    strcpy(dev->modelId, modelId->valuestring);
    strcpy(dev->alinkInfo.product_key, productKey->valuestring);
    strcpy(dev->alinkInfo.product_secret, productSecret->valuestring);
    strcpy(dev->alinkInfo.device_name, devId);
    strcpy(dev->alinkInfo.device_secret, deviceSecret->valuestring);

    cJSON *arraySub, *hyKey, *hyType, *cloudKey, *valueType;
    for (i = 0; i < arraySize; i++)
    {
        logInfo("attr");
        arraySub = cJSON_GetArrayItem(attr, i);
        if (arraySub == NULL)
            continue;

        hyKey = cJSON_GetObjectItem(arraySub, "hyKey");
        strcpy(dev->attr[i].hyKey, hyKey->valuestring);
        if (cJSON_HasObjectItem(arraySub, "hyType"))
        {
            hyType = cJSON_GetObjectItem(arraySub, "hyType");
            strcpy(dev->attr[i].hyType, hyType->valuestring);
        }
        if (cJSON_HasObjectItem(arraySub, "cloudKey"))
        {
            cloudKey = cJSON_GetObjectItem(arraySub, "cloudKey");
            strcpy(dev->attr[i].cloudKey, cloudKey->valuestring);
        }
        valueType = cJSON_GetObjectItem(arraySub, "valueType");
        dev->attr[i].valueType = valueType->valueint;
        int valueLen = getHyLinkValueType(dev->attr[i].valueType);
        if (valueLen < 0)
            continue;
    }
    if (cJSON_HasObjectItem(root, "services"))
    {
        logInfo("services");
        attr = cJSON_GetObjectItem(root, "services");
        if (attr == NULL)
        {
            logError("serverAttr is NULL\n");
            goto fail;
        }

        arraySize = cJSON_GetArraySize(attr);
        if (arraySize == 0)
        {
            logError("serverAttr arraySize is 0\n");
            goto fail;
        }
        dev->serverAttrLen = arraySize;
        dev->serverAttr = (CloudLinkServerAttr *)malloc(sizeof(CloudLinkServerAttr) * dev->serverAttrLen);
        memset(dev->serverAttr, 0, sizeof(CloudLinkServerAttr) * dev->serverAttrLen);

        cJSON *serverId, *hyKey, *key, *value;
        for (i = 0; i < arraySize; ++i)
        {
            arraySub = cJSON_GetArrayItem(attr, i);
            if (arraySub == NULL)
                continue;
            serverId = cJSON_GetObjectItem(arraySub, "serverId");
            strcpy(dev->serverAttr[i].serverId, serverId->valuestring);

            hyKey = cJSON_GetObjectItem(arraySub, "hyKey");
            strcpy(dev->serverAttr[i].hyKey, hyKey->valuestring);

            key = cJSON_GetObjectItem(arraySub, "key");
            strcpy(dev->serverAttr[i].key, key->valuestring);
            value = cJSON_GetObjectItem(arraySub, "value");
            strcpy(dev->serverAttr[i].value, value->valuestring);
        }
    }
    if (cJSON_HasObjectItem(root, "events"))
    {
        logInfo("events");
        attr = cJSON_GetObjectItem(root, "events");
        if (attr == NULL)
        {
            logError("eventAttr is NULL\n");
            goto fail;
        }

        arraySize = cJSON_GetArraySize(attr);
        if (arraySize == 0)
        {
            logError("eventAttr arraySize is 0\n");
            goto fail;
        }
        dev->eventAttrLen = arraySize;
        dev->eventAttr = (CloudLinkEventAttr *)malloc(sizeof(CloudLinkEventAttr) * dev->eventAttrLen);
        memset(dev->eventAttr, 0, sizeof(CloudLinkEventAttr) * dev->eventAttrLen);

        cJSON *eventId, *hyKey, *key, *value;
        for (i = 0; i < arraySize; ++i)
        {
            arraySub = cJSON_GetArrayItem(attr, i);
            if (arraySub == NULL)
                continue;
            eventId = cJSON_GetObjectItem(arraySub, "eventId");
            strcpy(dev->eventAttr[i].eventId, eventId->valuestring);
            hyKey = cJSON_GetObjectItem(arraySub, "hyKey");
            strcpy(dev->eventAttr[i].hyKey, hyKey->valuestring);

            key = cJSON_GetObjectItem(arraySub, "key");
            strcpy(dev->eventAttr[i].key, key->valuestring);

            value = cJSON_GetObjectItem(arraySub, "value");
            dev->eventAttr[i].value = value->valueint;
        }
    }
    cJSON_Delete(root);
    cloudLinkListAdd(dev);
    return dev;
fail:
    cJSON_Delete(root);
    return NULL;
}

//--------------------------------------------------------
void cloudLinkDevFree(CloudLinkDev *dev)
{
    if (dev == NULL)
        return;
    int i;
    for (i = 0; i < dev->attrLen; ++i)
    {
        if (dev->attr[i].value != NULL)
            free(dev->attr[i].value);
    }
    free(dev->attr);

    free(dev->serverAttr);

    free(dev->eventAttr);
    free(dev);
}

KHASH_MAP_INIT_STR(cloudLink, CloudLinkDev *)

khash_t(cloudLink) * cloudMap;
//-------------------------------
khint_t cloudLink_kh_begin()
{
    return kh_begin(cloudMap);
}

khint_t cloudLink_kh_end()
{
    return kh_end(cloudMap);
}
void *cloudLink_kh_exist(int k)
{
    if (k < kh_end(cloudMap) && kh_exist(cloudMap, k))
        return kh_value(cloudMap, k);

    return NULL;
}

//-------------------------------
void cloudLinkListInit(void)
{
    cloudMap = kh_init(cloudLink);
}

void cloudLinkListDestroy(void)
{
    kh_destroy(cloudLink, cloudMap);
}

void cloudLinkListAdd(void *node)
{
    if (node == NULL)
        return;
    CloudLinkDev *dev = (CloudLinkDev *)node;

    int ret;
    khint_t k = kh_put(cloudLink, cloudMap, dev->alinkInfo.device_name, &ret);
    if (ret < 0)
    {
        printf("kh_put error\n");
        return;
    }
    kh_value(cloudMap, k) = dev;
}

void cloudLinkListDelDev(CloudLinkDev *dev)
{
    if (dev == NULL)
        return;
    khint_t k = kh_get(cloudLink, cloudMap, dev->alinkInfo.device_name);
    kh_del(cloudLink, cloudMap, k);
    cloudLinkDevFree(dev);
}

void cloudLinkListEmpty(void)
{
    if (cloudMap == NULL)
    {
        return;
    }
    CloudLinkDev *dev;

#define LISTEMPTY              \
    do                         \
    {                          \
        cloudLinkDevFree(dev); \
    } while (0)

    kh_foreach_value(cloudMap, dev, LISTEMPTY);
    kh_clear(cloudLink, cloudMap);
}

CloudLinkDev *cloudLinkListGetById(const char *id)
{
    if (cloudMap == NULL)
    {
        return NULL;
    }
    khint_t k = kh_get(cloudLink, cloudMap, id);
    if (k == kh_end(cloudMap))
    {
        printf("cloudLinkListGetById id null:%d\n", k);
        return NULL;
    }
    return kh_value(cloudMap, k);
}

CloudLinkDev *cloudLinkListGetBySn(const int sn)
{
    if (cloudMap == NULL)
    {
        return NULL;
    }
    CloudLinkDev *dev;
#define CLOUDLINKSN        \
    do                     \
    {                      \
        if (dev->id == sn) \
            return dev;    \
    } while (0)

    kh_foreach_value(cloudMap, dev, CLOUDLINKSN);
    return NULL;
}

void cloudLinkListPrintf(void)
{
    if (cloudMap == NULL)
    {
        return;
    }
    CloudLinkDev *dev;
#define CLOUDLINKPRINTF                                         \
    do                                                          \
    {                                                           \
        printf("---devid:%s --\n", dev->alinkInfo.device_name); \
    } while (0)

    kh_foreach_value(cloudMap, dev, CLOUDLINKPRINTF);
}
