#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cJSON.h"
#include "logFunc.h"
#include "commonFunc.h"
#include "frameCb.h"

#include "hylinkSubDev.h"
#include "hylink.h"
#include "hylinkRecv.h"

static char *g_sLocalModel[] = {"_TYZB01_mq6qwmfy", "_TYZB01_i8yav3hg", "_TYZB01_42x30fz4", "_TYZB01_lc17o7gh", "_TZ3210_xblxvcat", "_TZ3210_pcikchu8", "_TZ3210_xoj72sps", "_TYZB01_kw2okqc3", "_TZE200_twuagcv5"};

const SAttrInfo g_SLocalModel = {
    .attr = g_sLocalModel,
    .attrLen = sizeof(g_sLocalModel) / sizeof(g_sLocalModel[0])};

static char *s_singleSwitchU2[] = {"Switch", "LedEnable", "PowerOffProtection"};
static char *s_doubleSwitchU2[] = {"Switch_1", "Switch_2", "LedEnable", "PowerOffProtection"};
static char *s_threeSwitchU2[] = {"Switch_1", "Switch_2", "Switch_3", "LedEnable", "PowerOffProtection"};
static char *s_DLTDimmingU2[] = {"ColorTemperature", "Luminance", "Switch"};
static char *s_singleSwitchModule[] = {"Switch", "LedEnable", "PowerOffProtection", "KeyMode"};
static char *s_doubleSwitchModule[] = {"Switch_1", "Switch_2", "LedEnable", "PowerOffProtection", "KeyMode"};
static char *s_doubleSwitchModuleCtrl[] = {"Switch_All"};
static char *s_threeSwitchModule[] = {"Switch_1", "Switch_2", "Switch_3", "LedEnable", "PowerOffProtection", "KeyMode"};
static char *s_threeSwitchModuleCtrl[] = {"Switch_All"};
static char *s_doorWindowSensorB[] = {"ContactAlarm", "BatteryPercentage", "LowBatteryAlarm", "TamperAlarm"};
static char *s_touchPanel02U2[] = {"CurrentTemperature_1", "Switch_1", "Switch_2", "Switch_3", "WindSpeed_1", "WindSpeed_2", "TargetTemperature_1", "TargetTemperature_3", "WorkMode_1", "Enable_1", "Enable_2", "Enable_3", "KeyFobValue", "SceName_", "ScePhoto_"};

const SAttrInfo g_SLocalAttr[] = {
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
    },
};

static const SAttrInfo s_SLocalAttrSize[] = {
    {.attrLen = sizeof(SingleSwitchU2)},
    {.attrLen = sizeof(DoubleSwitchU2)},
    {.attrLen = sizeof(ThreeSwitchU2)},
    {.attrLen = sizeof(DLTDimmingU2)},
    {.attrLen = sizeof(SingleSwitchModule)},
    {.attrLen = sizeof(DoubleSwitchModule)},
    {.attrLen = sizeof(ThreeSwitchModule)},
    {.attrLen = sizeof(DoorWindowSensorB)},
    {.attrLen = sizeof(TouchPanel02U2)},
};

int hylinkSubDevAttrUpdate(HylinkDev *hylinkDevData, cJSON *Data)
{
    int devType, attrType = 0;
    if (hylinkDevData->private == NULL)
    {
        devType = findStrIndex(hylinkDevData->ModelId, g_SLocalModel.attr, g_SLocalModel.attrLen);
        if (devType < 0)
        {
            logError("ModelId not exist:%s\n", hylinkDevData->ModelId);
            goto clean;
        }
        hylinkDevData->devType = devType;
        hylinkDevData->private = malloc(s_SLocalAttrSize[devType].attrLen);
        if (hylinkDevData->private == NULL)
        {
            logError("malloc error !!!!");
            goto clean;
        }
        memset(hylinkDevData->private, 0, s_SLocalAttrSize[devType].attrLen);
    }

    if (Data == NULL)
    {
        logDebug("hylinkSubDevAttrUpdate Data NULL\n");
        goto success;
    }
    devType = hylinkDevData->devType;
    logDebug("hylinkSubDevAttrUpdate:%d\n", devType);

    char *outByte = NULL, *outStr = NULL;
    int *outNum = NULL;

    cJSON *Key = cJSON_GetObjectItem(Data, STR_KEY);
    if (Key == NULL)
        goto fail;

    attrType = findStrIndex(Key->valuestring, g_SLocalAttr[devType].attr, g_SLocalAttr[devType].attrLen);
    if (attrType < 0)
    {
        char *rc = strchr(Key->valuestring, '_');
        logInfo("strchr:%s,%d\n", rc, rc - Key->valuestring);
        attrType = findStrnIndex(Key->valuestring, rc - Key->valuestring, g_SLocalAttr[devType].attr, g_SLocalAttr[devType].attrLen);
        if (attrType < 0)
            goto fail;
    }

    if (attrType < 0)
    {
        logError("Attr not exist:%s\n", Key->valuestring);
        goto fail;
    }

    switch (devType)
    {
    case SINGLESWITCHU2:
    {
        SingleSwitchU2 *dev = (SingleSwitchU2 *)hylinkDevData->private;

        switch (attrType)
        {
        case 0:
            outByte = &dev->Switch;
            break;
        case 1:
            outByte = &dev->LedEnable;
            break;
        case 2:
            outByte = &dev->PowerOffProtection;
            break;
        }
    }
    break;
    case DOUBLESWITCHU2:
    {
        DoubleSwitchU2 *dev = (DoubleSwitchU2 *)hylinkDevData->private;
        switch (attrType)
        {
        case 0:
            outByte = &dev->Switch[0];
            break;
        case 1:
            outByte = &dev->Switch[1];
            break;
        case 2:
            outByte = &dev->LedEnable;
            break;
        case 3:
            outByte = &dev->PowerOffProtection;
            break;
        }
    }
    break;
    case THREESWITCHU2:
    {
        ThreeSwitchU2 *dev = (ThreeSwitchU2 *)hylinkDevData->private;

        switch (attrType)
        {
        case 0:
            outByte = &dev->Switch[0];
            break;
        case 1:
            outByte = &dev->Switch[1];
            break;
        case 2:
            outByte = &dev->Switch[2];
            break;
        case 3:
            outByte = &dev->LedEnable;
            break;
        case 4:
            outByte = &dev->PowerOffProtection;
            break;
        }
    }
    break;
    case DLTDIMMINGU2:
    {
        DLTDimmingU2 *dev = (DLTDimmingU2 *)hylinkDevData->private;

        switch (attrType)
        {
        case 0:
            outNum = &dev->ColorTemperature;
            break;
        case 1:
            outByte = &dev->Luminance;
            break;
        case 2:
            outByte = &dev->Switch;
            break;
        }
    }
    break;
    case SINGLESWITCHMODULE:
    {
        SingleSwitchModule *dev = (SingleSwitchModule *)hylinkDevData->private;

        switch (attrType)
        {
        case 0:
            outByte = &dev->Switch;
            break;
        case 1:
            outByte = &dev->LedEnable;
            break;
        case 2:
            outByte = &dev->PowerOffProtection;
            break;
        case 3:
            outByte = &dev->KeyMode;
            break;
        }
    }
    break;
    case DOUBLESWITCHMODULE:
    {
        DoubleSwitchModule *dev = (DoubleSwitchModule *)hylinkDevData->private;

        switch (attrType)
        {
        case 0:
            outByte = &dev->Switch[0];
            break;
        case 1:
            outByte = &dev->Switch[1];
            break;
        case 2:
            outByte = &dev->LedEnable;
            break;
        case 3:
            outByte = &dev->PowerOffProtection;
            break;
        case 4:
            outByte = &dev->KeyMode;
            break;
        }
    }
    break;
    case THREESWITCHMODULE:
    {
        ThreeSwitchModule *dev = (ThreeSwitchModule *)hylinkDevData->private;

        switch (attrType)
        {
        case 0:
            outByte = &dev->Switch[0];
            break;
        case 1:
            outByte = &dev->Switch[1];
            break;
        case 2:
            outByte = &dev->Switch[2];
            break;
        case 3:
            outByte = &dev->LedEnable;
            break;
        case 4:
            outByte = &dev->PowerOffProtection;
            break;
        case 5:
            outByte = &dev->KeyMode;
            break;
        }
    }
    break;
    case DOORWINDOWSENSORB:
    {
        DoorWindowSensorB *dev = (DoorWindowSensorB *)hylinkDevData->private;

        switch (attrType)
        {
        case 0:
            outByte = &dev->ContactAlarm;
            break;
        case 1:
            outByte = &dev->BatteryPercentage;
            break;
        case 2:
            outByte = &dev->LowBatteryAlarm;
            break;
        case 3:
            outByte = &dev->TamperAlarm;
            break;
        }
    }
    break;
    case TOUCHPANEL02U2:
    {
        TouchPanel02U2 *dev = (TouchPanel02U2 *)hylinkDevData->private;

        switch (attrType)
        {
        case 0:
            outByte = &dev->CurrentTemperature;
            break;
        case 1: //Switch_
            outByte = &dev->Switch[0];
            break;
        case 2: //Switch_
            outByte = &dev->Switch[1];
            break;
        case 3: //Switch_
            outByte = &dev->Switch[2];
            break;
        case 4: //WindSpeed_
            outByte = &dev->WindSpeed[0];
            break;
        case 5: //WindSpeed_
            outByte = &dev->WindSpeed[1];
            break;
        case 6: //TargetTemperature_
            outByte = &dev->TargetTemperature[0];
            break;
        case 7: //TargetTemperature_
            outByte = &dev->TargetTemperature[1];
            break;
        case 8:
            outByte = &dev->WorkMode;
            break;
        case 9:
            outByte = &dev->Enable[0];
            break;
        case 10:
            outByte = &dev->Enable[1];
            break;
        case 11:
            outByte = &dev->Enable[2];
            break;
        case 12:
            outByte = &dev->KeyFobValue;
            break;
        case 13: //SceName_
        {
            int num_pos = strlen(g_SLocalAttr[devType].attr[attrType]);
            int pos = atoi(&Key->valuestring[num_pos]) - 1;
            if (pos < sizeof(dev->SceName) / sizeof(dev->SceName[0]))
                outStr = dev->SceName[pos];
        }
        break;
        case 14: //ScePhoto_
        {
            int num_pos = strlen(g_SLocalAttr[devType].attr[attrType]);
            int pos = atoi(&Key->valuestring[num_pos]) - 1;
            if (pos < sizeof(dev->ScePhoto))
                outByte = &dev->ScePhoto[pos];
        }
        break;
        }
    }
    break;
    default:
        logError("hylink modelId not exist");
        free(hylinkDevData->private);

        goto fail;
    }
    if (outByte)
    {
        getByteForJson(Data, STR_VALUE, outByte);
    }
    else if (outNum)
    {
        getNumForJson(Data, STR_VALUE, outNum);
    }
    else if (outStr)
    {
        getStrForJson(Data, STR_VALUE, outStr);
    }
    else
    {
        logError("outByte outNum outStr not exist\n");
    }

success:
    return attrType;
fail:
    return -1;
clean:
    return -2;
}
