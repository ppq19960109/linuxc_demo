#include <stdio.h>

#include "local_callback.h"

static openCallback_t openCallback = NULL;

void register_openCallback(openCallback_t callback)
{
    openCallback = callback;
}

void run_openCallback(void)
{
    if (openCallback != NULL)
        openCallback();
}

static writeCallback_t writeCallback = NULL;

void register_writeCallback(writeCallback_t callback)
{
    writeCallback = callback;
}

int run_writeCallback(char *data, unsigned int len)
{
    if (writeCallback != NULL)
        return writeCallback(data, len);
    return -1;
}

static closeCallback_t closeCallback = NULL;

void register_closeCallback(closeCallback_t callback)
{
    closeCallback = callback;
}

void run_closeCallback(void)
{
    if (closeCallback != NULL)
        closeCallback();
}