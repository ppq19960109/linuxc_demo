#ifndef _FRAMECB_H_
#define _FRAMECB_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        SYSTEM_OPEN = 0,
        SYSTEM_CLOSE = 1,
        SYSTEM_RESET = 2,
        HYLINK_OPEN,
        HYLINK_CLOSE,
        HYLINK_RESET,
        LAN_OPEN,
        LAN_CLOSE,
        LED_DRIVER_TIMER_OPEN,
        LED_DRIVER_TIMER_CLOSE,
        LED_DRIVER_TIMER_FILP,
        ZIGBEE_OPEN,
        ZIGBEE_CLOSE,
        ZIGBEE_RESET,
        SYSTEM_LAST,
    } SystemStatus;
    typedef int (*systemCb)(void);
    void registerSystemCb(systemCb cb, SystemStatus status);
    int runSystemCb(SystemStatus status);

    typedef enum
    {
        TRANSFER_HYLINK_WRITE = 0,
        TRANSFER_HYLINK_READ,
        TRANSFER_LAST,
    } TransferStatus;
    typedef int (*transferCb)(void *, unsigned int);
    void registerTransferCb(transferCb cb, TransferStatus status);
    int runTransferCb(void *, unsigned int, TransferStatus status);

    typedef enum
    {
        CMD_NETWORK_ACCESS = 0,
        CMD_DELETE_DEV,
        CMD_DEV_REPORT,
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