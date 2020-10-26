#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "local_list.h"

dev_local_t *list_get_by_id(const char *id, struct list_head *head)
{
    dev_local_t *ptr;
    if (head == NULL)
    {
        return NULL;
    }
    if (strcmp(id, STR_HOST_GATEWAYID) == 0)
        return local_get_gateway();
    list_for_each_entry(ptr, head, node)
    {
        if (strcmp(ptr->DeviceId, id) == 0)
        {
            return ptr;
        }
    }
    return NULL;
}

void list_del_dev(dev_local_t *ptr)
{
    list_del(&ptr->node);
    if (ptr->private != NULL)
    {
        free(ptr->private);
    }
    free(ptr);
    ptr = NULL;
}

int list_del_by_id(const char *id, struct list_head *head)
{

    dev_local_t *ptr = list_get_by_id(id, head);
    if (ptr == NULL)
    {
        return -1;
    }
    list_del_dev(ptr);
    return 0;
}

void list_del_all(struct list_head *head)
{
    dev_local_t *ptr, *next;
    if (head == NULL)
    {
        return;
    }
    list_for_each_entry_safe(ptr, next, head, node)
    {
        list_del_dev(ptr);
    }
}

void list_print_all(struct list_head *head)
{
    dev_local_t *ptr;
    if (head == NULL)
    {
        return;
    }
    log_debug(" ---------------------list_print_all\n");
    int i = 0;
    list_for_each_entry(ptr, head, node)
    {
        log_debug(" ---------------------entry num:%d\n", ++i);
        log_debug("GatewayId:%s\n", ptr->GatewayId);
        log_debug("DeviceId:%s\n", ptr->DeviceId);
        log_debug("ModelId:%s\n", ptr->ModelId);
        log_debug("Version:%s\n", ptr->Version);
        log_debug("Online:%d\n", ptr->Online);
    }
}
