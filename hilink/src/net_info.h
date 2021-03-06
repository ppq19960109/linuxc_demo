#ifndef _NET_INFO_H
#define _NET_INFO_H

#ifdef __cplusplus
extern "C"
{
#endif

#define ETH_NAME "eth0"//"wlan0" "eth0"//"ens33"

    int get_local_ip(const char *eth_inf, char *ip, unsigned char len);
    int get_local_mac(const char *eth_inf, char *mac, unsigned char len);
    int get_local_broadcastIp(const char *eth_inf, char *ip, unsigned char len);
    int get_local_all_ip(char *ip);
    int get_netlink_status(const char *if_name);
    int get_link_status(const char *if_name);


#ifdef __cplusplus
}
#endif
#endif
