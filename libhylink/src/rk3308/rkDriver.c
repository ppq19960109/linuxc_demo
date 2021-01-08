
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

#define DRIVER_LED_NAME "/dev/rkled1"

#define LED_ON 0
#define LED_OFF 1

typedef struct
{
    unsigned char flip;
    int ledfd;

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

int LedStatusForline(void *status)
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
int rkDriverClose(void)
{
    LedStatusForline(0);
    driverClose(rk_driver.ledfd);
    rk_driver.ledfd = 0;
    return 0;
}

void rkDriverOpen(void)
{
    registerCmdCb(LedStatusForline, LED_DRIVER_LINE);
    registerSystemCb(LedStatusFlash, LED_DRIVER_FLASH);
    registerSystemCb(ledTimerCallback, LED_DRIVER_TIMER_FILP);
    LedStatusForline(0);
}
