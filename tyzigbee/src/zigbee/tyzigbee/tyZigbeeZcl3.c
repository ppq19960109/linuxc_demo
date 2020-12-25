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

#include "frameCb.h"

int getZcl3DataType(unsigned char dataType, unsigned char *data, int *realShift)
{
    *realShift = 0;
    int byteLen = 0;
    switch (dataType)
    {
    case ZCL_DATA_TYPE_NO_DATA:
    case ZCL_DATA_TYPE_UNKNOW:
        byteLen = 0;
        break;
    case ZCL_DATA_TYPE_Boolean:
    case ZCL_DATA_TYPE_8_BIT_DATA:
    case ZCL_DATA_TYPE_8_BIT_BITMAP:
    case ZCL_DATA_TYPE_UINT_8:
    case ZCL_DATA_TYPE_INT_8:
    case ZCL_DATA_TYPE_8_BIT_ENUM:
        byteLen = 1;
        break;
    case ZCL_DATA_TYPE_16_BIT_DATA:
    case ZCL_DATA_TYPE_16_BIT_BITMAP:
    case ZCL_DATA_TYPE_UINT_16:
    case ZCL_DATA_TYPE_INT_16:
    case ZCL_DATA_TYPE_16_BIT_ENUM:
    case ZCL_DATA_TYPE_SEMI_PRECISION:
    case ZCL_DATA_TYPE_CLUSTER_ID:
    case ZCL_DATA_TYPE_ATTRIBUTE_ID:
        byteLen = 2;
        break;
    case ZCL_DATA_TYPE_24_BIT_DATA:
    case ZCL_DATA_TYPE_24_BIT_BITMAP:
    case ZCL_DATA_TYPE_UINT_24:
    case ZCL_DATA_TYPE_INT_24:
        byteLen = 3;
        break;
    case ZCL_DATA_TYPE_32_BIT_DATA:
    case ZCL_DATA_TYPE_32_BIT_BITMAP:
    case ZCL_DATA_TYPE_UINT_32:
    case ZCL_DATA_TYPE_INT_32:
    case ZCL_DATA_TYPE_SINGLE_PRECISION:
    case ZCL_DATA_TYPE_TIME_OF_DATE:
    case ZCL_DATA_TYPE_DATE:
    case ZCL_DATA_TYPE_UCT_TIME:
    case ZCL_DATA_TYPE_BAC_NET_OID:
        byteLen = 4;
        break;
    case ZCL_DATA_TYPE_40_BIT_DATA:
    case ZCL_DATA_TYPE_40_BIT_BITMAP:
    case ZCL_DATA_TYPE_UINT_40:
    case ZCL_DATA_TYPE_INT_40:
        byteLen = 5;
        break;
    case ZCL_DATA_TYPE_48_BIT_DATA:
    case ZCL_DATA_TYPE_48_BIT_BITMAP:
    case ZCL_DATA_TYPE_UINT_48:
    case ZCL_DATA_TYPE_INT_48:
        byteLen = 6;
        break;
    case ZCL_DATA_TYPE_56_BIT_DATA:
    case ZCL_DATA_TYPE_56_BIT_BITMAP:
    case ZCL_DATA_TYPE_UINT_56:
    case ZCL_DATA_TYPE_INT_56:
        byteLen = 7;
        break;
    case ZCL_DATA_TYPE_64_BIT_DATA:
    case ZCL_DATA_TYPE_64_BIT_BITMAP:
    case ZCL_DATA_TYPE_UINT_64:
    case ZCL_DATA_TYPE_INT_64:
    case ZCL_DATA_TYPE_DOUBLE_PRECISION:
    case ZCL_DATA_TYPE_IEEE_ADDRESS:
        byteLen = 8;
        break;
    case ZCL_DATA_TYPE_128_BIT_SECURITY_KEY:
        byteLen = 16;
        break;
    case ZCL_DATA_TYPE_OCTET_STRING:
    case ZCL_DATA_TYPE_CHAR_STRING:
        byteLen = *data;
        *realShift = 1;
        break;
    case ZCL_DATA_TYPE_LONG_OCTET_STRING:
    case ZCL_DATA_TYPE_LONG_CHAR_STRING:
        byteLen = data[0] + (data[1] << 8);
        *realShift = 2;
        break;
    default:
        printf("data type not exits\n");
        break;
    }
    return byteLen;
}

static int _dev_obj_cmd_cb(ty_obj_cmd_s *dp)
{
    int i = 0;

    log_notice("device obj cmd callback");
    log_notice("cmd_tp: %d, dtt_tp: %d, dps_cnt: %u", dp->cmd_tp, dp->dtt_tp, dp->dps_cnt);

    for (i = 0; i < dp->dps_cnt; i++)
    {
        log_notice("dpid: %d", dp->dps[i].dpid);
        switch (dp->dps[i].type)
        {
        case DP_TYPE_BOOL:
            log_notice("dp_bool value: %d", dp->dps[i].value.dp_bool);
            break;
        case DP_TYPE_VALUE:
            log_notice("dp_value value: %d", dp->dps[i].value.dp_value);
            break;
        case DP_TYPE_ENUM:
            log_notice("dp_enum value: %d", dp->dps[i].value.dp_enum);
            break;
        case DP_TYPE_STR:
            log_notice("dp_str value: %s", dp->dps[i].value.dp_str);
            break;
        }
    }
    /* USER TODO */

    if (dp->cid != NULL)
    {
    }

    return 0;
}

static int _dev_raw_cmd_cb(ty_raw_cmd_s *dp)
{
    int i = 0;

    log_notice("device raw cmd callback");
    log_notice("cmd_tp: %d, dtt_tp: %d, dpid: %d, len: %u", dp->cmd_tp, dp->dtt_tp, dp->dpid, dp->len);

    log_notice("data: ");
    for (i = 0; i < dp->len; i++)
        printf("%02x ", dp->data[i]);

    printf("\n");
    /* USER TODO */

    return 0;
}

static int _z3_dev_active_state_changed_cb(const char *id, int state)
{
    log_notice("device active state changed callback");
    log_notice("id: %s, state: %d", id, state);
    /* USER TODO */

    return 0;
}

static int _z3_dev_init_data_cb(void)
{
    int ret = 0;
    log_notice("_z3_dev_init_data_cb");
    return ret;
}

static int _z3_dev_join_cb(ty_z3_desc_s *desc)
{
    int ret = 0;

    log_notice("device join callback");
    log_notice("           id: %s", desc->id);
    log_notice("      node_id: 0x%04x", desc->node_id);
    log_notice("    manu_name: %s", desc->manu_name);
    log_notice("     model_id: %s", desc->model_id);
    log_notice("  rejoin_flag: %d", desc->rejoin_flag);
    log_notice(" power_source: %d", desc->power_source);
    log_notice("      version: 0x%02x", desc->version);
    /* USER TODO */
    char ver_str[16] = {0};

    snprintf(ver_str, sizeof(ver_str), "%d.%d.%d",
             (desc->version >> 6) & 0x03, (desc->version >> 4) & 0x03, desc->version & 0x0f);
    log_notice("version: %s", ver_str);

    runZigbeeCb((void *)desc->id, desc->model_id, ver_str, desc->manu_name, ZIGBEE_DEV_JOIN);
    return ret;
}

static int _z3_dev_leave_cb(const char *id)
{
    log_notice("device leave callback id:%s", id);
    /* USER TODO */
    runZigbeeCb((void *)id, NULL, NULL, NULL, ZIGBEE_DEV_LEAVE);
    return 0;
}

static int _z3_dev_zcl_report_cb(ty_z3_aps_frame_s *frame)
{
    int i = 0, ret = 0;

    log_notice("device zcl report callback");
    log_notice("        id: %s", frame->id);
    log_notice("profile_id: 0x%04x", frame->profile_id);
    log_notice("cluster_id: 0x%04x", frame->cluster_id);
    log_notice("   node_id: 0x%04x", frame->node_id);
    log_notice("    src_ep: %d", frame->src_endpoint);
    log_notice("    dst_ep: %d", frame->dst_endpoint);
    log_notice("  group_id: %d", frame->group_id);
    log_notice("  cmd_type: %d", frame->cmd_type);
    log_notice("   command: 0x%02x", frame->cmd_id);
    log_notice("frame_type: %d", frame->frame_type);
    log_notice("   msg_len: %d", frame->msg_length);
    log_notice("       msg: ");
    for (i = 0; i < frame->msg_length; i++)
        printf("%02x ", frame->message[i]);
    printf("\n");
    /* USER TODO */
    runCmdCb(frame, CMD_DEV_REPORT);
    return ret;
}

static int _z3_dev_online_fresh_cb(const char *id, uint8_t version)
{
    char ver_str[16] = {0};

    log_notice("device online callback id:%s", id);

    snprintf(ver_str, sizeof(ver_str), "%d.%d.%d",
             (version >> 6) & 0x03, (version >> 4) & 0x03, version & 0x0f);

    log_notice("version: %s,version:%d", ver_str, version);
    runZigbeeCb((void *)id, NULL, ver_str, NULL, ZIGBEE_DEV_ONLINE);
    return 0;
}

int tyZigbeeZcl3Init(void)
{
    int ret = 0;

    ty_dev_cmd_cbs_s dev_cmd_cbs = {
        .dev_obj_cmd_cb = _dev_obj_cmd_cb,
        .dev_raw_cmd_cb = _dev_raw_cmd_cb,
    };

    ty_z3_dev_cbs_s z3_dev_cbs = {
        .z3_dev_active_state_changed_cb = _z3_dev_active_state_changed_cb,
        .z3_dev_init_data_cb = _z3_dev_init_data_cb,
        .z3_dev_join_cb = _z3_dev_join_cb,
        .z3_dev_leave_cb = _z3_dev_leave_cb,
        .z3_dev_zcl_report_cb = _z3_dev_zcl_report_cb,
        .z3_dev_online_fresh_cb = _z3_dev_online_fresh_cb,
    };

    ret = tuya_user_iot_reg_dev_cmd_cb(&dev_cmd_cbs);
    if (ret != 0)
    {
        log_err("tuya_user_iot_reg_dev_cmd_cb failed, ret: %d", ret);
        return ret;
    }

    ret = tuya_user_iot_reg_z3_dev_cb(&z3_dev_cbs);
    if (ret != 0)
    {
        log_err("tuya_user_iot_reg_z3_dev_cb failed, ret: %d", ret);
        return ret;
    }

    return ret;
}
