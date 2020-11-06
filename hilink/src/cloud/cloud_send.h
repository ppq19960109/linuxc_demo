#ifndef _CLOUD_SEND_H_
#define _CLOUD_SEND_H_

#include "local_device.h"
#include "list.h"

#include "hilink_profile_bridge.h"

#define STR_ON "on"
#define STR_MODE "mode"
#define STR_GEAR "gear"
#define STR_COLORTEMPERATURE "colorTemperature"
#define STR_BRIGHTNESS "brightness"
#define STR_STATUS "status"
#define STR_CURRENT "current"
#define STR_TARGET "target"
#define STR_NUM "num"
#define STR_NAME "name"

typedef struct
{
    char *svcId;
    char *svcVal;
} DevSvc;

typedef struct
{
    BrgDevInfo brgDevInfo;
    DevSvc *devSvc;
    char devSvcNum;
    struct list_head node;
} dev_cloud_t;

typedef enum
{
    CLOUD_OFFLINE = 0,      /* 下线 */
    CLOUD_ONLINE = 1,       /* 上线 */
    CLOUD_REGISTERED = 2,   /* 注册 */
    CLOUD_UNREGISTERED = 3, /* 被解绑 */
} CloudStatus;

typedef struct
{
    pid_t pid;
    CloudStatus cloud_status;
    int registerFlag;
    struct list_head head;
} CloudControl_t;

extern const SAttrInfo g_SCloudAttr[];

void cloud_control_init();
void cloud_control_destory();
struct list_head *cloud_get_list_head();
CloudStatus get_cloud_status(void);
void set_cloud_status(CloudStatus status);
int get_registerFlag(void);
void set_registerFlag(void);
int local_tocloud(dev_local_t *src, const int index);

#endif