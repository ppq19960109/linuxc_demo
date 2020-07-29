#include "dev_private.h"
#include "hilink_cover.h"

char *dev_modeId[] = {"HY0095", "HY0096", "HY0097", "09223f", "HY0121", "HY0122", "HY0107", "HY0093", "HY0134"};

char *attr_HY0095[] = {"Switch", "LedEnable", "PowerOffProtection"};
char *attr_HY0096[] = {"Switch_1", "Switch_2", "LedEnable", "PowerOffProtection"};
char *attr_HY0097[] = {"Switch_1", "Switch_2", "Switch_3", "LedEnable", "PowerOffProtection"};

char *attr_09223f[] = {"ColorTemperature", "Luminance", "Switch"};

char *attr_HY0121[] = {"Switch", "LedEnable", "PowerOffProtection", "KeyMode"};
char *attr_HY0122[] = {"Switch_1", "Switch_2", "LedEnable", "PowerOffProtection", "KeyMode"};
char *attr_HY0107[] = {"Switch_1", "Switch_2", "Switch_3", "LedEnable", "PowerOffProtection", "KeyMode"};

char *attr_HY0093[] = {"ContactAlarm", "BatteryPercentage", "LowBatteryAlarm", "TamperAlarm"};

char *attr_HY0134[] = {"KeyFobValue", "SceName_", "Enable_", "WindSpeed_", "CurrentTemperature_1", "TargetTemperature_1", "WorkMode_1", "TargetTemperature_3"};

int dev_private_attribute(dev_data_t *dev_data, cJSON *Data)
{
    log_debug("dev_private_attribute\n");
    int index = -1, uploadState = 0xff;
    int newFlag = 0;
    cJSON *Key, *array_sub;
    char *out;
    if (Data != NULL)
    {
        array_sub = cJSON_GetArrayItem(Data, 0);
        Key = cJSON_GetObjectItem(array_sub, "Key");
    }
    else
    {
        newFlag = 1;
    }

    index = str_search(dev_data->ModelId, dev_modeId, sizeof(dev_modeId) / sizeof(dev_modeId[0]));
    switch (index)
    {
    case 0: //U2/天际系列：单键智能开关（HY0095）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0095_t));
            if (newFlag)
                goto cover;
        }
        dev_HY0095_t *dev = (dev_HY0095_t *)dev_data->private;
        int index_sub = str_search(Key->valuestring, attr_HY0095, sizeof(attr_HY0095) / sizeof(attr_HY0095[0]));

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
        char_copy_from_json(array_sub, "Value", out);
    }
    break;
    case 1: //U2/天际系列：双键智能开关（HY0096）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0096_t));
            if (newFlag)
                goto cover;
        }
        dev_HY0096_t *dev = (dev_HY0096_t *)dev_data->private;
        int index_sub = str_search(Key->valuestring, attr_HY0096, sizeof(attr_HY0096) / sizeof(attr_HY0096[0]));

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
        char_copy_from_json(array_sub, "Value", out);
    }
    break;
    case 2: //U2/天际系列：三键智能开关（HY0097）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0097_t));
            if (newFlag)
                goto cover;
        }
        dev_HY0097_t *dev = (dev_HY0097_t *)dev_data->private;
        int index_sub = str_search(Key->valuestring, attr_HY0097, sizeof(attr_HY0097) / sizeof(attr_HY0097[0]));

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
        char_copy_from_json(array_sub, "Value", out);
    }
    break;
    case 3: //U2/天际系列：DLT液晶调光器（09223f，型号U86KTGS150-ZXP）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_09223f_t));
            if (newFlag)
                goto cover;
        }

        dev_09223f_t *dev = (dev_09223f_t *)dev_data->private;
        int index_sub = str_search(Key->valuestring, attr_09223f, sizeof(attr_09223f) / sizeof(attr_09223f[0]));

        switch (index_sub)
        {
        case 0:
            int_copy_from_json(array_sub, "Value", &dev->ColorTemperature);
            break;
        case 1:
            char_copy_from_json(array_sub, "Value", &dev->Switch);
            break;
        case 2:
            char_copy_from_json(array_sub, "Value", &dev->Luminance);
            break;
        }
    }
    break;
    case 4: //1路智能开关模块（HY0121，型号IHC1238）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0121_t));
            if (newFlag)
                goto cover;
        }
        dev_HY0121_t *dev = (dev_HY0121_t *)dev_data->private;
        int index_sub = str_search(Key->valuestring, attr_HY0121, sizeof(attr_HY0121) / sizeof(attr_HY0121[0]));

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
        char_copy_from_json(array_sub, "Value", out);
    }
    break;
    case 5: //2路智能开关模块（HY0122，型号IHC1239）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0122_t));
            if (newFlag)
                goto cover;
        }
        dev_HY0122_t *dev = (dev_HY0122_t *)dev_data->private;
        int index_sub = str_search(Key->valuestring, attr_HY0122, sizeof(attr_HY0122) / sizeof(attr_HY0122[0]));

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
        char_copy_from_json(array_sub, "Value", out);
    }
    break;
    case 6: //3路智能开关模块（HY0107，型号IHC1240）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0107_t));
            if (newFlag)
                goto cover;
        }
        dev_HY0107_t *dev = (dev_HY0107_t *)dev_data->private;
        int index_sub = str_search(Key->valuestring, attr_HY0107, sizeof(attr_HY0107) / sizeof(attr_HY0107[0]));

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
        char_copy_from_json(array_sub, "Value", out);
    }
    break;
    case 7: //门磁传感器（HY0093，型号IHG5201）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0093_t));
            if (newFlag)
                goto cover;
        }

        dev_HY0093_t *dev = (dev_HY0093_t *)dev_data->private;
        int index_sub = str_search(Key->valuestring, attr_HY0093, sizeof(attr_HY0093) / sizeof(attr_HY0093[0]));

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
        char_copy_from_json(array_sub, "Value", out);
    }
    break;
    case 8: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {
        if (dev_data->private == NULL)
        {
            dev_data->private = malloc(sizeof(dev_HY0134_t));
            if (newFlag)
                goto cover;
        }

        dev_HY0134_t *dev = (dev_HY0134_t *)dev_data->private;
        int index_sub = strn_search(Key->valuestring, attr_HY0134, sizeof(attr_HY0134) / sizeof(attr_HY0134[0]), 7);

        switch (index_sub)
        {
        case 0:
            out = &dev->KeyFobValue;
            break;
        case 1:
            out = dev->SceName[atoi(&Key->valuestring[8] - 1)];
            str_copy_from_json(array_sub, "Value", out);
            goto cover;
        case 2:
            out = &dev->Enable[atoi(&Key->valuestring[7] - 1)];
            break;
        case 3:
            out = &dev->Switch[atoi(&Key->valuestring[10] - 1)];
            break;
        case 4:
            out = &dev->CurrentTemperature_1;
            break;
        case 5:
            out = &dev->TargetTemperature_1;
            break;
        case 6:
            out = &dev->WorkMode_1;
            break;
        case 7:
            out = &dev->TargetTemperature_3;
            break;
        }
        char_copy_from_json(array_sub, "Value", out);
    }
    break;
    }
cover:
    if (index != -1)
        local_tohilink(dev_data, index, uploadState);
    return 0;
}