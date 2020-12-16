/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: 网络适配实现 (需设备厂商实现)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <sys/stat.h>

#include "hilink_network_adapter.h"
#include "hilink_netconfig_mode_mgt.h"

#include "net_info.h"
#include "tool.h"
static int network_online = 0;

int splitToInt(char *str, char *out, unsigned char outlen)
{
    char *token = strtok(str, ":");
    // char *ptr;
    int i = 0;
    for (i = 0; token != NULL && i < outlen; ++i)
    {
        out[i] = strtol(token, NULL, 16);
        token = strtok(NULL, ":");
    }

    return i;
}

long getFileSize(const char *path)
{
    long filesize = -1;
    struct stat statbuff;
    if (stat(path, &statbuff) < 0)
    {
        return filesize;
    }
    else
    {
        filesize = statbuff.st_size;
    }
    return filesize;
}
/*
 * 获取本地ip
 * localIp表示存放Ip的缓冲
 * len表示存放Ip的缓冲长度
 * 返回0表示成功，返回-1表示失败
 * 注意: localIp为点分十进制格式
 */
int HILINK_GetLocalIp(char *localIp, unsigned char len)
{
    int ret = get_local_ip(ETH_NAME, localIp, len);
    if (ret < 0)
        goto fail;
    ret = get_link_status(ETH_NAME);
    // log_info("HILINK_GetLocalIp:%s %d\n", localIp, len);
    if (ret < 0)
        goto fail;
    if (network_online < 16)
    {
        log_info("network online\n");
        network_online = 16;
        // sleep(1);
    }
    // if (getFileSize("/userdata/resolv.conf") <= 10)
    // {
    //     log_info("getFileSize resolv.conf less\n");
    //     system("echo nameserver 114.114.114.114 > /userdata/resolv.conf");
    //     system("echo nameserver 8.8.8.8 >> /userdata/resolv.conf");
    // }
    return 0;
fail:
    if (network_online > 0)
    {
        --network_online;
        if (network_online == 0)
        {
            log_info("network unonline\n");
            system("dhclient -cf /userdata/app/dhclient.conf -r");
            system("dhclient -cf /userdata/app/dhclient.conf -nw");
        }
    }
    return -1;
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
    int ret;
    char str[20];

    ret = get_local_mac(ETH_NAME, str, sizeof(str));

    splitToInt(str, mac, len);

    // log_info("HILINK_GetMacAddr mac:%s,len:%d\n", mac, len);
    return ret;
}

/*
 * 获取WiFi ssid
 * ssid表示存放WiFi ssid的缓冲
 * ssidLen表示WiFi ssid的长度
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_GetWiFiSsid(char *ssid, unsigned int *ssidLen)
{
    log_info("HILINK_GetWiFiSsid\n");

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
    log_info("HILINK_SetWiFiInfo ssid:%s,ssidlen:%d,pwd:%s\n", ssid, ssidLen, pwd);
    if (ssid == NULL || pwd == NULL)
    {
        return -1;
    }

    return 0;
}

/* 断开并重连WiFi */
void HILINK_ReconnectWiFi(void)
{
    log_info("HILINK_ReconnectWiFi\n");
    return;
}

/*
 * 连接WiFi
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_ConnectWiFi(void)
{
    log_info("HILINK_ConnectWiFi\n");

    return 0;
}

/*
 * 获取网络状态
 * state为0表示网络断开或已连接但网卡未分配得ip，state为1表示已连接且分配得ip
 * 返回0表示成功，返回-1表示失败
 */

int HILINK_GetNetworkState(int *state)
{
    char ip[18];
    if (HILINK_GetLocalIp(ip, sizeof(ip)) != 0)
    {
        *state = 0;
        return -1;
    }
    if (strlen(ip) > 0)
    {
        *state = 1;
    }
    else
    {
        *state = 0;
    }

    // log_info("HILINK_GetNetworkState ip:%s\n", ip);
    return 0;
}

/*
 * 获取当前连接的WiFi的 bssid
 * bssid表示存放WiFi bssid的缓冲
 * bssidLen表示WiFi bssid长度
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_GetWiFiBssid(unsigned char *bssid, unsigned char *bssidLen)
{
    log_info("HILINK_GetWiFiBssid %s,%d\n", bssid, *bssidLen);

    return 0;
}

/*
 * 获取当前连接的WiFi信号强度，单位db
 * rssi表示信号强度
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_GetWiFiRssi(signed char *rssi)
{
    log_info("HILINK_GetWiFiRssi\n");

    return 0;
}

/*
 * 重启HiLink SDK
 * 若系统不可重启，建议重启HiLink进程
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_Restart(void)
{
    log_info("HILINK_Restart\n");
    sync();
    reboot(RB_AUTOBOOT);
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