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

#include "cloud_receive.h"
#include "cloud_send.h"
#include "uv_main.h"
#include "event_main.h"

const char *report_json[] = {
    //单键智能开关
    // "{\
    //    \"Command\":\"Report\",\
    //    \"FrameNumber\":\"00\",\
    //    \"GatewayId\" :\"0006D12345678909\",\
    //    \"Type\":\"Register\",\
    //    \"Data\":[\
    //      {\
    //           \"DeviceId\":\"4574567876543675\",\
    //           \"ModelId\":\"HY0095\",\
    //           \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
    //   }\
    // ]\
    // }",
    //双键智能开关
    // "{\
    //    \"Command\":\"Report\",\
    //    \"FrameNumber\":\"00\",\
    //    \"GatewayId\" :\"0006D12345678909\",\
    //    \"Type\":\"Register\",\
    //    \"Data\":[\
    //      {\
    //           \"DeviceId\":\"5234567876543432\",\
    //           \"ModelId\":\"HY0096\",\
    //           \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
    //   }\
    // ]\
    // }@",
    //三键智能开关
    // "{\        
    //    \"Command\":\"Report\",\
    //    \"FrameNumber\":\"00\",\
    //    \"GatewayId\" :\"0006D12345678909\",\
    //    \"Type\":\"Register\",\
    //    \"Data\":[\
    //      {\
    //           \"DeviceId\":\"1234567876543670\",\
    //           \"ModelId\":\"HY0097\",\
    //           \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
    //   }\
    // ]\
    // }",
    //DLT液晶调光器
    // "{\
    //    \"Command\":\"Report\",\
    //    \"FrameNumber\":\"00\",\
    //    \"GatewayId\" :\"0006D12345678909\",\
    //    \"Type\":\"Register\",\
    //    \"Data\":[\
    //      {\
    //           \"DeviceId\":\"2234567876543671\",\
    //           \"ModelId\":\"09223f\",\
    //           \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
    //   }\
    // ]\
    // }",
    //1路智能开关模块
    // "{\
    //    \"Command\":\"Report\",\
    //    \"FrameNumber\":\"00\",\
    //    \"GatewayId\" :\"0006D12345678909\",\
    //    \"Type\":\"Register\",\
    //    \"Data\":[\
    //      {\
    //           \"DeviceId\":\"5234564376543432\",\
    //           \"ModelId\":\"HY0121\",\
    //           \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
    //   }\
    // ]\
    // }4",
    //2路智能开关模块
    // "{\
    //    \"Command\":\"Report\",\
    //    \"FrameNumber\":\"00\",\
    //    \"GatewayId\" :\"0006D12345678909\",\
    //    \"Type\":\"Register\",\
    //    \"Data\":[\
    //      {\
    //           \"DeviceId\":\"523455676543432\",\
    //           \"ModelId\":\"HY0122\",\
    //           \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
    //   }\
    // ]\
    // }12",
    //3路智能开关模块
    // "{\
    //    \"Command\":\"Report\",\
    //    \"FrameNumber\":\"00\",\
    //    \"GatewayId\" :\"0006D12345678909\",\
    //    \"Type\":\"Register\",\
    //    \"Data\":[\
    //      {\
    //           \"DeviceId\":\"3234567876543673\",\
    //           \"ModelId\":\"HY0107\",\
    //           \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
    //   }\
    // ]\
    // }",
    //门磁传感器
    "{\
       \"Command\":\"Report\",\
       \"FrameNumber\":\"00\",\
       \"GatewayId\" :\"0006D12345678909\",\
       \"Type\":\"Register\",\
       \"Data\":[\
         {\
              \"DeviceId\":\"4234567876543674\",\
              \"ModelId\":\"HY0093\",\
              \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
      }\
    ]\
    }",
    // "{\
    //    \"Command\":\"Report\",\
    //    \"FrameNumber\":\"00\",\
    //    \"GatewayId\" :\"0006D12345678909\",\
    //    \"Type\":\"Register\",\
    //    \"Data\":[\
    //      {\
    //           \"DeviceId\":\"5234567876543675\",\
    //           \"ModelId\":\"_TZE200_twuagcv5\",\
    //           \"Secret\":\"kYulH7PhgrI44IcsesSJqkLbufGbUPjkNF2sImWm\"\
    //   }\
    // ]\
    // }",

};

void hilink_msleep(int);

int main(void)
{
  hilink_main();
  printf("Program is started.\n");

  // HiLinkSetGatewayMode(1);
  HILINK_SetLogLevel(HILINK_LOG_ERR);
  HILINK_EnableProcessDelErrCode(1);
  HILINK_SetNetConfigMode(HILINK_NETCONFIG_NONE); //HILINK_NETCONFIG_NONE
  // enum HILINK_NetConfigMode net_mode = HILINK_GetNetConfigMode();
  // log_debug("HILINK_NetConfigMode:%d\n", net_mode);
  // int devstatus = hilink_get_devstatus();
  // log_debug("hilink_get_devstatus:%d\n", devstatus);

  // HILINK_SetWiFiInfo("HUAWEI-WDNJ4L", 1, "1234567890", 1);
  // HILINK_StartSoftAp("rk_net", 0);
  // HILINK_DisconnectStation("10.201.126.157");
  //-------------------------------------------------
  /* hilink main需要运行，sleep 1s保证进程不会退出 */
  local_control_init(&g_SLocalControl);
  cloud_control_init(&g_SCloudControl);
  for (int i = 0; i < sizeof(report_json) / sizeof(report_json[0]); i++)
    read_from_local(report_json[i], local_get_list_head(&g_SLocalControl));
    
#if USE_LIBEVENT
  printf("libevent is start.\n");
  event_main();
#elif USE_LIBUV
  printf("libuv is start.\n");
  main_open();
#else
  while (1)
  {
    hilink_msleep(1000);
  }
  cloud_restart_reFactory(INT_RESTART);
#endif
  return 0;
}