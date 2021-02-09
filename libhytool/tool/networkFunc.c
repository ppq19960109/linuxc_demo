#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <ifaddrs.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

#include <arpa/inet.h>
#include "cJSON.h"

const char *getNetworkIp(const char *eth_inf, char *ip, unsigned char len)
{
    // struct sockaddr_in sin;
    struct ifreq ifr;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == fd)
    {
        printf("socket error: %s\n", strerror(errno));
        return NULL;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(fd);
        return NULL;
    }
    close(fd);

    // memcpy(&sin, &ifr.ifr_addr, sizeof(struct sockaddr_in));
    // snprintf(ip, len, "%s", inet_ntoa(sin.sin_addr));

    return inet_ntop(AF_INET, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, ip, len);
}

char *getNetworkMac(const char *eth_inf, char *mac, unsigned int len, const char *separator)
{
    struct ifreq ifr;
    int sd;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("get %s mac address socket creat error\n", eth_inf);
        return NULL;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);

    if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
    {
        printf("get %s mac address error\n", eth_inf);
        close(sd);
        return NULL;
    }

    close(sd);

    snprintf(mac, len, "%02x%s%02x%s%02x%s%02x%s%02x%s%02x",
             (unsigned char)ifr.ifr_hwaddr.sa_data[0], separator,
             (unsigned char)ifr.ifr_hwaddr.sa_data[1], separator,
             (unsigned char)ifr.ifr_hwaddr.sa_data[2], separator,
             (unsigned char)ifr.ifr_hwaddr.sa_data[3], separator,
             (unsigned char)ifr.ifr_hwaddr.sa_data[4], separator,
             (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

    return mac;
}

const char *getNetworkBroadcast(const char *eth_inf, char *broadcast, unsigned char broadcastLen)
{
    int sd;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return NULL;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);

    if (ioctl(sd, SIOCGIFBRDADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return NULL;
    }
    close(sd);

    return inet_ntop(AF_INET, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, broadcast, broadcastLen);
}

// if_name like "ath0", "eth0". Notice: call this function
// need root privilege.
// return value:
// -1 -- error , details can check errno
// 1 -- interface link up
// 0 -- interface link down.
int getNetlinkEthtool(const char *if_name)
{
    int sd;
    struct ifreq ifr;
    struct ethtool_value edata;

    edata.cmd = ETHTOOL_GLINK;
    edata.data = 0;

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_data = (char *)&edata;

    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return -1;

    if (ioctl(sd, SIOCETHTOOL, &ifr) == -1)
    {
        fprintf(stderr, "%s: ioctl SIOCETHTOOL error [%d] %s\n", if_name, errno, strerror(errno));
        close(sd);
        return -1;
    }

    close(sd);
    return edata.data;
}

int getNetlink(const char *if_name)
{
    int sd;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);

    if (ioctl(sd, SIOCGIFFLAGS, (char *)&ifr) < 0)
    {
        fprintf(stderr, "%s: ioctl SIOCGIFFLAGS error [%d] %s\n", if_name, errno, strerror(errno));
        close(sd);
        return -1;
    }
    close(sd);

    if (!(ifr.ifr_flags & IFF_UP))
    {
        fprintf(stderr, "DEVICE_DOWN\n");
        return 1;
    }

    if (!(ifr.ifr_flags & IFF_RUNNING))
    {
        fprintf(stderr, "DEVICE_UNPLUGGED\n");
        return 2;
    }

    fprintf(stderr, "DEVICE_LINKED\n");

    return 0;
}
