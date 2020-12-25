#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"
#include "nativeFrame.h"

#include "hylink.h"
#include "zigbee.h"
#include "cpython.h"

int mainClose(void)
{
    runSystemCb(LAN_CLOSE);
    runSystemCb(ZIGBEE_CLOSE);
    runSystemCb(HYLINK_CLOSE);
    cpythonDestroy();
    return 0;
}

int main()
{
    cpythonInit();
    // return pythonTest();
    registerSystemCb(mainClose, SYSTEM_CLOSE);
    hylinkMain();
    zigbeeMain();
    logInfo("hylinkapp main start");
    nativeFrameOpen();
    mainClose();
    return 0;
}