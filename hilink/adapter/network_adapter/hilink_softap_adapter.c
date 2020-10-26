/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: softAp适配实现 (需设备厂商实现)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hilink_softap_adapter.h"
#include "hilink_netconfig_mode_mgt.h"

#include "net_info.h"
#include "wifi.h"
#include "tool.h"
/*
 * 获取广播ip
 * broadcastIp表示存放Ip的缓冲
 * len表示存放Ip的缓冲长度
 * 返回0表示成功，返回-1表示失败
 * 注意: broadcastIp为点分十进制格式
 */
int HILINK_GetBroadcastIp(char *broadcastIp, unsigned char len)
{
    log_info("HILINK_GetBroadcastIp\n");
    int ret;
    enum HILINK_NetConfigMode net_mode = HILINK_GetNetConfigMode();
    if (net_mode == HILINK_NETCONFIG_WIFI)
    {
        ret = get_local_broadcastIp("wlan0", broadcastIp, len);
    }
    else
    {
        ret = get_local_broadcastIp(ETH_NAME, broadcastIp, len);
    }
    return ret;
}

/*
 * 将网卡切为AP模式并开启softAp热点
 * ssid 表示用于创建softAp的ssid
 * ssidLen表示ssid长度, 最大取值64
 * 返回0表示成功，返回-1表示失败
 */

int HILINK_StartSoftAp(const char *ssid, unsigned int ssidLen)
{
    log_info("HILINK_StartSoftAp ssid:%s\n", ssid);
    char cmdline[128] = {0};

    create_hostapd_file(AP_NAME, ssid, "1234567890");

    HILINK_StopSoftAp();
    system("echo \"1\" > /proc/sys/net/ipv4/ip_forward");

    sprintf(cmdline, "ifconfig %s %s.1 netmask 255.255.255.0", AP_NAME, IPADDR);
    console_run(cmdline);
    sprintf(cmdline, "route add default gw %s %s.1", AP_NAME, IPADDR);
    console_run(cmdline);

    sprintf(cmdline, "ifconfig %s down", AP_NAME);
    console_run(cmdline);
    sleep(1);
    sprintf(cmdline, "ifconfig %s up", AP_NAME);
    console_run(cmdline);

    sprintf(cmdline, "hostapd %s -B", HOSTAPD_CONF_DIR);
    console_run(cmdline);

    sprintf(cmdline, "dnsmasq -i%s  --dhcp-option=3,%s.1 --dhcp-range=%s.50,%s.200,24h", AP_NAME, IPADDR, IPADDR, IPADDR);
    console_run(cmdline);
    sprintf(cmdline, "iptables -t nat -A POSTROUTING -s %s.1/24 -o eth0 -j MASQUERADE", IPADDR);
    console_run(cmdline);
    return 0;
}

/*
 * 关闭softAp热点并将网卡切回station模式
 * 返回0表示成功，返回-1表示失败
 */
int HILINK_StopSoftAp(void)
{
    log_info("HILINK_StopSoftAp\n");
    if (get_hostapd_pid())
        system("killall hostapd");
    if (get_dnsmasq_pid())
        system("killall dnsmasq");
    system("iptables -t nat -F");
    return 0;
}