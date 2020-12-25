#ifndef _HYLINKSUBDEV_H_
#define _HYLINKSUBDEV_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "cJSON.h"
#include "hylinkListFunc.h"

    int hylinkSubDevAttrUpdate(HyLinkDev *dev, cJSON *Data);

#ifdef __cplusplus
}
#endif
#endif