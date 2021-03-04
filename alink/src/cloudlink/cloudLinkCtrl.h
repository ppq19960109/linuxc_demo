#ifndef _CLOUDLINKCTRL_H_
#define _CLOUDLINKCTRL_H_

#ifdef __cplusplus
extern "C"
{
#endif

    int cloudLinkCtrl(void *sn, const char *payload);
    int cloudLinkServicCtrl(const int devid, const char *serviceid, const int serviceid_len, const char *request, char **response, int *response_len);
    int cloudLinkDevDel(void *sn);

#ifdef __cplusplus
}
#endif
#endif