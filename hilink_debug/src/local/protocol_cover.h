#ifndef _PROTOCOL_COVER_H
#define _PROTOCOL_COVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "list.h"
#include "cJSON.h"
#include "log.h"
#include "client.h"

#define POINTER_SIZE 4
#define SENDTOLOCAL_SIZE 512
#define GATEWAYID "0000000000000000"
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

    struct local_data_t
    {
        char DeviceId[18];
        char ModelId[16];
        char Key[24];
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
        char discoverMode;
        char sendData[SENDTOLOCAL_SIZE];
        pthread_t pid;
        struct list_head dev_list;
    } protocol_data_t;
    //-----------------------------------------------
    extern protocol_data_t protocol_data;

    void protlcol_init();
    void protlcol_destory();
    char char_copy_from_json(cJSON *json, char *src, char *dst);
    int str_copy_from_json(cJSON *json, char *src, char *dst);
    int int_copy_from_json(cJSON *json, char *src, int *dst);

    int read_from_local(const char *json);
    int write_to_local(void *ptr);
    int isStrNotNull(const char *str);
    int str_search(const char *key, char **pstr, int num);
    int strn_search(const char *key, char **pstr, int num, int n);
    int writeToHaryan(const char *data, int socketfd, char *sendBuf, int bufLen);
    int write_cmd(char *cmd, char *DeviceId,char* Value);
    void local_reFactory();

#ifdef __cplusplus
}
#endif
#endif