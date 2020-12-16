#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nativeTimer.h"
#include "nativeSignal.h"
#include "nativeServer.h"
#include "frameCb.h"

static int nativeFrameClose(void)
{
    printf("nativeFrameClose\n");

    nativeServerEpollClose();

    return 0;
}

int nativeFrameOpen(void)
{
    printf("nativeFrameOpen\n");
    registerSystemCb(nativeTimerOpen, LED_DRIVER_TIMER_OPEN);
    registerSystemCb(nativeTimerClose, LED_DRIVER_TIMER_CLOSE);

    registerSystemCb(nativeFrameClose, LAN_CLOSE);
    registerTransferCb(nativeHylinkWrite, TRANSFER_HYLINK_WRITE);

    nativeSignal();
    nativeServerEpollMain();
    return 0;
}
