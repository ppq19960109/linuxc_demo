#include "list_hilink.h"

dev_hilink_t *list_get_by_id_hilink(const char *id, struct list_head *head)
{
    dev_hilink_t *ptr, *next;
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

void list_del_dev_hilink(dev_hilink_t *ptr)
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
}

int list_del_by_id_hilink(const char *id, struct list_head *head)
{

    dev_hilink_t *ptr = list_get_by_id_hilink(id, head);
    if (ptr == NULL)
    {
        return -1;
    }
    list_del_dev_hilink(ptr);
    return 0;
}

void list_del_all_hilink(struct list_head *head)
{
    dev_hilink_t *ptr, *next;
    if (head == NULL)
    {
        return;
    }
    list_for_each_entry_safe(ptr, next, head, node)
    {
        list_del_dev_hilink(ptr);
    }
}

void list_print_all_hilink(struct list_head *head)
{
    dev_hilink_t *ptr;
    if (head == NULL)
    {
        return;
    }

    int i = 0;
    list_for_each_entry(ptr, head, node)
    {
        log_info(" ---------------------hilink entry num:%d\n", i++);
        log_info("sn:%s\n", ptr->brgDevInfo.sn);
        log_info("prodId:%s\n", ptr->brgDevInfo.prodId);
        log_info("model:%s\n", ptr->brgDevInfo.model);
        log_info("devType:%s\n", ptr->brgDevInfo.devType);
        log_info("mac:%s\n", ptr->brgDevInfo.mac);
    }
}