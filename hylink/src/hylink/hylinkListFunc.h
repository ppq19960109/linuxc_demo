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

    typedef struct
    {
        char DeviceId[33];
        char ModelId[33];
        char online;
    } HylinkDev;

    void hylinkListInit(void);
    void hylinkListDestroy(void);
    void hylinkListAdd(void *node);
    void hylinkListDel(const char *key);
    void *hylinkListGet(const char *key);
    void hylinkListEmpty(void);
    int hylinkListSize(void);
#ifdef __cplusplus
}
#endif
#endif