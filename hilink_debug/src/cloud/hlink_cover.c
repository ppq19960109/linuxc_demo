#include "hilink_cover.h"
#include "protocol_cover.h"
#include "list_hilink.h"
#include "dev_private.h"

#include "hilink_profile_bridge.h"
#include "hilink_profile_adapter.h"

hilink_handle_t hilink_handle;

void hilink_handle_init()
{
    INIT_LIST_HEAD(&hilink_handle.node);
}

void hilink_handle_destory()
{
    list_del_all(&hilink_handle.node);
}

void BrgDevInfo_init(BrgDevInfo *brgDevInfo)
{
    strcpy(brgDevInfo->prodId, PRODUCT_ID);
    strcpy(brgDevInfo->hiv, "1.0.0");
    strcpy(brgDevInfo->fwv, "1.0.0");
    strcpy(brgDevInfo->hwv, "1.0.0");
    strcpy(brgDevInfo->swv, "1.0.0");
    brgDevInfo->protType = PROTOCOL_TYPE;
    strcpy(brgDevInfo->manu, MANUAFACTURER);

    strcpy(brgDevInfo->prodId, "1011");
    strcpy(brgDevInfo->sn, "12345678");
    strcpy(brgDevInfo->model, DEVICE_MODEL);
    strcpy(brgDevInfo->devType, DEVICE_TYPE);
    strcpy(brgDevInfo->mac, "123456789012");
}

char *svcId_500c33[] = {"switch1", "switch2", "switch3", "indicator"};
char *svcId_09223f[] = {"cct", "brightness", "switch"};
char *svcId_HY0107[] = {"switch1", "switch2", "switch3", "indicator", "switch"}; //, "relaystatus", "switchtype"
char *svcId_HY0093[] = {"doorEvent", "status"};

char *svcId_HY0134[] = {"scene", "button1"};                       //场景面板2ANF
char *svcId_HY0134_0[] = {"switch", "temperature"};                //地暖 2ANK
char *svcId_HY0134_1[] = {"switch", "temperature", "mode", "fan"}; //空调2ANJ
char *svcId_HY0134_2[] = {"switch", "fan"};                        //新风 2ANI

int modSvc(const char *sn, const char *svcId, char **svcVal, char *json)
{

    if (*svcVal == NULL || strcmp(*svcVal, json) != 0)
    {
        if (*svcVal != NULL)
        {
            free(*svcVal);
            HilinkUploadBrgDevCharState(sn, svcId);
        }
        *svcVal = json;
    }
    else
    {
        free(json);
    }
}

int local_tohilink(dev_data_t *src, int index, int uploadState)
{
    log_debug("local_tohilink index:%d", index);
    int newFlag = 0, pos = 0, i;
    char *json;

    dev_hilink_t *out = list_get_by_id_hilink(src->DeviceId, &hilink_handle.node);
    if (out == NULL)
    {
        newFlag = 1;
        out = malloc(sizeof(dev_hilink_t));
        memset(out, 0, sizeof(dev_hilink_t));
        list_add(&out->node, &hilink_handle.node);
    }

    DevSvc *devSvcArray;
    BrgDevInfo *brgDevInfo = &out->brgDevInfo;

    if (newFlag)
    {
        BrgDevInfo_init(brgDevInfo);
        strcpy(brgDevInfo->sn, src->DeviceId);
        strcpy(brgDevInfo->mac, src->GatewayId);
    }

    cJSON *root = cJSON_CreateObject();

    switch (index)
    {
    case 0: //U2/天际系列：三键智能开关（HY0097）
    {
        dev_500c33_t *dev_sub = (dev_500c33_t *)src->private;
        if (newFlag)
        {
            strcpy(brgDevInfo->prodId, "2AN9");
            strcpy(brgDevInfo->model, "U2-86K31ND10-ZD(HW)");
            strcpy(brgDevInfo->devType, "005");

            out->devSvcNum = sizeof(svcId_500c33) / sizeof(svcId_500c33[0]);
            out->devSvc = malloc(sizeof(DevSvc) * out->devSvcNum);
            memset(out->devSvc, 0, sizeof(DevSvc) * out->devSvcNum);
            for (i = 0; i < out->devSvcNum; ++i)
                out->devSvc[i].svcId = (char *)svcId_500c33[i];

            HilinkSyncBrgDevStatus(brgDevInfo->sn, DEV_ONLINE);
        }
        devSvcArray = out->devSvc;

        //Switch
        for (i = 0; i < 3; ++i)
        {
            dev_sub->Switch[i] = 1;
            cJSON_AddNumberToObject(root, "on", dev_sub->Switch[i]);
            json = cJSON_PrintUnformatted(root);
            cJSON_DeleteItemFromObject(root, "on");

            modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
            ++pos;
        }
        //indicator
        cJSON_AddNumberToObject(root, "mode", dev_sub->LedEnable);
        json = cJSON_PrintUnformatted(root);
        cJSON_DeleteItemFromObject(root, "mode");
        modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
    }
    break;
    case 1:
    {
        if (newFlag)
        {
            strcpy(brgDevInfo->prodId, "2AN7");
            strcpy(brgDevInfo->model, "U2-86KTGS150-ZXP(HW)");
            strcpy(brgDevInfo->devType, "01B");

            out->devSvcNum = sizeof(svcId_09223f) / sizeof(svcId_09223f[0]);
            out->devSvc = malloc(sizeof(DevSvc) * out->devSvcNum);
            memset(out->devSvc, 0, sizeof(DevSvc) * out->devSvcNum);
            for (i = 0; i < out->devSvcNum; ++i)
                out->devSvc[i].svcId = svcId_09223f[i];

            HilinkSyncBrgDevStatus(brgDevInfo->sn, DEV_ONLINE);
        }
        devSvcArray = out->devSvc;
        dev_09223f_t *dev_sub = (dev_09223f_t *)src->private;
        //cct
        cJSON_AddNumberToObject(root, "colorTemperature", dev_sub->ColorTemperature);
        json = cJSON_PrintUnformatted(root);
        cJSON_DeleteItemFromObject(root, "colorTemperature");
        modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
        ++pos;
        //brightness
        cJSON_AddNumberToObject(root, "brightness", dev_sub->Luminance);
        json = cJSON_PrintUnformatted(root);
        cJSON_DeleteItemFromObject(root, "brightness");
        modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
        ++pos;
        //Switch
        cJSON_AddNumberToObject(root, "on", dev_sub->Switch);
        json = cJSON_PrintUnformatted(root);
        cJSON_DeleteItemFromObject(root, "on");
        modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
    }
    break;
    case 2:
    {
        dev_HY0107_t *dev_sub = (dev_HY0107_t *)src->private;
        if (newFlag)
        {
            strcpy(brgDevInfo->prodId, "2ANO");
            strcpy(brgDevInfo->model, "IHC1240HW");
            strcpy(brgDevInfo->devType, "064");

            out->devSvcNum = sizeof(svcId_HY0107) / sizeof(svcId_HY0107[0]);
            out->devSvc = malloc(sizeof(DevSvc) * out->devSvcNum);
            memset(out->devSvc, 0, sizeof(DevSvc) * out->devSvcNum);
            for (i = 0; i < out->devSvcNum; ++i)
                out->devSvc[i].svcId = (char *)svcId_HY0107[i];

            HilinkSyncBrgDevStatus(brgDevInfo->sn, DEV_ONLINE);
        }
        devSvcArray = out->devSvc;

        //Switch
        for (i = 0; i < 3; ++i)
        {
            dev_sub->Switch[i] = 1;
            cJSON_AddNumberToObject(root, "on", dev_sub->Switch[i]);
            json = cJSON_PrintUnformatted(root);
            cJSON_DeleteItemFromObject(root, "on");

            modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
            ++pos;
        }
        //indicator
        cJSON_AddNumberToObject(root, "mode", dev_sub->LedEnable);
        json = cJSON_PrintUnformatted(root);
        cJSON_DeleteItemFromObject(root, "mode");
        modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
        ++pos;
        //switch
        cJSON_AddNumberToObject(root, "on", dev_sub->Switch_All);
        json = cJSON_PrintUnformatted(root);
        cJSON_DeleteItemFromObject(root, "on");
        modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
        // //relaystatus
        // cJSON_AddNumberToObject(root, "status", dev_sub->PowerOffProtection);
        // json = cJSON_PrintUnformatted(root);
        // cJSON_DeleteItemFromObject(root, "status");
        // modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
        // ++pos;
        // //switchtype
        // cJSON_AddNumberToObject(root, "on", dev_sub->KeyMode);
        // json = cJSON_PrintUnformatted(root);
        // cJSON_DeleteItemFromObject(root, "on");
        // modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
    }
    break;
    case 3:
    {
        if (newFlag)
        {
            strcpy(brgDevInfo->prodId, "2AN8");
            strcpy(brgDevInfo->model, "IHG5201HW");
            strcpy(brgDevInfo->devType, "018");

            out->devSvcNum = sizeof(svcId_HY0093) / sizeof(svcId_HY0093[0]);
            out->devSvc = malloc(sizeof(DevSvc) * out->devSvcNum);
            memset(out->devSvc, 0, sizeof(DevSvc) * out->devSvcNum);
            for (i = 0; i < out->devSvcNum; ++i)
                out->devSvc[i].svcId = svcId_HY0093[i];

            HilinkSyncBrgDevStatus(brgDevInfo->sn, DEV_ONLINE);
        }
        devSvcArray = out->devSvc;
        dev_HY0093_t *dev_sub = (dev_HY0093_t *)src->private;

        //doorEvent
        cJSON_AddNumberToObject(root, "on", dev_sub->ContactAlarm);
        json = cJSON_PrintUnformatted(root);
        cJSON_DeleteItemFromObject(root, "on");
        modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
        ++pos;
        //status
        cJSON_AddNumberToObject(root, "status", dev_sub->ContactAlarm);
        json = cJSON_PrintUnformatted(root);
        cJSON_DeleteItemFromObject(root, "status");
        modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
    }
    break;
    case 4: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {
        dev_HY0134_t *dev_sub = (dev_HY0134_t *)src->private;
        dev_hilink_t *out_sub[3] = {0};
        char sn[20] = {0};
        stpcpy(sn, src->DeviceId);
        int p = strlen(src->DeviceId);
        for (int j = 0; j < 3; j++)
        {
            sn[p] = j + '0';
            out_sub[j] = list_get_by_id_hilink(sn, &hilink_handle.node);

            if (out_sub[j] == NULL)
            {
                out_sub[j] = malloc(sizeof(dev_hilink_t));
                memset(out_sub[j], 0, sizeof(dev_hilink_t));
                list_add(&(out_sub[j]->node), &hilink_handle.node);

                BrgDevInfo *brgDevInfo = &out_sub[j]->brgDevInfo;
                BrgDevInfo_init(brgDevInfo);
                strcpy(brgDevInfo->sn, sn);
                strcpy(brgDevInfo->mac, src->GatewayId);

                strcpy(brgDevInfo->model, "U2-86QMP-Z02(HW)");
                char **svcId;
                switch (j)
                {
                case 0:

                    svcId = svcId_HY0134_0;
                    strcpy(brgDevInfo->prodId, "2ANK");
                    strcpy(brgDevInfo->devType, "012");
                    out_sub[j]->devSvcNum = sizeof(svcId_HY0134_0) / sizeof(svcId_HY0134_0[0]);
                    break;
                case 1:
                    svcId = svcId_HY0134_1;
                    strcpy(brgDevInfo->prodId, "2ANJ");
                    strcpy(brgDevInfo->devType, "012");
                    out_sub[j]->devSvcNum = sizeof(svcId_HY0134_1) / sizeof(svcId_HY0134_1[0]);
                    break;
                case 2:
                    svcId = svcId_HY0134_2;
                    strcpy(brgDevInfo->prodId, "2ANI");
                    strcpy(brgDevInfo->devType, "030");
                    out_sub[j]->devSvcNum = sizeof(svcId_HY0134_2) / sizeof(svcId_HY0134_2[0]);
                    break;
                default:
                    break;
                }
                out_sub[j]->devSvc = malloc(sizeof(DevSvc) * out_sub[j]->devSvcNum);
                memset(out_sub[j]->devSvc, 0, sizeof(DevSvc) * out_sub[j]->devSvcNum);
                for (i = 0; i < out_sub[j]->devSvcNum; ++i)
                    out_sub[j]->devSvc[i].svcId = svcId[i];
                HilinkSyncBrgDevStatus(brgDevInfo->sn, DEV_ONLINE);
            }

            devSvcArray = out_sub[j]->devSvc;
            pos = 0;
            switch (j)
            {
            case 0:
                cJSON_AddNumberToObject(root, "on", dev_sub->Switch[2]);
                json = cJSON_PrintUnformatted(root);
                cJSON_DeleteItemFromObject(root, "on");
                modSvc(out_sub[j]->brgDevInfo.sn, out_sub[j]->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
                pos++;

                cJSON_AddNumberToObject(root, "current", 0);
                cJSON_AddNumberToObject(root, "target", dev_sub->TargetTemperature_3);
                json = cJSON_PrintUnformatted(root);
                cJSON_DeleteItemFromObject(root, "current");
                cJSON_DeleteItemFromObject(root, "target");
                modSvc(out_sub[j]->brgDevInfo.sn, out_sub[j]->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
                break;
            case 1:
                cJSON_AddNumberToObject(root, "on", dev_sub->Switch[0]);
                json = cJSON_PrintUnformatted(root);
                cJSON_DeleteItemFromObject(root, "on");
                modSvc(out_sub[j]->brgDevInfo.sn, out_sub[j]->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
                pos++;

                cJSON_AddNumberToObject(root, "current", dev_sub->CurrentTemperature_1);
                cJSON_AddNumberToObject(root, "target", dev_sub->TargetTemperature_1);
                json = cJSON_PrintUnformatted(root);
                cJSON_DeleteItemFromObject(root, "current");
                cJSON_DeleteItemFromObject(root, "target");
                modSvc(out_sub[j]->brgDevInfo.sn, out_sub[j]->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
                pos++;

                cJSON_AddNumberToObject(root, "mode", dev_sub->WorkMode_1);
                json = cJSON_PrintUnformatted(root);
                cJSON_DeleteItemFromObject(root, "mode");
                modSvc(out_sub[j]->brgDevInfo.sn, out_sub[j]->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
                pos++;

                cJSON_AddNumberToObject(root, "gear", dev_sub->WindSpeed[0]);
                json = cJSON_PrintUnformatted(root);
                cJSON_DeleteItemFromObject(root, "gear");
                modSvc(out_sub[j]->brgDevInfo.sn, out_sub[j]->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);

                break;
            case 2:
                cJSON_AddNumberToObject(root, "on", dev_sub->Switch[1]);
                json = cJSON_PrintUnformatted(root);
                cJSON_DeleteItemFromObject(root, "on");
                modSvc(out_sub[j]->brgDevInfo.sn, out_sub[j]->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
                pos++;

                cJSON_AddNumberToObject(root, "gear", dev_sub->WindSpeed[1]);
                json = cJSON_PrintUnformatted(root);
                cJSON_DeleteItemFromObject(root, "gear");
                modSvc(out_sub[j]->brgDevInfo.sn, out_sub[j]->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
                break;
            default:
                break;
            }
        }

        if (newFlag)
        {
            strcpy(brgDevInfo->prodId, "2ANF");
            strcpy(brgDevInfo->model, "U2-86QMP-Z02(HW)");
            strcpy(brgDevInfo->devType, "075");

            out->devSvcNum = sizeof(svcId_HY0134) / sizeof(svcId_HY0134[0]);
            out->devSvc = malloc(sizeof(DevSvc) * out->devSvcNum);
            memset(out->devSvc, 0, sizeof(DevSvc) * out->devSvcNum);
            for (i = 0; i < out->devSvcNum; ++i)
                out->devSvc[i].svcId = svcId_HY0134[i];

            HilinkSyncBrgDevStatus(brgDevInfo->sn, DEV_ONLINE);
        }
        devSvcArray = out->devSvc;
        pos = 0;
        cJSON_AddNumberToObject(root, "num", dev_sub->KeyFobValue);
        json = cJSON_PrintUnformatted(root);
        cJSON_DeleteItemFromObject(root, "num");
        modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
        pos++;

        cJSON_AddStringToObject(root, "name", "智镜");
        json = cJSON_PrintUnformatted(root);
        cJSON_DeleteItemFromObject(root, "name");
        modSvc(out->brgDevInfo.sn, out->devSvc[pos].svcId, &devSvcArray[pos].svcVal, json);
    }
    break;
    case 5:
        break;
    case 6:
        break;
    default:
        goto fail;
    }
    list_print_all_hilink(&hilink_handle.node);

    free(root);

    return 0;
fail:
    free(root);
    return -1;
}

char *dev_hilink_modeId[] = {"2AN9", "2AN7", "2ANO", "2AN8", "2ANF", "2ANK", "2ANJ", "2ANI"};
int FrameNumber;

int getValueFromJson(cJSON *val, char *dst)
{
    if (val->valuestring != NULL)
    {
        strcpy(dst, val->valuestring);
    }
    else
    {
        char str[8];
        sprintf(str, "%d", val->valueint);
        strcpy(dst, str);
    }
    return 0;
}

int hilink_tolocal(const char *sn, const char *svcId, const char *payload)
{
    dev_hilink_t *in = list_get_by_id_hilink(sn, &hilink_handle.node);
    if (in == NULL)
        return -1;

    local_dev_t out = {0};
    int i;
    out.FrameNumber = FrameNumber++;

    strcpy(out.Data.DeviceId, sn);
    int modeId_index = str_search(in->brgDevInfo.prodId, dev_hilink_modeId, sizeof(dev_hilink_modeId) / sizeof(dev_hilink_modeId[0]));
    strcpy(out.Data.ModelId, dev_modeId[modeId_index]);

    cJSON *root = cJSON_Parse(payload);
    cJSON *val;
    switch (modeId_index)
    {
    case 0: //U2/天际系列：三键智能开关（HY0097）
    {
        int index = str_search(svcId, svcId_500c33, sizeof(svcId_500c33) / sizeof(svcId_500c33[0]));
        strcpy(out.Data.Key, attr_500c33[index]);

        if (cJSON_HasObjectItem(root, "on"))
        {
            strcpy(out.Type, "Ctrl");
            val = cJSON_GetObjectItem(root, "on");
        }
        else if (cJSON_HasObjectItem(root, "mode"))
        {
            strcpy(out.Type, "Attribute");
            val = cJSON_GetObjectItem(root, "mode");
        }
        else
        {
            goto fail;
        }
    }
    break;
    case 1: //U2/天际系列：DLT液晶调光器（09223f，型号U86KTGS150-ZXP）
    {
        int index = str_search(svcId, svcId_09223f, sizeof(svcId_09223f) / sizeof(svcId_09223f[0]));
        strcpy(out.Data.Key, attr_09223f[index]);

        strcpy(out.Type, "Ctrl");

        if (cJSON_HasObjectItem(root, "on"))
            val = cJSON_GetObjectItem(root, "on");
        else if (cJSON_HasObjectItem(root, "brightness"))
            val = cJSON_GetObjectItem(root, "brightness");
        else if (cJSON_HasObjectItem(root, "colorTemperature"))
            val = cJSON_GetObjectItem(root, "colorTemperature");
        else
            goto fail;
    }
    break;
    case 2: //3路智能开关模块（HY0107，型号IHC1240）
    {
        int index = str_search(svcId, svcId_HY0107, sizeof(svcId_HY0107) / sizeof(svcId_HY0107[0]));
        strcpy(out.Data.Key, attr_HY0107[index]);

        if (cJSON_HasObjectItem(root, "on"))
        {
            strcpy(out.Type, "Ctrl");
            val = cJSON_GetObjectItem(root, "on");
        }
        else if (cJSON_HasObjectItem(root, "mode"))
        {
            strcpy(out.Type, "Attribute");
            val = cJSON_GetObjectItem(root, "mode");
        }
        else
        {
            goto fail;
        }
    }
    break;
    case 3: //门磁传感器（HY0093，型号IHG5201）
        goto fail;
    case 4: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {
        int index = str_search(svcId, svcId_HY0134, sizeof(svcId_HY0134) / sizeof(svcId_HY0134[0]));
        if (index == 0)
        {
        }
        else if (index == 1)
        {
            strcpy(out.Data.Key, "SceName_1");
            val = cJSON_GetObjectItem(root, "name");
        }
        else
        {
            goto fail;
        }
        strcpy(out.Type, "Ctrl");
    }
    break;
    case 5: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {

        int index = str_search(svcId, svcId_HY0134_0, sizeof(svcId_HY0134_0) / sizeof(svcId_HY0134_0[0]));
        if (index == 0)
        {
            strcpy(out.Data.Key, "Switch_3");
            val = cJSON_GetObjectItem(root, "on");
        }
        else if (index == 1)
        {
            strcpy(out.Data.Key, "TargetTemperature_3");
            val = cJSON_GetObjectItem(root, "target");
        }
        else
        {
            goto fail;
        }
        strcpy(out.Type, "Ctrl");
    }
    break;
    case 6: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {
        int index = str_search(svcId, svcId_HY0134_1, sizeof(svcId_HY0134_1) / sizeof(svcId_HY0134_1[0]));
        if (index == 0)
        {
            strcpy(out.Data.Key, "Switch_1");
            val = cJSON_GetObjectItem(root, "on");
        }
        else if (index == 1)
        {
            strcpy(out.Data.Key, "TargetTemperature_1");
            val = cJSON_GetObjectItem(root, "target");
        }
        else if (index == 2)
        {
            strcpy(out.Data.Key, "WorkMode_1");
            val = cJSON_GetObjectItem(root, "mode");
        }
        else if (index == 3)
        {
            strcpy(out.Data.Key, "WindSpeed_1");
            val = cJSON_GetObjectItem(root, "gear");
        }
        else
        {
            goto fail;
        }
        strcpy(out.Type, "Ctrl");
    }
    break;
    case 7: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {
        int index = str_search(svcId, svcId_HY0134_2, sizeof(svcId_HY0134_2) / sizeof(svcId_HY0134_2[0]));
        if (index == 0)
        {
            strcpy(out.Data.Key, "Switch_2");
            val = cJSON_GetObjectItem(root, "on");
        }
        else if (index == 1)
        {
            strcpy(out.Data.Key, "WindSpeed_2");
            val = cJSON_GetObjectItem(root, "gear");
        }
        else
        {
            goto fail;
        }
        strcpy(out.Type, "Ctrl");
    }
    break;
    case 8:
    {
    }
    break;
    }
    getValueFromJson(val, out.Data.Value);
    write_to_local(&out);
    free(root);
    return 0;
fail:
    free(root);
    log_error("hilink_tolocal");
    return -1;
}