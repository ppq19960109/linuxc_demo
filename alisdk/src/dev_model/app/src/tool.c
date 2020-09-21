#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tool.h"

int str_search(const char *key, char *const *pstr, const int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        // 指针数组  p首先是个指针  然后指向类型是地址 所以是二级指针
        if (strcmp(*pstr++, key) == 0)
        {
            return i;
        }
    }

    return -1;
}

int strn_search(const char *key, char *const *pstr, const int num, const int n)
{
    int i;

    for (i = 0; i < num; i++)
    {
        // 指针数组  p首先是个指针  然后指向类型是地址 所以是二级指针
        if (strncmp(*pstr++, key, n) == 0)
        {
            return i;
        }
    }

    return -1;
}

char char_copy_from_json(cJSON *json, const char *src, char *dst)
{
    cJSON *obj = cJSON_GetObjectItem(json, src);
    if (obj != NULL)
    {
        *dst = atoi(obj->valuestring);
        return *dst;
    }
    return -1;
}

int int_copy_from_json(cJSON *json, const char *src, int *dst)
{
    cJSON *obj = cJSON_GetObjectItem(json, src);
    if (obj != NULL)
    {
        *dst = atoi(obj->valuestring);
        return *dst;
    }
    return -1;
}

int str_copy_from_json(cJSON *json, const char *src, char *dst)
{
    cJSON *obj = cJSON_GetObjectItem(json, src);
    if (obj != NULL)
    {
        strcpy(dst, obj->valuestring);
        return 0;
    }
    return -1;
}