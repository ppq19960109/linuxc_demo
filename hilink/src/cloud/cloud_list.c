#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cloud_list.h"

dev_cloud_t *list_get_by_id_cloud(const char *id, struct list_head *head)
{
    dev_cloud_t *ptr, *next;
    if (head == NULL)
    {
        return NULL;
    }
    list_for_each_entry_safe(ptr, next, head, node)
    {
        if (strcmp(ptr->brgDevInfo.sn, id) == 0)
        {
            return ptr;
        }
    }
    return NULL;
}

void list_del_dev_cloud(dev_cloud_t *ptr)
{
    list_del(&ptr->node);
    for (int i = 0; i < ptr->devSvcNum; ++i)
    {
        if (ptr->devSvc[i].svcVal != NULL)
            free(ptr->devSvc[i].svcVal);
    }
    if (ptr->devSvc != NULL)
        free(ptr->devSvc);

    free(ptr);
    ptr=NULL;
}

int list_del_by_id_cloud(const char *id, struct list_head *head)
{
    dev_cloud_t *ptr = list_get_by_id_cloud(id, head);
    if (ptr == NULL)
    {
        return -1;
    }
    list_del_dev_cloud(ptr);
    return 0;
}

void list_del_all_cloud(struct list_head *head)
{
    dev_cloud_t *ptr, *next;
    if (head == NULL)
    {
        return;
    }
    list_for_each_entry_safe(ptr, next, head, node)
    {
        list_del_dev_cloud(ptr);
    }
}

void list_print_all_cloud(struct list_head *head)
{
    dev_cloud_t *ptr;
    if (head == NULL)
    {
        return;
    }
    log_info(" ---------------------list_print_all_cloud\n");
    int i = 0;
    list_for_each_entry(ptr, head, node)
    {
        log_info(" ---------------------hilink entry num:%d\n", ++i);
        log_info("sn:%s\n", ptr->brgDevInfo.sn);
        log_info("prodId:%s\n", ptr->brgDevInfo.prodId);
        log_info("model:%s\n", ptr->brgDevInfo.model);
        log_info("devType:%s\n", ptr->brgDevInfo.devType);
        log_info("mac:%s\n", ptr->brgDevInfo.mac);
    }
}