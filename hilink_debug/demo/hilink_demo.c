/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HiLink SDK的demo程序
 * Create: 2019-03-05
 */
#include "hilink.h"
#include "hilink_interface.h"
#include "hilink_log_manage.h"
#include "hilink_netconfig_mode_mgt.h"

#include "hilink_network_adapter.h"
#include "hilink_softap_adapter.h"
#include "hilink_profile_bridge.h"

#include "net_info.h"
#include "protocol_cover.h"
#include "hilink_cover.h"

void hilink_msleep(int);

extern const char *report_json[];
int main(void)
{
    hilink_main();

    protlcol_init();
    hilink_handle_init();

    log_error("get_netlink_status %d", get_netlink_status(ETH_NAME));
    printf("Program is started.\r\n");
    HILINK_SetLogLevel(HILINK_LOG_ERR);
    HILINK_SdkAttr *SdkAttr = HILINK_GetSdkAttr();
    log_debug("HILINK_SdkAttr monitorTaskStackSize:%d,deviceMainTaskStackSize:%d,bridgeMainTaskStackSize:%d",
              SdkAttr->monitorTaskStackSize, SdkAttr->deviceMainTaskStackSize, SdkAttr->bridgeMainTaskStackSize);
    HiLinkSetGatewayMode(1);
    HILINK_EnableProcessDelErrCode(1);
    HILINK_SetNetConfigMode(HILINK_NETCONFIG_NONE);
    enum HILINK_NetConfigMode net_mode = HILINK_GetNetConfigMode();
    log_debug("HILINK_NetConfigMode:%d", net_mode);
    int devstatus = hilink_get_devstatus();
    log_debug("hilink_get_devstatus:%d", devstatus);
    //-------------------------------------------------

    // HilinkSyncBrgDevStatus("2389294",DEV_ONLINE);
    for (int i = 0; i < 5; i++)
        read_from_local(report_json[i]);
    /* hilink main需要运行，sleep 1s保证进程不会退出 */
    int i = 0;
    while (1)
    {
        hilink_msleep(1000);
    }
    hilink_handle_destory();
    protlcol_destory();
    return 0;
}