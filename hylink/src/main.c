#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"
#include "nativeFrame.h"

#include "database.h"
#include "hylink.h"
#include "rkDriver.h"

int mainClose(void)
{
    runSystemCb(LAN_CLOSE);
    runSystemCb(HYLINK_CLOSE);
    runSystemCb(DATABASE_CLOSE);
    runSystemCb(RK_DRIVER_CLOSE);
    return 0;
}

int main()
{
    registerSystemCb(mainClose, SYSTEM_CLOSE);

    hylinkMain();
    databaseInit();
    rkDriverOpen();
    logInfo("hylinkapp main start");
    nativeFrameOpen();
    mainClose();
    return 0;
}