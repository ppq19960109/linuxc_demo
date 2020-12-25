#ifndef _TYZIGBEE_H_
#define _TYZIGBEE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define Z3_PROFILE_ID_HA 0x0104

#define Z3_CMD_TYPE_GLOBAL 0x01
#define Z3_CMD_TYPE_PRIVATE 0x02
#define Z3_CMD_TYPE_ZDO 0x03

    enum TyFrameType
    {
        Z3_FRAME_TYPE_UNICAST = 0,
        Z3_FRAME_TYPE_MULTICAST = 1,
        Z3_FRAME_TYPE_BROADCAST = 2,
    };

#define ZCL_PRIVATE_CLUSTER 0xef00

    enum ZCL_PRIVATE_CLUSTER_CMOOAND_ID
    {
        TY_DATA_REQUEST = 0x00,         // 网关端数据请求
        TY_DATA_RESPONE = 0x01,         // MCU 侧数据请求的回复
        TY_DATA_REPORT = 0x02,          // MCU 侧数据主动上报（双向）
        TY_DATA_QUERY = 0x03,           // GW 下发，触发 MCU侧把当前的信息全部上报，没有 zcl payload.注：设备端可以做个策略，数 据最好不要集中上报
        TY_DATA_MODULE_RSP = 0x05,
        TY_DATA_MODULE = 0x06,

        TUYA_MCU_VERSION_REQ = 0x10,    //Gw->Zigbee 网关查询 mcu 版本
        TUYA_MCU_VERSION_RSP = 0x11,    //Zigbee->Gw mcu返回版本或主动上报版本
        TUYA_MCU_OTA_NOTIFY = 0x12,     // Gw->Zigbee 网关通知 mcu 升级
        TUYA_OTA_BLOCK_DATA_REQ = 0x13, //Zigbee->Gw 请求mcu 的升级包
        TUYA_OTA_BLOCK_DATA_RSP = 0x14, // Gw->Zigbee 网关返回请求的升级包
        TUYA_MCU_OTA_RESULT = 0x15,     //Zigbee->Gw 返回muc 的升级结果
        TUYA_MCU_SYNC_TIME = 0x24,      //时间同步（双向）
    };
    enum ZCL_PRIVATE_DATA_TYPE
    {
        ZCL_PRIVATE_DATA_TYPE_RAW = 0x00,
        ZCL_PRIVATE_DATA_TYPE_BOOL = 0x01,
        ZCL_PRIVATE_DATA_TYPE_VALUE = 0x02,
        ZCL_PRIVATE_DATA_TYPE_STRING = 0x03,
        ZCL_PRIVATE_DATA_TYPE_ENUM = 0x04,
        ZCL_PRIVATE_DATA_TYPE_BITMAP = 0x05,
    };
    int tyZigbeeInit(void);
    int getZcl3ProvateDataType(unsigned char dataType);
#ifdef __cplusplus
}
#endif
#endif