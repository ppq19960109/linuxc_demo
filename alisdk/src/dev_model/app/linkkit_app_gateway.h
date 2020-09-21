#ifndef _LINKKIT_APP_GATEWAY_H_
#define _LINKKIT_APP_GATEWAY_H_

#include "dev_model_api.h"

typedef enum
{
    DEV_OFFLINE = 0,        /* 设备下线 */
    DEV_ONLINE = 1,         /* 设备上线 */
    DEV_RESTORE = 2,        /* 设备恢复出厂，删除云端信息 */
    DEV_ADD = 3,            /* 设备恢复出厂之后重新注册 */
} OnlineStatus;

int linkkit_subdev_online(iotx_linkkit_dev_meta_info_t *meta_info, int *out_devid, OnlineStatus status);
void linkkit_user_post_property(const int devid, const char *json);

#endif