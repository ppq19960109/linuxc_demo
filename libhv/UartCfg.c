#define _GNU_SOURCE //在源文件开头定义_GNU_SOURCE宏
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <termios.h>

#include "UartCfg.h"
/**
 ** 串口配置
 **/
int uart_init(const char *device, unsigned int baudrate, enum UART_DATABIT databit, enum UART_PARITY parity, enum UART_STOPBIT stopbit, enum UART_FLOWCTRL flowCtrl, enum UART_BLOCKING blocking)
{
    struct termios old_cfg, new_cfg = {0};
    speed_t speed;

    /* 打开串口终端 */
    int flag = O_RDWR | O_NOCTTY;
    if (blocking == BLOCKING_NONBLOCK)
        flag |= O_NONBLOCK;

    int fd = open(device, flag);
    if (0 > fd)
    {
        fprintf(stderr, "open error: %s: %s\n", device, strerror(errno));
        return -1;
    }

    /* 获取串口当前的配置参数 */
    if (0 > tcgetattr(fd, &old_cfg))
    {
        fprintf(stderr, "tcgetattr error: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    /* 设置为原始模式 */
    cfmakeraw(&new_cfg);

    /* 使能接收 */
    new_cfg.c_cflag |= CLOCAL | CREAD;

    /* 设置波特率 */
    switch (baudrate)
    {
    case 1200:
        speed = B1200;
        break;
    case 1800:
        speed = B1800;
        break;
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
    case 57600:
        speed = B57600;
        break;
    case 115200:
        speed = B115200;
        break;
    case 230400:
        speed = B230400;
        break;
    case 460800:
        speed = B460800;
        break;
    case 500000:
        speed = B500000;
        break;
    default: //默认配置为115200
        speed = B115200;
        printf("default baud rate: 115200\n");
        break;
    }

    if (0 > cfsetspeed(&new_cfg, speed))
    {
        fprintf(stderr, "cfsetspeed error: %s\n", strerror(errno));
        return -1;
    }

    /* 设置数据位大小 */
    new_cfg.c_cflag &= ~CSIZE; //将数据位相关的比特位清零
    switch (databit)
    {
    case 5:
        new_cfg.c_cflag |= CS5;
        break;
    case 6:
        new_cfg.c_cflag |= CS6;
        break;
    case 7:
        new_cfg.c_cflag |= CS7;
        break;
    case 8:
        new_cfg.c_cflag |= CS8;
        break;
    default: //默认数据位大小为8
        new_cfg.c_cflag |= CS8;
        printf("default data bit size: 8\n");
        break;
    }

    /* 设置奇偶校验 */
    switch (parity)
    {
    case 'N': //无校验
        new_cfg.c_cflag &= ~PARENB;
        new_cfg.c_iflag &= ~INPCK;
        break;
    case 'O': //奇校验
        new_cfg.c_cflag |= (PARODD | PARENB);
        new_cfg.c_iflag |= INPCK;
        break;
    case 'E': //偶校验
        new_cfg.c_cflag |= PARENB;
        new_cfg.c_cflag &= ~PARODD; /* 清除PARODD标志，配置为偶校验 */
        new_cfg.c_iflag |= INPCK;
        break;
    default: //默认配置为无校验
        new_cfg.c_cflag &= ~PARENB;
        new_cfg.c_iflag &= ~INPCK;
        printf("default parity: N\n");
        break;
    }

    /* 设置停止位 */
    switch (stopbit)
    {
    case 1: //1个停止位
        new_cfg.c_cflag &= ~CSTOPB;
        break;
    case 2: //2个停止位
        new_cfg.c_cflag |= CSTOPB;
        break;
    default: //默认配置为1个停止位
        new_cfg.c_cflag &= ~CSTOPB;
        printf("default stop bit size: 1\n");
        break;
    }

    switch (flowCtrl)
    {
    case 0: //不使用流控制
        new_cfg.c_cflag &= ~CRTSCTS;
        break;

    case 1: //使用硬件流控制
        new_cfg.c_cflag |= CRTSCTS;
        break;
    case 2: //使用软件流控制
        new_cfg.c_cflag |= IXON | IXOFF | IXANY;
        break;
    }

    /* 将MIN和TIME设置为0 */
    new_cfg.c_cc[VTIME] = 0;
    new_cfg.c_cc[VMIN] = 0;

    /* 清空缓冲区 */
    if (0 > tcflush(fd, TCIOFLUSH))
    {
        fprintf(stderr, "tcflush error: %s\n", strerror(errno));
        return -1;
    }

    /* 写入配置、使配置生效 */
    if (0 > tcsetattr(fd, TCSANOW, &new_cfg))
    {
        fprintf(stderr, "tcsetattr error: %s\n", strerror(errno));
        return -1;
    }

    /* 配置OK 退出 */
    return fd;
}
