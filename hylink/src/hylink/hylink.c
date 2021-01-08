#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"
#include "cJSON.h"
#include "commonFunc.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "hylinkListFunc.h"
#include "hylinkSend.h"

typedef struct
{
  unsigned char reportBuf[4096];
  char mac[24];

} hylinkHandle_t;

hylinkHandle_t hylinkHandle;

unsigned char *getHylinkSendBuf(void)
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
int hylinkNetAccess(void *data, void *data2)
{
  int sec = *(unsigned char *)data;

  HylinkSend hylinkSend = {0};
  hylinkSend.Command = 1;
  strcpy(hylinkSend.Type, STR_ADD);
  hylinkSend.DataSize = 1;
  HylinkSendData hylinkSendData = {0};
  hylinkSend.Data = &hylinkSendData;

  strcpy(hylinkSendData.DeviceId, STR_GATEWAY_DEVID);
  strcpy(hylinkSendData.ModelId, STR_GATEWAY_MODELID);
  strcpy(hylinkSendData.Key, STR_TIME);
  sprintf(hylinkSendData.Value, "%d", sec);

  return hylinkSendFunc(&hylinkSend);
}

int hylinkSendReset(void)
{
  HylinkSend hylinkSend = {0};
  hylinkSend.Command = 0;
  strcpy(hylinkSend.Type, STR_REFACTORY);
  hylinkSendFunc(&hylinkSend);
  //-------------------------------
  hylinkSend.Command = 1;

  hylinkSendFunc(&hylinkSend);
  sleep(2);
  system("sh /userdata/app/restore.sh alink &");
  return 0;
}
//--------------------------------------------------------
int hylinkClose(void)
{
  hylinkListEmpty();
  return 0;
}

void hylinkMain(void)
{
  registerSystemCb(hylinkClose, HYLINK_CLOSE);
  registerSystemCb(hylinkSendReset, SYSTEM_RESET);

  registerTransferCb(hylinkRecvManage, TRANSFER_SERVER_HYLINK_READ);
  registerTransferCb(hylinkRecvManage, TRANSFER_SERVER_ZIGBEE_READ);
  registerCmdCb(hylinkNetAccess, CMD_NETWORK_ACCESS);
  hylinkListInit();
}
