/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: softAp适配头文件
 */
#ifndef HILINK_SOFTAP_ADAPTER_H
#define HILINK_SOFTAP_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 获取广播ip
 * broadcastIp表示存放Ip的缓冲
 * len表示存放Ip的缓冲长度
 * 返回0表示成功，返回-1表示失败
 * 注意: broadcastIp为点分十进制格式
 */
int HILINK_GetBroadcastIp(char *broadcastIp, unsigned char len);

/*
 * 将网卡切为AP模式并开启softAp热点
 * ssid 表示用于创建softAp的ssid
 * ssidLen表示ssid长度, 最大取值64
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_StartSoftAp(const char *ssid, unsigned int ssidLen);

/*
 * 关闭softAp热点并将网卡切回station模式
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_StopSoftAp(void);

#ifdef __cplusplus
}
#endif
#endif
