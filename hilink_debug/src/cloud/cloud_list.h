#ifndef _CLOUD_LIST_H_
#define _CLOUD_LIST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "cloud_send.h"

    void list_del_dev_hilink(dev_hilink_t *ptr);
    int list_del_by_id_hilink(const char *id, struct list_head *head);
    void list_del_all_hilink(struct list_head *head);
    dev_hilink_t *list_get_by_id_hilink(const char *id, struct list_head *head);
    void list_print_all_hilink(struct list_head *head);

#ifdef __cplusplus
}
#endif
#endif