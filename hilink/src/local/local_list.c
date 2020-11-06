#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "local_list.h"
#include "local_send.h"

static struct list_head *local_head = NULL;

void list_init_local(struct list_head *head)
{
    INIT_LIST_HEAD(head);
    local_head = head;
}

void list_add_local(struct list_head *node)
{
    list_add(node, local_head);
}

dev_local_t *list_get_by_id_local(const char *devid)
{
    dev_local_t *ptr;
    if (local_head == NULL || devid == NULL)
    {
        return NULL;
    }
    if (strcmp(devid, STR_HOST_GATEWAYID) == 0)
        return local_get_gateway();
    list_for_each_entry(ptr, local_head, node)
    {
        if (strcmp(ptr->DeviceId, devid) == 0)
        {
            return ptr;
        }
    }
    return NULL;
}

void list_del_dev_local(dev_local_t *ptr)
{
    list_del(&ptr->node);
    if (ptr->private != NULL)
    {
        free(ptr->private);
    }
    free(ptr);
    ptr = NULL;
}

int list_del_by_id_local(const char *devid)
{

    dev_local_t *ptr = list_get_by_id_local(devid);
    if (ptr == NULL)
    {
        return -1;
    }
    list_del_dev_local(ptr);
    return 0;
}

void list_del_all_local()
{
    dev_local_t *ptr, *next;
    if (local_head == NULL)
    {
        return;
    }
    list_for_each_entry_safe(ptr, next, local_head, node)
    {
        list_del_dev_local(ptr);
    }
}

void local_delete_all_dev()
{
    dev_local_t *ptr;
    list_for_each_entry(ptr, local_head, node)
    {
        write_delete_dev(ptr->DeviceId);
    }
}

void list_print_all_local()
{
    dev_local_t *ptr;
    if (local_head == NULL)
    {
        return;
    }
    log_debug(" ---------------------list_print_all_local\n");
    int i = 0;
    list_for_each_entry(ptr, local_head, node)
    {
        log_debug(" ---------------------entry num:%d\n", ++i);
        log_debug("GatewayId:%s\n", ptr->GatewayId);
        log_debug("DeviceId:%s\n", ptr->DeviceId);
        log_debug("ModelId:%s\n", ptr->ModelId);
        log_debug("Version:%s\n", ptr->Version);
        log_debug("Online:%d\n", ptr->Online);
    }
}
