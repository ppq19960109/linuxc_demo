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
#include <ctype.h>
#include <pthread.h>
#include "cJSON.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_state.h"
#include "infra_compat.h"
#include "infra_log.h"
#include "dev_model_api.h"
#include "dynreg_api.h"
#include "wrappers.h"
//--------------------------------
#include "linkkit_app_gateway.h"
#include "linkkit_subdev.h"
#include "linkkit_sdk_status.h"
//--------------------------------
#include "logFunc.h"
#include "networkFunc.h"
#include "commonFunc.h"
#include "frameCb.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "cloudLink.h"
#include "cloudLinkCtrl.h"
#include "cloudLinkListFunc.h"

#ifdef LINKKIT_GATEWAY_TEST_CMD
#include "simulate_subdev/testcmd.h"
#endif

#if defined(OTA_ENABLED) && defined(BUILD_AOS)
#include "ota_service.h"
#endif

char g_product_key[IOTX_PRODUCT_KEY_LEN + 1] = "";
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "";
char g_device_name[IOTX_DEVICE_NAME_LEN + 1] = "00e099080011";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1] = "";
#define USER_EXAMPLE_YIELD_TIMEOUT_MS (200)

static user_example_ctx_t g_user_example_ctx;

user_example_ctx_t *user_example_get_ctx(void)
{
    return &g_user_example_ctx;
}

static int user_timestamp_reply_event_handler(const char *timestamp)
{
    EXAMPLE_TRACE("Current Timestamp: %s", timestamp);

    return 0;
}

static int user_cloud_error_handler(const int code, const char *data, const char *detail)
{
    EXAMPLE_TRACE("code =%d ,data=%s, detail=%s", code, data, detail);

    if (-code == ERROR_TOPO_RELATION_NOT_EXIST)
    {
        cJSON *root = cJSON_Parse(data);
        cJSON *deviceName = cJSON_GetObjectItem(root, "deviceName");
        if (deviceName != NULL)
            cloudLinkDevDel(deviceName->valuestring);
        cJSON_Delete(root);
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

void *user_dispatch_yield(void *args)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    while (user_example_ctx->g_user_dispatch_thread_running)
    {
        IOT_Linkkit_Yield(USER_EXAMPLE_YIELD_TIMEOUT_MS);
    }

    return NULL;
}

// static int user_sdk_state_dump(int ev, const char *msg)
// {
//     EXAMPLE_TRACE("received state: -0x%04X(%s)", -ev, msg);
//     switch (ev)
//     {
//     case STATE_USER_INPUT_META_INFO:
//         EXAMPLE_TRACE("STATE_USER_INPUT_META_INFO\n");
//         break;
//     case STATE_USER_INPUT_DEVID:
//         EXAMPLE_TRACE("STATE_USER_INPUT_DEVID\n");
//         break;
//     case STATE_USER_INPUT_DEVICE_TYPE:
//         EXAMPLE_TRACE("STATE_USER_INPUT_DEVICE_TYPE\n");
//         break;
//     case STATE_USER_INPUT_MSG_TYPE:
//         EXAMPLE_TRACE("STATE_USER_INPUT_MSG_TYPE\n");
//         break;
//     case STATE_USER_INPUT_INVALID:
//         EXAMPLE_TRACE("STATE_USER_INPUT_INVALID\n");
//         break;
//     }
//     return 0;
// }
static int user_state_dev_bind(int ev, const char *msg)
{
    switch (ev)
    {
    case STATE_BIND_ASSEMBLE_APP_TOKEN_FAILED:
        logDebug("state: -0x%04X(%s)", -ev, msg);
        EXAMPLE_TRACE("STATE_BIND_ASSEMBLE_APP_TOKEN_FAILED\n");
        break;
    case STATE_BIND_TOKEN_EXPIRED:
        logDebug("state: -0x%04X(%s)", -ev, msg);
        EXAMPLE_TRACE("STATE_BIND_TOKEN_EXPIRED\n");
        break;
    case STATE_BIND_REPORT_RESET_SUCCESS:
        logDebug("state: -0x%04X(%s)", -ev, msg);
        EXAMPLE_TRACE("STATE_BIND_REPORT_RESET_SUCCESS\n");
        break;
    case STATE_BIND_RECV_CLOUD_NOTIFY:
        logDebug("state: -0x%04X(%s)", -ev, msg);
        EXAMPLE_TRACE("STATE_BIND_RECV_CLOUD_NOTIFY\n");
        {
            cJSON *root = cJSON_Parse(msg);
            cJSON *value = cJSON_GetObjectItem(root, "value");
            if (value != NULL)
            {
                cJSON *Operation = cJSON_GetObjectItem(value, "Operation");
                if (Operation != NULL)
                {
                    if (strcmp("Bind", Operation->valuestring) == 0)
                    {
                        runTransferCb(NULL, SUBDEV_ONLINE, TRANSFER_SUBDEV_LINE);
                        runCmdCb((void *)1, LED_DRIVER_LINE);
                    }
                    else if (strcmp("Unbind", Operation->valuestring) == 0)
                    {
                        //SUBDEV_RESTORE
                        runTransferCb(NULL, SUBDEV_OFFLINE, TRANSFER_SUBDEV_LINE);
                        runSystemCb(LED_DRIVER_FLASH);
                    }
                }
            }
            cJSON_Delete(root);
        }
        break;
    }
    return 0;
}
// static int user_state_dev_model(int ev, const char *msg)
// {
//     logDebug("state: -0x%04X(%s)", -ev, msg);
//     switch (ev)
//     {
//     case STATE_DEV_MODEL_DEVICE_ALREADY_EXIST:
//         EXAMPLE_TRACE("STATE_DEV_MODEL_DEVICE_ALREADY_EXIST\n");
//         break;
//     case STATE_DEV_MODEL_DEVICE_NOT_FOUND:
//         EXAMPLE_TRACE("STATE_DEV_MODEL_DEVICE_NOT_FOUND\n");
//         break;
//     case STATE_DEV_MODEL_SUBD_NOT_DELETEABLE:
//         EXAMPLE_TRACE("STATE_DEV_MODEL_SUBD_NOT_DELETEABLE\n");
//         break;
//     case STATE_DEV_MODEL_SUBD_NOT_LOGIN:
//         EXAMPLE_TRACE("STATE_DEV_MODEL_SUBD_NOT_LOGIN\n");
//         break;
//     case STATE_DEV_MODEL_INTERNAL_ERROR:
//         EXAMPLE_TRACE("STATE_DEV_MODEL_INTERNAL_ERROR\n");
//         break;
//     case STATE_DEV_MODEL_OTA_NOT_INITED:
//         EXAMPLE_TRACE("STATE_DEV_MODEL_OTA_NOT_INITED\n");
//         break;
//     case STATE_DEV_MODEL_OTA_INIT_FAILED:
//         EXAMPLE_TRACE("STATE_DEV_MODEL_OTA_INIT_FAILED\n");
//         break;
//     case STATE_DEV_MODEL_GATEWAY_NOT_ENABLED:
//         EXAMPLE_TRACE("STATE_DEV_MODEL_GATEWAY_NOT_ENABLED\n");
//         break;
//     case STATE_DEV_MODLE_ALCS_CONTROL:
//         EXAMPLE_TRACE("STATE_DEV_MODLE_ALCS_CONTROL\n");
//         break;
//     case STATE_DEV_MODEL_MSGQ_OPERATION:
//         EXAMPLE_TRACE("STATE_DEV_MODEL_MSGQ_OPERATION\n");
//         break;
//     case STATE_DEV_MODEL_YIELD_STOPPED:
//         EXAMPLE_TRACE("STATE_DEV_MODEL_YIELD_STOPPED\n");
//         break;
//     }
//     return 0;
// }
/** fota event handler **/
static int user_fota_event_handler(int type, const char *version)
{
    char buffer[1024] = {0};
    int buffer_length = 1024;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    /* 0 - new firmware exist, query the new firmware */
    if (type == 0)
    {
        EXAMPLE_TRACE("New Firmware Version: %s", version);

        IOT_Linkkit_Query(user_example_ctx->master_devid, ITM_MSG_QUERY_FOTA_DATA, (unsigned char *)buffer, buffer_length);
    }

    return 0;
}

//--------------------------------------------------

int main_close(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    user_example_ctx->g_user_dispatch_thread_running = 0;
    /*wait for  dispatch thread exit*/
    cloudLinkClose();
    HAL_SleepMs(1000);

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_SetLogLevel(IOT_LOG_NONE);
    //-----------------------------------------------
    HAL_MutexDestroy(user_example_ctx->mutex);
    return 0;
}

void main_before_init(void)
{
    registerSystemCb(main_close, SYSTEM_CLOSE);
    cloudLinkMain();
    CloudLinkDev *cloudLinkDev = (CloudLinkDev *)addProfileDev(ALILINK_PROFILE_PATH, STR_GATEWAY_DEVID, STR_GATEWAY_MODELID, cloudLinkParseJson);
    if (cloudLinkDev == NULL)
    {
        EXAMPLE_TRACE("gw cloudLinkDev is NULL");
        return;
    }
    strcpy(g_product_key, cloudLinkDev->alinkInfo.product_key);
    strcpy(g_product_secret, cloudLinkDev->alinkInfo.product_secret);
    strcpy(g_device_secret, cloudLinkDev->alinkInfo.device_secret);
    if (strlen(g_device_name) == 0)
    {
        getNetworkSmallMac(ETH_NAME, g_device_name, sizeof(g_device_name));
        // for (int i = 0; i < strlen(g_device_name); i++)
        //     g_device_name[i] = toupper(g_device_name[i]);
    }
    EXAMPLE_TRACE("linkkit gateway g_device_name:%s,g_device_secret:%s,g_product_key:%s,g_product_secret:%s", g_device_name, g_device_secret, g_product_key, g_product_secret);
}

void main_init(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    user_example_ctx->mutex = HAL_MutexCreate();

    CloudLinkDev *cloudLinkDev = cloudLinkListGetById(STR_GATEWAY_DEVID);
    if (cloudLinkDev == NULL)
    {
        EXAMPLE_TRACE("gw cloudLinkDev is NULL");
        return;
    }
    cloudLinkDev->id = user_example_ctx->master_devid;
    hylinkMain();
    // HAL_MutexLock();
    // HAL_MutexUnlock();
}

static int max_running_seconds = 0;
int main(int argc, char **argv)
{
    int res = 0;
    uint64_t time_prev_sec = 0, time_now_sec = 0, time_begin_sec = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    iotx_linkkit_dev_meta_info_t master_meta_info;

    memset(user_example_ctx, 0, sizeof(user_example_ctx_t));
    main_before_init();

    // IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, g_product_key);
    // IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_SECRET, g_product_secret);
    // IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_NAME, g_device_name);
    // IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_SECRET, g_device_secret);

    user_example_ctx->subdev_index = -1;

    IOT_SetLogLevel(IOT_LOG_WARNING);

    /* Register Callback */
    linkkit_sdk_status_register();
    linkkit_subdev_register();
    // IOT_RegisterCallback(ITE_STATE_EVERYTHING, user_sdk_state_dump);
    IOT_RegisterCallback(ITE_STATE_DEV_BIND, user_state_dev_bind);
    // IOT_RegisterCallback(ITE_STATE_DEV_MODEL, user_state_dev_model);

    IOT_RegisterCallback(ITE_TIMESTAMP_REPLY, user_timestamp_reply_event_handler);

    IOT_RegisterCallback(ITE_CLOUD_ERROR, user_cloud_error_handler);
    IOT_RegisterCallback(ITE_FOTA, user_fota_event_handler);

    memset(&master_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
    memcpy(master_meta_info.product_key, g_product_key, strlen(g_product_key));
    memcpy(master_meta_info.product_secret, g_product_secret, strlen(g_product_secret));
    memcpy(master_meta_info.device_name, g_device_name, strlen(g_device_name));
    memcpy(master_meta_info.device_secret, g_device_secret, strlen(g_device_secret));

    EXAMPLE_TRACE("product_key:%s\n", g_product_key);
    EXAMPLE_TRACE("product_secret:%s\n", g_product_secret);
    EXAMPLE_TRACE("device_name:%s\n", g_device_name);
    EXAMPLE_TRACE("device_secret:%s\n", g_device_secret);
    if (strlen(g_device_secret) == 0)
    {
        iotx_dev_meta_info_t meta;
        memset(&meta, 0, sizeof(iotx_dev_meta_info_t));
        memcpy(meta.product_key, g_product_key, strlen(g_product_key));
        memcpy(meta.product_secret, g_product_secret, strlen(g_product_secret));
        memcpy(meta.device_name, g_device_name, strlen(g_device_name));
        res = IOT_Dynamic_Register(IOTX_HTTP_REGION_SHANGHAI, &meta);
        if (res < 0)
        {
            EXAMPLE_TRACE("IOT_Dynamic_Register:%d fail\n", res);
            char buf[64] = {0};
            res = operateFile(0, ALINKGATEWAYFILE, buf, sizeof(buf));
            if (res < 0)
            {
                EXAMPLE_TRACE("operateFile:%s fail\n", ALINKGATEWAYFILE);
            }
            else if (res == 0)
            {
            }
            else
            {
                cJSON *root = cJSON_Parse(buf);
                cJSON *val = cJSON_GetObjectItem(root, "device_secret");
                if (val != NULL)
                    strcpy(master_meta_info.device_secret, val->valuestring);
                cJSON_Delete(root);
            }
        }
        else
        {
            cJSON *root = cJSON_CreateObject();
            cJSON_AddStringToObject(root, "device_secret", master_meta_info.device_secret);
            char *json = cJSON_PrintUnformatted(root);
            if (json != NULL)
                operateFile(1, ALINKGATEWAYFILE, json, strlen(json) + 1);
            free(json);
            cJSON_Delete(root);
        }
    }
    /* Choose Login Server */
    int domain_type = IOTX_CLOUD_REGION_SHANGHAI;
    IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);
    /* Choose Login Method */
    // int dynamic_register = 1;
    // IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

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

    /* Choose Whether You Need Post Property/Event Reply */
    int post_event_reply = 1;
    IOT_Ioctl(IOTX_IOCTL_RECV_EVENT_REPLY, (void *)&post_event_reply);
    ///子设备网关多对多: 当子设备A已在网关A下面登录时, 如果需要移动到网关B下面, 那么需要在网关B上面进行子设备的登录, 流程不变, 但在SDK运行前需要按照如下代码进行配置:
    int proxy_register = 1;
    IOT_Ioctl(IOTX_IOCTL_SET_PROXY_REGISTER, (void *)&proxy_register);

    int fota_timeout = 60;
    IOT_Ioctl(IOTX_IOCTL_FOTA_TIMEOUT_MS, (void *)&fota_timeout);

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

    int dynamic_register = 1;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    user_example_ctx->g_user_dispatch_thread_running = 1;
    res = HAL_ThreadCreate(&user_example_ctx->g_user_dispatch_thread, user_dispatch_yield, NULL, NULL, NULL);
    if (res < 0)
    {
        EXAMPLE_TRACE("HAL_ThreadCreate Failed\n");
        IOT_Linkkit_Close(user_example_ctx->master_devid);
        return -1;
    }

    time_begin_sec = user_update_sec();
    main_init();

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
