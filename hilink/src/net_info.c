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

#include "cJSON.h"

#define MAC_SIZE 18
#define IP_SIZE 16

int get_local_ip(const char *eth_inf, char *ip, unsigned char len)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr = {0};

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, len, "%s", inet_ntoa(sin.sin_addr));

    close(sd);
    return 0;
}

int get_local_mac(const char *eth_inf, char *mac, unsigned char len)
{
    struct ifreq ifr;
    int sd;

    bzero(&ifr, sizeof(struct ifreq));
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("get %s mac address socket creat error\n", eth_inf);
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, sizeof(ifr.ifr_name) - 1);

    if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
    {
        printf("get %s mac address error\n", eth_inf);
        close(sd);
        return -1;
    }
    if (len == -1)
    {
        sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
                (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
    }
    else
    {
        snprintf(mac, len, "%02x:%02x:%02x:%02x:%02x:%02x",
                 (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                 (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                 (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                 (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                 (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                 (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
    }

    close(sd);

    return 0;
}

int get_local_broadcastIp(const char *eth_inf, char *ip, unsigned char len)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFBRDADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, len, "%s", inet_ntoa(sin.sin_addr));

    close(sd);
}

int get_local_all_ip(char *ip)
{

    struct ifaddrs *ifAddrStruct;

    void *tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct != NULL)
    {

        if (ifAddrStruct->ifa_addr->sa_family == AF_INET)
        {

            tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;

            inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);

            printf("%s IP Address:%s \n", ifAddrStruct->ifa_name, ip);

            tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_ifu.ifu_broadaddr)->sin_addr;

            inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);

            printf("%s Broadcast IP Address:%s \n", ifAddrStruct->ifa_name, ip);
        }

        ifAddrStruct = ifAddrStruct->ifa_next;
    }

    //free ifaddrs

    freeifaddrs(ifAddrStruct);

    return 0;
}

// if_name like "ath0", "eth0". Notice: call this function
// need root privilege.
// return value:
// -1 -- error , details can check errno
// 1 -- interface link up
// 0 -- interface link down.
int get_netlink_status(const char *if_name)
{
    int skfd;
    struct ifreq ifr;
    struct ethtool_value edata;

    edata.cmd = ETHTOOL_GLINK;
    edata.data = 0;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name) - 1);
    ifr.ifr_data = (char *)&edata;

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return -1;

    if (ioctl(skfd, SIOCETHTOOL, &ifr) == -1)
    {
        fprintf(stderr, "%s: ioctl SIOCETHTOOL error [%d] %s\n", if_name, errno, strerror(errno));
        close(skfd);
        return -1;
    }

    close(skfd);
    return edata.data;
}

int get_link_status(const char *if_name)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
    if (ioctl(sd, SIOCGIFFLAGS, (char *)&ifr) < 0)
    {
        fprintf(stderr, "%s: ioctl SIOCGIFFLAGS error [%d] %s\n", if_name, errno, strerror(errno));
        close(sd);
        return -1;
    }

    if (!(ifr.ifr_flags & IFF_UP))
    {
        close(sd);
        // fprintf(stderr, "DEVICE_DOWN\n");
        return -1;
    }

    if (!(ifr.ifr_flags & IFF_RUNNING))
    {
        close(sd);
        // fprintf(stderr, "DEVICE_UNPLUGGED\n");
        return -2;
    }

    // fprintf(stderr, "DEVICE_LINKED\n");

    close(sd);
    return 0;
}
