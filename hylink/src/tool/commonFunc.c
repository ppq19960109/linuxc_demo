#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#include "commonFunc.h"

int operateFile(int action, const char *path, char *buf, int len)
{
    if (buf == NULL || len == 0)
        return -1;
    int ret = -1;
    int fd = open(path, O_RDWR | O_CREAT, 0777);
    if (fd < 0)
    {
        printf("open %s fail\n", path);
        return -1;
    }
    if (action)
    {
        ret = write(fd, buf, len);
        if (ret < 0)
        {
            printf("write %s fail,%s\n", path, strerror(ret));
        }
    }
    else
    {
        ret = read(fd, buf, len);
        if (ret < 0)
        {
            printf("read %s fail,%s\n", path, strerror(ret));
        }
    }

    return ret;
}

long *strToNum(const char *str, int base, long *out)
{
    // errno = 0;
    char *endptr;
    long val = strtol(str, &endptr, base);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0))
    {
        perror("strtol error");
        printf("strtol endptr:%s\n", endptr);
        return NULL;
    }
    if (out != NULL)
    {
        *out = val;
    }
    return out;
}

int findStrIndex(const char *key, char *const *array, const int arrayLen)
{
    if (key == NULL || array == NULL || arrayLen == 0)
        return -1;

    int i;

    for (i = 0; i < arrayLen; i++)
    {
        if (strcmp(*array++, key) == 0)
        {
            return i;
        }
    }

    return -1;
}

int findStrnIndex(const char *key, const int n, char *const *array, const int arrayLen)
{
    if (key == NULL || array == NULL || arrayLen == 0)
        return -1;

    int i;

    for (i = 0; i < arrayLen; i++)
    {
        if (strncmp(*array++, key, n) == 0)
        {
            return i;
        }
    }

    return -1;
}

char getByteForJson(cJSON *json, const char *src, char *dst)
{
    if (json == NULL || src == NULL || dst == NULL)
        return -1;
    cJSON *obj = cJSON_GetObjectItem(json, src);
    if (obj != NULL)
    {
        *dst = atoi(obj->valuestring);
        return *dst;
    }
    return -1;
}

int getNumForJson(cJSON *json, const char *src, int *dst)
{
    if (json == NULL || src == NULL || dst == NULL)
        return -1;
    cJSON *obj = cJSON_GetObjectItem(json, src);
    if (obj != NULL)
    {
        *dst = atoi(obj->valuestring);
        return *dst;
    }
    return -1;
}

char *getStrForJson(cJSON *json, const char *src, char *dst)
{
    if (json == NULL || src == NULL || dst == NULL)
        return NULL;
    cJSON *obj = cJSON_GetObjectItem(json, src);
    if (obj != NULL)
    {
        strcpy(dst, obj->valuestring);
        return dst;
    }
    return NULL;
}
