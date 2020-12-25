#ifndef _CPYTHON_H_
#define _CPYTHON_H_

#ifdef __cplusplus
extern "C"
{
#endif
    void cpythonInit(void);
    void cpythonDestroy(void);
    int hyLinkConver(const char *modelId, const char *key, const char *dir, char *in, int inLen, char *out, int *outLen);

#ifdef __cplusplus
}
#endif
#endif