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
} dev_hilink_t;

typedef struct
{
    struct list_head node;
} CloudControl_t;

extern CloudControl_t g_SCloudControl;

extern const SAttrInfo g_SCloudAttr[];

void cloud_control_init(CloudControl_t *cloudControl);
void cloud_control_destory(CloudControl_t *cloudControl);
struct list_head *cloud_get_list_head(CloudControl_t *cloudControl);

int local_tohilink(dev_data_t *src, const int index, struct list_head *cloudNode);
void cloud_hilink_upload_int(const char *svcId, const char *key, int value);
#endif