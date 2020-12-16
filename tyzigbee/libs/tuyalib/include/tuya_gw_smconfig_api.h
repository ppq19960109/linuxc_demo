/**
 * @file tuya_gw_smconfig_api.h
 * @author lwl@tuya.com
 * @brief 
 * @version 0.1
 * @date 2020-07-29
 * 
 * @copyright Copyright (c) tuya.inc 2020
 * 
 */
#ifndef TUYA_GW_SMCONFIG_API_H
#define TUYA_GW_SMCONFIG_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief    initiate smart config.
 *
 * @param ifname. WiFi interface name.
 * @param cb.     callback to get ssid and passwd for smart config.
 *
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_smconfig_init(const char *ifname,
                                int (*cb)(char *ssid, \
                                uint32_t ssid_size, \
                                char *passwd, \
                                uint32_t passwd_size));

/**
 * @brief    start smart config.
 *
 * @param timeout. timeout to stop smart config.
 *
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_smconfig_start(uint32_t timeout);

/**
 * @brief    stop smart config.
 *
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_smconfig_stop(void);

#ifdef __cplusplus
}
#endif
#endif // TUYA_GW_SMCONFIG_API_H