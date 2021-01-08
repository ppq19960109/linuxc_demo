#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "frameCb.h"

static void nativeSignalHandler(int signal)
{
    printf("nativeSignalHandler signal is %d\n", signal);
    if (signal == SIGHUP || signal == SIGINT || signal == SIGQUIT || signal == SIGKILL || signal == SIGTERM)
    {
        runSystemCb(SYSTEM_CLOSE);
        exit(0);
    }
}

void nativeSignal()
{
    signal(SIGQUIT, nativeSignalHandler);
    signal(SIGKILL, nativeSignalHandler);
    signal(SIGTERM, nativeSignalHandler);

    struct sigaction act, oldact;
    act.sa_handler = nativeSignalHandler;
    sigemptyset(&act.sa_mask);

    // sigaddset(&act.sa_mask, SIGINT); //见注(1)
    // act.sa_flags = SA_NODEFER; //| SA_NODEFER 见注(2)SA_RESETHAND
    act.sa_flags = 0; //见注(3)

    sigaction(SIGINT, &act, &oldact);
    sigaction(SIGHUP, &act, &oldact);
}
