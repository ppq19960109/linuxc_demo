/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
 * 返回0表示处理成功, 系统可以重启，使用硬重启;
 * 返回1表示处理成功, 如果通过HILINK_SetSdkAttr()注册了软重启(sdkAttr.rebootSoftware), 则使用软重启,
 *     否则不重启HiLink应用, 回退HiLink内部状态, 重新进入配网;
 * 返回负值表示处理失败, 系统不能重启
 * 注意，此函数由设备厂商实现
 */
int hilink_process_before_restart(int flag);

/*
 * 获取设备故障码，并通知APP
 * status表示是否发送故障，0表示不发送，1表示发送；code表示故障码
 * 返回0表示成功，返回非0失败
 */
int get_faultDetection_state(int *status, int *code);

/*
 * 获取当前设备唯一身份标识
 * 返回0，获取成功；返回非0，获取失败。
 * 注意: (1)仅android系统设备适配此接口
 *       (2)固定长度6字节
 *       (3)整个设备生命周期不可改变，包括设备重启和恢复出厂等
 */
int HILINK_GetUniqueIdentifier(unsigned char *id, unsigned int len);

#ifdef __cplusplus
}
#endif
#endif
