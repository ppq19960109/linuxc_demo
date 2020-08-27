#include <stdlib.h>
#include <string.h>

#include "local_device.h"
#include "cloud_send.h"

static char *g_sLocalModel[] = {"TS0001", "TS0002", "TS0003", "09223f", "HY0121", "HY0122", "HY0107", "HY0093", "HY0134", "HY0134", "HY0134", "HY0134"};
SAttrInfo g_SLocalModel = {
    .attr = g_sLocalModel,
    .attrLen = sizeof(g_sLocalModel) / sizeof(g_sLocalModel[0])};

static char *g_sHY0095[] = {"Switch", "LedEnable", "PowerOffProtection"};
static char *g_sHY0096[] = {"Switch_1", "Switch_2", "LedEnable", "PowerOffProtection"};
static char *g_sHY0097[] = {"Switch_1", "Switch_2", "Switch_3", "LedEnable", "PowerOffProtection"};
static char *g_s09223f[] = {"ColorTemperature", "Luminance", "Switch"};
static char *g_sHY0121[] = {"Switch", "LedEnable", "PowerOffProtection", "KeyMode"};
static char *g_sHY0122[] = {"Switch_1", "Switch_2", "LedEnable", "Switch_All", "PowerOffProtection", "KeyMode"};
static char *g_sHY0107[] = {"Switch_1", "Switch_2", "Switch_3", "LedEnable", "Switch_All", "PowerOffProtection", "KeyMode"};
static char *g_sHY0093[] = {"ContactAlarm", "BatteryPercentage", "LowBatteryAlarm", "TamperAlarm"};
static char *g_sHY0134[] = {"KeyFobValue", "SceName_", "Enable_", "Switch_", "WindSpeed_", "CurrentTemperature_1", "TargetTemperature_1", "WorkMode_1", "TargetTemperature_3"};
static char *g_sHY0134_0[] = {"Switch_3", "TargetTemperature_3"};
static char *g_sHY0134_1[] = {"Switch_1", "TargetTemperature_1", "WorkMode_1", "WindSpeed_1"};
static char *g_sHY0134_2[] = {"Switch_2", "WindSpeed_2"};
SAttrInfo g_SLocalAttr[] = {
    {.attr = g_sHY0095,
     .attrLen = sizeof(g_sHY0095) / sizeof(g_sHY0095[0])},
    {.attr = g_sHY0096,
     .attrLen = sizeof(g_sHY0096) / sizeof(g_sHY0096[0])},
    {.attr = g_sHY0097,
     .attrLen = sizeof(g_sHY0097) / sizeof(g_sHY0097[0])},
    {.attr = g_s09223f,
     .attrLen = sizeof(g_s09223f) / sizeof(g_s09223f[0])},
    {.attr = g_sHY0121,
     .attrLen = sizeof(g_sHY0121) / sizeof(g_sHY0121[0])},
    {.attr = g_sHY0122,
     .attrLen = sizeof(g_sHY0122) / sizeof(g_sHY0122[0])},
    {.attr = g_sHY0107,
     .attrLen = sizeof(g_sHY0107) / sizeof(g_sHY0107[0])},
    {.attr = g_sHY0093,
     .attrLen = sizeof(g_sHY0093) / sizeof(g_sHY0093[0])},
    {.attr = g_sHY0134,
     .attrLen = sizeof(g_sHY0134) / sizeof(g_sHY0134[0])},
    {.attr = g_sHY0134_0,
     .attrLen = sizeof(g_sHY0134_0) / sizeof(g_sHY0134_0[0])},
    {.attr = g_sHY0134_1,
     .attrLen = sizeof(g_sHY0134_1) / sizeof(g_sHY0134_1[0])},
    {.attr = g_sHY0134_2,
     .attrLen = sizeof(g_sHY0134_2) / sizeof(g_sHY0134_2[0])},
};

int local_attribute_update(dev_data_t *dev_data, cJSON *Data)
{
    log_debug("local_attribute_update\n");

    cJSON *Key, *array_sub;
    char *out;
    int array_size, cnt, index_sub;

    if (Data != NULL)
        array_size = cJSON_GetArraySize(Data);

    int index = str_search(dev_data->ModelId, g_SLocalModel.attr, g_SLocalModel.attrLen);
    switch (index)
    {
    case 0: //U2/天际系列：单键智能开关（HY0095）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0095_t));
            memset(dev_data->private, 0, sizeof(dev_HY0095_t));
        }
        if (Data == NULL)
            break;
        dev_HY0095_t *dev = (dev_HY0095_t *)dev_data->private;

        for (cnt = 0; cnt < array_size; ++cnt)
        {
            array_sub = cJSON_GetArrayItem(Data, cnt);
            Key = cJSON_GetObjectItem(array_sub, STR_KEY);

            index_sub = str_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen);

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
            }
            char_copy_from_json(array_sub, STR_VALUE, out);
        }
    }
    break;
    case 1: //U2/天际系列：双键智能开关（HY0096）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0096_t));
            memset(dev_data->private, 0, sizeof(dev_HY0096_t));
        }
        if (Data == NULL)
            break;
        dev_HY0096_t *dev = (dev_HY0096_t *)dev_data->private;

        for (cnt = 0; cnt < array_size; ++cnt)
        {
            array_sub = cJSON_GetArrayItem(Data, cnt);
            Key = cJSON_GetObjectItem(array_sub, STR_KEY);

            index_sub = str_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen);

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
            char_copy_from_json(array_sub, STR_VALUE, out);
        }
    }
    break;
    case 2: //U2/天际系列：三键智能开关（HY0097）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0097_t));
            memset(dev_data->private, 0, sizeof(dev_HY0097_t));
        }
        if (Data == NULL)
            break;
        dev_HY0097_t *dev = (dev_HY0097_t *)dev_data->private;

        for (cnt = 0; cnt < array_size; ++cnt)
        {
            array_sub = cJSON_GetArrayItem(Data, cnt);
            Key = cJSON_GetObjectItem(array_sub, STR_KEY);

            index_sub = str_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen);

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
            char_copy_from_json(array_sub, STR_VALUE, out);
        }
    }
    break;
    case 3: //U2/天际系列：DLT液晶调光器（09223f，型号U86KTGS150-ZXP）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_09223f_t));
            memset(dev_data->private, 0, sizeof(dev_09223f_t));
        }
        if (Data == NULL)
            break;
        dev_09223f_t *dev = (dev_09223f_t *)dev_data->private;
        index_sub = str_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen);
        for (cnt = 0; cnt < array_size; ++cnt)
        {
            switch (index_sub)
            {
            case 0:
                int_copy_from_json(array_sub, STR_VALUE, &dev->ColorTemperature);
                break;
            case 1:
                char_copy_from_json(array_sub, STR_VALUE, &dev->Switch);
                break;
            case 2:
                char_copy_from_json(array_sub, STR_VALUE, &dev->Luminance);
                break;
            }
        }
    }
    break;
    case 4: //1路智能开关模块（HY0121，型号IHC1238）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0121_t));
            memset(dev_data->private, 0, sizeof(dev_HY0121_t));
        }
        if (Data == NULL)
            break;
        dev_HY0121_t *dev = (dev_HY0121_t *)dev_data->private;
        index_sub = str_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen);
        for (cnt = 0; cnt < array_size; ++cnt)
        {
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
            char_copy_from_json(array_sub, STR_VALUE, out);
        }
    }
    break;
    case 5: //2路智能开关模块（HY0122，型号IHC1239）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0122_t));
            memset(dev_data->private, 0, sizeof(dev_HY0122_t));
        }
        if (Data == NULL)
            break;
        dev_HY0122_t *dev = (dev_HY0122_t *)dev_data->private;
        index_sub = str_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen);
        for (cnt = 0; cnt < array_size; ++cnt)
        {
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
            char_copy_from_json(array_sub, STR_VALUE, out);
        }
    }
    break;
    case 6: //3路智能开关模块（HY0107，型号IHC1240）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0107_t));
            memset(dev_data->private, 0, sizeof(dev_HY0107_t));
        }
        if (Data == NULL)
            break;
        dev_HY0107_t *dev = (dev_HY0107_t *)dev_data->private;
        int index_sub = str_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen);
        for (cnt = 0; cnt < array_size; ++cnt)
        {
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
            char_copy_from_json(array_sub, STR_VALUE, out);
        }
    }
    break;
    case 7: //门磁传感器（HY0093，型号IHG5201）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0093_t));
            memset(dev_data->private, 0, sizeof(dev_HY0093_t));
        }
        if (Data == NULL)
            break;
        dev_HY0093_t *dev = (dev_HY0093_t *)dev_data->private;
        index_sub = str_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen);
        for (cnt = 0; cnt < array_size; ++cnt)
        {
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
            char_copy_from_json(array_sub, STR_VALUE, out);
        }
    }
    break;
    case 8: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    case 9:
    case 10:
    case 11:
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0134_t));
            memset(dev_data->private, 0, sizeof(dev_HY0134_t));
        }

        dev_HY0134_t *dev = (dev_HY0134_t *)dev_data->private;
        dev->Switch[0] = 1;
        dev->CurrentTemperature_1 = 14;
        dev->TargetTemperature_1 = 28;
        dev->WorkMode_1 = 2;
        dev->WindSpeed[0] = 1;
        if (Data == NULL)
            break;
        index_sub = strn_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen, 7);
        for (cnt = 0; cnt < array_size; ++cnt)
        {
            switch (index_sub)
            {
            case 0:
                out = &dev->KeyFobValue;
                break;
            case 1:
                out = dev->SceName[atoi(&Key->valuestring[8]) - 1];
                str_copy_from_json(array_sub, STR_VALUE, out);
                break;
            case 2:
                out = &dev->Enable[atoi(&Key->valuestring[7]) - 1];
                break;
            case 3:
                out = &dev->Switch[atoi(&Key->valuestring[7]) - 1];
                break;
            case 4:
                out = &dev->WindSpeed[atoi(&Key->valuestring[10]) - 1];
                break;
            case 5:
                out = &dev->CurrentTemperature_1;
                break;
            case 6:
                out = &dev->TargetTemperature_1;
                break;
            case 7:
                out = &dev->WorkMode_1;
                break;
            case 8:
                out = &dev->TargetTemperature_3;
                break;
            }
            char_copy_from_json(array_sub, STR_VALUE, out);
        }
    }
    break;
    default:
        log_error("hanyar modelId not exist");
        return -1;
    }

    return local_tohilink(dev_data, index, cloud_get_list_head(&g_SCloudControl));
}