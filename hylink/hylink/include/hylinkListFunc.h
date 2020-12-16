#ifndef _HYLINKLISTFUNC_H_
#define _HYLINKLISTFUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "list.h"
#define HY_USER_KHASH

#ifndef HY_USER_KHASH
#define hylinkListEachEntrySafe(pos, n, head, member) list_for_each_entry_safe(pos, n, head, member)
#else
#include "khash.h"
khint_t hyLink_kh_begin();
khint_t hyLink_kh_end();
void *hyLink_kh_exist(int k);

#define hyLink_kh_foreach_value(vvar) for (khint_t __i = hyLink_kh_begin(); (vvar = hyLink_kh_exist(__i)) || __i != hyLink_kh_end(); ++__i)
#endif

    typedef struct
    {
        char DeviceId[24];
        char ModelId[24];
        char GatewayId[24];
        char Version[24];
        char Online;
        char devType;
        void *private;
        struct list_head hylinkNode;
    } HylinkDev;

    void hylinkListInit(void *head);
    void hylinkListDestroy(void);
    void hylinkListAdd(void *node);
    void hylinkDevFree(HylinkDev *ptr);
    void hylinkListDel(HylinkDev *ptr);
    int hylinkListDelById(const char *id);
    void hylinkListEmpty(void);

    HylinkDev *hylinkListGetById(const char *id);

#ifdef __cplusplus
}
#endif
#endif