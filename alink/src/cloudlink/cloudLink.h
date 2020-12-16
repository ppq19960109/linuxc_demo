#ifndef _CLOUDLINK_H_
#define _CLOUDLINK_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "cloudLinkListFunc.h"
    typedef struct
    {
        CloudLinkDev *cloudLinkGateway;
        pthread_mutex_t mutex;
        struct list_head head;
    } CloudLinkControl;

    void cloudLinkInit(void);
    void cloudLinkDestory(void);
    void cloudLinkMain(void);
    void cloudLinkClose(void);

    CloudLinkControl *cloudLinkControlGet(void);
#ifdef __cplusplus
}
#endif
#endif
