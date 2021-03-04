#ifndef _LINKKIT_SUBDEV_H_
#define _LINKKIT_SUBDEV_H_

#include "main.h"
void linkkit_devrst_evt_handle(iotx_devrst_evt_type_t evt, void *msg);
void linkkit_subdev_register(void);
void linkkit_user_post_property(const int devid, const char *payload);
int linkkit_subdev_status(iotx_linkkit_dev_meta_info_t *meta_info, int *id, SubDevStatus status);
int user_post_event(int devid, char *event_id, char *event_payload);
#endif