#include <stdio.h>

#include "frameCb.h"

static systemCb s_systemCb[SYSTEM_LAST];

void registerSystemCb(systemCb callback, SystemStatus status)
{
    s_systemCb[status] = callback;
}

int runSystemCb(SystemStatus status)
{
    if (s_systemCb[status] != NULL)
        return s_systemCb[status]();
    return -1;
}

static transferCb s_transferCb[TRANSFER_LAST];

void registerTransferCb(transferCb callback, TransferStatus status)
{
    s_transferCb[status] = callback;
}

int runTransferCb(void *data, unsigned int len, TransferStatus status)
{
    if (s_transferCb[status] != NULL)
        return s_transferCb[status](data, len);
    return -1;
}
