/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: softAp适配实现 (需设备厂商实现)
 */
#include "hilink_softap_adapter.h"
#include "hilink_network_adapter.h"

#include "net_info.h"
/*
 * 获取广播ip
 * broadcastIp表示存放Ip的缓冲
 * len表示存放Ip的缓冲长度
 * 返回0表示成功，返回-1表示失败
 * 注意: broadcastIp为点分十进制格式
 */
int HILINK_GetBroadcastIp(char *broadcastIp, unsigned char len)
{
    log_info("HILINK_GetBroadcastIp");
    return get_local_broadcastIp(ETH_NAME,broadcastIp,len);
}

/*
 * 将网卡切为AP模式并开启softAp热点
 * ssid 表示用于创建softAp的ssid
 * ssidLen表示ssid长度, 最大取值64
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_StartSoftAp(const char *ssid, unsigned int ssidLen)
{
    log_info("HILINK_StartSoftAp");
    system("killall hostapd");
    system("killall dnsmasq");
    system("wpa_cli -i wlan0 disable_network 0");

    system("ifconfig wlan0 down");
    sleep(1);
    system("ifconfig wlan0 up");
    sleep(1);
    system("echo \"1\" > /proc/sys/net/ipv4/ip_forward");

    system("ifconfig wlan0 192.168.72.1");

    system("hostapd /userdata/hostapd_rk3308.conf -B");
    system("dnsmasq -iwlan0  --dhcp-option=3,192.168.72.1 --dhcp-range=192.168.72.50,192.168.72.200,24h");
    system("iptables -t nat -A POSTROUTING -s 192.168.72.1/24 -o eth0 -j MASQUERADE");

    return 0;
}

/*
 * 关闭softAp热点并将网卡切回station模式
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_StopSoftAp(void)
{
    log_info("HILINK_StopSoftAp");
    system("killall hostapd");
    system("killall dnsmasq");
    return 0;
}