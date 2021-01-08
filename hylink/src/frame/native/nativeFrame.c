#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nativeTimer.h"
#include "nativeSignal.h"
#include "nativeServer.h"
#include "nativeClient.h"
#include "frameCb.h"

#define SERVER

static int nativeFrameClose(void)
{
    printf("nativeFrameClose\n");
    runSystemCb(LED_DRIVER_TIMER_CLOSE);

#ifdef SERVER
    nativeServerCLose();
#else
    nativeClientClose();
#endif
    return 0;
}

int nativeFrameOpen(void)
{
    printf("nativeFrameOpen\n");
    registerSystemCb(nativeTimerOpen, LED_DRIVER_TIMER_OPEN);
    registerSystemCb(nativeTimerClose, LED_DRIVER_TIMER_CLOSE);

    registerSystemCb(nativeFrameClose, LAN_CLOSE);
    registerCmdCb(nativeNetTimer, CMD_NETWORK_ACCESS_TIME);
    nativeSignal();

#ifdef SERVER
    nativeServerOpen();
#else
    nativeClientOpen();
#endif

    return 0;
}
