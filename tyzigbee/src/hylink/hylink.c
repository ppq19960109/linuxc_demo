#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"
#include "cJSON.h"
#include "commonFunc.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "hylinkListFunc.h"
#include "hylinkReport.h"

#include "zigbeeListFunc.h"

typedef struct
{
    unsigned char reportBuf[2048];
    char mac[24];
} hylinkHandle_t;

hylinkHandle_t hylinkHandle;

unsigned char *getHylinkReportBuf(void)
{
    return hylinkHandle.reportBuf;
}

int hylinkDevJoin(void *devId, void *modelId, void *version, void *manuName)
{
    if (zigbeeListGet(manuName) == NULL)
    {
        logError("modelId is null");
        return -1;
    }

    HylinkDev *hyDev = (HylinkDev *)malloc(sizeof(HylinkDev));
    memset(hyDev, 0, sizeof(HylinkDev));
    strcpy(hyDev->DeviceId, devId);
    strcpy(hyDev->ModelId, manuName);
    strcpy(hyDev->Version, version);
    hylinkListAdd(hyDev);

    HylinkReport hylinkReport = {0};
    HylinkReportData hylinkReportData = {0};
    hylinkReport.Data = &hylinkReportData;
    strcpy(hylinkReport.Type, STR_REGISTER);
    hylinkReport.DataSize = 1;

    strcpy(hylinkReportData.DeviceId, hyDev->DeviceId);
    strcpy(hylinkReportData.ModelId, hyDev->ModelId);

    hylinkReportFunc(&hylinkReport);

    strcpy(hylinkReport.Type, STR_ATTRIBUTE);
    strcpy(hylinkReportData.Key, STR_VERSION);
    strcpy(hylinkReportData.Value, hyDev->Version);
    hylinkReportFunc(&hylinkReport);

    runZigbeeCb((void *)hyDev->DeviceId, (void *)hyDev->ModelId, NULL, NULL, ZIGBEE_DEV_DISPATCH);
    return 0;
}

int hylinkOnlineFresh(void *devId, void *null, void *version, void *null1)
{
    HylinkDev *hyDev = (HylinkDev *)hylinkListGet(devId);
    if (hyDev == NULL)
    {
        logError("hyDev is null");
        runCmdCb(devId, CMD_DELETE_DEV);
        return -1;
    }
    strcpy(hyDev->Version, version);

    HylinkReport hylinkReport = {0};
    HylinkReportData hylinkReportData = {0};
    hylinkReport.Data = &hylinkReportData;
    hylinkReport.DataSize = 1;

    strcpy(hylinkReportData.DeviceId, hyDev->DeviceId);
    strcpy(hylinkReportData.ModelId, hyDev->ModelId);

    strcpy(hylinkReport.Type, STR_ONOFF);
    strcpy(hylinkReportData.Key, STR_ONLINE);
    strcpy(hylinkReportData.Value, "1");
    hylinkReportFunc(&hylinkReport);

    strcpy(hylinkReport.Type, STR_ATTRIBUTE);
    strcpy(hylinkReportData.Key, STR_VERSION);
    strcpy(hylinkReportData.Value, hyDev->Version);
    hylinkReportFunc(&hylinkReport);
    return 0;
}

int hylinkDevLeave(void *devId, void *null, void *null1, void *null2)
{
    HylinkDev *hyDev = (HylinkDev *)hylinkListGet(devId);
    if (hyDev == NULL)
    {
        logError("hyDev is null");
        return -1;
    }

    HylinkReport hylinkReport = {0};
    HylinkReportData hylinkReportData = {0};
    hylinkReport.Data = &hylinkReportData;
    hylinkReport.DataSize = 1;

    strcpy(hylinkReport.Type, STR_UNREGISTER);

    strcpy(hylinkReportData.DeviceId, hyDev->DeviceId);

    hylinkReportFunc(&hylinkReport);
    hylinkListDel(hyDev->DeviceId);
    return 0;
}

int hylinkDevZclReport(void *devId, void *hyKey, void *data, void *datalen)
{
    HylinkDev *hyDev = (HylinkDev *)hylinkListGet(devId);
    if (hyDev == NULL)
    {
        logError("hyDev is null");
        return -1;
    }

    HylinkReport hylinkReport = {0};
    HylinkReportData hylinkReportData = {0};
    hylinkReport.Data = &hylinkReportData;
    hylinkReport.DataSize = 1;

    strcpy(hylinkReport.Type, STR_ATTRIBUTE);

    strcpy(hylinkReportData.DeviceId, devId);
    strcpy(hylinkReportData.ModelId, hyDev->ModelId);

    strcpy(hylinkReportData.Key, hyKey);
    strcpy(hylinkReportData.Value, data);
    hylinkReportFunc(&hylinkReport);

    return 0;
}
//-----------------------------------------------------
static int hylinkReportDevInfo(void)
{
    HylinkReport hylinkReport = {0};
    HylinkReportData hylinkReportData = {0};
    hylinkReport.Data = &hylinkReportData;
    hylinkReport.DataSize = 1;

    strcpy(hylinkReport.Type, STR_DEVSINFO);
    strcpy(hylinkReportData.DeviceId, STR_GATEWAY_DEVID);
    strcpy(hylinkReportData.Key, STR_DEVSINFO);

    return hylinkReportFunc(&hylinkReport);
}
//--------------------------------------------------------
int hylinkClose(void)
{
    hylinkListEmpty();
    return 0;
}

void hylinkMain(void)
{
    registerSystemCb(hylinkClose, HYLINK_CLOSE);
    registerSystemCb(hylinkReportDevInfo, CMD_DEVSINFO);
    registerTransferCb(hylinkRecvManage, TRANSFER_SERVER_HYLINK_READ);
    registerZigbeeCb(hylinkDevJoin, ZIGBEE_DEV_JOIN);
    registerZigbeeCb(hylinkOnlineFresh, ZIGBEE_DEV_ONLINE);
    registerZigbeeCb(hylinkDevLeave, ZIGBEE_DEV_LEAVE);
    registerZigbeeCb(hylinkDevZclReport, ZIGBEE_DEV_REPORT);

    hylinkListInit();
}
