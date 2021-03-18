#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"

#include "hylink.h"
#include "rkDriver.h"

#include "hytool.h"
#include "log_api.h"
int mainClose(void)
{
    hytoolClose();

    hylinkClose();
    rkDriverClose();
    return 0;
}

int main()
{
    // InitLog(LOG_DEBUG, NULL);
    registerSystemCb(mainClose, SYSTEM_CLOSE);
    rkDriverOpen();
    hylinkOpen();

    logInfo("hylinkapp main start");
    hytoolOpen();
    mainClose();
    return 0;
}