#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "local_tcp_client.h"
#include "tool.h"
#include "cloud_receive.h"

#define DRIVER_LED_NAME "/dev/rkled1"
#define DRIVER_KEY_NAME "/dev/rkasync"

typedef struct
{
    char flip;
    int fd;
    int keyfd;
    timer_t timerid;
} rk_driver_t;

rk_driver_t rk_driver;

int led_driver_open()
{
    int fd;
    char *filename = DRIVER_LED_NAME;
    /* 打开 led 驱动 */
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("file %s open failed!\r\n", filename);
        return -1;
    }
    return fd;
}

int led_driver_write(int fd, char status)
{
    if (fd <= 0)
        return -1;
    char buf[1];
    if (status)
    {
        buf[0] = 1;
    }
    else
    {
        buf[0] = 0;
    }

    int ret = write(fd, buf, sizeof(buf));
    if (ret < 0)
    {
        printf("LED Control Failed!\r\n");
        return -1;
    }
    return 0;
}

int driver_close(int fd)
{
    if (fd <= 0)
        return -1;
    int ret = close(fd); /* 关闭文件 */
    if (ret < 0)
    {
        printf("fd %d close failed!\r\n", fd);
        return -1;
    }
    return 0;
}
/*-------------------------------------------------------------*/

static void driver_timer_thread_handler(union sigval v)
{
    // printf("driver_timer_thread_handler!:%d\n", rk_driver.flip);
    if (v.sival_int == 0)
    {
        rk_driver.flip = !rk_driver.flip;
        led_driver_write(rk_driver.fd, rk_driver.flip);
    }
}

void driver_deviceRegister()
{
    log_info("driver_deviceRegister\n");
    if (rk_driver.timerid != NULL)
    {
        timer_delete(rk_driver.timerid);
        rk_driver.timerid = NULL;
    }
    if (rk_driver.fd <= 0)
        rk_driver.fd = led_driver_open();
    led_driver_write(rk_driver.fd, 0);
    driver_close(rk_driver.fd);
    rk_driver.fd = 0;
}
void driver_deviceCloudOffline()
{
    log_info("driver_deviceCloudOffline\n");
    if (rk_driver.timerid != NULL)
    {
        timer_delete(rk_driver.timerid);
        rk_driver.timerid = NULL;
    }
    if (rk_driver.fd <= 0)
        rk_driver.fd = led_driver_open();
    led_driver_write(rk_driver.fd, 1);
    driver_close(rk_driver.fd);
    rk_driver.fd = 0;
}

void driver_deviceUnRegister()
{
    log_info("driver_deviceUnRegister\n");
    if (rk_driver.timerid != NULL)
    {
        return;
    }
    if (rk_driver.fd <= 0)
        rk_driver.fd = led_driver_open();
    rk_driver.timerid = start_timer(0, driver_timer_thread_handler, 1, 1);
}
//------------------------------------------

static void sigio_signal_func(int signum)
{
    printf("sigio_signal_func:%d,refactory\n", signum);
    cloud_restart_reFactory(INT_REFACTORY);
}

int driver_keyOpen()
{
    char *filename = DRIVER_KEY_NAME;

    int fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("Can't open file %s\r\n", filename);
        return -1;
    }

    /* 设置信号SIGIO的处理函数 */
    signal(SIGIO, sigio_signal_func);

    fcntl(fd, F_SETOWN, getpid());      /* 设置当前进程接收SIGIO信号 	*/
    int flags = fcntl(fd, F_GETFL);     /* 获取当前的进程状态 			*/
    fcntl(fd, F_SETFL, flags | FASYNC); /* 设置进程启用异步通知功能 	*/

    rk_driver.keyfd = fd;
    return 0;
}

void driver_exit()
{
    driver_close(rk_driver.fd);
    driver_close(rk_driver.keyfd);
}