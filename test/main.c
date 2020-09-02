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

static int CalCrc(int crc, const char *buf, int len)
{
    unsigned int byte;
    unsigned char k;
    unsigned short ACC, TOPBIT;
    unsigned short remainder = 0x0000;
    // unsigned short remainder = crc;
    TOPBIT = 0x8000;
    for (byte = 0; byte < len; ++byte)
    {
        ACC = buf[byte];
        remainder ^= (ACC << 8);
        for (k = 8; k > 0; --k)
        {
            if (remainder & TOPBIT)
            {
                remainder = (remainder << 1) ^ 0x8005;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }
    remainder = remainder ^ 0x0000;
    return remainder;
}

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

int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;
    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
    if (tcgetattr(fd, &oldtio) != 0)
    {
        perror("SetupSerial 1");
        printf("tcgetattr( fd,&oldtio) -> %d\n", tcgetattr(fd, &oldtio));
        return -1;
    }
    bzero(&newtio, sizeof(newtio));
    /*步骤一，设置字符大小*/
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    /*设置停止位*/
    switch (nBits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }
    /*设置奇偶校验位*/
    switch (nEvent)
    {
    case 'o':
    case 'O': //奇数
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'e':
    case 'E': //偶数
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'n':
    case 'N': //无奇偶校验位
        newtio.c_cflag &= ~PARENB;
        break;
    default:
        break;
    }
    /*设置波特率*/
    switch (nSpeed)
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    /*设置停止位*/
    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;
    /*设置等待时间和最小接收字符*/
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    /*处理未接收字符*/
    tcflush(fd, TCIFLUSH);
    /*激活新配置*/
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        perror("com set error");
        return -1;
    }
    printf("set done!\n");
    return 0;
}
int uart_test()
{
    int fd, nread;
    char buff[256];
    char *dev = "/dev/ttyS1"; //串口二
    fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        printf("open fail\n");
        perror(dev);
        return -1;
    }
    printf("open success\n");
    if (set_opt(fd, 115200, 8, 'N', 1) != 0)
    {
        printf("Set Parity Error\n");
        close(fd);
        exit(1);
    }
    while (1)
    {
        while ((nread = read(fd, buff, 256 - 1)) > 0)
        {
            printf("Len:%d\n", nread);
            buff[nread] = '\0';
            printf("buff:%s\n", buff);
            write(fd, buff, nread + 1);
        }
    }
    printf("Len %d\n", nread);
    close(fd);
    return 0;
}

int popen_test()
{
    FILE *pFile = popen("echo Link Quality=5/5  Signal level=dBm  Noise level=dBm | grep -Eo \'[\\-][0-9][0-9]*\' | awk \'NR==1{print $1}\'", "r");
    char szBuf[8] = {0};

    while (fgets(szBuf, sizeof(szBuf), pFile))
    {
        printf("process id is %s,%d\n", szBuf, atoi(szBuf));
    }
    pclose(pFile);
    return 0;
}

int file_size(char *filename)
{
    struct stat statbuf;
    stat(filename, &statbuf);
    int size = statbuf.st_size;
    return size;
}

int read_file()
{
    char *dev = "hilink_ac_1661.key";
    int fd = open(dev, O_RDWR);
    char buf[64] = {0};
    if (fd < 0)
    {
        perror(dev);
        return -1;
    }
    read(fd, buf, 64);
    for (int i = 0; i < 48; i++)
    {
        printf("0x%x,", (unsigned char)buf[i]);
    }
    printf("\n");
    close(fd);
    return 0;
}
#define NONE "\e[0m"
#define YELLOW "\e[1;33m"
#define BLUE "\e[0;34m"
#define log_color(color, fmt, ...)                                    \
    log_printf(color "%s-[%s-%d]: " fmt NONE, __FUNCTION__, __FILE__, \
               __LINE__, ##__VA_ARGS__)

#define log_debug(fmt, ...) log_color(BLUE, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...) log_color(YELLOW, fmt, ##__VA_ARGS__)
void log_printf(char* format, ...) {
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
int main(int agrc, char *agrv[])
{
    printf("main start \n");
    // udp_broadcast_client();
    // udp_broadcast_client_eth("wlan0");
    // uart_test();
    log_debug("abcd:%d",5);
    log_warn("abcd:%d\n",5);
    log_debug("abcd:%d\n",5);
    return 0;
}
