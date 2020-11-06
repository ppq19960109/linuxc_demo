#ifndef _LOCAL_LIST_H_
#define _LOCAL_LIST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "local_receive.h"

    void list_init_local(struct list_head *head);

    void list_add_local(struct list_head *node);
    
    void list_del_dev_local(dev_local_t *ptr);
    int list_del_by_id_local(const char *devid);
    void list_del_all_local();

    dev_local_t *list_get_by_id_local(const char *devid);
    void local_delete_all_dev();
    void list_print_all_local();

#ifdef __cplusplus
}
#endif
#endif