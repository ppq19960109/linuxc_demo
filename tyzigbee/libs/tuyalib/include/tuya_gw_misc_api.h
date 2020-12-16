/**
 * @file tuya_gw_misc_api.h
 * @author lwl@tuya.com
 * @brief 
 * @version 0.1
 * @date 2020-03-20
 * 
 * @copyright Copyright (c) tuya.inc 2020
 * 
 */
#ifndef TUYA_GW_MISC_API_H
#define TUYA_GW_MISC_API_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int (*misc_dev_add_cb)(bool permit, uint32_t timeout);
	int (*misc_dev_del_cb)(const char *dev_id);
	int (*misc_dev_bind_ifm_cb)(const char *dev_id, int result);
	int (*misc_dev_upgrade_cb)(const char *dev_id, const char *img);
	int (*misc_dev_reset_cb)(const char *dev_id);
} ty_misc_dev_cbs_s;

/**
 * @brief    register misc device management callbacks.
 *
 * @param cbs.  a set of callbacks.
 * 
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_reg_misc_dev_cb(ty_misc_dev_cbs_s *cbs);

/**
 * @brief    bind misc device to tuya cloud.
 *
 * @param uddd.   custom filed, the highest bit must be 1.
 * @param dev_id. device unique ID.
 * @param pid.    product ID.
 * @param ver.    device software version.
 * 
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_misc_dev_bind(uint32_t uddd, const char *dev_id, const char *pid, const char *ver);

/**
 * @brief    unbind misc device from tuya cloud.
 *
 * @param dev_id. device unique ID.
 * 
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_misc_dev_unbind(const char *dev_id);

/**
 * @brief    fresh misc device online status.
 *
 * @param dev_id.  device unique ID.
 * @param timeout. offline timeout, uint: seconds.
 * 
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_misc_dev_fresh_hb(const char *dev_id, uint32_t timeout);

#ifdef __cplusplus
}
#endif
#endif // TUYA_GW_MISC_API_H