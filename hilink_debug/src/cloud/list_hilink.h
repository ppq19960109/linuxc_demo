#ifndef _LIST_HILINK_H
#define _LIST_HILINK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hilink_cover.h"

    int list_del_by_id_hilink(const char *id, struct list_head *head);
    void list_del_all_hilink(struct list_head *head);
    dev_hilink_t *list_get_by_id_hilink(const char *id, struct list_head *head);
    void list_print_all_hilink(struct list_head *head);

#ifdef __cplusplus
}
#endif
#endif