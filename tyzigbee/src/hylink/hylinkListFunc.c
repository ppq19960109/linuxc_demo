#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hylinkListFunc.h"

void hylinkDevFree(void *hylinkDev)
{
    free(hylinkDev);
}

KHASH_MAP_INIT_STR(hyLink, HylinkDev *)

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
    HylinkDev *ptr = (HylinkDev *)node;

    int ret;
    khint_t k = kh_put(hyLink, hyMap, ptr->DeviceId, &ret);
    if (ret < 0)
    {
        printf("kh_put error\n");
        return;
    }
    kh_value(hyMap, k) = ptr;
}

void hylinkListDel(const char *key)
{
    if (key == NULL)
        return;
    khint_t k = kh_get(hyLink, hyMap, key);
    void *val = kh_value(hyMap, k);
    kh_del(hyLink, hyMap, k);
    hylinkDevFree(val);
}

void hylinkListEmpty(void)
{
    if (hyMap == NULL)
    {
        return;
    }
    HylinkDev *ptr;

#define LISTEMPTY           \
    do                      \
    {                       \
        hylinkDevFree(ptr); \
    } while (0)

    kh_foreach_value(hyMap, ptr, LISTEMPTY);
    kh_clear(hyLink, hyMap);
}

void *hylinkListGet(const char *key)
{
    if (hyMap == NULL)
    {
        return NULL;
    }
    khint_t k = kh_get(hyLink, hyMap, key);
    if (k == kh_end(hyMap))
    {
        printf("hylinkListGet key null:%d\n", k);
        return NULL;
    }
    return kh_value(hyMap, k);
}
