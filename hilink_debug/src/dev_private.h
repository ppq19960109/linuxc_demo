#ifndef _DEV_PRIVATE_H
#define _DEV_PRIVATE_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "protocol_cover.h"

    void dev_private_attribute(dev_data_t *dev, cJSON *Data);
    void dev_private_event(dev_data_t *dev, cJSON *Data);
#ifdef __cplusplus
}
#endif
#endif