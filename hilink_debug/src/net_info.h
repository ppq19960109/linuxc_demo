#ifndef _NET_INFO_H
#define _NET_INFO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "log.h"
#include "cJSON.h"
#define ETH_NAME "ens33" //"eth0"

    int get_local_ip(const char *eth_inf, char *ip, unsigned char len);
    int get_local_mac(const char *eth_inf, char *mac, unsigned char len);
    int get_local_broadcastIp(const char *eth_inf, char *ip, unsigned char len);
    int get_local_all_ip(char *ip);


#ifdef __cplusplus
}
#endif
#endif
