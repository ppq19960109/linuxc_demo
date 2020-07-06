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

#include "net_info.h"
#include "protocol_cover.h"

// typedef struct hilink_data
// {
//     char ip[INET_ADDRSTRLEN];
//     char mac[18];
//     char broadcastIp[INET_ADDRSTRLEN];
// } hilink_data_t;

// hilink_data_t hilink_data;

void hilink_msleep(int);

int main(void)
{
    
    hilink_main();

    // int ret = HILINK_GetLocalIp(hilink_data.ip, sizeof(hilink_data.ip));
    // log_info("ip:%s\n", hilink_data.ip);
    // ret = HILINK_GetMacAddr(hilink_data.mac, sizeof(hilink_data.mac));
    // log_info("mac:%s\n", hilink_data.mac);
    // ret = HILINK_GetBroadcastIp(hilink_data.broadcastIp, sizeof(hilink_data.broadcastIp));
    // log_info("broadcastIp:%s\n", hilink_data.broadcastIp);
    // get_local_all_ip(hilink_data.ip);

    protlcol_init();
    
    // local_dev_t local_dev;
    // local_dev.FrameNumber=11;
    // strcpy(local_dev.GatewayId,"ed334ggeewe");
    // strcpy(local_dev.Type,"Ctrl");
    // strcpy(local_dev.Data.DeviceId,"123456787654310");
    // strcpy(local_dev.Data.ModelId,"500c32");
    // strcpy(local_dev.Data.Key,"Switch");

    // write_to_local(&local_dev);
    printf("Program is started.\r\n");

    /* hilink main需要运行，sleep 1s保证进程不会退出 */
    while (1)
    {
        hilink_msleep(1000);
    }
    protlcol_destory();
    return 0;
}