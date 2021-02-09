#ifndef _HYLINKLISTFUNC_H_
#define _HYLINKLISTFUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "list.h"

#include "khash.h"
    khint_t hyLink_kh_begin();
    khint_t hyLink_kh_end();
    void *hyLink_kh_exist(int k);

#define hyLink_kh_foreach_value(vvar) for (khint_t __i = hyLink_kh_begin(); (vvar = hyLink_kh_exist(__i)) || __i != hyLink_kh_end(); ++__i)

#define HYLINK_PROFILE_PATH "hyprofile"

    enum REPORT_STATUS
    {
        NON_REPEAT_REPORT = 0x00,
        REPEAT_REPORT,
        ONLINE_NON_REPORT_AND_NON_REPEAT_REPORT
    };
    typedef struct
    {
        /*状态、属性鸿雁协议名称*/
        char hyKey[33];
        unsigned char valueType;
        char *value;
        unsigned char repeat;
    } HyLinkDevAttr;

    typedef struct
    {
        char devId[33];
        char modelId[33];
        char online;
        char first_online_report;
        HyLinkDevAttr *attr;
        unsigned char attrLen;
    } HyLinkDev;

    enum LINK_VALUE_TYPE
    {
        LINK_VALUE_TYPE_ENUM = 0x00,
        LINK_VALUE_TYPE_NUM = 0x01,
        LINK_VALUE_TYPE_STRING = 0x02,
    };

    int getLinkValueType(unsigned char dataType);
    void *hyLinkParseJson(const char *devId, const char *str);
    void *addProfileDev(const char *path, const char *devId, const char *modelId, void *(*func)(const char *, const char *));
    //-------------------------------------------
    void hylinkListInit(void);
    void hylinkListDestroy(void);
    void hylinkListAdd(void *node);
    void hylinkDevFree(HyLinkDev *ptr);
    void hylinkListDel(HyLinkDev *ptr);
    int hylinkListDelById(const char *id);
    void hylinkListEmpty(void);

    HyLinkDev *hylinkListGetById(const char *id);

#ifdef __cplusplus
}
#endif
#endif