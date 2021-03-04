/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HiLink SDK的demo程序
 * Create: 2019-03-05
 */
#include "main.h"
#include "cloudLink.h"

void hilink_msleep(int);

int HILINK_GetSubProdId(char *subProdId, int len)
{
  printf("HILINK_GetSubProdId\n");
  return -1;
}
int HILINK_rebootSoftware(void)
{
  printf("HILINK_rebootSoftware\n");
  return 0;
}
int HILINK_rebootHardware(void)
{
  printf("HILINK_rebootHardware\n");
  return 0;
}

static int main_close(void)
{
  cloudLinkClose();
  exit(0);
  return 0;
}
int main(void)
{

  //-------------------------------------------------
  HILINK_SdkAttr *sdkAttr = HILINK_GetSdkAttr();
  if (sdkAttr == NULL)
  {
    printf("hilink sdk attr is null\n");
    return -1;
  }
  printf("hilink sdk attr deviceMainTaskStackSize:%ld\n", sdkAttr->deviceMainTaskStackSize);
  printf("hilink sdk attr bridgeMainTaskStackSize:%ld\n", sdkAttr->bridgeMainTaskStackSize);
  printf("hilink sdk attr otaCheckTaskStackSize:%ld\n", sdkAttr->otaCheckTaskStackSize);
  printf("hilink sdk attr otaUpdateTaskStackSize:%ld\n", sdkAttr->otaUpdateTaskStackSize);

  // sdkAttr->rebootSoftware = HILINK_rebootSoftware;
  // sdkAttr->rebootHardware = HILINK_rebootHardware;
  // HILINK_SetSdkAttr(*sdkAttr);
  //-------------------------------------------------
  HILINK_SetScheduleInterval(100);
  HILINK_SetLogLevel(HILINK_LOG_WARN);
  HILINK_EnableProcessDelErrCode(1);
  HILINK_SetNetConfigMode(HILINK_NETCONFIG_NONE); //HILINK_NETCONFIG_NONE
  //-------------------------------------------------
  /* hilink main需要运行，sleep 1s保证进程不会退出 */
  hilink_main();
  //-------------------------------------------------
  registerSystemCb(main_close, SYSTEM_CLOSE);
  cloudLinkOpen();
  hylinkOpen();

  while (1)
  {
    hilink_msleep(1000);
  }

  return 0;
}