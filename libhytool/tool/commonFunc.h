#ifndef _COMMONFUNC_H_
#define _COMMONFUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "cJSON.h"
    int operateFile(int action, const char *path, char *buf, int len);
    void readFileList(const char *path, int (*readFileFunc)(const char *));
    long *strToNum(const char *str, int base, long *out);
    int findStrIndex(const char *key, char *const *array, const int arrayLen);
    int findStrnIndex(const char *key, const int n, char *const *array, const int arrayLen);
    char getByteForJson(cJSON *json, const char *src, char *dst);
    int getNumForJson(cJSON *json, const char *src, int *dst);
    char *getStrForJson(cJSON *json, const char *src, char *dst);

#ifdef __cplusplus
}
#endif
#endif