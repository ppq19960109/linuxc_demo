#ifndef _CLOUD_RECEIVE_H_
#define _CLOUD_RECEIVE_H_

#ifdef __cplusplus
extern "C"
{
#endif
#define INT_REBOOT 0
#define INT_REFACTORY 1
#define INT_OFFLINE 2


    int cloud_tolocal(const char *sn, const char *svcId, const char *payload);
    int cloud_delete_device(const char *sn);
    void cloud_restart_reFactory(int index);
#ifdef __cplusplus
}
#endif
#endif