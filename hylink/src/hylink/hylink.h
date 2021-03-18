#ifndef _HYLINK_H_
#define _HYLINK_H_

#ifdef __cplusplus
extern "C"
{
#endif

    void hylinkOpen(void);
    int hylinkClose(void);
    int addDevToHyList(const char *devId, const char *modelId);
#ifdef __cplusplus
}
#endif
#endif