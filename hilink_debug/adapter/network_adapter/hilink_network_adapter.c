/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: 网络适配实现 (需设备厂商实现)
 */
#include "hilink_network_adapter.h"

#include "net_info.h"

char wifi_ssid[32] = "HUAWEI-WDNJ4L";
char wifi_psk[16] = "1234567890";
char *cmd_remove = "wpa_cli -i wlan0 remove_network 0";
char *cmd_add = "wpa_cli -i wlan0 add_network";
char *cmd_disable = "wpa_cli -i wlan0 disable_network 0";
char *cmd_enable = "wpa_cli -i wlan0 enable_network 0";
char *cmd_udhcp = "udhcpc -i wlan0";

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
    log_info("HILINK_GetLocalIp:%s %d", localIp, ret);
    return ret;
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
    log_info("HILINK_GetMacAddr len:%d", len);
    int ret = get_local_mac(ETH_NAME, mac, len);
    log_info("HILINK_GetMacAddr:%s %d", mac, ret);
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
    log_info("HILINK_GetWiFiSsid");
    strcpy(ssid, wifi_ssid);
    *ssidLen = strlen(wifi_ssid); //rk3308_net
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
    log_info("HILINK_SetWiFiInfo ssid:%s,ssidlen:%d,pwd:%s", ssid, ssidLen, pwd);
    if (ssid == NULL || pwd == NULL)
    {
        return -1;
    }
    strcpy(wifi_ssid, ssid);
    strcpy(wifi_psk, pwd);
    return 0;
}

/* 断开并重连WiFi */
void HILINK_ReconnectWiFi(void)
{
    log_info("HILINK_ReconnectWiFi");
    system(cmd_disable);
    system(cmd_enable);
    system(cmd_udhcp);
    return;
}

/*
 * 连接WiFi
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_ConnectWiFi(void)
{
    // int ret = get_link_status(ETH_NAME);
    // if (ret == 0)
    // {
    //     return 0;
    // }
    log_info("HILINK_ConnectWiFi");
    system(cmd_remove);
    system(cmd_add);

    char cmd_ssid[64] = {"wpa_cli -i wlan0 set_network 0 ssid '\""};
    strcpy(&cmd_ssid[strlen(cmd_ssid)], wifi_ssid);
    strcpy(&cmd_ssid[strlen(cmd_ssid)], "\"'");

    char cmd_psk[64] = {"wpa_cli -i wlan0 set_network 0 psk '\""};
    strcpy(&cmd_psk[strlen(cmd_psk)], wifi_psk);
    strcpy(&cmd_psk[strlen(cmd_psk)], "\"'");

    log_info("cmd_ssid %s", cmd_ssid);
    log_info("cmd_psk %s", cmd_psk);
    system(cmd_ssid);
    system(cmd_psk);

    system(cmd_enable);
    system(cmd_udhcp);
    return 0;
}

/*
 * 获取网络状态
 * state为0表示网络断开或已连接但网卡未分配得ip，state为1表示已连接且分配得ip
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_GetNetworkState(int *state)
{
    char ip[16] = "";
    if (get_local_ip(ETH_NAME, ip, sizeof(ip)) == -1)
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

    // log_info("HILINK_GetNetworkState ip:%s", ip);
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
    log_info("HILINK_GetWiFiBssid");
    int ret = get_local_mac(ETH_NAME, bssid, *bssidLen);
    log_info("HILINK_GetWiFiBssid:%s %d", bssid, ret);
    return ret;
}

/*
 * 获取当前连接的WiFi信号强度，单位db
 * rssi表示信号强度
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_GetWiFiRssi(signed char *rssi)
{
    log_info("HILINK_GetWiFiRssi");
    // FILE *pFile = popen("iwconfig wlan0| grep Signal level -Eo '[\-][0-9][0-9]*' | awk 'NR==1{print $1}'iwconfig wlan0| grep Signal level -Eo \'[\\-][0-9][0-9]*\' | awk \'NR==1{print $1}\'", "r");

    char szBuf[8] = {0};

    popen_cmd("wpa_cli scan_results | grep HUAWEI-WDNJ4L |awk \'{print $3}\'", "r", szBuf, sizeof(szBuf));
    *rssi = atoi(szBuf);
    log_info("HILINK_GetWiFiRssi:%s,%d\n", szBuf, *rssi);

    return 0;
}

/*
 * 重启HiLink SDK
 * 若系统不可重启，建议重启HiLink进程
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_Restart(void)
{
    log_info("HILINK_Restart");
    system("reboot");
    return 0;
}

/* 限制最多同时接入两个station */
void HILINK_SetStationNumLimit(void)
{
    log_info("HILINK_SetStationNumLimit");
    system("hostapd_cli -iwlan0 set max_num_sta 2");
    system("hostapd_cli -iwlan0 reload");
    return;
}

/* SoftAp配网过程中，根据IP踢除对应的station */
void HILINK_DisconnectStation(const char *ip)
{
    log_info("HILINK_DisconnectStation");
    char buf[18];
    char cmd_arp[80] = "arp ";
    char *cmd_arp_ex = " | grep -Eo '([0-9a-fA-F]{2})(([\\s:-][0-9a-fA-F]{2}){5})'";
    strcpy(&cmd_arp[strlen(cmd_arp)], ip);
    strcpy(&cmd_arp[strlen(cmd_arp)], cmd_arp_ex);
    popen_cmd(cmd_arp, "r", buf, sizeof(buf));

    char cmd_disconnect[64] = "hostapd_cli -iwlan0 disassociate ";
    strcpy(&cmd_disconnect[strlen(cmd_disconnect)], buf);
    system(cmd_disconnect);
    return;
}