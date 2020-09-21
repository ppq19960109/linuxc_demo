#ifndef _TOOL_H_
#define _TOOL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "cJSON.h"
#include "log.h"

    char char_copy_from_json(cJSON *json, const char *src, char *dst);
    int str_copy_from_json(cJSON *json, const char *src, char *dst);
    int int_copy_from_json(cJSON *json, const char *src, int *dst);

    int str_search(const char *key, char *const *pstr, const int num);
    int strn_search(const char *key, char *const *pstr, const int num, const int n);
#ifdef __cplusplus
}
#endif
#endif