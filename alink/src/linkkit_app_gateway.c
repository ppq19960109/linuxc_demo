/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "main.h"

#include "cloudLink.h"
#include "cloudLinkCtrl.h"
#include "cloudLinkListFunc.h"

#ifdef LINKKIT_GATEWAY_TEST_CMD
#include "simulate_subdev/testcmd.h"
#endif

#if defined(OTA_ENABLED) && defined(BUILD_AOS)
#include "ota_service.h"
#endif

// char g_product_key[IOTX_PRODUCT_KEY_LEN + 1] = "";
// char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "";
// char g_device_name[IOTX_DEVICE_NAME_LEN + 1] = "";
// char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1] = "";
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
                        runSystemCb(LED_DRIVER_TIMER_OPEN);
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
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    /* 0 - new firmware exist, query the new firmware */
    if (type == 0)
    {
        char buffer[1024] = {0};
        int buffer_length = 1024;
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

int operate_gateway_info(char *device_name, char *device_secret, int flags)
{
    int res = 0;
    if (flags)
    {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "devicce_name", device_name);
        cJSON_AddStringToObject(root, "device_secret", device_secret);
        char *json = cJSON_PrintUnformatted(root);
        if (json != NULL)
        {
            res = operateFile(flags, ALINKGATEWAYFILE, json, strlen(json) + 1);
            free(json);
        }
        cJSON_Delete(root);
    }
    else
    {
        char buf[256] = {0};
        res = operateFile(flags, ALINKGATEWAYFILE, buf, sizeof(buf));
        if (res < 0)
        {
            EXAMPLE_TRACE("operateFile:%s read fail\n", ALINKGATEWAYFILE);
        }
        else if (res == 0)
        {
        }
        else
        {
            cJSON *root = cJSON_Parse(buf);
            if (root == NULL)
                return -1;
            cJSON *device_name_json = cJSON_GetObjectItem(root, "devicce_name");
            cJSON *device_secret_json = cJSON_GetObjectItem(root, "device_secret");
            if (device_secret_json == NULL)
            {
                res = -1;
            }
            else
            {
                if (device_name_json)
                    strcpy(device_name, device_name_json->valuestring);
                strcpy(device_secret, device_secret_json->valuestring);
            }
            cJSON_Delete(root);
        }
    }
    return res;
}
size_t get_write_cb(void *ptr, size_t size, size_t nmemb, void *stream)
{
    printf("get_write_cb size:%u,nmemb:%u\n", size, nmemb);
    printf("get_write_cb data:%s\n", (char *)ptr);
    iotx_linkkit_dev_meta_info_t *master_meta_info = (iotx_linkkit_dev_meta_info_t *)stream;

    cJSON *root = cJSON_Parse(ptr);
    if (root == NULL)
        return -1;
    cJSON *deviceName = cJSON_GetObjectItem(root, "deviceName");
    cJSON *deviceSecret = cJSON_GetObjectItem(root, "deviceSecret");

    strcpy(master_meta_info->device_name, deviceName->valuestring);
    strcpy(master_meta_info->device_secret, deviceSecret->valuestring);
    operate_gateway_info(master_meta_info->device_name, master_meta_info->device_secret, 1);

    cJSON_Delete(root);
    return size * nmemb;
}
int curl_http_get_gateway_info(iotx_linkkit_dev_meta_info_t *master_meta_info)
{
    char http_request_url[180] = {0};
    char gateway_mac[18] = {0};
    if (getNetworkMac(ETH_NAME, gateway_mac, sizeof(gateway_mac), "") == NULL)
    {
        fprintf(stderr, "get mac error\n");
        exit(1);
    }
    sprintf(http_request_url, "http://www.honyarcloud.com:8090/device/serial/code/query/?serial_code=W0011801&identity=%s&product_key=a1f0jNVDEPL&project_type=AL-Z", gateway_mac);
    printf("get request url:%s\n", http_request_url);
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    // get a curl handle
    CURL *curl = curl_easy_init();
    if (curl)
    {
        // set the URL with GET request
        curl_easy_setopt(curl, CURLOPT_URL, http_request_url);

        // write response msg into strResponse
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, master_meta_info);

        // perform the request, res will get the return code
        res = curl_easy_perform(curl);
        // check for errors
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else
        {
            fprintf(stderr, "curl_easy_perform() success.\n");
        }

        // always cleanup
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}
static void main_before_init(iotx_linkkit_dev_meta_info_t *master_meta_info)
{
    registerSystemCb(main_close, SYSTEM_CLOSE);

    cloudLinkOpen();
    CloudLinkDev *cloudLinkDev = (CloudLinkDev *)addProfileDev(PROFILE_PATH, STR_GATEWAY_DEVID, STR_GATEWAY_MODELID, cloudLinkParseJson);
    if (cloudLinkDev == NULL)
    {
        EXAMPLE_TRACE("gw cloudLinkDev is NULL");
        return;
    }

    strcpy(master_meta_info->product_key, cloudLinkDev->alinkInfo.product_key);
    strcpy(master_meta_info->product_secret, cloudLinkDev->alinkInfo.product_secret);
    strcpy(master_meta_info->device_secret, cloudLinkDev->alinkInfo.device_secret);
    if (strlen(master_meta_info->device_name) == 0)
    {
        getNetworkMac(ETH_NAME, master_meta_info->device_name, sizeof(master_meta_info->device_name), "");
        // for (int i = 0; i < strlen(g_device_name); i++)
        //     g_device_name[i] = toupper(g_device_name[i]);
    }

    if (strlen(master_meta_info->device_secret) == 0)
    {
        if (operate_gateway_info(master_meta_info->device_name, master_meta_info->device_secret, 0) < 0)
        {
            EXAMPLE_TRACE("local read gateway info fail!!!!!,start cloud read \n");
#if 0
            curl_http_get_gateway_info(master_meta_info);
#else
            iotx_dev_meta_info_t meta;
            memset(&meta, 0, sizeof(iotx_dev_meta_info_t));
            memcpy(meta.product_key, master_meta_info->product_key, strlen(master_meta_info->product_key));
            memcpy(meta.product_secret, master_meta_info->product_secret, strlen(master_meta_info->product_secret));
            memcpy(meta.device_name, master_meta_info->device_name, strlen(master_meta_info->device_name));
            EXAMPLE_TRACE("IOT_Dynamic_Register start\n");
            int res = IOT_Dynamic_Register(IOTX_HTTP_REGION_SHANGHAI, &meta);
            if (res < 0)
            {
                EXAMPLE_TRACE("IOT_Dynamic_Register:%d fail\n", res);
                exit(1);
            }
            else
            {
                strcpy(master_meta_info->device_secret, meta.device_secret);
                operate_gateway_info(master_meta_info->device_name, master_meta_info->device_secret, 1);
            }
#endif
        }
    }
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
    hylinkOpen();
    // HAL_MutexLock();
    // HAL_MutexUnlock();
}

static int max_running_seconds = 0;
int main(int argc, char **argv)
{
    int res = 0;
    uint64_t time_prev_sec = 0, time_now_sec = 0, time_begin_sec = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    iotx_linkkit_dev_meta_info_t master_meta_info = {0};
reconnect:
    memset(user_example_ctx, 0, sizeof(user_example_ctx_t));
    main_before_init(&master_meta_info);

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

    // memset(&master_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
    // memcpy(master_meta_info.product_key, g_product_key, strlen(g_product_key));
    // memcpy(master_meta_info.product_secret, g_product_secret, strlen(g_product_secret));
    // memcpy(master_meta_info.device_name, g_device_name, strlen(g_device_name));
    // memcpy(master_meta_info.device_secret, g_device_secret, strlen(g_device_secret));

    EXAMPLE_TRACE("login product_key:%s\n", master_meta_info.product_key);
    EXAMPLE_TRACE("login product_secret:%s\n", master_meta_info.product_secret);
    EXAMPLE_TRACE("login device_name:%s\n", master_meta_info.device_name);
    EXAMPLE_TRACE("login device_secret:%s\n", master_meta_info.device_secret);
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
    int reconnect_num = 0;
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
        if (++reconnect_num > 3)
        {
            EXAMPLE_TRACE("IOT_Linkkit_Connect reconnect...\n");
            goto reconnect;
        }
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
