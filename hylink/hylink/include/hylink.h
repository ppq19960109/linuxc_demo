#ifndef _HYLINKCONTROL_H_
#define _HYLINKCONTROL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include "cJSON.h"
#include "hylinkListFunc.h"

    typedef struct
    {
        char *const *attr;
        unsigned short attrLen;
        char *const *attrCtrl;
        unsigned short attrCtrlLen;
    } SAttrInfo;

    void hylinkMain(void);
    pthread_mutex_t *hylinkGetMutex(void);
    unsigned char *getHyDispatchBuf(void);
#ifdef __cplusplus
}
#endif
#endif