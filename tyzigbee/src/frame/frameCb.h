#ifndef _FRAMECB_H_
#define _FRAMECB_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        SYSTEM_CLOSE = 0,
        SYSTEM_RESET,
        HYLINK_CLOSE,
        HYLINK_RESET,
        LAN_OPEN,
        LAN_CLOSE,
        ZIGBEE_CLOSE,
        ZIGBEE_RESET,
        HYLINK_DEVSINFO,
        HYLINK_ZB_CHANNEL,
        SYSTEM_HEARTBEAT,
        SYSTEM_LAST,
    } SystemStatus;
    typedef int (*systemCb)(void);
    void registerSystemCb(systemCb cb, SystemStatus status);
    int runSystemCb(SystemStatus status);

    typedef enum
    {
        TRANSFER_CLIENT_WRITE = 0,
        TRANSFER_CLIENT_READ,
        TRANSFER_LAST,
    } TransferStatus;
    typedef int (*transferCb)(void *, unsigned int);
    void registerTransferCb(transferCb cb, TransferStatus status);
    int runTransferCb(void *, unsigned int, TransferStatus status);

    typedef enum
    {
        CMD_NETWORK_ACCESS = 0,
        CMD_HYLINK_NETWORK_ACCESS,
        CMD_DELETE_DEV,
        CMD_ZCL_FRAME_REPORT,
        CMD_LAST,
    } CmdStatus;
    typedef int (*CmdCb)(void *);
    void registerCmdCb(CmdCb cb, CmdStatus status);
    int runCmdCb(void *, CmdStatus status);

    typedef enum
    {
        ZIGBEE_DEV_JOIN = 0,
        ZIGBEE_DEV_ONLINE,
        ZIGBEE_DEV_LEAVE,
        ZIGBEE_DEV_REPORT,
        ZIGBEE_DEV_DISPATCH,
        ZIGBEE_LAST,
    } ZigbeeStatus;
    typedef int (*ZigbeeCb)(void *, void *, void *, void *);
    void registerZigbeeCb(ZigbeeCb cb, ZigbeeStatus status);
    int runZigbeeCb(void *, void *, void *, void *, ZigbeeStatus status);
#ifdef __cplusplus
}
#endif
#endif