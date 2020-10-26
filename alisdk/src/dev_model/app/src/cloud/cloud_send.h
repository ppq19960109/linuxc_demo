#ifndef _CLOUD_SEND_H_
#define _CLOUD_SEND_H_

#include "local_device.h"
#include "list.h"

#include "linkkit_app_gateway.h"

typedef struct
{
    int cloudDevId;
    char sn[32];
    char mac[32];
    char fwv[18]; /* 设备固件版本*/
    iotx_linkkit_dev_meta_info_t meta_info;
} CloudDevInfo;

typedef struct
{
    char *svcId;
    char *svcVal;
} DevSvc;

typedef struct
{
    CloudDevInfo brgDevInfo;
    DevSvc *devSvc;
    char devSvcNum;
    struct list_head node;
} dev_cloud_t;

typedef struct
{
    struct list_head node;
} CloudControl_t;

extern CloudControl_t g_SCloudControl;

extern const SAttrInfo g_SCloudAttr[];

void cloud_control_init(CloudControl_t *cloudControl);
void cloud_control_destory(CloudControl_t *cloudControl);
struct list_head *cloud_get_list_head(CloudControl_t *cloudControl);

void linkkit_online_all();
int local_tohilink(dev_data_t *src, const int index, struct list_head *cloudNode);
void cloud_hilink_upload_int(const char *svcId, const char *key, int value);
#endif