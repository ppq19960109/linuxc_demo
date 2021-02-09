#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"

#include "hylink.h"
#include "rkDriver.h"

#include "hytool.h"
int mainClose(void)
{
    hytoolClose();
 
    runSystemCb(HYLINK_CLOSE);
    runSystemCb(RK_DRIVER_CLOSE);
    return 0;
}

int main()
{
    registerSystemCb(mainClose, SYSTEM_CLOSE);

    hylinkMain();
    rkDriverOpen();
    logInfo("hylinkapp main start");
    hytoolOpen();
    mainClose();
    return 0;
}