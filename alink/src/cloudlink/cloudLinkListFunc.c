#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cloudLinkListFunc.h"

void cloudLinkDevFree(CloudLinkDev *ptr)
{
    if (ptr == NULL)
        return;
    for (int i = 0; i < ptr->devSvcNum; ++i)
    {
        if (ptr->devSvc[i].svcVal != NULL)
            free(ptr->devSvc[i].svcVal);
    }
    if (ptr->devSvc != NULL)
        free(ptr->devSvc);

    free(ptr);
    ptr = NULL;
}

#ifndef CLOUD_USER_KHASH
static struct list_head *cloudLinkListHead = NULL;

void cloudLinkListInit(void *head)
{
    INIT_LIST_HEAD(head);
    cloudLinkListHead = head;
}

void cloudLinkListDestroy(void)
{
}

void cloudLinkListAdd(void *head)
{
    if (head == NULL)
        return;
    list_add(head, cloudLinkListHead);
}

void cloudLinkListDelDev(CloudLinkDev *ptr)
{
    if (ptr == NULL)
        return;
    list_del(&ptr->cloudLinkNode);
    cloudLinkDevFree(ptr);
}

void cloudLinkListEmpty(void)
{
    if (cloudLinkListHead == NULL)
    {
        return;
    }
    CloudLinkDev *ptr, *next;

    list_for_each_entry_safe(ptr, next, cloudLinkListHead, cloudLinkNode)
    {
        cloudLinkListDelDev(ptr);
    }
}

CloudLinkDev *cloudLinkListGetById(const char *id)
{
    CloudLinkDev *ptr;
    if (cloudLinkListHead == NULL)
    {
        return NULL;
    }
    list_for_each_entry(ptr, cloudLinkListHead, cloudLinkNode)
    {
        if (strcmp(ptr->devInfo.sn, id) == 0)
        {
            return ptr;
        }
    }
    return NULL;
}

CloudLinkDev *cloudLinkListGetBySn(const int sn)
{
    CloudLinkDev *ptr;
    if (cloudLinkListHead == NULL)
    {
        return NULL;
    }
    list_for_each_entry(ptr, cloudLinkListHead, cloudLinkNode)
    {
        if (ptr->devInfo.id == sn)
            return ptr;
    }
    return NULL;
}

void cloudLinkListPrintf(void)
{
    CloudLinkDev *ptr;
    if (cloudLinkListHead == NULL)
    {
        printf("cloudLinkListHead id NULL\n");
        return;
    }
    list_for_each_entry(ptr, cloudLinkListHead, cloudLinkNode)
    {
        printf("cloudLinkList sn:%s\n", ptr->devInfo.sn);
    }
}
#else

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
void cloudLinkListInit(void *head)
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
    CloudLinkDev *ptr = NULL;
    ptr = list_entry(node, typeof(*ptr), cloudLinkNode);
    if (ptr == NULL)
    {
        printf("cloudLinkListAdd ptr null error\n");
        return;
    }
    int ret;
    khint_t k = kh_put(cloudLink, cloudMap, ptr->devInfo.sn, &ret);
    if (ret < 0)
    {
        printf("kh_put error\n");
        return;
    }
    kh_value(cloudMap, k) = ptr;
}

void cloudLinkListDelDev(CloudLinkDev *ptr)
{
    if (ptr == NULL)
        return;
    khint_t k = kh_get(cloudLink, cloudMap, ptr->devInfo.sn);
    kh_del(cloudLink, cloudMap, k);
    cloudLinkDevFree(ptr);
}

void cloudLinkListEmpty(void)
{
    if (cloudMap == NULL)
    {
        return;
    }
    CloudLinkDev *ptr;

#define CLOUDLINKEMPTY         \
    do                         \
    {                          \
        cloudLinkDevFree(ptr); \
    } while (0)

    kh_foreach_value(cloudMap, ptr, CLOUDLINKEMPTY);
    kh_clear(cloudLink, cloudMap);
}

CloudLinkDev *cloudLinkListGetById(const char *id)
{
    CloudLinkDev *ptr;
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
    CloudLinkDev *ptr;
#define CLOUDLINKSN                \
    do                             \
    {                              \
        if (ptr->devInfo.id == sn) \
            return ptr;            \
    } while (0)

    kh_foreach_value(cloudMap, ptr, CLOUDLINKSN);
    return NULL;
}

void cloudLinkListPrintf(void)
{
    if (cloudMap == NULL)
    {
        return;
    }
    CloudLinkDev *ptr;
#define CLOUDLINKPRINTF                              \
    do                                               \
    {                                                \
        printf("---devid:%s --\n", ptr->devInfo.sn); \
    } while (0)

    kh_foreach_value(cloudMap, ptr, CLOUDLINKPRINTF);
}

#endif
