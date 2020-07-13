#ifndef _PROTOCOL_COVER_H
#define _PROTOCOL_COVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "list.h"
#include "cJSON.h"
#include "sqlite3.h"
#include "log.h"
#include "client.h"

    typedef struct
    {
        char GatewayId[16];
        char DeviceType[16];
        char DeviceId[18];
        char ModelId[16];
        char Version[16];
        char Secret[40];
        char Online;
        char RegisterStatus;
        void *private;
        struct list_head node;
    } dev_data_t;

#include "list_tool.h"

    struct local_data_t
    {
        char DeviceId[18];
        char ModelId[16];
        char Key[16];
        char Value[16];
        void *private;
    };
    typedef struct
    {
        char Command;
        int FrameNumber;
        char Type[12];
        // char GatewayId[16];
        struct local_data_t Data;
        // void *private;
    } local_dev_t;

    typedef struct
    {
        int socketfd;
        struct list_head dev_list;
    } protocol_data_t;

    extern protocol_data_t protocol_data;

    void protlcol_init();
    void protlcol_destory();
    char char_copy_from_json(cJSON *json, char *src, char *dst);
    int str_copy_from_json(cJSON *json, char *src, char *dst);
    int int_copy_from_json(cJSON *json, char *src, int *dst);

    int read_from_local(const char *json);
    int write_to_local(void *ptr);
    int isStrNotNull(const char *str);
    int str_search(const char *key,  char **pstr, int num);
    int strn_search(const char *key, char **pstr, int num, int n);
#ifdef __cplusplus
}
#endif
#endif