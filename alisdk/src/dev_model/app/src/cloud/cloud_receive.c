#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cloud_list.h"
#include "cloud_send.h"
#include "cloud_receive.h"
#include "local_send.h"
#include "local_list.h"
#include "local_device.h"

#include "tool.h"

static char *g_sCloudModel[] = {"a1BqBAOw2ii", "a1mTtj3XyVA", "a1T6JIBWh5o", "a1OS7SzKDvr", "a1V9RC5hvfW", "a1H5zYD0dI9", "a10rNqQLbXN", "a1qv0dhUJoT", "a1A2bm9AaDT"};
//单键智能开关 双键智能开关 三键智能开关 DLT液晶调光器 1路智能开关模块 2路智能开关模块 3路智能开关模块 门磁传感器  智镜:场景面板 地暖 空调 新风
SAttrInfo g_SCloudModel = {
    .attr = g_sCloudModel,
    .attrLen = sizeof(g_sCloudModel) / sizeof(g_sCloudModel[0])};

static int g_iFrameNumber;

static int getValue_FromJson(cJSON *val, char *dst)
{
    if (val->valuestring != NULL)
    {
        strcpy(dst, val->valuestring);
    }
    else
    {
        sprintf(dst, "%d", val->valueint);
    }
    return 0;
}

//cloud转换成hanyar的json格式
int cloud_tolocal(void *sn, const char *payload)
{
    dev_cloud_t *in = list_get_by_devid_cloud((int)sn, cloud_get_list_head(&g_SCloudControl));
    if (in == NULL)
        return -1;

    local_dev_t out = {0};
    int i;
    out.FrameNumber = g_iFrameNumber++;

    strcpy(out.Data.DeviceId, in->brgDevInfo.sn);

    int index = str_search(in->brgDevInfo.meta_info.product_key, g_SCloudModel.attr, g_SCloudModel.attrLen);
    strcpy(out.Data.ModelId, g_SLocalModel.attr[index]);

    cJSON *root = cJSON_Parse(payload);
    cJSON *val;

    strcpy(out.Type, STR_CTRL);
    for (i = 0; i < g_SCloudAttr[index].attrLen; ++i)
    {
        if (cJSON_HasObjectItem(root, g_SCloudAttr[index].attr[i]))
        {
            if (index == 8)
            {
                int index_attr = str_search(g_SCloudAttr[index].attr[i], g_SCloudAttr[index].attr, g_SCloudAttr[index].attrLen);
                char *buffer;
                switch (index_attr)
                {
                case 0:
                    buffer = "CurrentTemperature_1";
                    break;
                case 1:
                    buffer = "Switch_1";
                    break;
                case 2:
                    buffer = "WindSpeed_1";
                    break;
                case 3:
                    buffer = "TargetTemperature_1";
                    break;
                case 4:
                    buffer = "WorkMode_1";
                    break;
                case 5:
                    buffer = "Switch_2";
                    break;
                case 6:
                    buffer = "WindSpeed_2";
                    break;
                case 7:
                    buffer = "Switch_3";
                    break;
                case 8:
                    buffer = "TargetTemperature_3";
                    break;
                }
                strcpy(out.Data.Key, buffer);
            }
            else
            {
                strcpy(out.Data.Key, g_SLocalAttr[index].attr[i]);
            }
            val = cJSON_GetObjectItem(root, g_SCloudAttr[index].attr[i]);
            break;
        }
    }
    getValue_FromJson(val, out.Data.Value);
    write_to_local(&out, &g_SLocalControl);
    cJSON_Delete(root);
    return 0;
fail:
    cJSON_Delete(root);
    log_error("cloud_tolocal fail\n");
    return -1;
}

int cloud_delete_device(const char *sn)
{
    local_dev_t out = {0};

    out.FrameNumber = g_iFrameNumber++;
    strcpy(out.Type, STR_DELETE);
    strcpy(out.Data.DeviceId, sn);

    return write_to_local(&out, &g_SLocalControl);
}

void cloud_restart_reFactory(int index)
{
    write_hanyar_cmd(STR_ADD, NULL, STR_NET_CLOSE);
    if (index)
    {
        write_hanyar_cmd(STR_REFACTORY, NULL, NULL);

        cloud_control_destory(&g_SCloudControl);
        local_control_destory(&g_SLocalControl);
    }
    else
    {

        cloud_control_destory(&g_SCloudControl);
        local_control_destory(&g_SLocalControl);
    }
}