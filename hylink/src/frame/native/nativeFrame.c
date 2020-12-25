#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nativeTimer.h"
#include "nativeSignal.h"
#include "nativeServer.h"
#include "nativeClient.h"
#include "frameCb.h"

// #define SERVER

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
    registerSystemCb(nativeLedTimerOpen, LED_DRIVER_TIMER_OPEN);
    registerSystemCb(nativeLedTimerClose, LED_DRIVER_TIMER_CLOSE);

    registerSystemCb(nativeFrameClose, LAN_CLOSE);
    nativeSignal();
#ifdef SERVER
    nativeServerOpen();
#else
    nativeClientOpen();
#endif

    return 0;
}
