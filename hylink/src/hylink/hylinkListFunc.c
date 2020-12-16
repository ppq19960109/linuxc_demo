#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hylinkListFunc.h"

void hylinkDevFree(HylinkDev *ptr)
{
    if (ptr == NULL)
        return;

    if (ptr->private != NULL)
    {
        free(ptr->private);
    }
    free(ptr);
    ptr = NULL;
}

#ifndef HY_USER_KHASH
static struct list_head *hylinkListHead = NULL;

void hylinkListInit(void *head)
{
    INIT_LIST_HEAD(head);
    hylinkListHead = head;
}

void hylinkListDestroy(void)
{
}

void hylinkListAdd(void *node)
{
    if (node == NULL)
        return;
    list_add(node, hylinkListHead);
}

void hylinkListDel(HylinkDev *ptr)
{
    if (ptr == NULL)
        return;
    list_del(&ptr->hylinkNode);
    hylinkDevFree(ptr);
}

int hylinkListDelById(const char *devid)
{
    if (devid == NULL)
        return -1;
    HylinkDev *ptr = hylinkListGetById(devid);
    if (ptr == NULL)
    {
        return -1;
    }
    hylinkListDel(ptr);
    return 0;
}

void hylinkListEmpty(void)
{
    HylinkDev *ptr, *next;
    if (hylinkListHead == NULL)
    {
        return;
    }
    list_for_each_entry_safe(ptr, next, hylinkListHead, hylinkNode)
    {
        hylinkListDel(ptr);
    }
}

HylinkDev *hylinkListGetById(const char *devid)
{
    if (devid == NULL)
        return;
    HylinkDev *ptr;
    if (hylinkListHead == NULL || devid == NULL)
    {
        return NULL;
    }

    list_for_each_entry(ptr, hylinkListHead, hylinkNode)
    {
        if (strcmp(ptr->DeviceId, devid) == 0)
        {
            return ptr;
        }
    }
    return NULL;
}

#else

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
void hylinkListInit(void *head)
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
    HylinkDev *ptr = NULL;
    ptr = list_entry(node, typeof(*ptr), hylinkNode);
    if (ptr == NULL)
    {
        printf("hylinkListAdd ptr null error\n");
        return;
    }
    int ret;
    khint_t k = kh_put(hyLink, hyMap, ptr->DeviceId, &ret);
    if (ret < 0)
    {
        printf("kh_put error\n");
        return;
    }
    kh_value(hyMap, k) = ptr;
}

void hylinkListDel(HylinkDev *ptr)
{
    if (ptr == NULL)
        return;
    khint_t k = kh_get(hyLink, hyMap, ptr->DeviceId);
    kh_del(hyLink, hyMap, k);
    hylinkDevFree(ptr);
}

int hylinkListDelById(const char *id)
{
    if (id == NULL)
        return -1;
    HylinkDev *ptr = hylinkListGetById(id);
    if (ptr == NULL)
    {
        return -1;
    }
    hylinkListDel(ptr);
    return 0;
}

void hylinkListEmpty(void)
{
    if (hyMap == NULL)
    {
        return;
    }
    HylinkDev *ptr;

#define CLOUDLINKEMPTY      \
    do                      \
    {                       \
        hylinkDevFree(ptr); \
    } while (0)

    kh_foreach_value(hyMap, ptr, CLOUDLINKEMPTY);
    kh_clear(hyLink, hyMap);
}

HylinkDev *hylinkListGetById(const char *id)
{
    if (hyMap == NULL)
    {
        return NULL;
    }
    khint_t k = kh_get(hyLink, hyMap, id);
    if (k == kh_end(hyMap))
    {
        printf("cloudLinkListGetById id null:%d\n", k);
        return NULL;
    }
    return kh_value(hyMap, k);
}

#endif