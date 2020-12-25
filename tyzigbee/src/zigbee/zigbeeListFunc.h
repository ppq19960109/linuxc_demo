#ifndef _ZIGBEELISTFUNC_H_
#define _ZIGBEELISTFUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "list.h"

#include "khash.h"
    khint_t zigbee_kh_begin();
    khint_t zigbee_kh_end();
    void *zigbee_kh_exist(int k);

#define zigbee_kh_foreach_value(vvar) for (khint_t __i = zigbee_kh_begin(); (vvar = zigbee_kh_exist(__i)) || __i != zigbee_kh_end(); ++__i)

    typedef struct
    {
        /*状态、属性鸿雁协议名称*/
        char hyKey[33];
        char hyKeyPrivate;
        /*属性endpoint，zigbee设备使用*/
        char srcEndpoint;
        char dstEndpoint;
        unsigned short AttributeId;
        unsigned short ClusterId;
        char z3CmdType;
        char z3CmdId;
        unsigned char dataType;
    } ZigbeeAttr;

    typedef struct
    {
        char modelId[33];
        char manuName[33];
        ZigbeeAttr *attr;
        char attrLen;
    } zigbeeDev;

    void zigbeeListInit(void);
    void zigbeeListDestroy(void);
    void zigbeeListAdd(void *node);
    void zigbeeListDel(const char *key);
    void *zigbeeListGet(const char *key);
    void zigbeeListEmpty(void);
    void zigbeeListPrintf(void);
#ifdef __cplusplus
}
#endif
#endif