#ifndef _LINKKIT_SUBDEV_H_
#define _LINKKIT_SUBDEV_H_

#include "dev_model_api.h"
#include "frameCb.h"

// typedef enum
// {
//     DEV_OFFLINE = 0, /* 设备下线 */
//     DEV_ONLINE = 1,  /* 设备上线 */
//     DEV_RESTORE = 2, /* 设备恢复出厂，删除云端信息 */
//     DEV_REONINK,
// } DevStatus;

void linkkit_subdev_register(void);
void linkkit_user_post_property(const int devid, const char *payload);
int linkkit_subdev_status(iotx_linkkit_dev_meta_info_t *meta_info, int *id, SubDevStatus status);

#endif