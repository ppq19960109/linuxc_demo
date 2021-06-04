#ifndef _POSIXTIMER_H_
#define _POSIXTIMER_H_
#include <time.h>
#include <signal.h>
typedef void (*POSIXTimerFunc)(union sigval);

int POSIXTimerSet(timer_t timerid, int interval_sec, int sec);
timer_t POSIXTimerCreate(int sival, POSIXTimerFunc POSIXTimerFun);
int POSIXTimerDelete(timer_t timerid);
#endif