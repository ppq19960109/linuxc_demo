#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cloud_send.h"
#include "cloud_list.h"

#include "tool.h"

static char *s_cloudHY0095[] = {"powerstate_1", "indicator"};
static char *s_cloudHY0096[] = {"powerstate_1", "powerstate_2", "indicator"};
static char *s_cloudHY0097[] = {"powerstate_1", "powerstate_2", "powerstate_3", "indicator"};
static char *s_cloud09223f[] = {"colorTemperature", "brightness", "PowerSwitch"};
static char *s_cloudHY0121[] = {"powerstate_1", "indicator", "relay_status", "switch_type"};
static char *s_cloudHY0122[] = {"powerstate_1", "powerstate_2", "indicator", "relay_status", "switch_type"};
static char *s_cloudHY0107[] = {"powerstate_1", "powerstate_2", "powerstate_3", "indicator", "relay_status", "switch_type"};
static char *s_cloudHY0093[] = {"doorStatus", "tamperState", "power", "batteryStatus", "lowBatteryEvent"};
static char *s_cloudHY0134[] = {"CurrentTemperature", "PowerSwitch", "WindSpeed", "TargetTemperature", "WorkMode", "PowerSwitch_1", "WindSpeed_1", "PowerSwitch_2", "TargetTemperature_2", "EnableConfig"};

const SAttrInfo g_SCloudAttr[] = {
    {.attr = s_cloudHY0095,
     .attrLen = sizeof(s_cloudHY0095) / sizeof(s_cloudHY0095[0])},
    {.attr = s_cloudHY0096,
     .attrLen = sizeof(s_cloudHY0096) / sizeof(s_cloudHY0096[0])},
    {.attr = s_cloudHY0097,
     .attrLen = sizeof(s_cloudHY0097) / sizeof(s_cloudHY0097[0])},
    {.attr = s_cloud09223f,
     .attrLen = sizeof(s_cloud09223f) / sizeof(s_cloud09223f[0])},
    {.attr = s_cloudHY0121,
     .attrLen = sizeof(s_cloudHY0121) / sizeof(s_cloudHY0121[0])},
    {.attr = s_cloudHY0122,
     .attrLen = sizeof(s_cloudHY0122) / sizeof(s_cloudHY0122[0])},
    {.attr = s_cloudHY0107,
     .attrLen = sizeof(s_cloudHY0107) / sizeof(s_cloudHY0107[0])},
    {.attr = s_cloudHY0093,
     .attrLen = sizeof(s_cloudHY0093) / sizeof(s_cloudHY0093[0])},
    {.attr = s_cloudHY0134,
     .attrLen = sizeof(s_cloudHY0134) / sizeof(s_cloudHY0134[0])},

};
static char *s_cloudPIdHY0095[] = {"a1BqBAOw2ii", "LCrelEX0Z9ky02tQ", "455769c377ce1ac8e6751e1c713027e2"};
static char *s_cloudPIdHY0096[] = {"a1mTtj3XyVA", "vzrk6s4GD8T1dREE", "2390391b3683670f7797d27591c90b31"};
static char *s_cloudPIdHY0097[] = {"a1T6JIBWh5o", "jxAGB21mg6lWwBtS", "76013cbc19edb408aa9ef880f7342fe1"};
static char *s_cloudPId09223f[] = {"a1OS7SzKDvr", "7IM4iCzoAmvJUhgD", "af1f1576e1ddc3fc9b080394ec79ea9a"};
static char *s_cloudPIdHY0121[] = {"a1V9RC5hvfW", "ycpeyn8pzrhYdPUF", "26c006602e526dc5dcfe1d2161a7c868"};
static char *s_cloudPIdHY0122[] = {"a1H5zYD0dI9", "Rjbp8eEmWy4RswP0", "928ce0c8cc7078a6231d3d1505fdb04e"};
static char *s_cloudPIdHY0107[] = {"a10rNqQLbXN", "D5xp8BGeV20ZqLjc", "0bc2eb496c6a88aaf762aa595543e74a"};
static char *s_cloudPIdHY0093[] = {"a1qv0dhUJoT", "K1c5GR2D3PUokhd0", "88c3fa699f968d35a732a8cd03bb5481"};
static char *s_cloudPIdHY0134[] = {"a1A2bm9AaDT", "gIC3p3pUNcEIWmg2", "d558bcce6a813a5398e5fd25b139c7d4"};

static const SAttrInfo g_SCloudProdId[] = {
    {.attr = s_cloudPIdHY0095},
    {.attr = s_cloudPIdHY0096},
    {.attr = s_cloudPIdHY0097},
    {.attr = s_cloudPId09223f},
    {.attr = s_cloudPIdHY0121},
    {.attr = s_cloudPIdHY0122},
    {.attr = s_cloudPIdHY0107},
    {.attr = s_cloudPIdHY0093},
    {.attr = s_cloudPIdHY0134},
};

CloudControl_t g_SCloudControl;

void cloud_control_init(CloudControl_t *cloudControl)
{
    INIT_LIST_HEAD(&cloudControl->node);
}

void cloud_control_destory(CloudControl_t *cloudControl)
{
    list_del_all_hilink(&cloudControl->node);
}

struct list_head *cloud_get_list_head(CloudControl_t *cloudControl)
{
    return &cloudControl->node;
}

int modSvc(const char *sn, const char *svcId, char **svcVal, char *json, const int devId)
{

    if (*svcVal == NULL || strcmp(*svcVal, json) != 0)
    {
        if (*svcVal != NULL)
        {
            free(*svcVal);
        }
        *svcVal = json;
        linkkit_user_post_property(devId, json);
    }
    else
    {
        free(json);
    }
}

void cloud_init_device_attr(const int index, CloudDevInfo *brgDevInfo)
{
    strcpy(brgDevInfo->meta_info.product_key, g_SCloudProdId[index].attr[0]);
    strcpy(brgDevInfo->meta_info.product_secret, g_SCloudProdId[index].attr[1]);
    strcpy(brgDevInfo->meta_info.device_secret, g_SCloudProdId[index].attr[2]);
}
void cloud_add_device_attr(const int index, dev_cloud_t *out, const char *sn)
{
    out->devSvcNum = g_SCloudAttr[index].attrLen;
    out->devSvc = malloc(sizeof(DevSvc) * out->devSvcNum);
    memset(out->devSvc, 0, sizeof(DevSvc) * out->devSvcNum);
    for (int i = 0; i < out->devSvcNum; ++i)
        out->devSvc[i].svcId = g_SCloudAttr[index].attr[i];
}
void cloud_add_device(const int index, dev_cloud_t **out, dev_data_t *local, struct list_head *cloudNode)
{
    *out = malloc(sizeof(dev_cloud_t));
    memset(*out, 0, sizeof(dev_cloud_t));
    list_add(&(*out)->node, cloudNode);

    CloudDevInfo *brgDevInfo = &(*out)->brgDevInfo;
    strcpy(brgDevInfo->sn, local->DeviceId);
    strcpy(brgDevInfo->meta_info.device_name, brgDevInfo->sn);

    cloud_init_device_attr(index, brgDevInfo);
    cloud_add_device_attr(index, *out, brgDevInfo->sn);

    if (local->Online)
        linkkit_subdev_online(&brgDevInfo->meta_info, &brgDevInfo->cloudDevId, local->Online);
}

void cloud_update_device_int(cJSON *root, char *key, int value, dev_cloud_t *out, int pos)
{
    cJSON_AddNumberToObject(root, key, value);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_DeleteItemFromObject(root, key);

    modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &out->devSvc[pos].svcVal, json, out->brgDevInfo.cloudDevId);
}

void cloud_update_device_int_array(cJSON *root, char **key, char *value, char len, dev_cloud_t *out, int pos)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        cJSON_AddNumberToObject(root, key[i], value[i]);
    }
    char *json = cJSON_PrintUnformatted(root);
    for (i = 0; i < len; ++i)
    {
        cJSON_DeleteItemFromObject(root, key[i]);
    }
    modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &out->devSvc[pos].svcVal, json, out->brgDevInfo.cloudDevId);
}

void cloud_update_device_str(cJSON *root, char *key, char *value, dev_cloud_t *out, int pos)
{
    cJSON_AddStringToObject(root, key, value);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_DeleteItemFromObject(root, key);

    modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &out->devSvc[pos].svcVal, json, out->brgDevInfo.cloudDevId);
}

int local_tohilink(dev_data_t *src, const int index, struct list_head *cloudNode)
{
    log_info("local_tohilink index:%d\n", index);
    int pos = 0, i;

    dev_cloud_t *out = list_get_by_id_hilink(src->DeviceId, cloudNode);
    if (out == NULL)
    {
        cloud_add_device(index, &out, src, cloudNode);
        strcpy(out->brgDevInfo.mac, src->GatewayId);
        if (strlen(src->Version) > 0 && strcmp(out->brgDevInfo.fwv, src->Version))
        {
            strcpy(out->brgDevInfo.fwv, src->Version);
        }
    }

    cJSON *root = cJSON_CreateObject();

    switch (index)
    {
    case 0: //U2/天际系列：单键智能开关（HY0095）
    {
        dev_HY0095_t *dev_sub = (dev_HY0095_t *)src->private;
        //Switch
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->Switch[0], out, pos);
        ++pos;
        //indicator
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->LedEnable, out, pos);
    }
    break;
    case 1: //U2/天际系列：双键智能开关（HY0096）
    {
        dev_HY0096_t *dev_sub = (dev_HY0096_t *)src->private;

        //Switch
        for (i = 0; i < 2; ++i)
        {
            cloud_update_device_int(root, g_SCloudAttr[index].attr[i], dev_sub->Switch[i], out, pos);
            ++pos;
        }
        //indicator
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->LedEnable, out, pos);
    }
    break;
    case 2: //U2/天际系列：三键智能开关（HY0097）
    {
        dev_HY0097_t *dev_sub = (dev_HY0097_t *)src->private;

        //Switch
        for (i = 0; i < 3; ++i)
        {
            cloud_update_device_int(root, g_SCloudAttr[index].attr[i], dev_sub->Switch[i], out, pos);
            ++pos;
        }
        //indicator
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->LedEnable, out, pos);
    }
    break;
    case 3: //DLT调光
    {
        dev_09223f_t *dev_sub = (dev_09223f_t *)src->private;

        //cct
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->ColorTemperature, out, pos);
        ++pos;
        //brightness
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->Luminance, out, pos);
        ++pos;
        //Switch
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->Switch, out, pos);
    }
    break;
    case 4: //1路智能开关模块（HY0121，型号IHC1238）
    {
        dev_HY0121_t *dev_sub = (dev_HY0121_t *)src->private;

        //Switch
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->Switch, out, pos);
        ++pos;
        //indicator
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->LedEnable, out, pos);
        ++pos;
        //PowerOffProtection
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->PowerOffProtection, out, pos);
        ++pos;
        //KeyMode
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->KeyMode, out, pos);
    }
    break;
    case 5: //2路智能开关模块（HY0122，型号IHC1239）
    {
        dev_HY0122_t *dev_sub = (dev_HY0122_t *)src->private;

        //Switch
        for (i = 0; i < 2; ++i)
        {
            cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->Switch[i], out, pos);
            ++pos;
        }
        //indicator
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->LedEnable, out, pos);

        ++pos;
        //PowerOffProtection
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->PowerOffProtection, out, pos);
        ++pos;
        //KeyMode
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->KeyMode, out, pos);
    }
    break;
    case 6: //3路智能开关模块（HY0107，型号IHC1240）
    {
        dev_HY0107_t *dev_sub = (dev_HY0107_t *)src->private;

        //Switch
        for (i = 0; i < 3; ++i)
        {
            cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->Switch[i], out, pos);
            ++pos;
        }
        //indicator
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->LedEnable, out, pos);
        ++pos;
        //PowerOffProtection
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->PowerOffProtection, out, pos);
        ++pos;
        //KeyMode
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->KeyMode, out, pos);
    }
    break;
    case 7: //门窗传感器
    {
        dev_HY0093_t *dev_sub = (dev_HY0093_t *)src->private;
        //status
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->ContactAlarm, out, pos);
        ++pos;
        //TamperAlarm
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->TamperAlarm, out, pos);
        ++pos;
        //BatteryPercentage
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->BatteryPercentage, out, pos);
        ++pos;
        //LowBatteryAlarm
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->LowBatteryAlarm, out, pos);
    }
    break;
    case 8: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {
        dev_HY0134_t *dev_sub = (dev_HY0134_t *)src->private;

        //CurrentTemperature
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->CurrentTemperature_1, out, pos);
        ++pos;
        //Switch
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->Switch[0], out, pos);
        ++pos;
        //WindSpeed
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->WindSpeed[0], out, pos);
        ++pos;
        //TargetTemperature
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->TargetTemperature[0], out, pos);
        ++pos;
        //WorkMode_1
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->WorkMode_1, out, pos);
        ++pos;
        //Switch
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->Switch[1], out, pos);
        ++pos;
        //WindSpeed
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->WindSpeed[1], out, pos);
        ++pos;
        //Switch
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->Switch[2], out, pos);
        ++pos;
        //TargetTemperature
        cloud_update_device_int(root, g_SCloudAttr[index].attr[pos], dev_sub->TargetTemperature[2], out, pos);
    }
    break;
    default:
        goto fail;
    }
    // list_print_all_hilink(cloudNode);

    cJSON_Delete(root);

    return 0;
fail:
    log_error("cloud modelId not exist");
    if (out != NULL)
    {
        list_del_dev_hilink(out);
    }
    cJSON_Delete(root);
    return -1;
}

void linkkit_online_all()
{
    dev_cloud_t *out;
    dev_data_t *in;
    struct list_head *head = local_get_list_head(&g_SLocalControl);
    if (head == NULL)
    {
        return;
    }

    list_for_each_entry(in, head, node)
    {
        if (strcmp(in->DeviceId, STR_HOST_GATEWAYID) == 0)
            continue;
        out = list_get_by_id_hilink(in->DeviceId, cloud_get_list_head(&g_SCloudControl));
        linkkit_subdev_online(NULL, &out->brgDevInfo.cloudDevId, in->Online);
        if (in->Online)
            local_attribute_update(in, NULL);
    }
}
