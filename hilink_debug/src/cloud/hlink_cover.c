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

char *svcId_500c33[] = {"switch", "backlight", "switchLock", "lightmode", "countdown1", "countdown2", "countdown3"};
char *svcId_HY0093[] = {"doorEvent", "status"};

int modSvc(char **svcVal, char *json)
{
    if (*svcVal != NULL)
        free(*svcVal);
    *svcVal = json;
}

int local_tohilink(dev_data_t *src, int index)
{
    log_debug("local_tohilink index:%d", index);
    int newFlag = 0;
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
        if (newFlag)
        {
            strcpy(brgDevInfo->prodId, "203I");
            strcpy(brgDevInfo->model, "U2-86K31ND10-ZD(TY)-GY");
            strcpy(brgDevInfo->devType, "005");

            out->devSvcNum = sizeof(svcId_500c33) / sizeof(svcId_500c33[0]);
            out->devSvc = malloc(sizeof(DevSvc) * out->devSvcNum);
            memset(out->devSvc, 0, sizeof(DevSvc) * out->devSvcNum);
            for (int i = 0; i < out->devSvcNum; ++i)
                out->devSvc[i].svcId = (char *)svcId_500c33[i];

            HilinkSyncBrgDevStatus(brgDevInfo->sn, DEV_ONLINE);
        }
        devSvcArray = out->devSvc;
        dev_500c33_t *dev_sub = (dev_500c33_t *)src->private;

        //Switch
        for (int i = 0; i < 3; ++i)
        {
            dev_sub->Switch[i] = 1;
            cJSON_AddNumberToObject(root, "on", dev_sub->Switch[i]);
            json = cJSON_PrintUnformatted(root);
            cJSON_DeleteItemFromObject(root, "on");

            modSvc(&devSvcArray[i].svcVal, json);
        }
        //lightmode
        cJSON_AddNumberToObject(root, "status", dev_sub->LedEnable);
        json = cJSON_PrintUnformatted(root);
        cJSON_DeleteItemFromObject(root, "status");
        modSvc(&devSvcArray[3].svcVal, json);
    }
    break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;
    case 4:
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

char *dev_hilink_modeId[] = {"203I", "2", "3", "4", "5"};
int FrameNumber;

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
    switch (modeId_index)
    {
    case 0: //U2/天际系列：三键智能开关（HY0097）
    {
        int index = str_search(svcId, svcId_500c33, sizeof(svcId_500c33) / sizeof(svcId_500c33[0]));
        strcpy(out.Data.ModelId, attr_500c33[index]);

        cJSON *val;
        if (cJSON_HasObjectItem(root, "on"))
        {
            strcpy(out.Type, "Ctrl");
            val = cJSON_GetObjectItem(root, "on");
        }
        else if (cJSON_HasObjectItem(root, "status"))
        {
            strcpy(out.Type, "Attribute");
            val = cJSON_GetObjectItem(root, "status");
        }
        else
        {
            goto fail;
        }

        if (val->valuestring != NULL)
        {
            strcpy(out.Data.Value, val->valuestring);
        }
        else
        {
            char str[3];
            sprintf(str, "%d", val->valueint);
            strcpy(out.Data.Value, str);
        }
    }
    break;
    case 1: //U2/天际系列：DLT液晶调光器（09223f，型号U86KTGS150-ZXP）
    {
    }
    break;
    }
    write_to_local(&out);
    free(root);
    return 0;
fail:
    free(root);
    log_error("hilink_tolocal");
    return -1;
}