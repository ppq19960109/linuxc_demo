/**
 * @file tuya_gw_z3_api.h
 * @author lwl@tuya.com
 * @brief
 * @version 0.1
 * @date 2020-03-20
 *
 * @copyright Copyright (c) tuya.inc 2020
 *
 */
#ifndef TUYA_GW_Z3_API_H
#define TUYA_GW_Z3_API_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define Z3_DEV_ID_LEN    32
#define ENDPOINT_MAX     10
#define CLUSTER_LIST_MAX 10
#define MANU_NAME_LEN    32
#define MODEL_ID_LEN     32

typedef struct {
    char     id[Z3_DEV_ID_LEN+1];
    uint16_t node_id;
    char     manu_name[MANU_NAME_LEN+1];
    char     model_id[MODEL_ID_LEN+1];
    char     rejoin_flag;
    char     power_source;
    uint8_t  version;
} ty_z3_desc_s;

typedef struct {
    char id[Z3_DEV_ID_LEN+1];
    uint16_t node_id;
    uint16_t profile_id;
    uint16_t cluster_id;
    uint8_t src_endpoint;
    uint8_t dst_endpoint;
    uint16_t group_id;
    uint8_t cmd_type;
    uint8_t cmd_id;
    uint8_t frame_type;
    char disable_ack;
    uint16_t msg_length;
    uint8_t *message;
} ty_z3_aps_frame_s;

typedef struct {
    int (*z3_dev_active_state_changed_cb)(const char *id, int state);
    int (*z3_dev_init_data_cb)(void);
    int (*z3_dev_join_cb)(ty_z3_desc_s *desc);
    int (*z3_dev_leave_cb)(const char *id);
    int (*z3_dev_zcl_report_cb)(ty_z3_aps_frame_s *frame);
    int (*z3_dev_online_fresh_cb)(const char *id, uint8_t version);
    int (*z3_dev_upgrade_status_cb)(const char *id, int rc, uint8_t version);
} ty_z3_dev_cbs_s;

/**
 * @brief    register ZigBee3.0 device management callbacks.
 *
 * @param cbs.  a set of callbacks.
 *
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_reg_z3_dev_cb(ty_z3_dev_cbs_s *cbs);

/**
 * @brief    bind z3 device to tuya cloud.
 *
 * @param uddd.   custom filed, the highest bit must be 1.
 * @param dev_id. device unique ID.
 * @param pid.    product ID.
 * @param ver.    device software version.
 *
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_z3_dev_bind(uint32_t uddd, const char *dev_id, const char *pid, const char *ver);

/**
 * @brief    unbind z3 device from tuya cloud.
 *
 * @param dev_id. device unique ID.
 *
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_z3_dev_unbind(const char *dev_id);

/**
 * @brief    send ZCL command to device.
 *
 * @param frame.  ZigBee3.0 ZCL message header and payload.
 *
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_z3_dev_send_zcl_cmd(ty_z3_aps_frame_s *frame);

/**
 * @brief    take device off ZigBee network.
 *
 * @param id.  unique ID for device.
 *
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_z3_dev_del(const char *id);

/**
 * @brief    upgrade device firmware.
 *
 * @param id.    unique ID for device.
 * @param img.   firmare file.
 *
 * @retval   0: sucess
 * @retval  !0: failure
 */
int tuya_user_iot_z3_dev_upgrade(const char *id, const char *img);

#ifdef __cplusplus
}
#endif
#endif // TUYA_GW_Z3_API_H