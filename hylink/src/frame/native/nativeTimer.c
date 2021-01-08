#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "frameCb.h"

typedef struct
{
#define LED_TIMEER_INDEX 1
    timer_t timerid;
#define NET_TIMEER_INDEX 2
    timer_t netTimerid;
} nativeTimer_t;

static nativeTimer_t nativeTimer;

static void POSIXTTimerThreadHandler(union sigval v)
{
    if (v.sival_int == LED_TIMEER_INDEX)
    {
        runSystemCb(LED_DRIVER_TIMER_FILP);
    }
    else if (v.sival_int == NET_TIMEER_INDEX)
    {
        printf("netTimer time out......\n");
        unsigned char cmd = 0;
        runCmdCb(&cmd, NULL, CMD_NETWORK_ACCESS);
        runCmdCb((void *)1, NULL, LED_DRIVER_LINE);
    }
    else
    {
        /* code */
    }
}

// static void POSIXTTimerSignalHandler(int signal)
// {
// }

int setPOSIXTimer(timer_t timerid, int interval_sec, int sec)
{
    /* 第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长,就是说it.it_value变0的时候会>装载it.it_interval的值 */
    struct itimerspec it;
    it.it_interval.tv_sec = interval_sec; // 回调函数执行频率为1s运行1次
    it.it_interval.tv_nsec = 0;
    it.it_value.tv_sec = sec; // 倒计时3秒开始调用回调函数
    it.it_value.tv_nsec = 0;

    int ret = timer_settime(timerid, 0, &it, NULL);
    if (ret < 0)
    {
        perror("fail to timer_settime!");
    }
    return ret;
}

timer_t createPOSIXTimer(int sival, void (*timerFunc)(union sigval))
{

    timer_t timerid;
    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent)); //清零初始化
#if 1
    evp.sigev_value.sival_int = sival;     //也是标识定时器的，回调函数可以获得
    evp.sigev_notify = SIGEV_THREAD;       //线程通知的方式，派驻新线程
    evp.sigev_notify_function = timerFunc; //线程函数地址
#else
    // evp.sigev_signo = SIGUSR1;
    // evp.sigev_notify = SIGEV_SIGNAL;

    // struct sigaction act;
    // act.sa_handler = fun;
    // sigemptyset(&act.sa_mask);
    // act.sa_flags = SA_RESTART;
    // sigaction(SIGUSR1, &act, NULL);
#endif

    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)
    {
        perror("fail to timer_create");
        return NULL;
    }
    return timerid;
}

//---------------------------------------------------------------
int nativeTimerOpen(void)
{
    if (nativeTimer.timerid != NULL)
    {
        return -1;
    }
    nativeTimer.timerid = createPOSIXTimer(LED_TIMEER_INDEX, POSIXTTimerThreadHandler);
    setPOSIXTimer(nativeTimer.timerid, 1, 1);
    return 0;
}
int nativeTimerClose(void)
{
    if (nativeTimer.timerid != NULL)
    {
        timer_delete(nativeTimer.timerid);
        nativeTimer.timerid = NULL;
    }
    return 0;
}

int nativeNetTimer(void *data, void *data2)
{
    int sec = *(unsigned char *)data;
    sec = sec < 60 ? 60 : sec;
    runSystemCb(LED_DRIVER_FLASH);
    if (nativeTimer.netTimerid == NULL)
    {
        nativeTimer.netTimerid = createPOSIXTimer(NET_TIMEER_INDEX, POSIXTTimerThreadHandler);
    }
    return setPOSIXTimer(nativeTimer.netTimerid, 0, sec);
}
