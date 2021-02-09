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
        LED_DRIVER_TIMER_FILP,
        RK_DRIVER_CLOSE,
        DATABASE_CLOSE,
        DATABASE_RESET,
        CMD_DEVSINFO,
        SYSTEM_LAST,
    } SystemStatus;
    typedef int (*systemCb)(void);
    void registerSystemCb(systemCb cb, SystemStatus status);
    int runSystemCb(SystemStatus status);

    typedef enum
    {
        TRANSFER_CLIENT_WRITE = 0,
        TRANSFER_CLIENT_READ,
        TRANSFER_SERVER_HYLINK_WRITE,
        TRANSFER_SERVER_HYLINK_READ,
        TRANSFER_SERVER_ZIGBEE_WRITE,
        TRANSFER_SERVER_ZIGBEE_READ,
        TRANSFER_LAST,
    } TransferStatus;
    typedef int (*transferCb)(void *, unsigned int);
    void registerTransferCb(transferCb cb, TransferStatus status);
    int runTransferCb(void *, unsigned int, TransferStatus status);

    typedef enum
    {
        CMD_NETWORK_ACCESS = 0,
        CMD_NETWORK_ACCESS_TIME,
        LED_DRIVER_LINE,
        CMD_LAST,
    } CmdStatus;
    typedef int (*CmdCb)(void *);
    void registerCmdCb(CmdCb cb, CmdStatus status);
    int runCmdCb(void *, CmdStatus status);

#ifdef __cplusplus
}
#endif
#endif