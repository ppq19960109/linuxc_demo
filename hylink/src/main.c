#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"

#include "hylink.h"

#include "hytool.h"
#include "log_api.h"
#ifndef ARCH
#include "rkDriver.h"
#endif // !ARCH
int mainClose(void)
{
    hytoolClose();

    hylinkClose();
#ifndef ARCH
    rkDriverClose();
#endif // !ARCH
    return 0;
}

int main()
{
#ifndef ARCH
    rkDriverOpen();
#endif // !ARCH
    // InitLog(LOG_DEBUG, NULL);
    registerSystemCb(mainClose, SYSTEM_CLOSE);
    hylinkOpen();

    logInfo("hylinkapp main start");
    hytoolOpen();
    mainClose();
    return 0;
}