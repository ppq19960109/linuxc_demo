#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

#include "tuya_gw_infra_api.h"
#include "tuya_gw_z3_api.h"
#include "tuya_gw_dp_api.h"

#include "tyZigbee.h"
#include "tyZigbeeZcl3.h"

#include "frameCb.h"
#include "logFunc.h"
#include "commonFunc.h"

#include "hylink.h"
#include "zigbee.h"
#include "zigbeeManage.h"
#include "zigbeeReport.h"
#include "zigbeeDispatch.h"
#include "zigbeeListFunc.h"
#include "zigbeeProfile.h"

static int zigbeeCmdNetAccess(void *cmd)
{
    logDebug("zigbeeCmdNetAccess");
    if (cmd == NULL)
        return -1;
    unsigned char time = *(unsigned char *)cmd;
    bool permit;
    if (time)
    {
        permit = true;
    }
    else
        permit = false;

    int ret = 0;

    ret = tuya_user_iot_permit_join(permit);
    if (ret != 0)
    {
        logError("tuya_user_iot_permit_join error, ret: %d", ret);
    }
    else
    {
        runCmdCb(&time, CMD_HYLINK_NETWORK_ACCESS);
    }

    return ret;
}

static int zigbeeCmdDelDev(void *id)
{
    logDebug("zigbeeCmdDelDev");
    int ret = 0;

    ret = tuya_user_iot_z3_dev_del((const char *)id);
    if (ret != 0)
    {
        logError("tuya_user_iot_z3_dev_del error, ret: %d", ret);
    }
    return ret;
}

int zigbeeClose(void)
{
    zigbeeListEmpty();
    return 0;
}

void zigbeeOpen(void)
{
    registerCmdCb(zigbeeCmdNetAccess, CMD_NETWORK_ACCESS);
    registerCmdCb(zigbeeCmdDelDev, CMD_DELETE_DEV);
    registerCmdCb(zigbeeZclReport, CMD_ZCL_FRAME_REPORT);
    registerZigbeeCb(zigbeeZclDispatch, ZIGBEE_DEV_DISPATCH);

    zigbeeListInit();

    readFileList(ZIGBEE_PROFILE_PATH, readProfileFile);

    tyZigbeeInit();
    // zigbeeListPrintf();
    // tuya_user_iot_permit_join(true);
}
