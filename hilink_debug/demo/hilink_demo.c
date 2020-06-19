/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HiLink SDK的demo程序
 * Create: 2019-03-05
 */
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>

#include "hilink.h"
#include "hilink_network_adapter.h"
#include "hilink_softap_adapter.h"

#include "net_info.h"
#include "protocol_cover.h"

typedef struct hilink_data
{
    char ip[INET_ADDRSTRLEN];
    char mac[18];
    char broadcastIp[INET_ADDRSTRLEN];
} hilink_data_t;

hilink_data_t hilink_data;

int main(void)
{
    // hilink_main();

    int ret = HILINK_GetLocalIp(hilink_data.ip, sizeof(hilink_data.ip));
    log_info("ip:%s\n", hilink_data.ip);
    ret = HILINK_GetMacAddr(hilink_data.mac, sizeof(hilink_data.mac));
    log_info("mac:%s\n", hilink_data.mac);
    ret = HILINK_GetBroadcastIp(hilink_data.broadcastIp, sizeof(hilink_data.broadcastIp));
    log_info("broadcastIp:%s\n", hilink_data.broadcastIp);
    get_local_all_ip(hilink_data.ip);

    read_from_bottom(0);
    write_to_bottom(0);
    printf("Program is started.\r\n");

    /* hilink main需要运行，sleep 1s保证进程不会退出 */
    while (1)
    {
        sleep(1);
        // hilink_msleep(1000);
    }
    return 0;
}