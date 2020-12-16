#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zigbeeListFunc.h"

void zigbeeDevFree(void *dev)
{
    if (dev == NULL)
    {
        printf("zigbeeDevFree dev id null");
        return;
    }
    zigbeeDev *zDev = (zigbeeDev *)dev;
    if (zDev->attr != NULL)
    {
        free(zDev->attr);
    }
    free(zDev);
}

KHASH_MAP_INIT_STR(zigbee, zigbeeDev *)

khash_t(zigbee) * zigbeeMap;
//-------------------------------
khint_t zigbee_kh_begin()
{
    return kh_begin(zigbeeMap);
}

khint_t zigbee_kh_end()
{
    return kh_end(zigbeeMap);
}
void *zigbee_kh_exist(int k)
{
    if (k < kh_end(zigbeeMap) && kh_exist(zigbeeMap, k))
        return kh_value(zigbeeMap, k);

    return NULL;
}

//-------------------------------
void zigbeeListInit(void)
{
    zigbeeMap = kh_init(zigbee);
}

void zigbeeListDestroy(void)
{
    kh_destroy(zigbee, zigbeeMap);
}

void zigbeeListAdd(void *node)
{
    if (node == NULL)
        return;
    zigbeeDev *ptr = (zigbeeDev *)node;

    int ret;
    khint_t k = kh_put(zigbee, zigbeeMap, ptr->manuName, &ret);
    if (ret < 0)
    {
        printf("kh_put error\n");
        return;
    }
    kh_value(zigbeeMap, k) = ptr;
}

void zigbeeListDel(const char *key)
{
    if (key == NULL)
        return;
    khint_t k = kh_get(zigbee, zigbeeMap, key);
    void *val = kh_value(zigbeeMap, k);
    kh_del(zigbee, zigbeeMap, k);
    zigbeeDevFree(val);
}

void zigbeeListEmpty(void)
{
    if (zigbeeMap == NULL)
    {
        return;
    }
    zigbeeDev *ptr;

#define LISTEMPTY           \
    do                      \
    {                       \
        zigbeeDevFree(ptr); \
    } while (0)

    kh_foreach_value(zigbeeMap, ptr, LISTEMPTY);
    kh_clear(zigbee, zigbeeMap);
}

void *zigbeeListGet(const char *key)
{
    if (zigbeeMap == NULL)
    {
        return NULL;
    }
    khint_t k = kh_get(zigbee, zigbeeMap, key);
    if (k == kh_end(zigbeeMap))
    {
        printf("zigbeeListGet key:%s null:%d\n", key, k);
        return NULL;
    }
    return kh_value(zigbeeMap, k);
}

void zigbeeListPrintf(void)
{
    if (zigbeeMap == NULL)
    {
        return;
    }
    printf("zigbeeListPrintf all:%d\n", kh_end(zigbeeMap));
    for (khint_t __i = zigbee_kh_begin(); __i != zigbee_kh_end(); ++__i)
    {
        if (!zigbee_kh_exist(__i))
            continue;
        const char *id = kh_key(zigbeeMap, __i);
        printf("zigbeeListPrintf index:%d,id:%s\n", __i, id);
    }
}
