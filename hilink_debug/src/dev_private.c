#include "dev_private.h"

void dev_private_attribute(dev_data_t *dev_data, cJSON *Data)
{
    log_debug("dev_private_attribute\n");
    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);
    cJSON *Key = cJSON_GetObjectItem(array_sub, "Key");
    if (Key == NULL)
    {
        return;
    }
    if (strcmp(dev_data->ModelId, "500c32") == 0)
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(1);
        }

        if (strcmp(Key->valuestring, "Switch") == 0)
        {
            char_copy_from_json(array_sub, "Value", dev_data->private);
        }
        log_debug("500c32:%d\n", *(char *)dev_data->private);
    }
    else if (strcmp(dev_data->ModelId, "500c33") == 0) //U2/天际系列：三键智能开关（HY0097）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_500c33_t));
        }

        dev_500c33_t *dev = (dev_500c33_t *)dev_data->private;
        char *target;
        if (strcmp(Key->valuestring, "Switch_1") == 0)
        {
            target = &dev->Switch_1;
        }
        else if (strcmp(Key->valuestring, "Switch_2") == 0)
        {
            target = &dev->Switch_2;
        }
        else if (strcmp(Key->valuestring, "Switch_3") == 0)
        {
            target = &dev->Switch_3;
        }
        else if (strcmp(Key->valuestring, "LedEnable") == 0)
        {
            target = &dev->LedEnable;
        }
        else if (strcmp(Key->valuestring, "PowerOffProtection") == 0)
        {
            target = &dev->PowerOffProtection;
        }
        else
        {
            return;
        }
        char_copy_from_json(array_sub, "Value", target);
    }
    else if (strcmp(dev_data->ModelId, "09223f") == 0) //U2/天际系列：DLT液晶调光器（09223f，型号U86KTGS150-ZXP）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_09223f_t) == 0);
        }

        dev_09223f_t *dev = (dev_09223f_t *)dev_data->private;
        char *target;
        if (strcmp(Key->valuestring, "ColorTemperature") == 0)
        {
            int_copy_from_json(array_sub, "Value", &dev->ColorTemperature);
            return;
        }
        else if (strcmp(Key->valuestring, "Switch") == 0)
        {
            target = &dev->Switch;
        }
        else if (strcmp(Key->valuestring, "Luminance") == 0)
        {
            target = &dev->Luminance;
        }
        else
        {
            return;
        }
        char_copy_from_json(array_sub, "Value", target);
    }
    else if (strcmp(dev_data->ModelId, "HY0107") == 0) //3路智能开关模块（HY0107，型号IHC1240）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0107_t));
        }
        dev_HY0107_t *dev = (dev_HY0107_t *)dev_data->private;
        char *target;
        if (strcmp(Key->valuestring, "Switch_1") == 0)
        {
            target = &dev->Switch_1;
        }
        else if (strcmp(Key->valuestring, "Switch_2") == 0)
        {
            target = &dev->Switch_2;
        }
        else if (strcmp(Key->valuestring, "Switch_3") == 0)
        {
            target = &dev->Switch_3;
        }
        else if (strcmp(Key->valuestring, "LedEnable") == 0)
        {
            target = &dev->LedEnable;
        }
        else if (strcmp(Key->valuestring, "PowerOffProtection") == 0)
        {
            target = &dev->PowerOffProtection;
        }
        else if (strcmp(Key->valuestring, "KeyMode") == 0)
        {
            target = &dev->KeyMode;
        }
        else
        {
            return;
        }
        char_copy_from_json(array_sub, "Value", target);
    }
}

void dev_private_event(dev_data_t *dev_data, cJSON *Data)
{
    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);
    cJSON *Key = cJSON_GetObjectItem(array_sub, "Key");
    if (Key == NULL)
    {
        return;
    }
    if (strcmp(dev_data->ModelId, "5f0cf1") == 0)
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(1);
        }

        if (strcmp(Key->valuestring, "KeyFobValue") == 0)
        {
            char_copy_from_json(array_sub, "Value", dev_data->private);
        }
    }
    else if (strcmp(dev_data->ModelId, "5f0c3b") == 0)
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(1);
        }

        if (strcmp(Key->valuestring, "KeyFobValue") == 0)
        {
            char_copy_from_json(array_sub, "Value", dev_data->private);
        }
    }
    else if (strcmp(dev_data->ModelId, "HY0093") == 0) //门磁传感器（HY0093，型号IHG5201）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0093_t));
        }

        dev_HY0093_t *dev = (dev_HY0093_t *)dev_data->private;
        char *target;
        if (strcmp(Key->valuestring, "ContactAlarm") == 0)
        {
            target = &dev->ContactAlarm;
        }
        else if (strcmp(Key->valuestring, "BatteryPercentage") == 0)
        {
            target = &dev->BatteryPercentage;
        }
        else if (strcmp(Key->valuestring, "LowBatteryAlarm") == 0)
        {
            target = &dev->LowBatteryAlarm;
        }
        else if (strcmp(Key->valuestring, "TamperAlarm") == 0)
        {
            target = &dev->TamperAlarm;
        }
        else
        {
            return;
        }
        char_copy_from_json(array_sub, "Value", target);
    }
    else if (strcmp(dev_data->ModelId, "HY0134") == 0) //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0134_t));
        }

        dev_HY0134_t *dev = (dev_HY0134_t *)dev_data->private;
        char *target;
        if (strcmp(Key->valuestring, "KeyFobValue") == 0)
        {
            target = &dev->KeyFobValue;
        }
        else if (strncmp(Key->valuestring, "SceName_", 8) == 0)
        {
            target = dev->SceName[atoi(&Key->valuestring[8] - 1)];
            str_copy_from_json(array_sub, "Value", target);
            return;
        }
        else if (strncmp(Key->valuestring, "Enable_", 7) == 0)
        {
            target = &dev->Enable[atoi(&Key->valuestring[7] - 1)];
        }
        else if (strncmp(Key->valuestring, "WindSpeed_", 10) == 0)
        {
            target = &dev->Switch[atoi(&Key->valuestring[10] - 1)];
        }
        else if (strcmp(Key->valuestring, "CurrentTemperature_1") == 0)
        {
            target = &dev->CurrentTemperature_1;
        }
        else if (strcmp(Key->valuestring, "TargetTemperature_1") == 0)
        {
            target = &dev->TargetTemperature_1;
        }
        else if (strcmp(Key->valuestring, "WorkMode_1") == 0)
        {
            target = &dev->CurrentTemperature_1;
        }
        else if (strcmp(Key->valuestring, "TargetTemperature_3") == 0)
        {
            target = &dev->TargetTemperature_1;
        }
        else
        {
            return;
        }
        char_copy_from_json(array_sub, "Value", target);
    }
}