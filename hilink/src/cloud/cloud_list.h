#ifndef _CLOUD_LIST_H_
#define _CLOUD_LIST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "cloud_send.h"
    void list_init_cloud(struct list_head *head);
    void list_add_cloud(struct list_head *node);

    void list_del_dev_cloud(dev_cloud_t *ptr);
    int list_del_by_id_cloud(const char *devid);

    void list_del_all_cloud();

    dev_cloud_t *list_get_by_id_cloud(const char *devid);

    void list_print_all_cloud();

#ifdef __cplusplus
}
#endif
#endif