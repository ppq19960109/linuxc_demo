#ifndef _CLOUDLINKLISTFUNC_H_
#define _CLOUDLINKLISTFUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "list.h"
#include "linkkit_subdev.h"

#define CLOUD_USER_KHASH

#ifndef CLOUD_USER_KHASH
#define cloudLinkListEachEntry(pos, head, member) list_for_each_entry(pos, head, member)
#define cloudLinkListEachEntrySafe(pos, n, head, member) list_for_each_entry_safe(pos, n, head, member)
#else
#include "khash.h"
khint_t cloudLink_kh_begin();
khint_t cloudLink_kh_end();
void *cloudLink_kh_exist(int k);
#define cloudLink_kh_foreach_value(vvar) \
    for (khint_t __i = cloudLink_kh_begin(), vvar = NULL; (vvar = cloudLink_kh_exist(__i)) || __i != cloudLink_kh_end(); ++__i, vvar = NULL)
#endif
    typedef struct
    {
        int id;
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
        CloudDevInfo devInfo;
        DevSvc *devSvc;
        char devSvcNum;
        // char Online;
        char devType;
        struct list_head cloudLinkNode;
    } CloudLinkDev;

    void cloudLinkListInit(void *head);
    void cloudLinkListDestroy(void);
    void cloudLinkListAdd(void *node);

    void cloudLinkDevFree(CloudLinkDev *ptr);
    void cloudLinkListDelDev(CloudLinkDev *ptr);
    void cloudLinkListEmpty(void);
    CloudLinkDev *cloudLinkListGetById(const char *devid);
    CloudLinkDev *cloudLinkListGetBySn(const int sn);
    void cloudLinkListPrintf(void);

#ifdef __cplusplus
}
#endif
#endif