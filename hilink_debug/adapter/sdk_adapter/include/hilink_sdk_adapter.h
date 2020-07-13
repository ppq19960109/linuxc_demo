/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HiLink SDK适配头文件
 * Create: 2019-04-20
 * Notes: 该文件中的接口需要对外提供给第三方厂商使用，为了前向兼容，部分老接口暂不按最新编码规范整改.
 */
#ifndef __HILINK_SDK_ADAPTER_H__
#define __HILINK_SDK_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* HiLink SDK 通知厂商模组重启原因 */
#define HILINK_REBOOT_WATCHDOG  0
#define HILINK_REBOOT_DEVDELETE 1

/* 设备与云端连接断开(版本向前兼容) */
#define HILINK_M2M_CLOUD_OFFLINE        0
/* 设备连接云端成功，处于正常工作态(版本向前兼容) */
#define HILINK_M2M_CLOUD_ONLINE         1
/* 设备与云端连接长时间断开(版本向前兼容) */
#define HILINK_M2M_LONG_OFFLINE         2
/* 设备与云端连接长时间断开后进行重启(版本向前兼容) */
#define HILINK_M2M_LONG_OFFLINE_REBOOT  3
/* HiLink线程未启动 */
#define HILINK_UNINITIALIZED            4
/* 设备处于配网模式 */
#define HILINK_LINK_UNDER_AUTO_CONFIG   5
/* 设备处于10分钟超时状态 */
#define HILINK_LINK_CONFIG_TIMEOUT      6
/* 设备正在连接路由器 */
#define HILINK_LINK_CONNECTTING_WIFI    7
/* 设备已经连上路由器 */
#define HILINK_LINK_CONNECTED_WIFI      8
/* 设备正在连接云端 */
#define HILINK_M2M_CONNECTTING_CLOUD    9
/* 设备与路由器的连接断开 */
#define HILINK_M2M_CLOUD_DISCONNECT     10
/* 设备被注册 */
#define HILINK_DEVICE_REGISTERED        11
/* 设备被解绑 */
#define HILINK_DEVICE_UNREGISTER        12
/* 设备复位标记置位 */
#define HILINK_REVOKE_FLAG_SET          13
/* 设备协商注册信息失败 */
#define HILINK_NEGO_REG_INFO_FAIL       14

/*
 * 通知设备的状态
 * status表示设备当前的状态
 * 注意，此函数由设备厂商根据产品业务选择性实现
 */
void hilink_notify_devstatus(int status);

/*
 * 实现模组重启前的设备操作
 * flag为0表示HiLink SDK 线程看门狗触发模组重启; 为1表示APP删除设备触发模组重启
 * 返回0表示处理成功, 系统可以重启，使用硬重启; 返回1表示处理成功, 系统可以重启，使用软重启;
 * 返回负值表示处理失败, 系统不能重启
 * 注意，此函数由设备厂商实现；若APP删除设备触发模组重启时，设备操作完务必返回0，否则会导致删除设备异常
 */
int hilink_process_before_restart(int flag);

/*
 * 获取设备故障码，并通知APP
 * status表示是否发送故障，0表示不发送，1表示发送；code表示故障码
 * 返回0表示成功，返回非0失败
 */
int get_faultDetection_state(int *status, int *code);

#ifdef __cplusplus
}
#endif
#endif
