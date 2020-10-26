#ifndef _LOCAL_LIST_H_
#define _LOCAL_LIST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "local_receive.h"

void list_del_dev(dev_local_t *ptr);
int list_del_by_id(const char *id, struct list_head *head);
void list_del_all(struct list_head *head);
dev_local_t *list_get_by_id(const char *id, struct list_head *head);
void list_print_all(struct list_head *head);

#ifdef __cplusplus
}
#endif
#endif