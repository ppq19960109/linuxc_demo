#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"

#include "hylink.h"
#include "zigbee.h"
#include "cpython.h"
#include "heartbeat.h"

#include "hytool.h"

static int mainClose(void)
{
    hytoolClose();
    runSystemCb(ZIGBEE_CLOSE);
    runSystemCb(HYLINK_CLOSE);
    cpythonDestroy();
    return 0;
}

int main()
{
    cpythonInit();
    // return pythonTest();
    registerSystemCb(heartbeat, SYSTEM_HEARTBEAT);
    registerSystemCb(mainClose, SYSTEM_CLOSE);
    hylinkMain();
    zigbeeMain();
    logInfo("tuyazigbee app main start");
    hytoolOpen();
    while (1)
    {
        sleep(1);
    }
    mainClose();
    return 0;
}