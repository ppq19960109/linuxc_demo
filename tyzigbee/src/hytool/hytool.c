#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "signalQuit.h"
#include "POSIXTimer.h"

#include "frameCb.h"

#include "client.h"
static timer_t heartTimerid;

static int systemQuit(void)
{
    runSystemCb(SYSTEM_CLOSE);
    return 0;
}
static void timerThreadHandler(union sigval v)
{
    runSystemCb(SYSTEM_HEARTBEAT);
}
//------------------------------
int hytoolClose(void)
{
    if (heartTimerid != NULL)
        POSIXTimerDelete(heartTimerid);
    clientClose();
    return 0;
}

int hytoolOpen(void)
{
    registerQuitCb(systemQuit);
    signalQuit();

    heartTimerid = POSIXTimerCreate(1, timerThreadHandler);
    POSIXTimerSet(heartTimerid, 300, 300);

    clientOpen();
    return 0;
}