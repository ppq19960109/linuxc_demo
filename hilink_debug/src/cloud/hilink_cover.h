#ifndef _HILINK_COVER_H
#define _HILINK_COVER_H

#include "protocol_cover.h"
#include "hilink_profile_bridge.h"
#include "list.h"

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
} dev_hilink_t; //

typedef struct
{
    struct list_head node;
} hilink_handle_t; //

extern hilink_handle_t hilink_handle;
void hilink_handle_init();
void hilink_handle_destory();

void BrgDevInfo_init(BrgDevInfo *brgDevInfo);
int local_tohilink(dev_data_t *src, int index);
int hilink_tolocal(const char *sn, const char *svcId, const char *payload);
#endif