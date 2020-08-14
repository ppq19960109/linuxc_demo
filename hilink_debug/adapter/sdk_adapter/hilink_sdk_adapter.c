/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HiLink SDK适配实现源文件
 * Create: 2019-04-20
 * Notes: 该文件中的接口需要对外提供给第三方厂商使用，为了前向兼容，部分老接口暂不按最新编码规范整改.
 */
#include "hilink_sdk_adapter.h"
#include "hilink_softap_adapter.h"

#include "protocol_cover.h"
#include "hilink_cover.h"
/*
 * 通知设备的状态
 * status表示设备当前的状态
 * 注意，此函数由设备厂商根据产品业务选择性实现
 */
void hilink_notify_devstatus(int status)
{
    switch (status)
    {
    case HILINK_M2M_CLOUD_OFFLINE:
        log_info("HILINK_M2M_CLOUD_OFFLINE\n");
        /* 设备与云端连接断开，请在此处添加实现 */
        break;
    case HILINK_M2M_CLOUD_ONLINE:
        log_info("HILINK_M2M_CLOUD_ONLINE\n");
        /* 设备连接云端成功，请在此处添加实现 */
        break;
    case HILINK_M2M_LONG_OFFLINE:
        log_info("HILINK_M2M_LONG_OFFLINE\n");
        /* 设备与云端连接长时间断开，请在此处添加实现 */
        break;
    case HILINK_M2M_LONG_OFFLINE_REBOOT:
        log_info("HILINK_M2M_LONG_OFFLINE_REBOOT\n");
        /* 设备与云端连接长时间断开后进行重启，请在此处添加实现 */
        hilink_process_before_restart(1);
        break;
    case HILINK_UNINITIALIZED:
        log_info("HILINK_UNINITIALIZED\n");
        /* HiLink线程未启动，请在此处添加实现 */
        break;
    case HILINK_LINK_UNDER_AUTO_CONFIG:
        log_info("HILINK_LINK_UNDER_AUTO_CONFIG\n");
        /* 设备处于配网模式，请在此处添加实现 */
        break;
    case HILINK_LINK_CONFIG_TIMEOUT:
        log_info("HILINK_LINK_CONFIG_TIMEOUT\n");
        /* 设备处于10分钟超时状态，请在此处添加实现 */
        break;
    case HILINK_LINK_CONNECTTING_WIFI:
        log_info("HILINK_LINK_CONNECTTING_WIFI\n");
        /* 设备正在连接路由器，请在此处添加实现 */
        break;
    case HILINK_LINK_CONNECTED_WIFI:
        log_info("HILINK_LINK_CONNECTED_WIFI\n");
        /* 设备已经连上路由器，请在此处添加实现 */
        break;
    case HILINK_M2M_CONNECTTING_CLOUD:
        log_info("HILINK_M2M_CONNECTTING_CLOUD\n");
        /* 设备正在连接云端，请在此处添加实现 */
        break;
    case HILINK_M2M_CLOUD_DISCONNECT:
        log_info("HILINK_M2M_CLOUD_DISCONNECT\n");
        /* 设备与路由器的连接断开，请在此处添加实现 */
        break;
    case HILINK_DEVICE_REGISTERED:
        log_info("HILINK_DEVICE_REGISTERED\n");
        /* 设备被注册，请在此处添加实现 */
        break;
    case HILINK_DEVICE_UNREGISTER:
        log_info("HILINK_DEVICE_UNREGISTER\n");
        /* 设备被解绑，请在此处添加实现 */
        hilink_restore_factory_settings();
        break;
    case HILINK_REVOKE_FLAG_SET:
        log_info("HILINK_REVOKE_FLAG_SET\n");
        /* 设备复位标记置位，请在此处添加实现 */

        break;
    case HILINK_NEGO_REG_INFO_FAIL:
        log_info("HILINK_NEGO_REG_INFO_FAIL\n");
        /* 设备协商配网信息失败 */
        break;
    default:
        break;
    }

    return;
}

/*
 * 实现模组重启前的设备操作
 * flag为0表示HiLink SDK 线程看门狗触发模组重启; 为1表示APP删除设备触发模组重启
 * 返回0表示处理成功, 系统可以重启，使用硬重启; 返回1表示处理成功, 系统可以重启，使用软重启;
 * 返回负值表示处理失败, 系统不能重启
 * 注意，此函数由设备厂商实现；若APP删除设备触发模组重启时，设备操作完务必返回0，否则会导致删除设备异常
 */
int hilink_process_before_restart(int flag)
{
    log_info("hilink_process_before_restart");

    hilink_handle_destory();
    protlcol_destory();
    // HILINK_StopSoftAp();
    /* HiLink SDK线程看门狗超时触发模组重启 */
    if (flag == HILINK_REBOOT_WATCHDOG)
    {

        /* 实现模组重启前的操作(如:保存系统状态等) */
        return 1;
    }

    /* APP删除设备触发模组重启 */
    if (flag == HILINK_REBOOT_DEVDELETE)
    {

        /* 实现模组重启前的操作(如:保存系统状态等) */
        return 1;
    }

    return -1;
}

/*
 * 获取设备故障码，并通知APP
 * status表示是否发送故障，0表示不发送，1表示发送；code表示故障码
 * 返回0表示成功，返回非0失败
 */
int get_faultDetection_state(int *status, int *code)
{
    /* 由设备厂商实现，将服务faultDetection属性当前值赋予出参 */
    log_info("get_faultDetection_state status:%d,code:%d", *status, *code);
    *status = 1;
    return 0;
}

/*
 * 获取当前设备唯一身份标识
 * 返回0，获取成功；返回非0，获取失败。
 * 注意: (1)仅android系统设备适配此接口
 *       (2)固定长度6字节
 *       (3)整个设备生命周期不可改变，包括设备重启和恢复出厂等
 */
int HILINK_GetUniqueIdentifier(unsigned char *id, unsigned int len)
{
    return 0;
}
