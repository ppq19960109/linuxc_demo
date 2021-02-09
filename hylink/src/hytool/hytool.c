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
#define NET_TIMEER_INDEX 2
static timer_t led_timer_id;
static timer_t net_access_timer_id;

static int systemQuit(void)
{
    runSystemCb(SYSTEM_CLOSE);
    return 0;
}
//-------------------------------
static int led_timer_open(void);
static int led_timer_close(void);

void net_timer_close(void)
{
    printf("net_access_timer time out......\n");
    led_timer_close();
    unsigned char cmd = 0;
    runCmdCb(&cmd, CMD_NETWORK_ACCESS);
    runCmdCb((void *)1, LED_DRIVER_LINE);
}

static void timerThreadHandler(union sigval v)
{
    if (v.sival_int == LED_TIMEER_INDEX)
    {
        runSystemCb(LED_DRIVER_TIMER_FILP);
    }
    else if (v.sival_int == NET_TIMEER_INDEX)
    {
        net_timer_close();
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

static int net_access_timer(void *data)
{
    if (net_access_timer_id == NULL)
    {
        net_access_timer_id = POSIXTimerCreate(NET_TIMEER_INDEX, timerThreadHandler);
    }

    int sec = *(unsigned char *)data;
    if (sec)
    {
        sec = sec < 120 ? 120 : sec;
        led_timer_open();

        return POSIXTimerSet(net_access_timer_id, 0, sec);
    }
    else
    {
        POSIXTimerSet(net_access_timer_id, 0, 0);
        net_timer_close();
    }
    return 0;
}
//------------------------------
int hytoolClose(void)
{
    if (led_timer_id != NULL)
        POSIXTimerDelete(led_timer_id);
    if (net_access_timer_id != NULL)
        POSIXTimerDelete(net_access_timer_id);

    clientClose();
    return 0;
}

int hytoolOpen(void)
{
    registerCmdCb(net_access_timer, CMD_NETWORK_ACCESS_TIME);

    registerQuitCb(systemQuit);
    signalQuit();

    clientOpen();
    return 0;
}