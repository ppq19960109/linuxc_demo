/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: softAp适配实现 (需设备厂商实现)
 */
#include "hilink_softap_adapter.h"
#include <stdio.h>
#include "main.h"
/*
 * 获取广播ip
 * broadcastIp表示存放Ip的缓冲
 * len表示存放Ip的缓冲长度
 * 返回0表示成功，返回-1表示失败
 * 注意: broadcastIp为点分十进制格式
 */
int HILINK_GetBroadcastIp(char *broadcastIp, unsigned char len)
{
    const char *ip = getNetworkBroadcast(ETH_NAME, broadcastIp, len);
    if (ip == NULL)
        return -1;
    printf("HILINK_GetBroadcastIp:%s,len:%d\n", broadcastIp, len);
    return 0;
}

/*
 * 将网卡切为AP模式并开启softAp热点
 * ssid 表示用于创建softAp的ssid
 * ssidLen表示ssid长度, 最大取值64
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_StartSoftAp(const char *ssid, unsigned int ssidLen)
{
    return 0;
}

/*
 * 关闭softAp热点并将网卡切回station模式
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_StopSoftAp(void)
{
    return 0;
}