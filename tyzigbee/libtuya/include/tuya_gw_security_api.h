/**
 * @file tuya_gw_security_api.h
 * @author lwl@tuya.com
 * @brief 
 * @version 0.1
 * @date 2020-05-12
 * 
 * @copyright Copyright (c) tuya.inc 2020
 * 
 */
#ifndef TUYA_GW_SECURITY_API_H
#define TUYA_GW_SECURITY_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*home_security_alarm_cb)(void);
    void (*home_security_alarm_cancel_cb)(void);
    void (*home_security_disarmed_cb)(void);
    void (*home_security_away_armed_cb)(void);
    void (*home_security_stay_armed_cb)(void);
    void (*home_security_arm_ignore_cb)(void);
    void (*home_security_arm_countdown_cb)(uint32_t time);
    void (*home_security_alarm_countdown_cb)(uint32_t time);
    void (*home_security_door_opened_cb)(void);
    void (*home_security_alarm_dev_cb)(char *dev);
} ty_home_security_cbs_s;

/**
 * @brief    register misc device management callbacks.
 *
 * @param cbs.  a set of callbacks.
 * 
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_reg_home_security_cb(ty_home_security_cbs_s *cbs);

#ifdef __cplusplus
}
#endif
#endif // TUYA_GW_SECURITY_API_H
