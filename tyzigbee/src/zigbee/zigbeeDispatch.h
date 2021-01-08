#ifndef _ZIGBEEDISPATCH_H_
#define _ZIGBEEDISPATCH_H_

#ifdef __cplusplus
extern "C"
{
#endif
    int zigbeeZclDispatch(void *devId, void *modelId, void *key, void *value);

#ifdef __cplusplus
}
#endif
#endif