#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "frameCb.h"
#include "logFunc.h"
#include "cJSON.h"
#include "commonFunc.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "hylinkListFunc.h"
#include "hylinkSend.h"

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

/*********************************************************************************
  *Function:  hylinkDevJoin
  * Description： report zigbee device registriation information
  *Input:  
    devId:device id
    modelId:invalid parameter
    version:version information
    manuName:tuya zigbee device model id
  *Return:  0:success -1:fail
**********************************************************************************/
static int hylinkDevJoin(void *devId, void *modelId, void *version, void *manuName)
{
    if (zigbeeListGet(manuName) == NULL)
    {
        logError("modelId is null");
        runCmdCb(devId, CMD_DELETE_DEV);
        return -1;
    }

    HylinkDev *hyDev = (HylinkDev *)malloc(sizeof(HylinkDev));
    memset(hyDev, 0, sizeof(HylinkDev));
    strcpy(hyDev->DeviceId, devId);
    strcpy(hyDev->ModelId, manuName);
    strcpy(hyDev->Version, version);
    hylinkListAdd(hyDev);

    HylinkSend hylinkSend = {0};
    HylinkSendData hylinkSendData = {0};
    hylinkSend.Data = &hylinkSendData;
    strcpy(hylinkSend.Type, STR_REGISTER);
    hylinkSend.DataSize = 1;

    strcpy(hylinkSendData.DeviceId, hyDev->DeviceId);
    strcpy(hylinkSendData.ModelId, hyDev->ModelId);

    hylinkSendFunc(&hylinkSend);

    strcpy(hylinkSend.Type, STR_ATTRIBUTE);
    strcpy(hylinkSendData.Key, STR_VERSION);
    strcpy(hylinkSendData.Value, hyDev->Version);
    hylinkSendFunc(&hylinkSend);

    runZigbeeCb((void *)hyDev->DeviceId, (void *)hyDev->ModelId, NULL, NULL, ZIGBEE_DEV_DISPATCH);
    return 0;
}

static int hylinkOnlineFresh(void *devId, void *null, void *version, void *null1)
{
    HylinkDev *hyDev = (HylinkDev *)hylinkListGet(devId);
    if (hyDev == NULL)
    {
        logError("hyDev is null");
        // runCmdCb(devId, CMD_DELETE_DEV);
        return -1;
    }

    hyDev->activeTime = time(NULL);
    //-------------------------------
    strcpy(hyDev->Version, version);

    HylinkSend hylinkSend = {0};
    HylinkSendData hylinkSendData = {0};
    hylinkSend.Data = &hylinkSendData;
    hylinkSend.DataSize = 1;

    strcpy(hylinkSendData.DeviceId, hyDev->DeviceId);
    strcpy(hylinkSendData.ModelId, hyDev->ModelId);

    strcpy(hylinkSend.Type, STR_ONOFF);
    strcpy(hylinkSendData.Key, STR_ONLINE);
    strcpy(hylinkSendData.Value, "1");
    hylinkSendFunc(&hylinkSend);

    strcpy(hylinkSend.Type, STR_ATTRIBUTE);
    strcpy(hylinkSendData.Key, STR_VERSION);
    strcpy(hylinkSendData.Value, hyDev->Version);
    hylinkSendFunc(&hylinkSend);
    return 0;
}

static int hylinkDevLeave(void *devId, void *null, void *null1, void *null2)
{
    HylinkDev *hyDev = (HylinkDev *)hylinkListGet(devId);
    if (hyDev == NULL)
    {
        logError("hyDev is null");
        return -1;
    }

    HylinkSend hylinkSend = {0};
    HylinkSendData hylinkSendData = {0};
    hylinkSend.Data = &hylinkSendData;
    hylinkSend.DataSize = 1;

    strcpy(hylinkSend.Type, STR_UNREGISTER);

    strcpy(hylinkSendData.DeviceId, hyDev->DeviceId);

    hylinkSendFunc(&hylinkSend);
    hylinkListDel(hyDev->DeviceId);
    return 0;
}

static int hylinkDevZclReport(void *devId, void *hyKey, void *data, void *datalen)
{
    HylinkDev *hyDev = (HylinkDev *)hylinkListGet(devId);
    if (hyDev == NULL)
    {
        logError("hyDev is null");
        return -1;
    }

    HylinkSend hylinkSend = {0};
    HylinkSendData hylinkSendData = {0};
    hylinkSend.Data = &hylinkSendData;
    hylinkSend.DataSize = 1;

    strcpy(hylinkSend.Type, STR_ATTRIBUTE);

    strcpy(hylinkSendData.DeviceId, devId);
    strcpy(hylinkSendData.ModelId, hyDev->ModelId);

    strcpy(hylinkSendData.Key, hyKey);
    strcpy(hylinkSendData.Value, data);
    hylinkSendFunc(&hylinkSend);

    return 0;
}
//-----------------------------------------------------
static int hylinkReportDevInfo(void)
{
    HylinkSend hylinkSend = {0};
    HylinkSendData hylinkSendData = {0};
    hylinkSend.Data = &hylinkSendData;
    hylinkSend.DataSize = 1;

    strcpy(hylinkSend.Type, STR_DEVSINFO);
    strcpy(hylinkSendData.DeviceId, STR_GATEWAY_DEVID);
    strcpy(hylinkSendData.Key, STR_DEVSINFO);

    return hylinkSendFunc(&hylinkSend);
}

static int hylinkReportNetAccess(void *data)
{
    unsigned char sec = *(unsigned char *)data;
    HylinkSend hylinkSend = {0};
    HylinkSendData hylinkSendData = {0};
    hylinkSend.Data = &hylinkSendData;
    hylinkSend.DataSize = 1;

    strcpy(hylinkSend.Type, STR_ATTRIBUTE);

    strcpy(hylinkSendData.DeviceId, STR_GATEWAY_DEVID);
    strcpy(hylinkSendData.ModelId, STR_GATEWAY_MODELID);

    strcpy(hylinkSendData.Key, STR_TIME);

    sprintf(hylinkSendData.Value, "%d", sec);

    return hylinkSendFunc(&hylinkSend);
}

static int hylinkZigbeeChannel(void)
{
    char buf[64] = {0};
    int ret = popenRun("grep -i 'channel' storage/zigbeeNetInfo.txt", "r", buf, sizeof(buf));
    if (ret < 0)
        return -1;
    long num = 0;
    strToNum(strchr(buf, ':') + 1, 10, &num);
    logWarn("popenRun: %s,%d", buf, num);

    //-------------------
    HylinkSend hylinkSend = {0};
    HylinkSendData hylinkSendData = {0};
    hylinkSend.Data = &hylinkSendData;
    strcpy(hylinkSend.Type, STR_ATTRIBUTE);
    hylinkSend.DataSize = 1;

    strcpy(hylinkSendData.DeviceId, STR_GATEWAY_DEVID);
    strcpy(hylinkSendData.ModelId, STR_GATEWAY_MODELID);

    strcpy(hylinkSendData.Key, "ZB_Channel");
    sprintf(hylinkSendData.Value, "%ld", num);

    return hylinkSendFunc(&hylinkSend);
}
//--------------------------------------------------------
static int hylinkClose(void)
{
    hylinkListEmpty();
    return 0;
}

void hylinkMain(void)
{
    registerSystemCb(hylinkClose, HYLINK_CLOSE);
    registerSystemCb(hylinkZigbeeChannel, HYLINK_ZB_CHANNEL);
    registerSystemCb(hylinkReportDevInfo, HYLINK_DEVSINFO);

    registerCmdCb(hylinkReportNetAccess, CMD_HYLINK_NETWORK_ACCESS);

    registerTransferCb(hylinkRecvManage, TRANSFER_CLIENT_READ);

    registerZigbeeCb(hylinkDevJoin, ZIGBEE_DEV_JOIN);
    registerZigbeeCb(hylinkOnlineFresh, ZIGBEE_DEV_ONLINE);
    registerZigbeeCb(hylinkDevLeave, ZIGBEE_DEV_LEAVE);
    registerZigbeeCb(hylinkDevZclReport, ZIGBEE_DEV_REPORT);

    hylinkListInit();
}
