/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HiLink SDK的demo程序
 * Create: 2019-03-05
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "hilink.h"
#include "hilink_interface.h"
#include "hilink_log_manage.h"
#include "hilink_netconfig_mode_mgt.h"

#include "hilink_network_adapter.h"
#include "hilink_softap_adapter.h"
#include "hilink_profile_bridge.h"
#include "hilink_sdk_adapter.h"

#include "protocol_cover.h"
#include "hilink_cover.h"

void hilink_msleep(int);

extern const char *report_json[];


void signal_handler(int signal)
{
    printf("signal is %d\n", signal);
    if (signal == SIGINT || signal == SIGQUIT || signal == SIGKILL || signal == SIGTERM)
    {
        hilink_process_before_restart(1);
        printf(" exit(0)\n");
        exit(0);
    }
}

int main(void)
{

    signal(SIGQUIT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGTERM, signal_handler);

    struct sigaction act, oldact;
    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    // sigaddset(&act.sa_mask, SIGQUIT); //见注(1)
    act.sa_flags = SA_RESETHAND | SA_NODEFER; //见注(2)
    // act.sa_flags = 0; //见注(3)

    sigaction(SIGINT, &act, &oldact);

    //----------------------------------------------------


    hilink_main();
    printf("Program is started.\r\n");
    
    // HILINK_SdkAttr *SdkAttr = HILINK_GetSdkAttr();
    // log_debug("HILINK_SdkAttr monitorTaskStackSize:%d,deviceMainTaskStackSize:%d,bridgeMainTaskStackSize:%d",
    //           SdkAttr->monitorTaskStackSize, SdkAttr->deviceMainTaskStackSize, SdkAttr->bridgeMainTaskStackSize);
    // HiLinkSetGatewayMode(1);
    HILINK_SetLogLevel(HILINK_LOG_EMERG);
    HILINK_EnableProcessDelErrCode(1);
    HILINK_SetNetConfigMode(HILINK_NETCONFIG_NONE); //HILINK_NETCONFIG_NONE
    enum HILINK_NetConfigMode net_mode = HILINK_GetNetConfigMode();
    log_debug("HILINK_NetConfigMode:%d", net_mode);
    int devstatus = hilink_get_devstatus();
    log_debug("hilink_get_devstatus:%d", devstatus);

    // HILINK_SetWiFiInfo("HUAWEI-WDNJ4L", 1, "1234567890", 1);
    // HILINK_StartSoftAp("rk_net", 0);
    // HILINK_DisconnectStation("10.201.126.157");
    //-------------------------------------------------
    protlcol_init();
    hilink_handle_init();

    /* hilink main需要运行，sleep 1s保证进程不会退出 */
    // for (int i = 0; i < 1; i++)
    //     read_from_local(report_json[i]);

    while (1)
    {
        hilink_msleep(1000);
    }
    hilink_process_before_restart(1);

    return 0;
}