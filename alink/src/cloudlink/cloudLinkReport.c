#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "logFunc.h"
#include "commonFunc.h"
#include "frameCb.h"
#include "hylinkListFunc.h"
#include "hylinkSubDev.h"
#include "cloudLinkReport.h"
#include "cloudLink.h"

#include "linkkit_subdev.h"

static char *s_cloudGateway[] = {"powerstate"};
const SAttrInfo g_GatewaAttr = {
    .attr = s_cloudGateway,
    .attrLen = sizeof(s_cloudGateway) / sizeof(s_cloudGateway[0])};

static char *s_singleSwitchU2[] = {"powerstate_1", "indicator"};
static char *s_doubleSwitchU2[] = {"powerstate_1", "powerstate_2", "indicator"};
static char *s_threeSwitchU2[] = {"powerstate_1", "powerstate_2", "powerstate_3", "indicator"};
static char *s_DLTDimmingU2[] = {"colorTemperature", "brightness", "PowerSwitch"};
static char *s_singleSwitchModule[] = {"powerstate_1", "indicator", "relay_status", "switch_type"};
static char *s_doubleSwitchModule[] = {"powerstate_1", "powerstate_2", "indicator", "relay_status", "switch_type"};
static char *s_doubleSwitchModuleCtrl[] = {"switch_all"};
static char *s_threeSwitchModule[] = {"powerstate_1", "powerstate_2", "powerstate_3", "indicator", "relay_status", "switch_type"};
static char *s_threeSwitchModuleCtrl[] = {"switch_all"};
static char *s_doorWindowSensorB[] = {"doorStatus", "tamperState", "power", "batteryStatus"};
static char *s_touchPanel02U2[] = {"CurrentTemperature", "PowerSwitch", "PowerSwitch_1", "PowerSwitch_2", "WindSpeed", "WindSpeed_1", "TargetTemperature", "TargetTemperature_2", "WorkMode", "AirConditioner", "FreshAir", "FloorHeating", "KeyValueNotification"};
static char *s_touchPanel02U2Ctrl[] = {"WordsCfg", "PictureCfg"};
const SAttrInfo g_SCloudAttr[] = {
    {
        .attr = s_singleSwitchU2,
        .attrLen = sizeof(s_singleSwitchU2) / sizeof(s_singleSwitchU2[0]),
    },
    {
        .attr = s_doubleSwitchU2,
        .attrLen = sizeof(s_doubleSwitchU2) / sizeof(s_doubleSwitchU2[0]),
    },
    {
        .attr = s_threeSwitchU2,
        .attrLen = sizeof(s_threeSwitchU2) / sizeof(s_threeSwitchU2[0]),
    },
    {
        .attr = s_DLTDimmingU2,
        .attrLen = sizeof(s_DLTDimmingU2) / sizeof(s_DLTDimmingU2[0]),
    },
    {
        .attr = s_singleSwitchModule,
        .attrLen = sizeof(s_singleSwitchModule) / sizeof(s_singleSwitchModule[0]),
    },
    {
        .attr = s_doubleSwitchModule,
        .attrLen = sizeof(s_doubleSwitchModule) / sizeof(s_doubleSwitchModule[0]),
        .attrCtrl = s_doubleSwitchModuleCtrl,
        .attrCtrlLen = sizeof(s_doubleSwitchModuleCtrl) / sizeof(s_doubleSwitchModuleCtrl[0]),
    },
    {
        .attr = s_threeSwitchModule,
        .attrLen = sizeof(s_threeSwitchModule) / sizeof(s_threeSwitchModule[0]),
        .attrCtrl = s_threeSwitchModuleCtrl,
        .attrCtrlLen = sizeof(s_threeSwitchModuleCtrl) / sizeof(s_threeSwitchModuleCtrl[0]),
    },
    {
        .attr = s_doorWindowSensorB,
        .attrLen = sizeof(s_doorWindowSensorB) / sizeof(s_doorWindowSensorB[0]),
    },
    {
        .attr = s_touchPanel02U2,
        .attrLen = sizeof(s_touchPanel02U2) / sizeof(s_touchPanel02U2[0]),
        .attrCtrl = s_touchPanel02U2Ctrl,
        .attrCtrlLen = sizeof(s_touchPanel02U2Ctrl) / sizeof(s_touchPanel02U2Ctrl[0]),
    },

};
static char *s_PIDsingleSwitchU2[] = {"a1BqBAOw2ii", "LCrelEX0Z9ky02tQ", ""};
static char *s_PIDdoubleSwitchU2[] = {"a1mTtj3XyVA", "vzrk6s4GD8T1dREE", ""};
static char *s_PIDthreeSwitchU2[] = {"a1T6JIBWh5o", "jxAGB21mg6lWwBtS", ""};
static char *s_PIDDLTDimmingU2[] = {"a1OS7SzKDvr", "7IM4iCzoAmvJUhgD", ""};
static char *s_PIDsingleSwitchModule[] = {"a1V9RC5hvfW", "ycpeyn8pzrhYdPUF", ""};
static char *s_PIDdoubleSwitchModule[] = {"a1H5zYD0dI9", "Rjbp8eEmWy4RswP0", ""};
static char *s_PIDthreeSwitchModule[] = {"a10rNqQLbXN", "D5xp8BGeV20ZqLjc", ""};
static char *s_PIDdoorWindowSensorB[] = {"a1qv0dhUJoT", "K1c5GR2D3PUokhd0", ""};
static char *s_PIDtouchPanel02U2[] = {"a1A2bm9AaDT", "gIC3p3pUNcEIWmg2", ""};//gIC3p3pUNcEIWmg2 sydbSMHWg6J9VaBY
const SAttrInfo g_SCloudProdId[] = {
    {.attr = s_PIDsingleSwitchU2},
    {.attr = s_PIDdoubleSwitchU2},
    {.attr = s_PIDthreeSwitchU2},
    {.attr = s_PIDDLTDimmingU2},
    {.attr = s_PIDsingleSwitchModule},
    {.attr = s_PIDdoubleSwitchModule},
    {.attr = s_PIDthreeSwitchModule},
    {.attr = s_PIDdoorWindowSensorB},
    {.attr = s_PIDtouchPanel02U2},
};

static char *g_sCloudModel[] = {"a1BqBAOw2ii", "a1mTtj3XyVA", "a1T6JIBWh5o", "a1OS7SzKDvr", "a1V9RC5hvfW", "a1H5zYD0dI9", "a10rNqQLbXN", "a1qv0dhUJoT", "a1A2bm9AaDT"}; //a1A2bm9AaDT a1aqqjEXCWa
//单键智能开关 双键智能开关 三键智能开关 DLT液晶调光器 1路智能开关模块 2路智能开关模块 3路智能开关模块 门磁传感器  智镜:场景面板 地暖 空调 新风
const SAttrInfo g_SCloudModel = {
    .attr = g_sCloudModel,
    .attrLen = sizeof(g_sCloudModel) / sizeof(g_sCloudModel[0])};

static void cloudLinkInitDevAttr(const unsigned char cloudType, CloudLinkDev *cloudLinkDev)
{
    strcpy(cloudLinkDev->devInfo.meta_info.product_key, g_SCloudProdId[cloudType].attr[0]);
    strcpy(cloudLinkDev->devInfo.meta_info.product_secret, g_SCloudProdId[cloudType].attr[1]);
    strcpy(cloudLinkDev->devInfo.meta_info.device_secret, g_SCloudProdId[cloudType].attr[2]);

    cloudLinkDev->devSvcNum = g_SCloudAttr[cloudType].attrLen;
    cloudLinkDev->devSvc = malloc(sizeof(DevSvc) * cloudLinkDev->devSvcNum);
    memset(cloudLinkDev->devSvc, 0, sizeof(DevSvc) * cloudLinkDev->devSvcNum);
    for (int i = 0; i < cloudLinkDev->devSvcNum; ++i)
        cloudLinkDev->devSvc[i].svcId = g_SCloudAttr[cloudType].attr[i];
}

static int cloudLinkAddDev(CloudLinkDev **pCloudLinkDev, const HylinkDev *hylinkDev)
{
    logInfo("cloudLinkAddDev-------------------------");
    *pCloudLinkDev = malloc(sizeof(CloudLinkDev));

    CloudLinkDev *cloudLinkDev = *pCloudLinkDev;
    if (cloudLinkDev == NULL)
        return -1;
    memset(cloudLinkDev, 0, sizeof(CloudLinkDev));
    cloudLinkDev->devInfo.id = -1;
    cloudLinkDev->devType = hylinkDev->devType;

    strcpy(cloudLinkDev->devInfo.mac, hylinkDev->GatewayId);
    if (strlen(hylinkDev->Version) > 0 && isdigit(hylinkDev->Version[0]) && strcmp(cloudLinkDev->devInfo.fwv, hylinkDev->Version))
    {
        strcpy(cloudLinkDev->devInfo.fwv, hylinkDev->Version);
    }

    strcpy(cloudLinkDev->devInfo.sn, hylinkDev->DeviceId);
    strcpy(cloudLinkDev->devInfo.meta_info.device_name, cloudLinkDev->devInfo.sn);

    cloudLinkInitDevAttr(cloudLinkDev->devType, cloudLinkDev);
    cloudLinkListAdd(&cloudLinkDev->cloudLinkNode);
    return 0;
}

int cloudAttrMod(const int devId, char **svcVal, char *json)
{
    if (*svcVal == NULL || strcmp(*svcVal, json) != 0)
    {
        if (*svcVal != NULL)
        {
            free(*svcVal);
        }
        *svcVal = json;
    }
    else
    {
        free(json);
        return 0;
    }
    return 1;
}

int cloudAttrNumUpdate(const char *key, const int value, CloudLinkDev *cloudLinkDev, unsigned char pos)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, key, value);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return cloudAttrMod(cloudLinkDev->devInfo.id, &cloudLinkDev->devSvc[pos].svcVal, json);
}

int cloudAttrArrayUpdate(char **key, char *value, char len, CloudLinkDev *cloudLinkDev, unsigned char pos)
{
    cJSON *root = cJSON_CreateObject();
    unsigned char i;
    for (i = 0; i < len; i++)
    {
        cJSON_AddNumberToObject(root, key[i], value[i]);
    }
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return cloudAttrMod(cloudLinkDev->devInfo.id, &cloudLinkDev->devSvc[pos].svcVal, json);
}

int cloudAttrStrUpdate(const char *key, char *value, CloudLinkDev *cloudLinkDev, unsigned char pos)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, key, value);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return cloudAttrMod(cloudLinkDev->devInfo.id, &cloudLinkDev->devSvc[pos].svcVal, json);
}

int cloudAttrReport(CloudLinkDev *cloudLinkDev, const int attrPos)
{
    int res = 0;
    if (attrPos == ATTR_REPORT_ALL)
    {
        for (int i = 0; i < cloudLinkDev->devSvcNum; i++)
        {
            linkkit_user_post_property(cloudLinkDev->devInfo.id, cloudLinkDev->devSvc[i].svcVal);
        }
    }
    else
    {
        if (attrPos < cloudLinkDev->devSvcNum)
            linkkit_user_post_property(cloudLinkDev->devInfo.id, cloudLinkDev->devSvc[attrPos].svcVal);
        else
        {
            return -1;
        }
    }
    return res;
}

int cloudAttrUpdate(HylinkDev *hylinkDev, CloudLinkDev *cloudLinkDev, const int attrPos)
{
    int res = 0;
    unsigned char i;
    unsigned char devType = cloudLinkDev->devType;
    unsigned char start = 0, end = cloudLinkDev->devSvcNum;

    if (attrPos == ATTR_REPORT_ALL)
    {
        res = ATTR_REPORT_ALL;
    }
    else if (attrPos > end - 1)
    {
        logError("attrPos Out of range\n");
        start = attrPos;
        end = start + 1;
        // return -1;
    }
    else
    {
        start = attrPos;
        end = start + 1;
        res = attrPos;
    }

    logInfo("cloudAttrUpdate:%d,%d", start, end);
    for (unsigned char attrType = start; attrType < end; ++attrType)
    {
        switch (devType)
        {
        case SINGLESWITCHU2:
        {
            SingleSwitchU2 *subDev = (SingleSwitchU2 *)hylinkDev->private;
            switch (attrType)
            {
            case 0:
                //Switch
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch, cloudLinkDev, attrType);
                break;
            case 1:
                //indicator
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->LedEnable, cloudLinkDev, attrType);
                break;
            default:
                break;
            }
        }
        break;
        case DOUBLESWITCHU2:
        {
            DoubleSwitchU2 *subDev = (DoubleSwitchU2 *)hylinkDev->private;
            switch (attrType)
            {
            case 0:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[attrType], cloudLinkDev, attrType);
                break;
            case 1:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[attrType], cloudLinkDev, attrType);
                break;
            case 2:
                //indicator
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->LedEnable, cloudLinkDev, attrType);
                break;
            default:
                break;
            }
        }
        break;
        case THREESWITCHU2:
        {
            ThreeSwitchU2 *subDev = (ThreeSwitchU2 *)hylinkDev->private;
            switch (attrType)
            {
            case 0:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[attrType], cloudLinkDev, attrType);
                break;
            case 1:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[attrType], cloudLinkDev, attrType);
                break;
            case 2:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[attrType], cloudLinkDev, attrType);
                break;
            case 3:
                //indicator
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->LedEnable, cloudLinkDev, attrType);
                break;
            default:
                break;
            }
        }
        break;
        case DLTDIMMINGU2:
        {
            DLTDimmingU2 *subDev = (DLTDimmingU2 *)hylinkDev->private;
            switch (attrType)
            {
            case 0:
                //cct
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->ColorTemperature, cloudLinkDev, attrType);
                break;
            case 1:
                //brightness
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Luminance, cloudLinkDev, attrType);
                break;
            case 2:
                //Switch
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch, cloudLinkDev, attrType);
                break;
            default:
                break;
            }
        }
        break;
        case SINGLESWITCHMODULE:
        {
            SingleSwitchModule *subDev = (SingleSwitchModule *)hylinkDev->private;
            switch (attrType)
            {
            case 0:
                //Switch
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch, cloudLinkDev, attrType);
                break;
            case 1:
                //indicator
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->LedEnable, cloudLinkDev, attrType);
                break;
            case 2:
                //PowerOffProtection
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->PowerOffProtection, cloudLinkDev, attrType);
                break;
            case 3:
                //KeyMode
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->KeyMode, cloudLinkDev, attrType);
                break;
            default:
                break;
            }
        }
        break;
        case DOUBLESWITCHMODULE:
        {
            DoubleSwitchModule *subDev = (DoubleSwitchModule *)hylinkDev->private;
            switch (attrType)
            {
            case 0:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[attrType], cloudLinkDev, attrType);
                break;
            case 1:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[attrType], cloudLinkDev, attrType);
                break;
            case 2:
                //indicator
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->LedEnable, cloudLinkDev, attrType);
                break;
            case 3:
                //PowerOffProtection
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->PowerOffProtection, cloudLinkDev, attrType);
                break;
            case 4:
                //KeyMode
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->KeyMode, cloudLinkDev, attrType);
                break;
            default:
                break;
            }
        }
        break;
        case THREESWITCHMODULE:
        {
            ThreeSwitchModule *subDev = (ThreeSwitchModule *)hylinkDev->private;
            switch (attrType)
            {
            case 0:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[attrType], cloudLinkDev, attrType);
                break;
            case 1:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[attrType], cloudLinkDev, attrType);
                break;
            case 2:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[attrType], cloudLinkDev, attrType);
                break;
            case 3:
                //indicator
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->LedEnable, cloudLinkDev, attrType);
                break;
            case 4:
                //PowerOffProtection
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->PowerOffProtection, cloudLinkDev, attrType);
                break;
            case 5:
                //KeyMode
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->KeyMode, cloudLinkDev, attrType);
                break;
            default:
                break;
            }
        }
        break;
        case DOORWINDOWSENSORB:
        {
            DoorWindowSensorB *subDev = (DoorWindowSensorB *)hylinkDev->private;
            switch (attrType)
            {
            case 0:
                //status
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->ContactAlarm, cloudLinkDev, attrType);
                break;
            case 1:
                //TamperAlarm
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->TamperAlarm, cloudLinkDev, attrType);
                break;
            case 2:
                //BatteryPercentage
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->BatteryPercentage, cloudLinkDev, attrType);
                break;
            case 3:
                //LowBatteryAlarm
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->LowBatteryAlarm, cloudLinkDev, attrType);
                if (subDev->LowBatteryAlarm > 0)
                {
                    char buf[24];
                    const char *event_payload = "{\"power\":%d}";
                    sprintf(buf, event_payload, subDev->BatteryPercentage);
                    user_post_event(cloudLinkDev->devInfo.id, "lowBatteryEvent", buf);
                }
                break;
            default:
                break;
            }
        }
        break;
        case TOUCHPANEL02U2:
        {
            TouchPanel02U2 *subDev = (TouchPanel02U2 *)hylinkDev->private;
            switch (attrType)
            {
            case 0:
                //CurrentTemperature
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->CurrentTemperature, cloudLinkDev, attrType);
                break;
            case 1:
                //Switch
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[0], cloudLinkDev, attrType);
                break;
            case 2:
                //Switch
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[1], cloudLinkDev, attrType);
                break;
            case 3:
                //Switch
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Switch[2], cloudLinkDev, attrType);
                break;
            case 4:
                //WindSpeed
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->WindSpeed[0], cloudLinkDev, attrType);
                break;
            case 5:
                //WindSpeed
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->WindSpeed[1], cloudLinkDev, attrType);
                break;
            case 6:
                //TargetTemperature
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->TargetTemperature[0], cloudLinkDev, attrType);
                break;
            case 7:
                //TargetTemperature
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->TargetTemperature[1], cloudLinkDev, attrType);
                break;
            case 8:
                //WorkMode_1
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->WorkMode, cloudLinkDev, attrType);
                break;
            case 9:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Enable[0], cloudLinkDev, attrType);
                break;
            case 10:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Enable[1], cloudLinkDev, attrType);
                break;
            case 11:
                cloudAttrNumUpdate(g_SCloudAttr[devType].attr[attrType], subDev->Enable[2], cloudLinkDev, attrType);
                break;
            case 12:
                if (subDev->KeyFobValue != 0)
                {
                    char buf[24];
                    const char *event_payload = "{\"KeyValue\":%d}";
                    sprintf(buf, event_payload, subDev->KeyFobValue);
                    user_post_event(cloudLinkDev->devInfo.id, g_SCloudAttr[devType].attr[attrType], buf);
                }
                break;
            case 13:
            case 14:

                break;
            default:
                break;
            }
        }
        break;
        default:
            logError("cloud dev modelId not exist");
            return -1;
        }
    }
    return res;
}

int cloudLinkGatewayReport(void *payload)
{
    int pos = 0;
    HylinkDev *hylinkDev = (HylinkDev *)payload;
    HylinkDevGateway *gatewayDev = (HylinkDevGateway *)hylinkDev->private;

    CloudLinkDev *cloudLinkDev = cloudLinkControlGet()->cloudLinkGateway;
    cloudAttrNumUpdate(g_GatewaAttr.attr[pos], gatewayDev->PermitJoining, cloudLinkDev, pos);
    // logInfo("cloudLinkGatewayReport:%s,%d", cloudLinkDev->devSvc[0].svcVal, g_GatewaAttr.attrLen);
    return cloudAttrReport(cloudLinkDev, ATTR_REPORT_ALL);
}

int cloudUpdate(HylinkDev *hylinkDev, const unsigned int attrType)
{

    logInfo("cloudUpdate devType:%d,attrType:%d,devid:%s", hylinkDev->devType, attrType, hylinkDev->DeviceId);

    int res = 0;

    if (hylinkDev->devType == GATEWAYTYPE)
    {
        return cloudLinkGatewayReport(hylinkDev);
    }

    CloudLinkDev *cloudLinkDev = cloudLinkListGetById(hylinkDev->DeviceId);

    if (cloudLinkDev == NULL)
    {
        cloudLinkAddDev(&cloudLinkDev, hylinkDev);

        res = cloudAttrUpdate(hylinkDev, cloudLinkDev, attrType);
        if (res < 0)
        {
            logError("cloudAttrUpdate error");
            cloudLinkListDelDev(cloudLinkDev);
            return -1;
        }
        if (hylinkDev->Online == SUBDEV_ONLINE)
        {
            res = runTransferCb(cloudLinkDev->devInfo.sn, hylinkDev->Online, TRANSFER_SUBDEV_LINE);
            if (res < 0)
                logError("cloudUpdate-%d TRANSFER_SUBDEV_LINE onlink error:%d", cloudLinkDev->devType, res);
            else
                logInfo("cloudUpdate-%d TRANSFER_SUBDEV_LINE onlink success:%d", cloudLinkDev->devType, res);
        }
        return res;
    }

    res = cloudAttrUpdate(hylinkDev, cloudLinkDev, attrType);
    if (res < 0)
    {
        logError("cloudAttrUpdate error");
        cloudLinkListDelDev(cloudLinkDev);
        return -1;
    }

    if (hylinkDev->Online == SUBDEV_ONLINE)
    {
        cloudAttrReport(cloudLinkDev, res);
    }
    else
    {
    }

    return res;
}
