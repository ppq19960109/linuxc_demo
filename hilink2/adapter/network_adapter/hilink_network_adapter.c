/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: 网络适配实现 (需设备厂商实现)
 */
#include "hilink_network_adapter.h"
#include <stdio.h>
#include "main.h"
/*
 * 获取本地ip
 * localIp表示存放Ip的缓冲
 * len表示存放Ip的缓冲长度
 * 返回0表示成功，返回-1表示失败
 * 注意: localIp为点分十进制格式
 */
int HILINK_GetLocalIp(char *localIp, unsigned char len)
{
    if (getNetworkIp(ETH_NAME, localIp, len) == NULL)
    {
        printf("HILINK_GetLocalIp fail\n");
        return -1;
    }
    return 0;
}

/*
 * 获取网络mac地址
 * mac表示存放MAC地址的缓冲
 * len表示缓冲长度
 * 返回0表示成功，返回-1表示失败
 * 注意: mac格式为a1b2c3d4e5f6
 */
int HILINK_GetMacAddr(unsigned char *mac, unsigned char len)
{
    char mac_str[18];
    const char *delim = ":";

    if (getNetworkMac(ETH_NAME, mac_str, sizeof(mac_str), delim) == NULL)
    {
        printf("HILINK_GetMacAddr fail\n");
        return -1;
    }
    //----------------------
    printf("HILINK_GetMacAddr %s\n", mac_str);
    char *token = strtok(mac_str, delim);
    int i = 0;
    for (i = 0; token != NULL && i < len; ++i)
    {
        mac[i] = strtol(token, NULL, 16);
        token = strtok(NULL, delim);
    }
    if (i != len)
    {
        printf("HILINK_GetMacAddr len fail,i=%d,len=%d\n", i, len);
        return -1;
    }
    //----------------------
    return 0;
}

/*
 * 获取WiFi ssid
 * ssid表示存放WiFi ssid的缓冲
 * ssidLen表示WiFi ssid的长度
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_GetWiFiSsid(char *ssid, unsigned int *ssidLen)
{
    return 0;
}

/*
 * 设置WiFi账号信息
 * ssid表示WiFi ssid
 * ssidLen表示WiFi ssid的长度
 * pwd表示WiFi密码
 * pwdLen表示WiFi密码长度
 * 返回0表示成功，返回-1表示失败
 * 注意：(1) ssid和pwd为空表示清除WiFi信息;
 *       (2) 设置的WiFi信息需要持久化，以确保设备重启后依然可以获得WiFi配置信息
 */
int HILINK_SetWiFiInfo(const char *ssid, unsigned int ssidLen, const char *pwd, unsigned int pwdLen)
{
    return 0;
}

/* 断开并重连WiFi */
void HILINK_ReconnectWiFi(void)
{
    return;
}

/*
 * 连接WiFi
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_ConnectWiFi(void)
{
    return 0;
}

/*
 * 获取网络状态
 * state为0表示网络断开或已连接但网卡未分配得ip，state为1表示已连接且分配得ip
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_GetNetworkState(int *state)
{
    int ret = getNetlink(ETH_NAME);
    if (ret == 0)
        *state = 1;
    else
        *state = 0;
    return ret;
}

/*
 * 获取当前连接的WiFi的 bssid
 * bssid表示存放WiFi bssid的缓冲
 * bssidLen表示WiFi bssid长度
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_GetWiFiBssid(unsigned char *bssid, unsigned char *bssidLen)
{
    return 0;
}

/*
 * 获取当前连接的WiFi信号强度，单位db
 * rssi表示信号强度
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_GetWiFiRssi(signed char *rssi)
{
    return 0;
}

/*
 * 重启HiLink SDK
 * 若系统不可重启，建议重启HiLink进程
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_Restart(void)
{
    printf("HILINK_Restart\n");
    runSystemCb(SYSTEM_RESTART);
    return 0;
}

/* 限制最多同时接入两个station */
void HILINK_SetStationNumLimit(void)
{
    return;
}

/* SoftAp配网过程中，根据IP踢除对应的station */
void HILINK_DisconnectStation(const char *ip)
{
    return;
}