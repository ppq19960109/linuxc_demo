#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

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

void readFileList(const char *path, int (*readFileFunc)(const char *))
{
    DIR *dir;
    struct dirent *ptr;
    char base[64] = {0};

    if ((dir = opendir(path)) == NULL)
    {
        perror("Open dir error...");
        return;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) ///current dir OR parrent dir
            continue;
        else if (ptr->d_type == 8) ///file
        {
            strcpy(base, path);
            strcat(base, "/");
            strcat(base, ptr->d_name);
            printf("d_name:%s,path:%s\n", ptr->d_name, base);
            readFileFunc(base);
        }
        else if (ptr->d_type == 10) ///link file
            printf("d_name:%s\n", ptr->d_name);
        else if (ptr->d_type == 4) ///dir
        {
            memset(base, 0, sizeof(base));
            strcpy(base, path);
            strcat(base, "/");
            strcat(base, ptr->d_name);
            readFileList(base, readFileFunc);
        }
    }
    closedir(dir);
}

long *strToNum(const char *str, int base, long *out)
{
    errno = 0;
    char *endptr;
    long val = strtol(str, &endptr, base);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0))
    {
        perror("strtol error");
        printf("strtol endptr:%s,%ld\n", endptr, val);
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
