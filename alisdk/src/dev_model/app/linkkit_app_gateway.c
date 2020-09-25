/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include "infra_config.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "cJSON.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "infra_log.h"
#include "dev_model_api.h"
#include "wrappers.h"
#include <pthread.h>
#include "local_tcp_client.h"
#include "local_receive.h"
#include "cloud_send.h"
#include "cloud_receive.h"

pthread_mutex_t mutex;

#ifdef LINKKIT_GATEWAY_TEST_CMD
#include "simulate_subdev/testcmd.h"
#endif

#if defined(OTA_ENABLED) && defined(BUILD_AOS)
#include "ota_service.h"
#endif

char g_product_key[IOTX_PRODUCT_KEY_LEN + 1] = "a1f0jNVDEPL";
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "iaBya0CvodivLBg6";
char g_device_name[IOTX_DEVICE_NAME_LEN + 1] = "gate123";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1] = "058c8a6503a8e4336f4f06931e74de34";

#define USER_EXAMPLE_YIELD_TIMEOUT_MS (200)

#define EXAMPLE_TRACE(...)                                      \
    do                                                          \
    {                                                           \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__); \
        HAL_Printf(__VA_ARGS__);                                \
        HAL_Printf("\033[0m\r\n");                              \
    } while (0)

#define EXAMPLE_SUBDEV_ADD_NUM 3
#define EXAMPLE_SUBDEV_MAX_NUM 20
const iotx_linkkit_dev_meta_info_t subdevArr[EXAMPLE_SUBDEV_MAX_NUM] = {
    {"a1sXAdh5Fha",
     "tdCVNqzkFQrfAwRO",
     "zhjing123",
     ""}, //30a8afc52652245e01dc26e87c6f6ac7
    {"a1uRs9hBYcn",
     "akO617Wsq2v9ARg2",
     "demo123456",
     ""}, //d1726ffab64ee42e447a822462c08e3f
    {"a1uRs9hBYcn",
     "akO617Wsq2v9ARg2",
     "demo123",
     ""},

};

typedef struct
{
    int auto_add_subdev;
    int master_devid;
    int cloud_connected;
    int master_initialized;
    int subdev_index;
    int permit_join;
    void *g_user_dispatch_thread;
    int g_user_dispatch_thread_running;
} user_example_ctx_t;

static user_example_ctx_t g_user_example_ctx;

void *example_malloc(size_t size)
{
    return HAL_Malloc(size);
}

void example_free(void *ptr)
{
    HAL_Free(ptr);
}

static user_example_ctx_t *user_example_get_ctx(void)
{
    return &g_user_example_ctx;
}

static int user_connected_event_handler(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("Cloud Connected");

    user_example_ctx->cloud_connected = 1;
    linkkit_online_all();
    return 0;
}

static int user_disconnected_event_handler(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("Cloud Disconnected");

    user_example_ctx->cloud_connected = 0;

    return 0;
}

static int user_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    EXAMPLE_TRACE("Property Set Received, Devid: %d, Request: %s", devid, request);

    // res = IOT_Linkkit_Report(devid, ITM_MSG_POST_PROPERTY,
    //                          (unsigned char *)request, request_len);
    // EXAMPLE_TRACE("Post Property Message ID: %d", res);
    cloud_tolocal(devid, request);
    return 0;
}

static int user_report_reply_event_handler(const int devid, const int msgid, const int code, const char *reply,
                                           const int reply_len)
{
    const char *reply_value = (reply == NULL) ? ("NULL") : (reply);
    const int reply_value_len = (reply_len == 0) ? (strlen("NULL")) : (reply_len);

    EXAMPLE_TRACE("Message Post Reply Received, Devid: %d, Message ID: %d, Code: %d, Reply: %.*s", devid, msgid, code,
                  reply_value_len,
                  reply_value);
    return 0;
}

static int user_timestamp_reply_event_handler(const char *timestamp)
{
    EXAMPLE_TRACE("Current Timestamp: %s", timestamp);

    return 0;
}

static int user_cloud_error_handler(const int code, const char *data, const char *detail)
{
    EXAMPLE_TRACE("code =%d ,data=%s, detail=%s", code, data, detail);
    return 0;
}

static int user_initialized(const int devid)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    EXAMPLE_TRACE("Device Initialized, Devid: %d", devid);

    if (user_example_ctx->master_devid == devid)
    {
        user_example_ctx->master_initialized = 1;
    }
    user_example_ctx->subdev_index++;
    if(user_example_ctx->subdev_index>2)
    {
        IOT_Linkkit_Query(devid,ITM_MSG_QUERY_TOPOLIST,NULL,0);
    }
    return 0;
}

static uint64_t user_update_sec(void)
{
    static uint64_t time_start_ms = 0;

    if (time_start_ms == 0)
    {
        time_start_ms = HAL_UptimeMs();
    }

    return (HAL_UptimeMs() - time_start_ms) / 1000;
}

void linkkit_user_post_property(const int devid, const char *json)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    // char *property_payload = "{\"Counter\":1}";

    res = IOT_Linkkit_Report(devid, ITM_MSG_POST_PROPERTY, (unsigned char *)json, strlen(json));
    EXAMPLE_TRACE("Post Property Message ID: %d", res);
}

void user_deviceinfo_update(void)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    char *device_info_update = "[{\"attrKey\":\"abc\",\"attrValue\":\"hello,world\"}]";

    res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_DEVICEINFO_UPDATE,
                             (unsigned char *)device_info_update, strlen(device_info_update));
    EXAMPLE_TRACE("Device Info Update Message ID: %d", res);
}

void user_deviceinfo_delete(void)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    char *device_info_delete = "[{\"attrKey\":\"abc\"}]";

    res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_DEVICEINFO_DELETE,
                             (unsigned char *)device_info_delete, strlen(device_info_delete));
    EXAMPLE_TRACE("Device Info Delete Message ID: %d", res);
}

static int user_master_dev_available(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    if (user_example_ctx->cloud_connected && user_example_ctx->master_initialized)
    {
        return 1;
    }

    return 0;
}

int linkkit_subdev_online(iotx_linkkit_dev_meta_info_t *meta_info, int *dst_devid, OnlineStatus status)
{
    pthread_mutex_lock(&mutex);
    int res = -1, devid = -1;

    if (dst_devid == NULL)
        goto fail;

    switch (status)
    {
    case DEV_OFFLINE:
        if (*dst_devid < 0)
            goto fail;
        devid = *dst_devid;
        res = IOT_Linkkit_Report(devid, ITM_MSG_LOGOUT, NULL, 0);
        if (res == FAIL_RETURN)
        {
            EXAMPLE_TRACE("subdev logout Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev logout success: devid = %d\n", devid);
        break;
    case DEV_ONLINE:

        devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_SLAVE, meta_info);
        if (devid <= 0)
        {
            EXAMPLE_TRACE("subdev open Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev open susseed, devid = %d\n", devid);
        *dst_devid = devid;

        res = IOT_Linkkit_Connect(devid);
        if (res < 0)
        {
            EXAMPLE_TRACE("subdev connect Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev connect success: devid = %d\n", devid);

        res = IOT_Linkkit_Report(devid, ITM_MSG_LOGIN, NULL, 0);
        if (res < 0)
        {
            EXAMPLE_TRACE("subdev login Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev login success: devid = %d,%s\n", devid, meta_info->device_secret);

        break;
    case DEV_RESTORE:
        if (*dst_devid < 0)
            goto fail;
        devid = *dst_devid;
        res = IOT_Linkkit_Report(devid, ITM_MSG_DELETE_TOPO, NULL, 0);
        if (res == FAIL_RETURN)
        {
            EXAMPLE_TRACE("subdev logout DELETE_TOPO Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev logout DELETE_TOPO success: devid = %d\n", devid);
        break;

    default:
        break;
    }
fail:
    pthread_mutex_unlock(&mutex);
    return res;
}

int user_permit_join_event_handler(const char *product_key, const int time)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("Product Key: %s, Time: %d", product_key, time);

    user_example_ctx->permit_join = 1;

    return 0;
}

void *user_dispatch_yield(void *args)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    while (user_example_ctx->g_user_dispatch_thread_running)
    {
        IOT_Linkkit_Yield(USER_EXAMPLE_YIELD_TIMEOUT_MS);
    }

    return NULL;
}
#ifdef DEV_BIND_ENABLED
static int user_dev_bind_handler(const char *detail)
{
    EXAMPLE_TRACE("get bind event:%s", detail);
    return 0;
}
#endif
static int user_sdk_state_dump(int ev, const char *msg)
{
    printf("received state: -0x%04X(%s)\n", -ev, msg);
    return 0;
}

int user_state_dev_bind_handler(const int state_code, const char *state_message)
{
    EXAMPLE_TRACE("user_state_dev_bind_handler:%d,%s", state_code, state_message);
    return 0;
}

int user_state_dev_model_handler(const int state_code, const char *state_message)
{
    EXAMPLE_TRACE("user_state_dev_model_handler:%d,%s", state_code, state_message);
    return 0;
}
int user_topolist_handler(const int devid, const int msgid, const int code, const char *payload, const int payload_len)
{
    EXAMPLE_TRACE("user_topolist_handler devid:%d,msgid:%d,code:%d,%s", devid, msgid, code, payload);
    return 0;
}
static int max_running_seconds = 0;

void main_close()
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    user_example_ctx->g_user_dispatch_thread_running = 0;
    /*wait for  dispatch thread exit*/
    HAL_SleepMs(1000);

    IOT_Linkkit_Close(user_example_ctx->master_devid);

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_SetLogLevel(IOT_LOG_NONE);

    tcp_client_close();
    local_control_destory(&g_SLocalControl);
    cloud_control_destory(&g_SCloudControl);
    pthread_mutex_destroy(&mutex);
}

int main(int argc, char **argv)
{
    int res = 0;
    uint64_t time_prev_sec = 0, time_now_sec = 0, time_begin_sec = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    iotx_linkkit_dev_meta_info_t master_meta_info;
    int domain_type = 0;
    int dynamic_register = 0;
    int post_event_reply = 0;

    memset(user_example_ctx, 0, sizeof(user_example_ctx_t));

    pthread_mutex_init(&mutex, NULL);
    local_control_init(&g_SLocalControl);
    cloud_control_init(&g_SCloudControl);
#if defined(__UBUNTU_SDK_DEMO__)
    if (argc > 1)
    {
        int tmp = atoi(argv[1]);

        if (tmp >= 60)
        {
            max_running_seconds = tmp;
            EXAMPLE_TRACE("set [max_running_seconds] = %d seconds\n", max_running_seconds);
        }
    }

    if (argc > 2)
    {
        if (strlen("auto") == strlen(argv[2]) &&
            memcmp("auto", argv[2], strlen(argv[2])) == 0)
        {
            user_example_ctx->auto_add_subdev = 1;
        }
    }
#endif

    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, g_product_key);
    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_SECRET, g_product_secret);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_NAME, g_device_name);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_SECRET, g_device_secret);

    user_example_ctx->subdev_index = -1;

    IOT_SetLogLevel(IOT_LOG_WARNING);

    /* Register Callback */
    IOT_RegisterCallback(ITE_STATE_EVERYTHING, user_sdk_state_dump);
    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);
    IOT_RegisterCallback(ITE_DISCONNECTED, user_disconnected_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_TIMESTAMP_REPLY, user_timestamp_reply_event_handler);
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, user_initialized);
    IOT_RegisterCallback(ITE_PERMIT_JOIN, user_permit_join_event_handler);
    IOT_RegisterCallback(ITE_CLOUD_ERROR, user_cloud_error_handler);
    IOT_RegisterCallback(ITE_TOPOLIST_REPLY, user_topolist_handler);
#ifdef DEV_BIND_ENABLED
    IOT_RegisterCallback(ITE_BIND_EVENT, user_dev_bind_handler);
    // IOT_RegisterCallback(ITE_STATE_DEV_BIND, user_state_dev_bind_handler);
    // IOT_RegisterCallback(ITE_STATE_DEV_MODEL, user_state_dev_model_handler);
#endif
    memset(&master_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
    memcpy(master_meta_info.product_key, g_product_key, strlen(g_product_key));
    memcpy(master_meta_info.product_secret, g_product_secret, strlen(g_product_secret));
    memcpy(master_meta_info.device_name, g_device_name, strlen(g_device_name));
    memcpy(master_meta_info.device_secret, g_device_secret, strlen(g_device_secret));

    /* Create Master Device Resources */
    do
    {
        user_example_ctx->master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &master_meta_info);
        if (user_example_ctx->master_devid >= 0)
        {
            break;
        }
        EXAMPLE_TRACE("IOT_Linkkit_Open failed! retry after %d ms\n", 2000);
        HAL_SleepMs(2000);
    } while (1);
    /* Choose Login Server */
    domain_type = IOTX_CLOUD_REGION_SHANGHAI;
    IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);

    /* Choose Login Method */
    dynamic_register = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    /* Choose Whether You Need Post Property/Event Reply */
    post_event_reply = 0;
    IOT_Ioctl(IOTX_IOCTL_RECV_EVENT_REPLY, (void *)&post_event_reply);

    /* Start Connect Aliyun Server */
    do
    {
        res = IOT_Linkkit_Connect(user_example_ctx->master_devid);
        if (res >= 0)
        {
            break;
        }
        EXAMPLE_TRACE("IOT_Linkkit_Connect failed! retry after %d ms\n", 5000);
        HAL_SleepMs(5000);
    } while (1);

    user_example_ctx->g_user_dispatch_thread_running = 1;
    res = HAL_ThreadCreate(&user_example_ctx->g_user_dispatch_thread, user_dispatch_yield, NULL, NULL, NULL);
    if (res < 0)
    {
        EXAMPLE_TRACE("HAL_ThreadCreate Failed\n");
        IOT_Linkkit_Close(user_example_ctx->master_devid);
        return -1;
    }

    tcp_client_open();
    time_begin_sec = user_update_sec();
    while (1)
    {
        HAL_SleepMs(200);

        time_now_sec = user_update_sec();
        if (time_prev_sec == time_now_sec)
        {
            continue;
        }
        if (max_running_seconds && (time_now_sec - time_begin_sec > max_running_seconds))
        {
            EXAMPLE_TRACE("Example Run for Over %d Seconds, Break Loop!\n", max_running_seconds);
            break;
        }

        time_prev_sec = time_now_sec;
    }
    main_close();
    return 0;
}
