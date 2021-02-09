#ifndef _CLOUDLINKCTRL_H_
#define _CLOUDLINKCTRL_H_

#ifdef __cplusplus
extern "C"
{
#endif
    
    int cloudLinkCtrl(void *sn, const char *svcId, const char *payload);
    int cloudLinkDevDel(const char *sn);
    void getValueForJson(cJSON *val, char *dst);
#ifdef __cplusplus
}
#endif
#endif