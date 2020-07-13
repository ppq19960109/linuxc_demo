/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: HiLink SDK配网模式管理
 */
#ifndef HILINK_NETCONFIG_MODE_MGT_H
#define HILINK_NETCONFIG_MODE_MGT_H

#ifdef __cplusplus
extern "C" {
#endif

/* HiLink SDK支持的配网模式 */
enum HILINK_NetConfigMode {
    HILINK_NETCONFIG_NONE, /* 不用配网, 通过网线等手段连接到网络 */
    HILINK_NETCONFIG_WIFI, /* HiLink SDK提供的WiFi自动配网 */
    HILINK_NETCONFIG_OTHER, /* 其他配网模式, APP发送WiFi的信息, 集成方收到WiFi信息数据后, 设置到HiLink SDK */
    HILINK_NETCONFIG_BOTH, /* 其他配网模式和WiFi配网组合 */
    HILINK_NETCONFIG_REGISTER_ONLY, /* HiLink SDK SoftAp配网仅接收注册信息 */
    HILINK_NETCONFIG_NO_SOFTAP_REGISTER_ONLY, /* 不启动SoftAp, PIN码配网仅接收注册信息(通过网线/4G/5G等接入网络) */
    HILINK_NETCONFIG_BUTT /* 非法配网模式 */
};

/* 设置产品配网模式, 注意: 需要在启动HiLink SDK任务之前调用本接口设置配网模式 */
int HILINK_SetNetConfigMode(enum HILINK_NetConfigMode netConfigMode);

/* 查询当前产品的配网模式, 返回值为当前产品的配网模式 */
enum HILINK_NetConfigMode HILINK_GetNetConfigMode(void);

#ifdef __cplusplus
}
#endif

#endif
