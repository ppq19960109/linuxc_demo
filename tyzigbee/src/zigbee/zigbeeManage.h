#ifndef _zigbeeManage_H_
#define _zigbeeManage_H_

#ifdef __cplusplus
extern "C"
{
#endif

int zigbeeDevZclReport(void *recv);
int zigbeeDevZclDispatch(void *devId, void *modelId, void *key, void *value);
#ifdef __cplusplus
}
#endif
#endif