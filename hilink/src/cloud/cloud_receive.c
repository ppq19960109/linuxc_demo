#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/reboot.h>

#include "cloud_list.h"
#include "cloud_send.h"
#include "cloud_receive.h"
#include "local_send.h"
#include "local_list.h"
#include "local_device.h"
#include "local_callback.h"

#include "tool.h"
#include "rk_driver.h"
#include "hilink.h"
#include "hilink_softap_adapter.h"

static char *g_sCloudModel[] = {"2AP1", "2AP0", "2AN9", "2AN7", "2AOZ", "2AOY", "2ANO", "2AN8", "2ANF", "2ANK", "2ANJ", "2ANI"};
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
int cloud_tolocal(const char *sn, const char *svcId, const char *payload)
{
    dev_cloud_t *in = list_get_by_id_hilink(sn, cloud_get_list_head());
    if (in == NULL)
        return -1;

    local_dev_t out = {0};
    int i;
    out.FrameNumber = g_iFrameNumber++;

    strcpy(out.Data.DeviceId, sn);
    int modelIndex = str_search(in->brgDevInfo.prodId, g_SCloudModel.attr, g_SCloudModel.attrLen);
    strcpy(out.Data.ModelId, g_SLocalModel.attr[modelIndex]);

    cJSON *root = cJSON_Parse(payload);
    cJSON *val;
    int index = str_search(svcId, g_SCloudAttr[modelIndex].attr, g_SCloudAttr[modelIndex].attrLen);
    strcpy(out.Data.Key, g_SLocalAttr[modelIndex].attr[index]);
    switch (modelIndex)
    {
    case 0: //U2/天际系列：单键智能开关（HY0095）
    case 1: //U2/天际系列：双键智能开关（HY0096）
    case 2: //U2/天际系列：三键智能开关（HY0097）
    {

        if (cJSON_HasObjectItem(root, STR_ON))
        {
            strcpy(out.Type, STR_CTRL);
            val = cJSON_GetObjectItem(root, STR_ON);
        }
        else if (cJSON_HasObjectItem(root, STR_MODE))
        {
            strcpy(out.Type, STR_CTRL);
            val = cJSON_GetObjectItem(root, STR_MODE);
        }
        else
        {
            goto fail;
        }
    }
    break;
    case 3: //U2/天际系列：DLT液晶调光器（09223f，型号U86KTGS150-ZXP）
    {

        strcpy(out.Type, STR_CTRL);

        if (cJSON_HasObjectItem(root, STR_ON))
            val = cJSON_GetObjectItem(root, STR_ON);
        else if (cJSON_HasObjectItem(root, STR_BRIGHTNESS))
            val = cJSON_GetObjectItem(root, STR_BRIGHTNESS);
        else if (cJSON_HasObjectItem(root, STR_COLORTEMPERATURE))
            val = cJSON_GetObjectItem(root, STR_COLORTEMPERATURE);
        else
            goto fail;
    }
    break;
    case 4: //1路智能开关模块（HY0121，型号IHC1238）
    case 5: //2路智能开关模块（HY0122，型号IHC1239）
    case 6: //3路智能开关模块（HY0107，型号IHC1240）
    {

        if (cJSON_HasObjectItem(root, STR_ON))
        {
            strcpy(out.Type, STR_CTRL);
            val = cJSON_GetObjectItem(root, STR_ON);
        }
        else if (cJSON_HasObjectItem(root, STR_MODE))
        {
            strcpy(out.Type, STR_CTRL);
            val = cJSON_GetObjectItem(root, STR_MODE);
        }
        else
        {
            goto fail;
        }
    }
    break;
    case 7: //门磁传感器（HY0093，型号IHG5201）
        goto fail;
    case 8: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {

        if (cJSON_HasObjectItem(root, STR_NAME))
        {
            sprintf(out.Data.Key, "SceName_%d", index);
            val = cJSON_GetObjectItem(root, STR_NAME);
        }
        else
        {
            goto fail;
        }
        strcpy(out.Type, STR_CTRL);
    }
    break;
    case 9: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {
        out.Data.DeviceId[strlen(out.Data.DeviceId) - 1] = '\0';
        if (cJSON_HasObjectItem(root, STR_ON))
        {
            val = cJSON_GetObjectItem(root, STR_ON);
        }
        else if (cJSON_HasObjectItem(root, STR_TARGET))
        {
            val = cJSON_GetObjectItem(root, STR_TARGET);
        }
        else
        {
            goto fail;
        }
        strcpy(out.Type, STR_CTRL);
    }
    break;
    case 10: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {
        out.Data.DeviceId[strlen(out.Data.DeviceId) - 1] = '\0';
        if (cJSON_HasObjectItem(root, STR_ON))
        {
            val = cJSON_GetObjectItem(root, STR_ON);
        }
        else if (cJSON_HasObjectItem(root, STR_MODE))
        {
            val = cJSON_GetObjectItem(root, STR_MODE);
        }
        else if (cJSON_HasObjectItem(root, STR_TARGET))
        {
            val = cJSON_GetObjectItem(root, STR_TARGET);
        }
        else if (cJSON_HasObjectItem(root, STR_GEAR))
        {
            val = cJSON_GetObjectItem(root, STR_GEAR);
        }
        else
        {
            goto fail;
        }
        strcpy(out.Type, STR_CTRL);
    }
    break;
    case 11: //U2/天际系列：智镜/全面屏/触控屏（HY0134）
    {
        out.Data.DeviceId[strlen(out.Data.DeviceId) - 1] = '\0';

        if (cJSON_HasObjectItem(root, STR_ON))
        {
            strcpy(out.Data.Key, "Switch_2");
            val = cJSON_GetObjectItem(root, STR_ON);
        }
        else if (cJSON_HasObjectItem(root, STR_GEAR))
        {
            strcpy(out.Data.Key, "WindSpeed_2");
            val = cJSON_GetObjectItem(root, STR_GEAR);
        }
        else
        {
            goto fail;
        }
        strcpy(out.Type, STR_CTRL);
    }
    break;
    default:
        break;
    }

    getValue_FromJson(val, out.Data.Value);
    write_to_local(&out);
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
    char ssn[32] = {0};

    dev_cloud_t *dev_cloud = list_get_by_id_hilink(sn, cloud_get_list_head());
    int index = str_search(dev_cloud->brgDevInfo.prodId, g_SCloudModel.attr, g_SCloudModel.attrLen);

    stpcpy(ssn, sn);

    switch (index)
    {
    case 8:
    {
        int p = strlen(sn);
        for (int j = 0; j < 3; j++)
        {
            ssn[p] = j + '0';
            list_del_by_id_hilink(ssn, cloud_get_list_head());
            HilinkSyncBrgDevStatus(ssn, DEV_RESTORE);
        }
        ssn[p] = 0;
    }
    break;
    case 9:
    case 10:
    case 11:
    {
        int p = strlen(sn);
        ssn[p] = 0;
        --p;
        for (int j = 0; j < 3; j++)
        {
            ssn[p] = j + '0';
            list_del_by_id_hilink(ssn, cloud_get_list_head());
            HilinkSyncBrgDevStatus(ssn, DEV_RESTORE);
        }
        ssn[p] = 0;
    }
    break;
    default:
        break;
    }

    list_del_by_id_hilink(ssn, cloud_get_list_head());

    HilinkSyncBrgDevStatus(ssn, DEV_RESTORE);
    list_del_by_id(ssn, local_get_list_head());

    out.FrameNumber = g_iFrameNumber++;
    strcpy(out.Type, STR_DELETE);
    strcpy(out.Data.DeviceId, ssn);

    return write_to_local(&out);
}

void cloud_restart_reFactory(int index)
{
    log_info("cloud_restart_reFactory:%d\n", index);
    write_hanyar_cmd(STR_ADD, NULL, STR_NET_CLOSE);
    HILINK_StopSoftAp();

    if (index == INT_REFACTORY)
    {
        write_hanyar_cmd(STR_REFACTORY, NULL, NULL);

        hilink_all_online(0, DEV_RESTORE);
        cloud_control_destory();
        local_control_destory();
        sync();
        run_closeCallback();
        driver_exit();
        hilink_restore_factory_settings();

        sleep(2);
        system("sh /userdata/app/restore.sh &");
    }
    else
    {
        hilink_all_online(0, DEV_OFFLINE);
        cloud_control_destory();
        local_control_destory();
        sync();
        run_closeCallback();
        driver_exit();
        if (INT_REBOOT == index)
        {
            reboot(RB_AUTOBOOT);
        }
    }
    printf("\texit(0)\t\n");
    exit(0);
}