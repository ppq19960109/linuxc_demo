#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <stdarg.h>

#define BROADCAST_PORT 9999
#define IFNAME "ens33"

int udp_broadcast_client()
{
    int sock = -1, ret;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        perror("socket error\n");
        return -1;
    }

    const int opt = 1;
    //设置该套接字为广播类型，
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
    if (ret == -1)
    {
        perror("set socket error...\n");
        return -11;
    }
    //指定Server IP  和  发送给Client的端口
    struct sockaddr_in addrto;
    bzero(&addrto, sizeof(struct sockaddr_in));
    addrto.sin_family = AF_INET;
    addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    addrto.sin_port = htons(BROADCAST_PORT);
    printf("BROADCAST addr:%s\n", inet_ntoa(addrto.sin_addr));

    char smsg[] = {"udp broadcast"};
    while (1)
    {
        //从广播地址发送消息
        ret = sendto(sock, smsg, strlen(smsg), 0, (struct sockaddr *)&addrto, sizeof(addrto));
        if (ret < 0)
        {
            perror("send error....\n");
            continue;
        }
        printf("send success\n");
        sleep(2);
    }
    return 0;
}

int udp_broadcast_client_eth(char *eth_inf)
{
    int ret = -1;
    int sock = -1;
    struct ifreq *ifr;
    struct ifconf ifc;
    char buffer[1024];

    //建立数据报套接字
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("create socket failed:");
        return -1;
    }

    // 获取所有套接字接口
    ifc.ifc_len = sizeof(buffer);
    ifc.ifc_buf = buffer;
    if (ioctl(sock, SIOCGIFCONF, (char *)&ifc) < 0)
    {
        perror("ioctl-conf:");
        return -1;
    }
    ifr = ifc.ifc_req;

    for (int i = 0; i < ifc.ifc_len / sizeof(struct ifreq); i++, ifr++)
    {
        if (!strcmp(ifr->ifr_name, eth_inf))
        {
            break;
        }
    }

    for (;;)
    {
        if (ioctl(sock, SIOCGIFFLAGS, ifr) < 0)
        {
            fprintf(stderr, "%s: ioctl SIOCGIFFLAGS error [%d] %s\r\n", ifr->ifr_name, errno, strerror(errno));
            continue;
        }
        if (!(ifr->ifr_flags & IFF_UP))
        {
            fprintf(stderr, "DEVICE_DOWN\r\n");
            continue;
        }

        if (ifr->ifr_flags & IFF_RUNNING)
        {
            break;
        }
    }
    //将使用的网络接口名字复制到ifr.ifr_name中，由于不同的网卡接口的广播地址是不一样的，因此指定网卡接口
    //strncpy(ifr.ifr_name, IFNAME, strlen(IFNAME));
    //发送命令，获得网络接口的广播地址
    if (ioctl(sock, SIOCGIFBRDADDR, ifr) == -1)
    {
        perror("ioctl error");
        return -1;
    }

    struct sockaddr_in broadcast_addr; //广播地址
    //将获得的广播地址复制到broadcast_addr
    memcpy(&broadcast_addr, (char *)&ifr->ifr_broadaddr, sizeof(struct sockaddr_in));
    //设置广播端口号
    printf("Broadcast IP: %s\n", inet_ntoa(broadcast_addr.sin_addr));

    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(BROADCAST_PORT);

    int opt = 1;
    //默认的套接字描述符sock是不支持广播，必须设置套接字描述符以支持广播
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt,
                     sizeof(opt));

    //发送多次广播，看网络上是否有服务器存在
    char smsg[] = {"udp broadcast"};
    while (1)
    {
        //从广播地址发送消息
        ret = sendto(sock, smsg, strlen(smsg), 0, (struct sockaddr *)&broadcast_addr, sizeof(struct sockaddr));
        if (ret <= 0)
        {
            perror("send error....\n");
            continue;
        }
        // printf("send success\n");
        sleep(2);
    }
    return 0;
}