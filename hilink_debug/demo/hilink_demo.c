/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HiLink SDK的demo程序
 * Create: 2019-03-05
 */
#include <stdio.h>
#include <stdlib.h>

#include "hilink.h"
#include "hilink_interface.h"
#include "hilink_log_manage.h"
#include "hilink_netconfig_mode_mgt.h"

#include "hilink_network_adapter.h"
#include "hilink_softap_adapter.h"

#include "local_send.h"
#include "local_tcp_client.h"
#include "cloud_send.h"

void hilink_msleep(int);

extern const char *report_json[];

int main(void)
{
    hilink_main();
    printf("Program is started.\r\n");

    // HiLinkSetGatewayMode(1);
    HILINK_SetLogLevel(HILINK_LOG_ERR);
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
    local_control_init(&g_SLocalControl);
    cloud_control_init(&g_SCloudControl);
    main_thread_set_signal();
    /* hilink main需要运行，sleep 1s保证进程不会退出 */
    for (int i = 0; i < 6; i++)
        read_from_local(report_json[i], local_get_list_head(&g_SLocalControl));

    while (1)
    {
        hilink_msleep(1000);
    }
    local_restart_reFactory(false);

    return 0;
}