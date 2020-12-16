#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/reboot.h>

#include "local_device.h"
#include "local_tcp_client.h"
#include "local_send.h"
#include "local_callback.h"
#include "local_list.h"

#include "cloud_list.h"

#include "rk_driver.h"

#include "hilink.h"
#include "hilink_softap_adapter.h"

#define HY0134_INDEX 8
#define HY0134 "_TZE200_twuagcv5"
static char *g_sLocalModel[] = {"_TYZB01_mq6qwmfy", "_TYZB01_i8yav3hg", "_TYZB01_42x30fz4", "_TYZB01_lc17o7gh", "_TZ3210_xblxvcat", "_TZ3210_pcikchu8", "_TZ3210_xoj72sps", "_TYZB01_kw2okqc3", HY0134, HY0134, HY0134, HY0134};
const SAttrInfo g_SLocalModel = {
    .attr = g_sLocalModel,
    .attrLen = sizeof(g_sLocalModel) / sizeof(g_sLocalModel[0])};

static char *s_sHY0095[] = {"Switch", "LedEnable", "PowerOffProtection"};
static char *s_sHY0096[] = {"Switch_1", "Switch_2", "LedEnable", "PowerOffProtection"};
static char *s_sHY0097[] = {"Switch_1", "Switch_2", "Switch_3", "LedEnable", "PowerOffProtection"};
static char *s_s09223f[] = {"ColorTemperature", "Luminance", "Switch"};
static char *s_sHY0121[] = {"Switch", "LedEnable", "PowerOffProtection", "KeyMode"};
static char *s_sHY0122[] = {"Switch_1", "Switch_2", "LedEnable", "Switch_All", "PowerOffProtection", "KeyMode"};
static char *s_sHY0107[] = {"Switch_1", "Switch_2", "Switch_3", "LedEnable", "Switch_All", "PowerOffProtection", "KeyMode"};
static char *s_sHY0093[] = {"ContactAlarm", "BatteryPercentage", "LowBatteryAlarm", "TamperAlarm"};
static char *s_sHY0134[] = {"CurrentTemperature_1", "Switch_1", "Switch_2", "Switch_3", "WindSpeed_1", "WindSpeed_2",
                            "TargetTemperature_1", "TargetTemperature_3", "WorkMode_1", "Enable_1", "Enable_2", "Enable_3", "KeyFobValue", "SceName_", "ScePhoto_"};
static char *s_sHY0134_0[] = {"Switch_3", "TargetTemperature_3"};
static char *s_sHY0134_1[] = {"Switch_1", "TargetTemperature_1", "WorkMode_1", "WindSpeed_1"};
static char *s_sHY0134_2[] = {"Switch_2", "WindSpeed_2"};
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
    {.attr = s_sHY0134_0,
     .attrLen = sizeof(s_sHY0134_0) / sizeof(s_sHY0134_0[0])},
    {.attr = s_sHY0134_1,
     .attrLen = sizeof(s_sHY0134_1) / sizeof(s_sHY0134_1[0])},
    {.attr = s_sHY0134_2,
     .attrLen = sizeof(s_sHY0134_2) / sizeof(s_sHY0134_2[0])},
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
    {.attrLen = sizeof(dev_HY0134_t)},
    {.attrLen = sizeof(dev_HY0134_t)},
    {.attrLen = sizeof(dev_HY0134_t)},
};

int local_attribute_update(dev_local_t *dev_data, cJSON *Data)
{
    int res = -1;
    // pthread_mutex_lock(local_get_mutex());
    int index = str_search(dev_data->ModelId, g_SLocalModel.attr, g_SLocalModel.attrLen);
    if (index < 0)
    {
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
        goto dev_register; // goto cloud;

    cJSON *Key, *array_sub;
    int array_size, cnt, index_sub;
    char *out;
    array_size = cJSON_GetArraySize(Data);
    for (cnt = 0; cnt < array_size; ++cnt)
    {
        array_sub = cJSON_GetArrayItem(Data, cnt);
        Key = cJSON_GetObjectItem(array_sub, STR_KEY);

        index_sub = str_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen);
        if (index == HY0134_INDEX)
        {
            if (index_sub < 0)
                index_sub = strn_search(Key->valuestring, g_SLocalAttr[index].attr, g_SLocalAttr[index].attrLen, 8);
        }

        out = NULL;
        if (index_sub < 0)
        {
            log_error("local Attr not exist:%s\n", Key->valuestring);
            continue;
        }
        // log_debug("local_attribute_update index_sub:%d\n", index_sub);
        switch (index)
        {
        case 0: //U2/天际系列：单键智能开关（HY0095）
        {
            dev_HY0095_t *dev = (dev_HY0095_t *)dev_data->private;

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
        case 9:
        case 10:
        case 11:
        {
            dev_HY0134_t *dev = (dev_HY0134_t *)dev_data->private;
            int pos;
            int num_pos = strlen(g_SLocalAttr[index].attr[index_sub]);
            switch (index_sub)
            {
            case 0:
                out = &dev->CurrentTemperature;
                break;
            case 1: //Switch_
                out = &dev->Switch[0];
                break;
            case 2: //Switch_
                out = &dev->Switch[1];
                break;
            case 3: //Switch_
                out = &dev->Switch[2];
                break;
            case 4: //WindSpeed_
                out = &dev->WindSpeed[0];
                break;
            case 5: //WindSpeed_
                out = &dev->WindSpeed[1];
                break;
            case 6: //TargetTemperature_
                out = &dev->TargetTemperature[0];
                break;
            case 7: //TargetTemperature_
                out = &dev->TargetTemperature[1];
                break;
            case 8:
                out = &dev->WorkMode;
                break;
            case 9:
                out = &dev->Enable[0];
                break;
            case 10:
                out = &dev->Enable[1];
                break;
            case 11:
                out = &dev->Enable[2];
                break;
            case 12:
                out = &dev->KeyFobValue;
                break;
            case 13: //SceName_
                pos = atoi(&Key->valuestring[num_pos]) - 1;

                if (pos < sizeof(dev->SceName) / sizeof(dev->SceName[0]))
                {
                    out = dev->SceName[pos];
                    str_copy_from_json(array_sub, STR_VALUE, out);
                    dev->SceName[pos][sizeof(dev->SceName[0]) - 1] = '\0';
                }
                continue;
            case 14: //ScePhoto_
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
    if (array_size != 1)
        index_sub = -1;
    res = local_tocloud(dev_data, index, index_sub);
    // pthread_mutex_unlock(local_get_mutex());
    return res;
dev_register:
    return 0;
fail:
    // pthread_mutex_unlock(local_get_mutex());
    return -1;
}

void hyLinkDevStatus(dev_local_t *src, int status)
{
    if (src == NULL)
        return;
    HilinkSyncBrgDevStatus(src->DeviceId, status);

    log_info("HilinkSyncBrgDevStatus:%s,%d\n", src->DeviceId, status);
    if (strcmp(src->ModelId, g_SLocalModel.attr[HY0134_INDEX]) == 0)
    {
        char sn[32] = {0};
        strcpy(sn, src->DeviceId);
        int p = strlen(src->DeviceId);
        for (int j = 0; j < 3; j++)
        {
            sn[p] = j + '0';
            HilinkSyncBrgDevStatus(sn, status);
        }
    }
}

void hyLinkStatus(int online, int status)
{
    log_info("hyLinkStatus,%d\n", status);

    dev_local_t *ptr;
    struct list_head *head = local_get_list_head();
    if (head == NULL)
    {
        return;
    }

    if (online)
    {
        list_for_each_entry(ptr, head, node)
        {
            hyLinkDevStatus(ptr, ptr->Online);
        }
    }
    else
    {
        list_for_each_entry(ptr, head, node)
        {
            hyLinkDevStatus(ptr, status);
        }
        sleep(4);
    }
}

void hyLinkSystem(int index)
{
    log_info("hyLinkSystem:%d\n", index);
    write_hanyar_cmd(STR_ADD, NULL, STR_NET_CLOSE);
    HILINK_StopSoftAp();

    if (index == INT_REFACTORY)
    {
        hyLinkStatus(0, DEV_RESTORE);
        local_delete_all_dev();
        sleep(1);
        write_hanyar_cmd(STR_REFACTORY, NULL, NULL);

        run_closeCallback();
        cloud_control_destory();
        local_control_destory();
        sync();
        driver_exit();
        hilink_restore_factory_settings();

        sleep(1);
        system("sh /userdata/app/restore.sh &");
    }
    else
    {
        hyLinkStatus(0, DEV_OFFLINE);

        run_closeCallback();
        cloud_control_destory();
        local_control_destory();
        sync();
        driver_exit();

        if (INT_REBOOT == index)
        {
            reboot(RB_AUTOBOOT);
        }
    }
    printf("\texit(0)\t\n");
    exit(0);
}
