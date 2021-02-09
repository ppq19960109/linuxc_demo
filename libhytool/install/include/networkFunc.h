#ifndef _NETWORKFUNC_H_
#define _NETWORKFUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif

    const char *getNetworkIp(const char *eth_inf, char *ip, unsigned char len);
    char *getNetworkMac(const char *eth_inf, char *mac, unsigned int len, const char *separator);
    char *getNetworkBroadcast(const char *eth_inf, char *broadcast, unsigned char broadcastLen);
    int getNetlinkEthtool(const char *if_name);
    int getNetlink(const char *if_name);

#ifdef __cplusplus
}
#endif
#endif
