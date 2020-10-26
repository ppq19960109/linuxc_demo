#ifndef _CLOUD_RECEIVE_H_
#define _CLOUD_RECEIVE_H_

#ifdef __cplusplus
extern "C"
{
#endif

    int cloud_tolocal(const char *sn, const char *svcId, const char *payload);
    int cloud_delete_device(const char *sn);

#ifdef __cplusplus
}
#endif
#endif