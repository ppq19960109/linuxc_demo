#include "cloudLinkListFunc.h"

static int user_initialized(const int devid)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    EXAMPLE_TRACE("user_initialized, Devid: %d", devid);

    if (user_example_ctx->master_devid == devid)
    {
        user_example_ctx->master_initialized = 1;
    }
    user_example_ctx->subdev_index++;
    if (user_example_ctx->subdev_index > 0)
    {
        // EXAMPLE_TRACE("IOT_Linkkit_Query:%d", user_example_ctx->master_devid);
        // IOT_Linkkit_Query(user_example_ctx->master_devid, ITM_MSG_QUERY_TOPOLIST, NULL, 0);
    }
    return 0;
}

static int user_connected_event_handler(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("user_connected_event_handler subdev_index:%d", user_example_ctx->subdev_index);

    user_example_ctx->cloud_connected = 1;
    if (user_example_ctx->subdev_index > 0)
    {
        runTransferCb(NULL, SUBDEV_ONLINE, TRANSFER_SUBDEV_LINE);
    }
    runCmdCb((void *)1, LED_DRIVER_LINE);
    return 0;
}

static int user_connect_fail_event_handler(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("user_connect_fail_event_handler");

    user_example_ctx->cloud_connected = 0;
    runCmdCb((void *)0, LED_DRIVER_LINE);
    return 0;
}

static int user_disconnected_event_handler(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("user_disconnected_event_handler");

    user_example_ctx->cloud_connected = 0;
    runCmdCb((void *)0, LED_DRIVER_LINE);
    return 0;
}

static int user_dynamic_device_secret(const char *device_secret)
{
    EXAMPLE_TRACE("device_secret:%s", device_secret);

    return 0;
}

void linkkit_sdk_status_register(void)
{
    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);
    IOT_RegisterCallback(ITE_CONNECT_FAIL, user_connect_fail_event_handler);

    IOT_RegisterCallback(ITE_DISCONNECTED, user_disconnected_event_handler);
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, user_initialized);

    IOT_RegisterCallback(ITE_DYNREG_DEVICE_SECRET, user_dynamic_device_secret);
}
