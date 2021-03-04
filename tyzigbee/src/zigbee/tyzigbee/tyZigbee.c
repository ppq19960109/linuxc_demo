#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "tuya_gw_infra_api.h"
#include "tuya_gw_z3_api.h"
#include "tuya_gw_dp_api.h"

#include "tyZigbee.h"
#include "tyZigbeeZcl3.h"

/* must apply for uuid, authkey and product key from tuya iot develop platform */
#define UUID "003tuyatestf7f149185"
#define AUTHKEY "NeA8Wc7srpAZHEMuru867oblOLN2QCC1"
#define PRODUCT_KEY "GXxoKf27eVjA7x1c"

int getZcl3ProvateDataType(unsigned char dataType)
{
    int byteLen = 0;
    switch (dataType)
    {
    case ZCL_PRIVATE_DATA_TYPE_RAW:
    case ZCL_PRIVATE_DATA_TYPE_STRING:
        byteLen = 0xff;
        break;
    case ZCL_PRIVATE_DATA_TYPE_BOOL:
    case ZCL_PRIVATE_DATA_TYPE_ENUM:
        byteLen = 1;
        break;
    case ZCL_PRIVATE_DATA_TYPE_VALUE:
        byteLen = 4;
        break;
    default:
        byteLen = -1;
        break;
    }
    return byteLen;
}
static int _iot_get_uuid_authkey_cb(char *uuid, int uuid_size, char *authkey, int authkey_size)
{
    strncpy(uuid, UUID, uuid_size);
    strncpy(authkey, AUTHKEY, authkey_size);

    return 0;
}

static int _iot_get_product_key_cb(char *pk, int pk_size)
{
    strncpy(pk, PRODUCT_KEY, pk_size);

    return 0;
}

static int _gw_configure_op_mode_cb(ty_op_mode_t mode)
{
    log_debug("gw configure operation mode callback");

    /* USER TODO */
    switch (mode)
    {
    case TY_OP_MODE_ADD_START:
        log_debug("add device start");
        break;
    case TY_OP_MODE_ADD_STOP:
        log_debug("add device stop");
        break;
    default:
        break;
    }

    return 0;
}

static int _gw_active_status_changed_cb(ty_gw_status_t status)
{
    log_debug("active status changed, status: %d", status);

    return 0;
}

static int _gw_online_status_changed_cb(bool registered, bool online)
{
    log_debug("online status changed, registerd: %d, online: %d", registered, online);

    return 0;
}

static int _gw_fetch_local_log_cb(char *path, int path_len)
{
    // char cmd[256] = {0};

    log_debug("gw fetch local log callback");
    /* USER TODO */

    /*
    snprintf(file, len, "/tmp/log.tgz");

    snprintf(cmd, sizeof(cmd), "tar -zvcf %s --absolute-names /tmp/tuya.log", file);
    system(cmd);
    */

    return 0;
}

static void _gw_reboot_cb(void)
{
    log_debug("gw reboot callback");
    /* USER TODO */

    return;
}

static void _gw_reset_cb(void)
{
    log_debug("gw reset callback");
    /* USER TODO */

    return;
}

static int _gw_upgrade_cb(const char *img)
{
    log_debug("gw upgrade callback");
    /* USER TODO */

    return 0;
}
//-------------------------------------------
#include "frameCb.h"
#define STORAGE_PATH "storage"
static int zigbeeReset(void)
{
    system("rm -rf " STORAGE_PATH);
    system("rm -rf *.db*");
    system("rm -rf *.txt");
    return 0;
}

int tyZigbeeInit(void)
{
    int ret = 0;
    registerSystemCb(zigbeeReset, ZIGBEE_RESET);
    ty_gw_attr_s gw_attr = {
        .storage_path = STORAGE_PATH,
        .cache_path = "/tmp/",
        .tty_device = "/dev/ttyS1",
        .tty_baudrate = 115200,
        .eth_ifname = "eth0",
        .ver = "1.0.0",
        .uz_cfg = "devices.json",
        .log_level = TY_LOG_NOTICE};

    ty_gw_infra_cbs_s gw_infra_cbs = {
        .get_uuid_authkey_cb = _iot_get_uuid_authkey_cb,
        .get_product_key_cb = _iot_get_product_key_cb,
        .gw_fetch_local_log_cb = _gw_fetch_local_log_cb,
        .gw_configure_op_mode_cb = _gw_configure_op_mode_cb,
        .gw_reboot_cb = _gw_reboot_cb,
        .gw_reset_cb = _gw_reset_cb,
        .gw_upgrade_cb = _gw_upgrade_cb,
        .gw_active_status_changed_cb = _gw_active_status_changed_cb,
        .gw_online_status_changed_cb = _gw_online_status_changed_cb,
    };

    ret = tyZigbeeZcl3Init();
    if (ret != 0)
    {
        log_err("tyZigbeeZcl3Init failed");
        exit(1);
        return ret;
    }
    ret = tuya_user_iot_init(&gw_attr, &gw_infra_cbs);
    if (ret != 0)
    {
        log_err("tuya_user_iot_init failed");
        exit(1);
        return ret;
    }
    ret = tuya_user_iot_unactive_gw();
    if (ret != 0)
    {
        log_err("tuya_user_iot_unactive_gw failed");
        return ret;
    }
    return ret;
}
