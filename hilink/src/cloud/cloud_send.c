#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include "cloud_send.h"
#include "cloud_list.h"

#include "tool.h"

#include "hilink.h"
#include "hilink_profile_adapter.h"

static char *s_cloudHY0095[] = {"switch1", "indicator"};
static char *s_cloudHY0096[] = {"switch1", "switch2", "indicator"};
static char *s_cloudHY0097[] = {"switch1", "switch2", "switch3", "indicator"};
static char *s_cloud09223f[] = {"cct", "brightness", "switch"};
static char *s_cloudHY0121[] = {"switch", "indicator"};
static char *s_cloudHY0122[] = {"switch1", "switch2", "indicator", "switch"};
static char *s_cloudHY0107[] = {"switch1", "switch2", "switch3", "indicator", "switch"};
static char *s_cloudHY0093[] = {"doorEvent", "status"};
static char *s_cloudHY0134[] = {"scene", "button1", "button2", "button3", "button4", "button5", "button6"}; //场景面板2ANF
static char *s_cloudHY0134_0[] = {"switch", "temperature"};                                                 //地暖 2ANK
static char *s_cloudHY0134_1[] = {"switch", "temperature", "mode", "fan"};                                  //空调2ANJ
static char *s_cloudHY0134_2[] = {"switch", "fan"};                                                         //新风 2ANI

static char *s_sCloudHY0134Temperature[] = {STR_CURRENT, STR_TARGET};

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
    {.attr = s_cloudHY0134_0,
     .attrLen = sizeof(s_cloudHY0134_0) / sizeof(s_cloudHY0134_0[0])},
    {.attr = s_cloudHY0134_1,
     .attrLen = sizeof(s_cloudHY0134_1) / sizeof(s_cloudHY0134_1[0])},
    {.attr = s_cloudHY0134_2,
     .attrLen = sizeof(s_cloudHY0134_2) / sizeof(s_cloudHY0134_2[0])},
};
static char *s_cloud2AP1[] = {"2AP1", "U2-86K11ND10-ZD(HW)", "005"};
static char *s_cloud2AP0[] = {"2AP0", "U2-86K21ND10-ZD(HW)", "005"};
static char *s_cloud2AN9[] = {"2AN9", "U2-86K31ND10-ZD(HW)", "005"};
static char *s_cloud2AN7[] = {"2AN7", "U2-86KTGS150-ZXP(HW)", "01B"};
static char *s_cloud2AOZ[] = {"2AOZ", "IHC1238HW", "064"};
static char *s_cloud2AOY[] = {"2AOY", "IHC1239HW", "064"};
static char *s_cloud2ANO[] = {"2ANO", "IHC1240HW", "064"};
static char *s_cloud2AN8[] = {"2AN8", "IHG5201HW", "018"};
static char *s_cloud2ANF[] = {"2ANF", "U2-86QMP-Z02(HW)", "075"};
static char *s_cloud2ANK[] = {"2ANK", "U2-86QMP-Z02(HW)", "012"};
static char *s_cloud2ANJ[] = {"2ANJ", "U2-86QMP-Z02(HW)", "012"};
static char *s_cloud2ANI[] = {"2ANI", "U2-86QMP-Z02(HW)", "030"};
static const SAttrInfo g_SCloudProdId[] = {
    {.attr = s_cloud2AP1},
    {.attr = s_cloud2AP0},
    {.attr = s_cloud2AN9},
    {.attr = s_cloud2AN7},
    {.attr = s_cloud2AOZ},
    {.attr = s_cloud2AOY},
    {.attr = s_cloud2ANO},
    {.attr = s_cloud2AN8},
    {.attr = s_cloud2ANF},
    {.attr = s_cloud2ANK},
    {.attr = s_cloud2ANJ},
    {.attr = s_cloud2ANI},
};

int cloud_get_attr(dev_local_t *src, const int index, dev_cloud_t *out);

static CloudControl_t g_SCloudControl;

void cloud_control_init()
{
    list_init_cloud(&g_SCloudControl.head);
    g_SCloudControl.pid = getpid();
}

void cloud_control_destory()
{
    list_del_all_cloud();
}

struct list_head *cloud_get_list_head()
{
    return &g_SCloudControl.head;
}

CloudStatus get_cloud_status(void)
{
    return g_SCloudControl.cloud_status;
}

void set_cloud_status(CloudStatus status)
{
    g_SCloudControl.cloud_status = status;
}
int get_registerFlag(void)
{
    return g_SCloudControl.registerFlag;
}

void set_registerFlag(void)
{
    g_SCloudControl.registerFlag = 1;
}
void BrgDevInfo_init(BrgDevInfo *brgDevInfo)
{
    // strcpy(brgDevInfo->prodId, PRODUCT_ID);
    strcpy(brgDevInfo->hiv, "1.0.0");
    strcpy(brgDevInfo->fwv, "1.0.0");
    strcpy(brgDevInfo->hwv, "1.0.0");
    strcpy(brgDevInfo->swv, "1.0.0");
    brgDevInfo->protType = PROTOCOL_TYPE;
    strcpy(brgDevInfo->manu, MANUAFACTURER);

    // strcpy(brgDevInfo->sn, "12345678");
    // strcpy(brgDevInfo->model, DEVICE_MODEL);
    // strcpy(brgDevInfo->devType, DEVICE_TYPE);
    strcpy(brgDevInfo->mac, "000000000000");
}

int modSvc(const char *sn, const char *svcId, char **svcVal, char *json)
{
    if (json == NULL)
        return -1;
    // if (*svcVal != NULL)
    // {
    // }

    if (*svcVal == NULL || strcmp(*svcVal, json) != 0)
    {
        if (*svcVal != NULL)
        {
            free(*svcVal);
            *svcVal = json;
            if (HilinkUploadBrgDevCharState(sn, svcId) != 0)
            {
                log_error("HilinkUploadBrgDevCharState error,%s,%s\n", svcId, json);
                if (HilinkReportBrgDevCharState(sn, svcId, json, strlen(json) + 1, g_SCloudControl.pid) != 0)
                {
                    log_error("HilinkReportBrgDevCharState error,%d,%d,%s,%s\n", getpid(), pthread_self(), svcId, json);
                }
                else
                {
                    log_info("HilinkReportBrgDevCharState success,%s,%s\n", svcId, json);
                }
            }
            else
            {
                log_info("HilinkUploadBrgDevCharState success,%s,%s\n", svcId, json);
            }
        }
        else
        {
            *svcVal = json;
        }
    }
    else
    {
        free(json);
    }
    return 0;
}

void cloud_init_device_attr(const int index, BrgDevInfo *brgDevInfo)
{
    strcpy(brgDevInfo->prodId, g_SCloudProdId[index].attr[0]);
    strcpy(brgDevInfo->model, g_SCloudProdId[index].attr[1]);
    strcpy(brgDevInfo->devType, g_SCloudProdId[index].attr[2]);
}
void cloud_add_device_attr(const int index, dev_cloud_t *out, const char *sn)
{
    out->devSvcNum = g_SCloudAttr[index].attrLen;
    out->devSvc = malloc(sizeof(DevSvc) * out->devSvcNum);
    memset(out->devSvc, 0, sizeof(DevSvc) * out->devSvcNum);
    for (int i = 0; i < out->devSvcNum; ++i)
        out->devSvc[i].svcId = g_SCloudAttr[index].attr[i];
}

void reportBrgAllSubDevState(const int index, const char *sn, dev_local_t *local, dev_cloud_t *out)
{
    cJSON *arrayroot = cJSON_CreateArray();
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddItemToArray(arrayroot, obj);

    cJSON_AddStringToObject(obj, "sn", sn);
    cJSON *array = cJSON_AddArrayToObject(obj, "services");
    cloud_get_attr(local, index, out);
    for (int i = 0; i < out->devSvcNum; ++i)
    {
        cJSON *svc = cJSON_CreateObject();
        cJSON_AddStringToObject(svc, "sid", out->devSvc[i].svcId);
        if (out->devSvc[i].svcVal != NULL)
        {
            cJSON *svcVal = cJSON_Parse(out->devSvc[i].svcVal);
            cJSON_AddItemToObject(svc, "data", svcVal);
        }
        cJSON_AddItemToArray(array, svc);
    }
    char *json = cJSON_Print(arrayroot);

    log_error("reportBrgAllSubDevState:%s\n", json);
    int ret = HILINK_ReportBrgAllSubDevState(json);
    if (ret != 0)
    {
        log_error("HILINK_ReportBrgAllSubDevState error\n");
    }
    free(json);
    cJSON_Delete(arrayroot);
}

int cloud_add_device(const int index, dev_cloud_t **out, const char *sn, dev_local_t *local)
{
    *out = malloc(sizeof(dev_cloud_t));
    memset(*out, 0, sizeof(dev_cloud_t));
    list_add_cloud(&(*out)->node);

    BrgDevInfo *brgDevInfo = &(*out)->brgDevInfo;
    BrgDevInfo_init(brgDevInfo);
    strcpy(brgDevInfo->sn, sn);
    strcpy(brgDevInfo->mac, local->GatewayId);
    if (strlen(local->Version) > 0 && isdigit(local->Version[0]) && strcmp(brgDevInfo->fwv, local->Version))
    {
        strcpy(brgDevInfo->fwv, local->Version);
    }

    cloud_init_device_attr(index, brgDevInfo);
    cloud_add_device_attr(index, *out, brgDevInfo->sn);

    return 0;
}
void cloud_update_device_int(cJSON *root, char *key, int value, dev_cloud_t *out, int pos)
{
    cJSON_AddNumberToObject(root, key, value);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_DeleteItemFromObject(root, key);

    modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &out->devSvc[pos].svcVal, json);
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
    modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &out->devSvc[pos].svcVal, json);
}

void cloud_update_device_str(cJSON *root, char *key, char *value, dev_cloud_t *out, int pos)
{
    cJSON_AddStringToObject(root, key, value);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_DeleteItemFromObject(root, key);

    modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &out->devSvc[pos].svcVal, json);
}

int cloud_get_attr(dev_local_t *src, const int index, dev_cloud_t *out)
{

    int pos = 0, i;
    cJSON *root = cJSON_CreateObject();

    switch (index)
    {
    case 0: //U2/天际系列：单键智能开关（HY0095）
    {
        dev_HY0095_t *dev_sub = (dev_HY0095_t *)src->private;
        //Switch
        cloud_update_device_int(root, STR_ON, dev_sub->Switch[0], out, pos++);

        //indicator
        cloud_update_device_int(root, STR_MODE, dev_sub->LedEnable, out, pos++);
    }
    break;
    case 1: //U2/天际系列：双键智能开关（HY0096）
    {
        dev_HY0096_t *dev_sub = (dev_HY0096_t *)src->private;

        //Switch
        for (i = 0; i < 2; ++i)
        {
            cloud_update_device_int(root, STR_ON, dev_sub->Switch[i], out, pos++);
        }
        //indicator
        cloud_update_device_int(root, STR_MODE, dev_sub->LedEnable, out, pos++);
    }
    break;
    case 2: //U2/天际系列：三键智能开关（HY0097）
    {
        dev_HY0097_t *dev_sub = (dev_HY0097_t *)src->private;

        //Switch
        for (i = 0; i < 3; ++i)
        {
            cloud_update_device_int(root, STR_ON, dev_sub->Switch[i], out, pos++);
        }
        //indicator
        cloud_update_device_int(root, STR_MODE, dev_sub->LedEnable, out, pos++);
    }
    break;
    case 3: //DLT调光
    {

        dev_09223f_t *dev_sub = (dev_09223f_t *)src->private;

        //cct
        cloud_update_device_int(root, STR_COLORTEMPERATURE, dev_sub->ColorTemperature, out, pos++);

        //brightness
        cloud_update_device_int(root, STR_BRIGHTNESS, dev_sub->Luminance, out, pos++);

        //Switch
        cloud_update_device_int(root, STR_ON, dev_sub->Switch, out, pos++);
    }
    break;
    case 4: //1路智能开关模块（HY0121，型号IHC1238）
    {
        dev_HY0121_t *dev_sub = (dev_HY0121_t *)src->private;

        //Switch
        cloud_update_device_int(root, STR_ON, dev_sub->Switch, out, pos++);

        //indicator
        cloud_update_device_int(root, STR_MODE, dev_sub->LedEnable, out, pos++);
    }
    break;
    case 5: //2路智能开关模块（HY0122，型号IHC1239）
    {
        dev_HY0122_t *dev_sub = (dev_HY0122_t *)src->private;

        //Switch
        for (i = 0; i < 2; ++i)
        {
            cloud_update_device_int(root, STR_ON, dev_sub->Switch[i], out, pos++);
        }
        //indicator
        cloud_update_device_int(root, STR_MODE, dev_sub->LedEnable, out, pos++);

        cloud_update_device_int(root, STR_ON, 0, out, pos++);
    }
    break;
    case 6: //3路智能开关模块（HY0107，型号IHC1240）
    {
        dev_HY0107_t *dev_sub = (dev_HY0107_t *)src->private;

        //Switch
        for (i = 0; i < 3; ++i)
        {
            cloud_update_device_int(root, STR_ON, dev_sub->Switch[i], out, pos++);
        }
        //indicator
        cloud_update_device_int(root, STR_MODE, dev_sub->LedEnable, out, pos++);

        cloud_update_device_int(root, STR_ON, 0, out, pos++);
    }
    break;
    case 7: //门窗传感器
    {
        dev_HY0093_t *dev_sub = (dev_HY0093_t *)src->private;

        //doorEvent
        if (out->devSvc[pos].svcVal == NULL)
        {
            dev_sub->init_ContactAlarm = dev_sub->ContactAlarm;
            dev_sub->door_event = 0xff;
        }
        else if (dev_sub->door_event == 0xff)
        {

            if (dev_sub->init_ContactAlarm != dev_sub->ContactAlarm)
            {
                dev_sub->door_event = dev_sub->ContactAlarm;
            }
        }
        else
        {
            dev_sub->door_event = dev_sub->ContactAlarm;
        }

        cloud_update_device_int(root, STR_ON, dev_sub->door_event, out, pos++);

        //status
        cloud_update_device_int(root, STR_STATUS, dev_sub->ContactAlarm, out, pos++);
    }
    break;
    case 8: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    case 9:
    case 10:
    case 11:
    {
        dev_HY0134_t *dev_sub = (dev_HY0134_t *)src->private;
        //-------------------------------------------------------------
        //场景面板

        // if (out->devSvc[pos].svcVal == NULL)
        // {
        //     dev_sub->KeyFobValue = 0xff;
        // }

        // if (dev_sub->KeyFobValue != 0xff)
        // {
        //     free(out->devSvc[pos].svcVal);
        //     out->devSvc[pos].svcVal = malloc(1);
        //     out->devSvc[pos].svcVal[0] = 0;
        // }

        cloud_update_device_int(root, STR_NUM, dev_sub->KeyFobValue, out, pos++);
        dev_sub->KeyFobValue = 0;

        // const char name[] = {0xe5, 0x9b, 0x9e, 0xe5, 0xae, 0xb6, 0x00};

        for (i = 0; i < 6; ++i)
        {
            if (strlen(dev_sub->SceName[i]) == 0)
                sprintf(dev_sub->SceName[i], "场景%d", i + 1);

            cloud_update_device_str(root, STR_NAME, dev_sub->SceName[i], out, pos++);
        }
        //-------------------------------------------------------------

        dev_cloud_t *out_sub[3] = {0};
        char sn[32] = {0};
        stpcpy(sn, src->DeviceId);
        int p = strlen(src->DeviceId);
        for (int j = 0; j < 3; j++)
        {
            sn[p] = j + '0';
            out_sub[j] = list_get_by_id_cloud(sn);

            if (out_sub[j] == NULL)
            {
                cloud_add_device(index + j + 1, &out_sub[j], sn, src);
            }

            pos = 0;
            switch (j)
            {
            case 0:
            {
                cloud_update_device_int(root, STR_ON, dev_sub->Switch[2], out_sub[j], pos++);

                char value[2] = {dev_sub->CurrentTemperature_1, dev_sub->TargetTemperature[2]};
                cloud_update_device_int_array(root, s_sCloudHY0134Temperature, value, sizeof(value), out_sub[j], pos++);
            }
            break;
            case 1:
            {
                cloud_update_device_int(root, STR_ON, dev_sub->Switch[0], out_sub[j], pos++);

                char value[2] = {dev_sub->CurrentTemperature_1, dev_sub->TargetTemperature[0]};
                cloud_update_device_int_array(root, s_sCloudHY0134Temperature, value, sizeof(value), out_sub[j], pos++);

                cloud_update_device_int(root, STR_MODE, dev_sub->WorkMode_1, out_sub[j], pos++);

                cloud_update_device_int(root, STR_GEAR, dev_sub->WindSpeed[0], out_sub[j], pos++);
            }
            break;
            case 2:
                cloud_update_device_int(root, STR_ON, dev_sub->Switch[1], out_sub[j], pos++);

                cloud_update_device_int(root, STR_GEAR, dev_sub->WindSpeed[1], out_sub[j], pos++);
                break;
            default:
                break;
            }
        }
        //-------------------------------------------------------------
    }
    break;
    default:
        goto fail;
    }
    // list_print_all_cloud();

    cJSON_Delete(root);

    return 0;
fail:
    log_error("hilink modelId not exist:%d\n", index);

    cJSON_Delete(root);
    return -1;
}

int local_tocloud(dev_local_t *src, const int index)
{

    log_info("local_tocloud index:%d\n", index);
    char addFlag = 0;
    dev_cloud_t *out = list_get_by_id_cloud(src->DeviceId);
    if (out == NULL)
    {
        addFlag = 1;
        // log_info("cloud_add_device %s,%d\n",src->DeviceId,src->Online);
        cloud_add_device(index, &out, src->DeviceId, src);
    }
    int ret = cloud_get_attr(src, index, out);
    if (ret < 0)
    {
        if (out != NULL)
        {
            list_del_dev_cloud(out);
        }
    }
    else
    {
        if (addFlag)
        {
            if (src->Online)
            {
                local_singleDevice_onlineStatus(src, DEV_ONLINE);
            }
            else
            {
                if (get_registerFlag())
                {
                    // local_singleDevice_onlineStatus(src, DEV_ONLINE);
                }
            }
            log_info("local_tocloud HilinkSyncBrgDevStatus:%s,%d\n", src->DeviceId, src->Online);
        }
    }
    return ret;
}
