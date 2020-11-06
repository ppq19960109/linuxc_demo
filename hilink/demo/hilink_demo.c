/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HiLink SDK的demo程序
 * Create: 2019-03-05
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hilink.h"
#include "hilink_interface.h"
#include "hilink_log_manage.h"
#include "hilink_netconfig_mode_mgt.h"

#include "hilink_network_adapter.h"
#include "hilink_softap_adapter.h"

#include "local_device.h"
#include "cloud_send.h"

#include "local_callback.h"

#include "rk_driver.h"

void hilink_msleep(int);

int HILINK_GetSubProdId(char *subProdId, int len)
{
  log_info("HILINK_GetSubProdId\n");
  // strcpy(subProdId, "0A");
  // return 0;
  return -1;
}
int HILINK_rebootSoftware(void)
{
  log_info("HILINK_rebootSoftware\n");
  return 0;
}
int HILINK_rebootHardware(void)
{
  log_info("HILINK_rebootHardware\n");
  return 0;
}
int main(void)
{
  local_control_init();
  cloud_control_init();

  // HILINK_SdkAttr *sdkAttr = HILINK_GetSdkAttr();
  // if (sdkAttr == NULL)
  // {
  //   printf("sdk attr is null\r\n");
  //   return -1;
  // }
  // sdkAttr->rebootSoftware = HILINK_rebootSoftware;
  // sdkAttr->rebootHardware = HILINK_rebootHardware;
  // HILINK_SetSdkAttr(*sdkAttr);
  HILINK_SetScheduleInterval(100);
  HILINK_SetLogLevel(HILINK_LOG_CRIT);
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
  hilink_main();

  if (!HILINK_IsRegister())
  {
    set_cloud_status(CLOUD_REGISTERED);
    log_debug("main,HILINK_IsRegister:\n");
  }

  driver_keyOpen();
  run_openCallback();
  while (1)
  {
    hilink_msleep(1000);
  }
  local_system_restartOrReFactory(INT_OFFLINE);

  return 0;
}