#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "signalQuit.h"
#include "POSIXTimer.h"

#include "frameCb.h"

#include "client.h"
#define LED_TIMEER_INDEX 1
static timer_t led_timer_id;

static int systemQuit(void)
{
    runSystemCb(SYSTEM_CLOSE);
    return 0;
}
//------------------------------
static void timerThreadHandler(union sigval v)
{
    if (v.sival_int == LED_TIMEER_INDEX)
    {
        runSystemCb(LED_DRIVER_TIMER_FILP);
    }
    else
    {
        /* code */
    }
}
static int led_timer_open(void)
{
    if (led_timer_id != NULL)
    {
        return -1;
    }
    led_timer_id = POSIXTimerCreate(LED_TIMEER_INDEX, timerThreadHandler);
    POSIXTimerSet(led_timer_id, 1, 1);
    return 0;
}
static int led_timer_close(void)
{
    if (led_timer_id != NULL)
    {
        POSIXTimerDelete(led_timer_id);
        led_timer_id = NULL;
    }
    return 0;
}
//------------------------------
int hytoolClose(void)
{
    if (led_timer_id != NULL)
        POSIXTimerDelete(led_timer_id);
    clientClose();
    return 0;
}

int hytoolOpen(void)
{
    registerSystemCb(led_timer_open, LED_DRIVER_TIMER_OPEN);
    registerSystemCb(led_timer_close, LED_DRIVER_TIMER_CLOSE);

    registerQuitCb(systemQuit);
    signalQuit();

    clientOpen();
    return 0;
}