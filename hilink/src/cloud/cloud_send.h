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

typedef struct
{
    struct list_head head;
} CloudControl_t;

extern const SAttrInfo g_SCloudAttr[];

void cloud_control_init();
void cloud_control_destory();
struct list_head *cloud_get_list_head();

void hilink_onlineStatus(dev_data_t *src, DevOnlineStatus status);
void hilink_all_online(int online,DevOnlineStatus status);

int local_tohilink(dev_data_t *src, const int index, struct list_head *cloudNode);
// void cloud_hilink_upload_int(const char *svcId, const char *key, int value);
#endif