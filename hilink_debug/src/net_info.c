#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#define MAC_SIZE 18
#define IP_SIZE 16

int get_local_ip(const char *eth_inf, char *ip, unsigned char len)
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
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, len, "%s", inet_ntoa(sin.sin_addr));

    close(sd);
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

    snprintf(mac, len, "%02x%02x%02x%02x%02x%02x",
             (unsigned char)ifr.ifr_hwaddr.sa_data[0],
             (unsigned char)ifr.ifr_hwaddr.sa_data[1],
             (unsigned char)ifr.ifr_hwaddr.sa_data[2],
             (unsigned char)ifr.ifr_hwaddr.sa_data[3],
             (unsigned char)ifr.ifr_hwaddr.sa_data[4],
             (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

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