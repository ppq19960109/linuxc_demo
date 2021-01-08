#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "signalQuit.h"
static Quit_cb quit_cb = NULL;

void registerQuitCb(Quit_cb quitCb)
{
    quit_cb = quitCb;
}

static void signalHandler(int signal)
{
    printf("signalHandler signal is %d\n", signal);
    if (signal == SIGHUP || signal == SIGINT || signal == SIGQUIT || signal == SIGKILL || signal == SIGTERM)
    {
        if (quit_cb != NULL)
            quit_cb();
        exit(0);
    }
}

void signalQuit(void)
{
    signal(SIGQUIT, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGTERM, signalHandler);

    struct sigaction act, oldact;
    act.sa_handler = signalHandler;
    sigemptyset(&act.sa_mask);

    // sigaddset(&act.sa_mask, SIGINT); //见注(1)
    // act.sa_flags = SA_NODEFER; //| SA_NODEFER 见注(2)SA_RESETHAND
    act.sa_flags = 0; //见注(3)

    sigaction(SIGINT, &act, &oldact);
    sigaction(SIGHUP, &act, &oldact);
}
