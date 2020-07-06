#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>

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

const char data[] = {};
int main(int agrc, char *agrv[])
{
    printf("main start \n");
    // int* a=1;

    // int crc=CalCrc(0,data,sizeof(data));
    // printf("crc value:%x len:%x\n",crc,sizeof(data));
    // printf("main end %d\n",&(*a));

    // int fd, nread;
    // char buff[256];
    // char *dev = "/dev/ttyS0"; //串口二
    // fd = open(dev, O_RDWR | O_NOCTTY);
    // if (fd < 0)
    // {
    //     perror(dev);
    //     return -1;
    // }

    // if (set_opt(fd, 115200, 8, 'N', 1) != 0)
    // {
    //     printf("Set Parity Error\n");
    //     close(fd);
    //     exit(1);
    // }
    // while (1)
    // {
    //     while ((nread = read(fd, buff, 256 - 1)) > 0)
    //     {
    //         printf("Len:%d\n", nread);
    //         buff[nread + 1] = '\0';
    //         printf("buff:%s\n", buff);
    //         write(fd, buff, nread + 1);
    //     }
    // }
    // printf("Len %d\n", nread);
    // close(fd);

    // FILE *pFile = popen("echo Link Quality=5/5  Signal level=dBm  Noise level=dBm | grep -Eo \'[\\-][0-9][0-9]*\' | awk \'NR==1{print $1}\'", "r");
    // char szBuf[8] = {0};

    // while (fgets(szBuf, sizeof(szBuf), pFile))
    // {
    //     printf("process id is %s,%d\n", szBuf,atoi(szBuf));

    // }
    // pclose(pFile);
    char *ptr;
    char *str="SceName_11";
    printf("val:%d,%d\n", atoi(&str[8]),strtol(&str[8], NULL, 10));
    return 0;
}
