#ifndef _NETWORKFUNC_H_
#define _NETWORKFUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <arpa/inet.h>

#define ETH_NAME "eth0" //"wlan0"

    char *getNetworkIp(const char *eth_inf, char *ip, socklen_t ipLen);
    char *getNetworkMac(const char *eth_inf, char *mac, unsigned char len);
    char *getNetworkSmallMac(const char *eth_inf, char *mac, unsigned char len);
    char *getNetworkBroadcast(const char *eth_inf, char *broadcast, unsigned char broadcastLen);
    int getNetlinkEthtool(const char *if_name);
    int getNetlink(const char *if_name);

#ifdef __cplusplus
}
#endif
#endif
