
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "rkDriver.h"
#include "frameCb.h"

#define DRIVER_LED_NAME "/dev/rkled0"
#define DRIVER_KEY_NAME "/dev/rkasync"
#define LED_ON 0
#define LED_OFF 1

typedef struct
{
    unsigned char flip;
    int ledfd;
    int keyfd;
} rk_driver_t;

rk_driver_t rk_driver;

int driverOpen(const char *driverPath)
{
    if (driverPath == NULL)
    {
        printf("driver path is NULL\n");
        return -1;
    }
    /* 打开驱动 */
    int fd = open(driverPath, O_RDWR);
    if (fd < 0)
    {
        printf("file %s open failed!\r\n", driverPath);
    }
    return fd;
}

int driverRead(int fd, char *buf, int bufLen)
{
    if (fd <= 0 || buf == NULL || bufLen == 0)
        return -1;

    if (read(fd, buf, bufLen) < 0)
    {
        printf("driver read failed\r\n");
        return -1;
    }
    return 0;
}

int driverClose(int fd)
{
    if (fd <= 0)
        return -1;
    if (close(fd) < 0)
    {
        printf("driver close failed!\r\n");
        return -1;
    }
    return 0;
}

int ledDriverWrite(int fd, char status)
{
    if (fd <= 0)
    {
        printf("led driver fd error\n");
        return -1;
    }
    char buf[1];
    if (status)
    {
        buf[0] = 1;
    }
    else
    {
        buf[0] = 0;
    }

    if (write(fd, buf, sizeof(buf)) < 0)
    {
        printf("led driver write failed!\r\n");
        return -1;
    }
    return 0;
}

/*-------------------------------------------------------------*/
int ledTimerCallback(void)
{
    if (rk_driver.ledfd <= 0)
        return -1;
    rk_driver.flip = !rk_driver.flip;
    return ledDriverWrite(rk_driver.ledfd, rk_driver.flip);
}

int LedStatusForline(void *status, void *status2)
{
    int line = (int)status;
    printf("LedStatusForline:%d\n", line);

    runSystemCb(LED_DRIVER_TIMER_CLOSE);

    if (rk_driver.ledfd <= 0)
        rk_driver.ledfd = driverOpen(DRIVER_LED_NAME);

    if (line > 0)
        ledDriverWrite(rk_driver.ledfd, LED_ON);
    else
        ledDriverWrite(rk_driver.ledfd, LED_OFF);

    driverClose(rk_driver.ledfd);
    rk_driver.ledfd = 0;
    return 0;
}

int LedStatusFlash(void)
{
    printf("LedStatusFlash\n");
    runSystemCb(LED_DRIVER_TIMER_OPEN);

    if (rk_driver.ledfd <= 0)
        rk_driver.ledfd = driverOpen(DRIVER_LED_NAME);
    return 0;
}
//------------------------------------------

static void sigioFunc(int signum)
{
    printf("sigioFunc:%d\n", signum);
    if (signum == SIGIO)
    {
        char buf;
        int ret = driverRead(rk_driver.keyfd, &buf, sizeof(buf));
        if (ret < 0)
        {
            return;
        }
        printf("sigioFunc read:%d\n", buf);
        if (buf == 1)
        {
            runSystemCb(SYSTEM_RESET);
        }
        else if (buf == 2)
        {
            unsigned char cmd = 120;
            runCmdCb(&cmd, NULL, CMD_NETWORK_ACCESS);
            runCmdCb(&cmd, NULL, CMD_NETWORK_ACCESS_TIME);
        }
        else
        {
            /* code */
        }
    }
}

int driverKeyOpen(void)
{
    int fd = open(DRIVER_KEY_NAME, O_RDWR);
    if (fd < 0)
    {
        printf("Can't open file %s\r\n", DRIVER_KEY_NAME);
        return -1;
    }

    /* 设置信号SIGIO的处理函数 */
    signal(SIGIO, sigioFunc);
    signal(SIGURG, sigioFunc);

    fcntl(fd, F_SETOWN, getpid());      /* 设置当前进程接收SIGIO信号 	*/
    int flags = fcntl(fd, F_GETFL);     /* 获取当前的进程状态 			*/
    fcntl(fd, F_SETFL, flags | FASYNC); /* 设置进程启用异步通知功能 	*/

    rk_driver.keyfd = fd;
    return 0;
}

int rkDriverClose(void)
{
    LedStatusForline((void *)0, NULL);
    driverClose(rk_driver.ledfd);
    rk_driver.ledfd = 0;
    driverClose(rk_driver.keyfd);
    rk_driver.keyfd = 0;
    return 0;
}

void rkDriverOpen(void)
{
    registerSystemCb(rkDriverClose, RK_DRIVER_CLOSE);
    registerCmdCb(LedStatusForline, LED_DRIVER_LINE);
    registerSystemCb(LedStatusFlash, LED_DRIVER_FLASH);
    registerSystemCb(ledTimerCallback, LED_DRIVER_TIMER_FILP);
    driverKeyOpen();
    LedStatusForline((void *)1, NULL);
}
