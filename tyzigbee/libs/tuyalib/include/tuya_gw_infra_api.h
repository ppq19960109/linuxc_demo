/**
 * @file tuya_gw_infra_api.h
 * @author lwl@tuya.com
 * @brief 
 * @version 0.1
 * @date 2020-03-20
 * 
 * @copyright Copyright (c) tuya.inc 2020
 * 
 */
#ifndef TUYA_GW_INFRA_API_H
#define TUYA_GW_INFRA_API_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void log_base(const char *file, 
    const char *func, 
    int line, 
    int level, 
    const char *fmt, ...);

typedef enum {
    TY_LOG_ERR     = 0,
    TY_LOG_WARN    = 1,
    TY_LOG_NOTICE  = 2,
    TY_LOG_INFO    = 3,
    TY_LOG_DEBUG   = 4,
    TY_LOG_TRACE   = 5
} ty_log_lovel_t;

typedef enum {
	TY_CONN_MODE_AP_ONLY      = 0,
	TY_CONN_MODE_EZ_ONLY      = 1,
    TY_CONN_MODE_AP_FIRST     = 2,
    TY_CONN_MODE_EZ_FIRST     = 3,
} ty_conn_mode_t;

typedef enum {
    TY_GW_STATUS_UNREGISTERED = 0,
    TY_GW_STATUS_REGISTERED   = 1,
} ty_gw_status_t;

typedef enum {
    TY_ZIGBEE_STATUS_POWERUP  = 0,
    TY_ZIGBEE_STATUS_PAIRING  = 1,
    TY_ZIGBEE_STATUS_NORMAL   = 2,
} ty_zigbee_status_t;

typedef enum {
    TY_OP_MODE_ADD_START      = 0,
    TY_OP_MODE_ADD_STOP       = 1,
    TY_OP_MODE_AP             = 2,
    TY_OP_MODE_EZ             = 3,
} ty_op_mode_t;

typedef struct {
    char *storage_path;
    char *cache_path;
    char *tty_device;
    int tty_baudrate;
    char *eth_ifname;
    char *ssid;
    char *password;
    char *ver;
    int is_engr;
    char *uz_cfg;
    ty_conn_mode_t wifi_mode;
    ty_log_lovel_t log_level;
} ty_gw_attr_s;

typedef struct {
    int  (*get_uuid_authkey_cb)(char *uuid, int uuid_size, char *authkey, int authkey_size);
    int  (*get_product_key_cb)(char *pk, int pk_size);
    int  (*gw_upgrade_cb)(const char *img_file);
    void (*gw_reboot_cb)(void);
    void (*gw_reset_cb)(void);
    void (*gw_engineer_finished_cb)(void);
    int  (*gw_fetch_local_log_cb)(char *path, int path_len);
    int  (*gw_configure_op_mode_cb)(ty_op_mode_t mode);
    int  (*gw_zigbee_status_changed_cb)(ty_zigbee_status_t status);
    int  (*gw_active_status_changed_cb)(ty_gw_status_t status);
    int  (*gw_online_status_changed_cb)(bool registered, bool online);
} ty_gw_infra_cbs_s;

#define log_err(fmt, ...) \
    log_base(__FILE__, __func__, __LINE__, TY_LOG_ERR,    fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...) \
    log_base(__FILE__, __func__, __LINE__, TY_LOG_WARN,   fmt, ##__VA_ARGS__)
#define log_notice(fmt, ...) \
    log_base(__FILE__, __func__, __LINE__, TY_LOG_NOTICE, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) \
    log_base(__FILE__, __func__, __LINE__, TY_LOG_INFO,   fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...) \
    log_base(__FILE__, __func__, __LINE__, TY_LOG_DEBUG,  fmt, ##__VA_ARGS__)

/**
 * @brief    initiate sdk.
 *
 * @retval   0: success
 * @retval  !0: failure
 */
int tuya_user_iot_init(ty_gw_attr_s *attr, ty_gw_infra_cbs_s *cbs);

/**
 * @brief    unactive gateway.
 *
 * @retval   0: success
 * @retval  !0: failure
 */
int tuya_user_iot_unactive_gw(void);

/**
 * @brief    active gateway.
 *
 * @retval   0: success
 * @retval  !0: failure
 */
int tuya_user_iot_active_gw(const char *token);

/**
 * @brief    allow or disallow subdevice to join network.
 *
 * @param permit. 0: disallow, 1: allow.
 * 
 * @retval   0: success
 * @retval  !0: failure
 */
int tuya_user_iot_permit_join(bool permit);

/**
 * @brief    update device version.
 * 
 * @param dev_id.  device unique ID.
 * @param version. device version.
 * 
 * @retval   0: success
 * @retval  !0: failure
 */
int tuya_user_iot_update_version(const char *dev_id, const char *version);

/**
 * @brief    set time zone.
 * 
 * @param time_zone. an identifier for a time offset from UTC, such as "+08:00"
 * 
 * @retval   0: success
 * @retval  !0: failure
 */
int tuya_user_set_time_zone(const char *time_zone);

/**
 * @brief    update sdk inside time.
 * 
 * @param time. the time as the number of seconds since 1970-01-01 00:00:00 +0000 (UTC)
 * 
 * @retval   0: success
 * @retval  !0: failure
 */
int tuya_user_set_posix(uint32_t time);

/**
 * @brief    get sdk inside time.
 * 
 * @retval   the time as the number of seconds since 1970-01-01 00:00:00 +0000 (UTC)
 */
uint32_t tuya_user_get_posix(void);

#ifdef __cplusplus
}
#endif
#endif // TUYA_GW_INFRA_API_H
