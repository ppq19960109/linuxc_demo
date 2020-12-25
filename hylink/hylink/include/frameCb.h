#ifndef _FRAMECB_H_
#define _FRAMECB_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define ATTR_REPORT_ALL 0xFF

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
        CMD_HEART,
        CMD_NETWORK,
        CMD_DEVSINFO,
        SYSTEM_LAST,
    } SystemStatus;
    typedef int (*systemCb)(void);
    void registerSystemCb(systemCb cb, SystemStatus status);
    int runSystemCb(SystemStatus status);

    typedef enum
    {
        SUBDEV_OFFLINE = 0, /* 设备下线 */
        SUBDEV_ONLINE = 1,  /* 设备上线 */
        SUBDEV_RESTORE,     /* 删除云端信息 */
        SUBDEV_LAST,        //
    } SubDevStatus;

    typedef enum
    {
        TRANSFER_CLIENT_WRITE = 0,
        TRANSFER_CLIENT_READ,
        TRANSFER_SERVER_HYLINK_WRITE,
        TRANSFER_SERVER_HYLINK_READ,
        TRANSFER_SUBDEV_LINE,
        TRANSFER_CLOUD_REPORT,
        TRANSFER_SCENE_REPORT,
        TRANSFER_DEVATTR,
        TRANSFER_LAST,
    } TransferStatus;
    typedef int (*transferCb)(void *, unsigned int);
    void registerTransferCb(transferCb cb, TransferStatus status);
    int runTransferCb(void *, unsigned int, TransferStatus status);

#ifdef __cplusplus
}
#endif
#endif