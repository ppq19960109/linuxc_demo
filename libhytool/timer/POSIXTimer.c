#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


#include "POSIXTimer.h"

// static void POSIXTTimerThreadHandler(union sigval v)
// {

// }

// static void POSIXTTimerSignalHandler(int signal)
// {
// }

int POSIXTimerSet(timer_t timerid, int interval_sec, int sec)
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

timer_t POSIXTimerCreate(int sival, POSIXTimerFunc POSIXTimerFun)
{

    timer_t timerid = 0;
    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent)); //清零初始化
#if 1
    evp.sigev_value.sival_int = sival;         //也是标识定时器的，回调函数可以获得
    evp.sigev_notify = SIGEV_THREAD;           //线程通知的方式，派驻新线程
    evp.sigev_notify_function = POSIXTimerFun; //线程函数地址
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

int POSIXTimerDelete(timer_t timerid)
{
    return timer_delete(timerid);
}
