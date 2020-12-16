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
//--------------------------------------------------------
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
//--------------------------------------------------------
static CmdCb s_cmdCb[CMD_LAST];

void registerCmdCb(CmdCb callback, CmdStatus status)
{
    s_cmdCb[status] = callback;
}

int runCmdCb(void *data, CmdStatus status)
{
    if (s_cmdCb[status] != NULL)
        return s_cmdCb[status](data);
    return -1;
}
//--------------------------------------------------------
static ZigbeeCb s_zigbeeCb[ZIGBEE_LAST];

void registerZigbeeCb(ZigbeeCb callback, ZigbeeStatus status)
{
    s_zigbeeCb[status] = callback;
}

int runZigbeeCb(void *data1, void *data2, void *data3, void *data4, ZigbeeStatus status)
{
    if (s_zigbeeCb[status] != NULL)
        return s_zigbeeCb[status](data1, data2, data3, data4);
    return -1;
}
