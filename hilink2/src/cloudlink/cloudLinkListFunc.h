#ifndef _CLOUDLINKLISTFUNC_H_
#define _CLOUDLINKLISTFUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#include "khash.h"
    khint_t cloudLink_kh_begin();
    khint_t cloudLink_kh_end();
    void *cloudLink_kh_exist(int k);
#define cloudLink_kh_foreach_value(vvar) \
    for (khint_t __i = cloudLink_kh_begin(); (vvar = cloudLink_kh_exist(__i)) || __i != cloudLink_kh_end(); ++__i)

    typedef struct
    {
        char hyKey[33];
        char hyType[8];
        char cloudSid[33];
        char cloudKey[33];
        char repeat;
    } CloudLinkDevAttr;

    typedef struct
    {
        CloudLinkDevAttr *attr;
        char attrLen;
        BrgDevInfo brgDevInfo;
    } CloudLinkSubDev;

    typedef struct
    {
        char modelId[33];

        CloudLinkDevAttr *attr;
        char attrLen;
        BrgDevInfo brgDevInfo;

        CloudLinkSubDev *cloudLinkSubDev;
        char cloudLinkSubDevLen;
#ifdef HILINK_REPORT_SYNC
        char hilink_now_online;
#endif
    } CloudLinkDev;

    void *cloudLinkParseJson(const char *devId, const char *str);

    void cloudLinkListInit(void);
    void cloudLinkListDestroy(void);
    void cloudLinkListAdd(void *node);

    void cloudLinkListDelDev(CloudLinkDev *ptr);
    void cloudLinkListEmpty(void);
    CloudLinkDev *cloudLinkListGetById(const char *id);
    void cloudLinkListPrintf(void);

#ifdef __cplusplus
}
#endif
#endif