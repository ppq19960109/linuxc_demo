#include "cloudLinkListFunc.h"

static void BrgDevInfo_init(BrgDevInfo *brgDevInfo)
{
    // strcpy(brgDevInfo->prodId, PRODUCT_ID);
    strcpy(brgDevInfo->hiv, "1.0.0");
    strcpy(brgDevInfo->fwv, "1.0.0");
    strcpy(brgDevInfo->hwv, "1.0.0");
    strcpy(brgDevInfo->swv, "1.0.0");
    brgDevInfo->protType = 3;
    strcpy(brgDevInfo->manu, MANUAFACTURER);
    // strcpy(brgDevInfo->sn, "12345678");
    // strcpy(brgDevInfo->model, DEVICE_MODEL);
    // strcpy(brgDevInfo->devType, DEVICE_TYPE);
    strcpy(brgDevInfo->mac, "000000000000");
}

#define _PARSEJSON_DEVATTR(T) PARSEJSON_DEVATTR$_##T
#define PARSEJSON_DEVATTR(T) _PARSEJSON_DEVATTR(T)
#define _PARSEJSON_DEVATTR_DEF(T)                                                        \
    void PARSEJSON_DEVATTR(T)(const char *devId, T *dev, cJSON *root)                    \
    {                                                                                    \
        int i;                                                                           \
        cJSON *prodId = cJSON_GetObjectItem(root, "prodId");                             \
        if (prodId == NULL)                                                              \
        {                                                                                \
            logError("prodId is NULL\n");                                                \
            goto fail;                                                                   \
        }                                                                                \
        cJSON *model = cJSON_GetObjectItem(root, "model");                               \
        if (model == NULL)                                                               \
        {                                                                                \
            logError("model is NULL\n");                                                 \
            goto fail;                                                                   \
        }                                                                                \
        cJSON *devType = cJSON_GetObjectItem(root, "devType");                           \
        if (devType == NULL)                                                             \
        {                                                                                \
            logError("devType is NULL\n");                                               \
            goto fail;                                                                   \
        }                                                                                \
        cJSON *attr = cJSON_GetObjectItem(root, "attr");                                 \
        if (attr == NULL)                                                                \
        {                                                                                \
            logError("attr is NULL\n");                                                  \
            goto fail;                                                                   \
        }                                                                                \
        int arraySize = cJSON_GetArraySize(attr);                                        \
        if (arraySize == 0)                                                              \
        {                                                                                \
            logError("attr arraySize is 0\n");                                           \
            goto fail;                                                                   \
        }                                                                                \
        BrgDevInfo_init(&dev->brgDevInfo);                                               \
        strcpy(dev->brgDevInfo.sn, devId);                                               \
        strcpy(dev->brgDevInfo.prodId, prodId->valuestring);                             \
        strcpy(dev->brgDevInfo.model, model->valuestring);                               \
        strcpy(dev->brgDevInfo.devType, devType->valuestring);                           \
        dev->attrLen = arraySize;                                                        \
        dev->attr = (CloudLinkDevAttr *)malloc(sizeof(CloudLinkDevAttr) * dev->attrLen); \
        memset(dev->attr, 0, sizeof(CloudLinkDevAttr) * dev->attrLen);                   \
        cJSON *arraySub, *hyKey, *hyType, *cloudSid, *cloudKey, *repeat;                 \
        for (i = 0; i < arraySize; i++)                                                  \
        {                                                                                \
            arraySub = cJSON_GetArrayItem(attr, i);                                      \
            if (arraySub == NULL)                                                        \
                continue;                                                                \
            hyKey = cJSON_GetObjectItem(arraySub, "hyKey");                              \
            strcpy(dev->attr[i].hyKey, hyKey->valuestring);                              \
            if (cJSON_HasObjectItem(arraySub, "hyType"))                                 \
            {                                                                            \
                hyType = cJSON_GetObjectItem(arraySub, "hyType");                        \
                strcpy(dev->attr[i].hyType, hyType->valuestring);                        \
            }                                                                            \
            if (cJSON_HasObjectItem(arraySub, "cloudSid"))                               \
            {                                                                            \
                cloudSid = cJSON_GetObjectItem(arraySub, "cloudSid");                    \
                strcpy(dev->attr[i].cloudSid, cloudSid->valuestring);                    \
            }                                                                            \
            if (cJSON_HasObjectItem(arraySub, "cloudKey"))                               \
            {                                                                            \
                cloudKey = cJSON_GetObjectItem(arraySub, "cloudKey");                    \
                strcpy(dev->attr[i].cloudKey, cloudKey->valuestring);                    \
            }                                                                            \
            if (cJSON_HasObjectItem(arraySub, "repeat"))                                 \
            {                                                                            \
                repeat = cJSON_GetObjectItem(arraySub, "repeat");                        \
                dev->attr[i].repeat = repeat->valueint;                                  \
            }                                                                            \
        }                                                                                \
    fail:                                                                                \
        return;                                                                          \
    }
#define PARSEJSON_DEVATTR_DEF(T) _PARSEJSON_DEVATTR_DEF(T)

PARSEJSON_DEVATTR_DEF(CloudLinkDev);
PARSEJSON_DEVATTR_DEF(CloudLinkSubDev);

void *cloudLinkParseJson(const char *devId, const char *str)
{
    int i;
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

    CloudLinkDev *dev = (CloudLinkDev *)malloc(sizeof(CloudLinkDev));
    memset(dev, 0, sizeof(CloudLinkDev));
    strcpy(dev->modelId, modelId->valuestring);
    //----------------------------------------
    PARSEJSON_DEVATTR(CloudLinkDev)
    (devId, dev, root);
    //----------------------------------------

    cJSON *attr, *arraySub;
    if (cJSON_HasObjectItem(root, "subdev"))
    {
        char subdevId[33] = {0};
        int devIdLen = strlen(devId);
        strcpy(subdevId, devId);

        attr = cJSON_GetObjectItem(root, "subdev");
        if (attr == NULL)
        {
            logError("subdev is NULL\n");
            goto fail;
        }
        int arraySize = cJSON_GetArraySize(attr);
        if (arraySize == 0)
        {
            logError("subdev attr arraySize is 0\n");
            goto fail;
        }

        dev->cloudLinkSubDevLen = arraySize;
        dev->cloudLinkSubDev = (CloudLinkSubDev *)malloc(sizeof(CloudLinkSubDev) * dev->cloudLinkSubDevLen);
        memset(dev->cloudLinkSubDev, 0, sizeof(CloudLinkSubDev) * dev->cloudLinkSubDevLen);

        for (i = 0; i < dev->cloudLinkSubDevLen; ++i)
        {
            arraySub = cJSON_GetArrayItem(attr, i);
            if (arraySub == NULL)
                continue;
            subdevId[devIdLen] = '0' + i;

            PARSEJSON_DEVATTR(CloudLinkSubDev)
            (subdevId, &dev->cloudLinkSubDev[i], arraySub);
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
static void cloudLinkDevFree(CloudLinkDev *dev)
{
    if (dev == NULL)
        return;

    if (dev->attr != NULL)
        free(dev->attr);

    if (dev->cloudLinkSubDev != NULL)
        free(dev->cloudLinkSubDev);

    free(dev);
    dev = NULL;
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
    khint_t k = kh_put(cloudLink, cloudMap, dev->brgDevInfo.sn, &ret);
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
    khint_t k = kh_get(cloudLink, cloudMap, dev->brgDevInfo.sn);
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

void cloudLinkListPrintf(void)
{
    if (cloudMap == NULL)
    {
        return;
    }
    CloudLinkDev *dev;
#define CLOUDLINKPRINTF                                 \
    do                                                  \
    {                                                   \
        printf("---devid:%s --\n", dev->brgDevInfo.sn); \
    } while (0)

    kh_foreach_value(cloudMap, dev, CLOUDLINKPRINTF);
}
