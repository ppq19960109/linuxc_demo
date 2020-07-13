#ifndef _LIST_TOOL_H
#define _LIST_TOOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "protocol_cover.h"

int list_del_by_id(const char *id, struct list_head *head);
void list_del_all(struct list_head *head);
dev_data_t *list_get_by_id(const char *id, struct list_head *head);
void list_print_all(struct list_head *head);

#ifdef __cplusplus
}
#endif
#endif