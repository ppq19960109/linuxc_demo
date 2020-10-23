#include <stdlib.h>
#include <string.h>

#include "local_device.h"
#include "cloud_send.h"

#define HY0134_INDEX 8
#define HY0134 "_TZE200_twuagcv5"
static char *g_sLocalModel[] = {"_TYZB01_mq6qwmfy", "_TYZB01_i8yav3hg", "_TYZB01_42x30fz4", "_TYZB01_lc17o7gh", "_TZ3210_xblxvcat", "_TZ3210_pcikchu8", "_TZ3210_xoj72sps", "_TYZB01_kw2okqc3", HY0134};
const SAttrInfo g_SLocalModel = {
    .attr = g_sLocalModel,
    .attrLen = sizeof(g_sLocalModel) / sizeof(g_sLocalModel[0])};

static char *s_sHY0095[] = {"Switch", "LedEnable", "PowerOffProtection"};
static char *s_sHY0096[] = {"Switch_1", "Switch_2", "LedEnable", "PowerOffProtection"};
static char *s_sHY0097[] = {"Switch_1", "Switch_2", "Switch_3", "LedEnable", "PowerOffProtection"};
static char *s_s09223f[] = {"ColorTemperature", "Luminance", "Switch"};
static char *s_sHY0121[] = {"Switch", "LedEnable", "PowerOffProtection", "KeyMode"};
static char *s_sHY0122[] = {"Switch_1", "Switch_2", "LedEnable", "PowerOffProtection", "KeyMode"};
static char *s_sHY0107[] = {"Switch_1", "Switch_2", "Switch_3", "LedEnable", "PowerOffProtection", "KeyMode"};
static char *s_sHY0093[] = {"ContactAlarm", "BatteryPercentage", "LowBatteryAlarm", "TamperAlarm"};
static char *s_sHY0134[] = {"CurrentTemperature_1", "Switch_", "WindSpeed_", "TargetTemperature_", "WorkMode_1", "Enable_", "KeyFobValue", "SceName_", "ScePhoto_"};

const SAttrInfo g_SLocalAttr[] = {
    {.attr = s_sHY0095,
     .attrLen = sizeof(s_sHY0095) / sizeof(s_sHY0095[0])},
    {.attr = s_sHY0096,
     .attrLen = sizeof(s_sHY0096) / sizeof(s_sHY0096[0])},
    {.attr = s_sHY0097,
     .attrLen = sizeof(s_sHY0097) / sizeof(s_sHY0097[0])},
    {.attr = s_s09223f,
     .attrLen = sizeof(s_s09223f) / sizeof(s_s09223f[0])},
    {.attr = s_sHY0121,
     .attrLen = sizeof(s_sHY0121) / sizeof(s_sHY0121[0])},
    {.attr = s_sHY0122,
     .attrLen = sizeof(s_sHY0122) / sizeof(s_sHY0122[0])},
    {.attr = s_sHY0107,
     .attrLen = sizeof(s_sHY0107) / sizeof(s_sHY0107[0])},
    {.attr = s_sHY0093,
     .attrLen = sizeof(s_sHY0093) / sizeof(s_sHY0093[0])},
    {.attr = s_sHY0134,
     .attrLen = sizeof(s_sHY0134) / sizeof(s_sHY0134[0])},

};

static const SAttrInfo s_SLocalAttrSize[] = {
    {.attrLen = sizeof(dev_HY0095_t)},
    {.attrLen = sizeof(dev_HY0096_t)},
    {.attrLen = sizeof(dev_HY0097_t)},
    {.attrLen = sizeof(dev_09223f_t)},
    {.attrLen = sizeof(dev_HY0121_t)},
    {.attrLen = sizeof(dev_HY0122_t)},
    {.attrLen = sizeof(dev_HY0107_t)},
    {.attrLen = sizeof(dev_HY0093_t)},
    {.attrLen = sizeof(dev_HY0134_t)},
};

int hostGateway_attr(dev_data_t *dev_data, cJSON *Data)
{
    cJSON *Key, *array_sub;
    if (strcmp(STR_HOST_GATEWAYID, dev_data->DeviceId))
        return -1;
    if (dev_data->private == NULL)
    {
        dev_data->private = malloc(sizeof(DevGateway_t));
        memset(dev_data->private, 0, sizeof(DevGateway_t));
    }

    if (Data == NULL)
        return 0;

    DevGateway_t *dev = (DevGateway_t *)dev_data->private;
    int array_size = cJSON_GetArraySize(Data);
    for (int cnt = 0; cnt < array_size; ++cnt)
    {
        array_sub = cJSON_GetArrayItem(Data, cnt);
        Key = cJSON_GetObjectItem(array_sub, STR_KEY);
        if (Key == NULL)
            continue;
        if (strcmp(Key->valuestring, STR_PERMITJOINING) == 0)
        {
            char_copy_from_json(array_sub, STR_VALUE, &dev->PermitJoining);
        }
    }
    return 0;
}

int local_attribute_update(dev_data_t *dev_data, cJSON *Data)
{
    pthread_mutex_lock(&g_SLocalControl.mutex);
    int ret = -1;
    int index = str_search(dev_data->ModelId, g_SLocalModel.attr, g_SLocalModel.attrLen);
    if (index < 0)
    {
        ret = hostGateway_attr(dev_data, Data);
        if (ret == 0)
            goto fail;
        log_error("local ModelId not exist:%s\n", dev_data->ModelId);
        goto fail;
    }

    log_debug("local_attribute_update:%d\n", index);
    if (dev_data->private == NULL)
    {
        dev_data->private = malloc(s_SLocalAttrSize[index].attrLen);
        memset(dev_data->private, 0, s_SLocalAttrSize[index].attrLen);
    }
    if (Data == NULL)
        goto cloud;

    cJSON *Key, *array_sub;
    int array_size, cnt, index_sub;
    char *out;
    array_size = cJSON_GetArraySize(Data);
    for (cnt = 0; cnt < array_size; ++cnt)
    {
        array_sub = cJSON_GetArrayItem(Data, cnt);
        Key = cJSON_GetObjectItem(array_sub, STR_KEY);
        if (index == HY0134_INDEX)
        {
            index_sub = strn_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen, 7);
        }
        else
        {
            index_sub = str_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen);
        }
        out = NULL;
        if (index_sub < 0)
        {
            log_error("local Attr not exist:%s\n", Key->valuestring);
            continue;
        }
        log_debug("local_attribute_update index_sub:%d\n", index_sub);
        switch (index)
        {
        case 0: //U2/天际系列：单键智能开关（HY0095）
        {
            dev_HY0095_t *dev = (dev_HY0095_t *)dev_data->private;

            switch (index_sub)
            {
            case 0:
                out = &dev->Switch[0];
                break;
            case 1:
                out = &dev->LedEnable;
                break;
            case 2:
                out = &dev->PowerOffProtection;
                break;
            }
        }
        break;
        case 1: //U2/天际系列：双键智能开关（HY0096）
        {
            dev_HY0096_t *dev = (dev_HY0096_t *)dev_data->private;
            switch (index_sub)
            {
            case 0:
                out = &dev->Switch[0];
                break;
            case 1:
                out = &dev->Switch[1];
                break;
            case 2:
                out = &dev->LedEnable;
                break;
            case 3:
                out = &dev->PowerOffProtection;
                break;
            }
        }
        break;
        case 2: //U2/天际系列：三键智能开关（HY0097）
        {
            dev_HY0097_t *dev = (dev_HY0097_t *)dev_data->private;

            switch (index_sub)
            {
            case 0:
                out = &dev->Switch[0];
                break;
            case 1:
                out = &dev->Switch[1];
                break;
            case 2:
                out = &dev->Switch[2];
                break;
            case 3:
                out = &dev->LedEnable;
                break;
            case 4:
                out = &dev->PowerOffProtection;
                break;
            }
        }
        break;
        case 3: //U2/天际系列：DLT液晶调光器（09223f，型号U86KTGS150-ZXP）
        {
            dev_09223f_t *dev = (dev_09223f_t *)dev_data->private;

            switch (index_sub)
            {
            case 0:
                int_copy_from_json(array_sub, STR_VALUE, &dev->ColorTemperature);
                continue;
            case 1:
                out = &dev->Luminance;
                break;
            case 2:
                out = &dev->Switch;
                break;
            }
        }
        break;
        case 4: //1路智能开关模块（HY0121，型号IHC1238）
        {
            dev_HY0121_t *dev = (dev_HY0121_t *)dev_data->private;

            switch (index_sub)
            {
            case 0:
                out = &dev->Switch;
                break;
            case 1:
                out = &dev->LedEnable;
                break;
            case 2:
                out = &dev->PowerOffProtection;
                break;
            case 3:
                out = &dev->KeyMode;
                break;
            }
        }
        break;
        case 5: //2路智能开关模块（HY0122，型号IHC1239）
        {
            dev_HY0122_t *dev = (dev_HY0122_t *)dev_data->private;

            switch (index_sub)
            {
            case 0:
                out = &dev->Switch[0];
                break;
            case 1:
                out = &dev->Switch[1];
                break;
            case 2:
                out = &dev->LedEnable;
                break;
            case 3:
                out = &dev->PowerOffProtection;
                break;
            case 4:
                out = &dev->KeyMode;
                break;
            }
        }
        break;
        case 6: //3路智能开关模块（HY0107，型号IHC1240）
        {

            dev_HY0107_t *dev = (dev_HY0107_t *)dev_data->private;

            switch (index_sub)
            {
            case 0:
                out = &dev->Switch[0];
                break;
            case 1:
                out = &dev->Switch[1];
                break;
            case 2:
                out = &dev->Switch[2];
                break;
            case 3:
                out = &dev->LedEnable;
                break;
            case 4:
                out = &dev->PowerOffProtection;
                break;
            case 5:
                out = &dev->KeyMode;
                break;
            }
        }
        break;
        case 7: //门磁传感器（HY0093，型号IHG5201）
        {
            dev_HY0093_t *dev = (dev_HY0093_t *)dev_data->private;

            switch (index_sub)
            {
            case 0:
                out = &dev->ContactAlarm;
                break;
            case 1:
                out = &dev->BatteryPercentage;
                break;
            case 2:
                out = &dev->LowBatteryAlarm;
                break;
            case 3:
                out = &dev->TamperAlarm;
                break;
            }
        }
        break;
        case 8: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
        {
            dev_HY0134_t *dev = (dev_HY0134_t *)dev_data->private;
            int pos;
            int num_pos = strlen(g_SLocalAttr[index].attr[index_sub]);
            switch (index_sub)
            {
            case 0:
                out = &dev->CurrentTemperature_1;
                break;
            case 1: //Switch_
                pos = atoi(&Key->valuestring[num_pos]) - 1;
                if (pos < sizeof(dev->Switch))
                    out = &dev->Switch[pos];
                break;
            case 2: //WindSpeed_
                pos = atoi(&Key->valuestring[num_pos]) - 1;
                if (pos < sizeof(dev->WindSpeed))
                    out = &dev->WindSpeed[pos];
                break;

            case 3: //TargetTemperature_
                pos = atoi(&Key->valuestring[num_pos]) - 1;
                if (pos < sizeof(dev->TargetTemperature))
                    out = &dev->TargetTemperature[pos];
                break;
            case 4:
                out = &dev->WorkMode_1;
                break;
            case 5: //Enable_
                pos = atoi(&Key->valuestring[num_pos]) - 1;
                if (pos < sizeof(dev->Enable))
                    out = &dev->Enable[pos];
                break;
            case 6:
                out = &dev->KeyFobValue;
                break;
            case 7: //SceName_
                pos = atoi(&Key->valuestring[num_pos]) - 1;

                if (pos < sizeof(dev->SceName) / sizeof(dev->SceName[0]))
                {
                    out = dev->SceName[pos];
                    str_copy_from_json(array_sub, STR_VALUE, out);
                }
                continue;
            case 8: //ScePhoto_
                pos = atoi(&Key->valuestring[num_pos]) - 1;
                if (pos < sizeof(dev->ScePhoto))
                    out = &dev->ScePhoto[pos];

                break;
            }
        }
        break;
        default:
            free(dev_data->private);
            log_error("hanyar modelId not exist\n");
            goto fail;
        }
        if (out)
        {
            char_copy_from_json(array_sub, STR_VALUE, out);
        }
    }
cloud:

    ret = local_tohilink(dev_data, index, cloud_get_list_head(&g_SCloudControl));
fail:
    pthread_mutex_unlock(&g_SLocalControl.mutex);

    return ret;
}