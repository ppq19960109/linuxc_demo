#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "local_tcp_client.h"
#include "tool.h"

int led_driver_open()
{
    int fd;
    char *filename = "/dev/rkled1";
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
typedef struct
{
    int fd;
    int keyfd;
    timer_t timerid;
    char flip;
} led_driver_t;

led_driver_t led_driver;

static void driver_timer_thread_handler(union sigval v)
{
    // printf("driver_timer_thread_handler!:%d\n", led_driver.flip);
    if (v.sival_int == 0)
    {
        led_driver.flip = !led_driver.flip;
        led_driver_write(led_driver.fd, led_driver.flip);
    }
}

void driver_deviceRegister()
{
    log_info("driver_deviceRegister\n");
    if (led_driver.timerid != NULL)
    {
        timer_delete(led_driver.timerid);
        led_driver.timerid = NULL;
    }
    if (led_driver.fd <= 0)
        led_driver.fd = led_driver_open();
    led_driver_write(led_driver.fd, 0);
    driver_close(led_driver.fd);
    led_driver.fd = 0;
}

void driver_deviceUnRegister()
{
    log_info("driver_deviceUnRegister\n");
    if (led_driver.timerid != NULL)
    {
        return;
    }
    if (led_driver.fd <= 0)
        led_driver.fd = led_driver_open();
    led_driver.timerid = start_timer(0, driver_timer_thread_handler, 1, 1);
}
//------------------------------------------

static void sigio_signal_func(int signum)
{

    printf("sigio_signal_func\n");
    system("sh /userdata/app/restore.sh");
}

int driver_keyOpen()
{
    int flags = 0;
    char *filename = "/dev/rkasync";

    int fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("Can't open file %s\r\n", filename);
        return -1;
    }

    /* 设置信号SIGIO的处理函数 */
    signal(SIGIO, sigio_signal_func);

    fcntl(fd, F_SETOWN, getpid());      /* 设置当前进程接收SIGIO信号 	*/
    flags = fcntl(fd, F_GETFL);         /* 获取当前的进程状态 			*/
    fcntl(fd, F_SETFL, flags | FASYNC); /* 设置进程启用异步通知功能 	*/

    led_driver.keyfd = fd;
    return 0;
}

void driver_exit()
{
    driver_close(led_driver.fd);
    driver_close(led_driver.keyfd);
}