/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: 此文件提供了集成网桥功能所需开发者实现的接口的说明，HiLink SDK会在适时调用对应接口;
 *              需开发者调用的接口详见hilink_profile_bridge.h;
 *              请开发者仔细阅读文件中的注释说明，参考或修改实现。
 * Create: 2019-03-27
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hilink_profile_bridge.h"
#include "hilink_profile_adapter.h"

#include "hilink_cover.h"
#include "list_hilink.h"
#include "protocol_cover.h"
#include "list_tool.h"

#ifndef NULL
#define NULL 0
#endif
int hilink_strlen(const char *);

/*
 * 获取设备的设备信息
 * 该函数由设备开发者或厂商实现
 * sn: Bridge下挂设备唯一标识
 * devInfo: sdk开辟空间，由厂商填充设备信息
 * 返回0: 设备信息获取成功，devInfo指向的设备信息正确有效
 * 返回非0: 设备信息获取失败，devInfo指向的设备信息无效
 */
int HilinkGetBrgDevInfo(const char *sn, BrgDevInfo *devInfo)
{

    /* 厂商实现此接口 */
    if ((sn == NULL) || (devInfo == NULL))
    {
        return -1;
    }
    dev_hilink_t *dev = list_get_by_id_hilink(sn, &hilink_handle.node);
    if (dev == NULL)
    {
        return -1;
    }
    memcpy(devInfo, &dev->brgDevInfo, sizeof(BrgDevInfo));
    log_debug("HilinkGetBrgDevInfo sn:%s", sn);
    return 0;
}

/*
 * 获取设备的服务信息。
 * 该函数由设备开发者或厂商实现
 * sn: Bridge下挂设备唯一标识。
 * svcInfo: sdk开辟空间，由厂商填充
 * svcNum: 由厂商根据实际填充的服务个数填充该值
 * 返回0: 服务信息获取成功，svcInfo指向的服务信息正确有效
 * 返回非0: 服务信息获取失败，svcInfo指向的服务信息无效
 */
int HilinkGetBrgSvcInfo(const char *sn, BrgDevSvcInfo *svcInfo, unsigned int *svcNum)
{
    log_debug("HilinkGetBrgSvcInfo sn:%s ", sn);
    /* 厂商实现此接口 */
    if ((sn == NULL) || (svcInfo == NULL) || (svcNum == NULL))
    {
        log_debug("HilinkGetBrgSvcInfo NULL");
        return -1;
    }

    dev_hilink_t *dev = list_get_by_id_hilink(sn, &hilink_handle.node);
    if (dev == NULL)
    {
        log_debug("HilinkGetBrgSvcInfo err");
        return -1;
    }
    for (int i = 0; i < dev->devSvcNum; ++i)
    {
        strcpy(svcInfo->st[i], dev->devSvc[i].svcId);
        strcpy(svcInfo->svcId[i], dev->devSvc[i].svcId);
    }
    *svcNum = dev->devSvcNum;
    log_debug("HilinkGetBrgSvcInfo svcInfo:%s ", svcInfo->st[0]);
    return 0;
}

/*
 * 设置服务状态
 * 该函数由设备开发者或厂商实现
 * sn: Bridge桥下挂设备唯一标识
 * svcId: 服务ID
 * payload: 需要设置的Json格式的字段与其值
 * len: 接收到的payload的长度，范围为[0，512)
 * 返回M2M_SEARCH_GW_INVALID_PACKET(-101): 获得报文不符合要求
 * 返回M2M_SVC_STUTAS_VALUE_MODIFYING(-111): 服务状态值正在修改中，修改成功后底层设备必须主动上报
 * 返回M2M_NO_ERROR(0): 服务状态值修改成功，不需要底层设备主动上报，由Hilink Device SDK上报
 */
int HilinkPutBrgDevCharState(const char *sn, const char *svcId, const char *payload, unsigned int len)
{
    log_debug("HilinkPutBrgDevCharState svcId:%s,payload:%s", svcId, payload);
    /* 厂商实现此接口 */
    if ((sn == NULL) || (svcId == NULL) || (payload == NULL))
    {
        return -1;
    }

    return hilink_tolocal(sn, svcId, payload);
}

/*
 * 获取服务状态
 * 该函数由设备开发者或厂商实现
 * sn: Bridge桥下挂设备唯一标识
 * in: 获取哪些服务属性的值
 * out: 返回保存服务字段值内容的指针,内存由厂商开辟，使用完成后，由Hilink Device SDK释放
 * outLen: 读取到的payload的长度，范围为[0，512)
 * 返回0: 获取服务状态成功
 * 返回非0: 获取服务状态失败
 * 注意: out需要动态申请，Hilink Device SDK使用完成后会调用hilink_free接口释放
 */
int HilinkGetBrgDevCharState(const char *sn, GetBrgDevCharState *in, char **out, unsigned int *outLen)
{
    log_debug("HilinkGetBrgDevCharState svcId:%s,json:%s,jsonLen:%d", in->svcId, in->json, in->jsonLen);
    /* 厂商实现此接口 */
    if ((sn == NULL) || (in == NULL) || (out == NULL) || (outLen == NULL))
    {
        return -1;
    }

    dev_hilink_t *dev = list_get_by_id_hilink(sn, &hilink_handle.node);
    if (dev == NULL)
    {
        return -1;
    }
    for (int i = 0; i < dev->devSvcNum; ++i)
    {
        if (strcmp(dev->devSvc[i].svcId, in->svcId) == 0)
        {
            if (dev->devSvc[i].svcVal == NULL)
                return -1;

            *outLen = strlen(dev->devSvc[i].svcVal) + 1;
            *out = malloc(*outLen);
            strcpy(*out, dev->devSvc[i].svcVal);
            log_debug("HilinkGetBrgDevCharState %s", *out);
            break;
        }
    }

    return 0;
}

/*
 * 获取Bridge下挂设备房间号
 * 该函数由设备开发者或厂商实现
 * sn: Bridge桥下挂设备唯一标识
 * roomId: 返回设备房间号
 * roomIdLen: roomId内存大小，拷贝不可超过该长度，返回实际拷贝的字节数
 * devName: 返回设备名称（可选）
 * devNameLen: devName内存大小，拷贝不可超过该长度，返回实际拷贝的字节数
 * 返回0: 获取下挂设备房间号成功
 * 返回非0: 获取下挂设备房间号失败
 */
int HILINK_GetBrgSubDevRoomInfo(const char *sn, char *roomId, unsigned int *roomIdLen,
                                char *devName, unsigned int *devNameLen)

{
    /* 厂商实现此接口 */
    if ((sn == NULL) || (roomId == NULL) || (roomIdLen == NULL) || (devName == NULL) || (devNameLen == NULL))
    {
        return -1;
    }
    char* name="1";
    strcpy(roomId,name);
    *roomIdLen=strlen(name)+1;
    return 0;
}

/*
 * 删除bridge下挂设备
 * 该函数由设备开发者或厂商实现
 * sn: Bridge桥下挂设备唯一标识
 */
int HilinkDelBrgDev(const char *sn)
{
    log_debug("HilinkDelBrgDev sn:%s", sn);
    /* 厂商实现此接口 */
    if (sn == NULL)
    {
        return -1;
    }
    HilinkSyncBrgDevStatus(sn, DEV_RESTORE);
    list_del_by_id_hilink(sn, &hilink_handle.node);
    list_del_by_id(sn, &protocol_data.dev_list);
    list_print_all_hilink(&hilink_handle.node);
    list_print_all(&protocol_data.dev_list);
    return hilink_delete(sn);
}

/*
 * 根据sn返回三元组的confirmationKey信息
 * 如果未计算完成则返回1，失败返回-1，成功返回0
 * 该函数由设备开发者或厂商实现
 * sn: Bridge桥下挂设备唯一标识
 */
int HILINK_GetBrgDevCfmKey(const char *sn, char *devConfirmationKey, unsigned int devConfirmationKeyLen)
{
    log_debug("HILINK_GetBrgDevCfmKey sn:%s", sn);
    /* 厂商实现此接口 */
    if ((sn == NULL) || (devConfirmationKey == NULL))
    {
        return -1;
    }

    if (hilink_strlen(devConfirmationKey) > devConfirmationKeyLen)
    {
        return -1;
    }

    return 0;
}

int HILINK_GetBrgDevThirdProdId(const char *sn, char *thirdProdId, unsigned int thirdProdIdLen)
{
    log_debug("HILINK_GetBrgDevThirdProdId sn:%s", sn);
    /* 厂商实现此接口 */
    if ((sn == NULL) || (thirdProdId == NULL))
    {
        return -1;
    }

    if (hilink_strlen(thirdProdId) > thirdProdIdLen)
    {
        return -1;
    }

    return 0;
}

/*
 * 根据sn返回三元组的confirmation和random信息
 * 如果未计算完成则返回或者失败都返回-1，成功返回0
 * 该函数由设备开发者或厂商实现
 * sn: Bridge桥下挂设备唯一标识
 */
int HILINK_GetBrgDevCfmAndRnd(const char *sn, char *devConfirmation, unsigned int devConfirmationLen,
                              char *devRandom, unsigned int devRandomLen)
{
    log_debug("HILINK_GetBrgDevCfmAndRnd sn:%s", sn);
    /* 厂商实现此接口 */
    if ((sn == NULL) || (devConfirmation == NULL) || (devRandom == NULL))
    {
        return -1;
    }

    if ((hilink_strlen(devConfirmation) > devConfirmationLen) || (hilink_strlen(devRandom) > devRandomLen))
    {
        return -1;
    }

    return 0;
}

/*
 * 根据sn返回云的confirmation和random信息
 * 如果未计算完成则返回或者失败都返回-1，成功返回0
 * 该函数由设备开发者或厂商实现
 * sn: Bridge桥下挂设备唯一标识
 */
int HILINK_NotifyBrgSubDevCloudCfmAndRnd(const char *sn, const char *cloudConfirmation,
                                         unsigned int cloudConfirmationLen, const char *cloudRandom, unsigned int cloudRandomLen)
{
    log_debug("HILINK_NotifyBrgSubDevCloudCfmAndRnd sn:%s", sn);
    /* 厂商实现此接口 */
    if ((sn == NULL) || (cloudConfirmation == NULL) || (cloudRandom == NULL))
    {
        return -1;
    }

    if ((hilink_strlen(cloudConfirmation) > cloudConfirmationLen) || (hilink_strlen(cloudRandom) > cloudRandomLen))
    {
        return -1;
    }

    return 0;
}

/*
 * 根据sn返回云的鉴权信息
 * 如果失败返回-1，成功返回0
 * 该函数由设备开发者或厂商实现
 * sn: Bridge桥下挂设备唯一标识
 */
int HILINK_NotifyBrgSubDevCloudCfmResult(const char *sn, int result)
{
    log_debug("HILINK_NotifyBrgSubDevCloudCfmResult sn:%s", sn);
    /* 厂商实现此接口 */
    if (sn == NULL)
    {
        return -1;
    }
    return 0;
}

/*
 * 获取子设备的最大数目,最大数目不超过500,不小于64,超出该范围
 * 默认使用64.
 * 该函数由设备开发者或厂商实现
 * 返回值：设备开发者设置的下挂设备的最大数目
 */
unsigned int HILINK_GetBrgSubDevMaxNum()
{
    /* 厂商实现此接口 */
    unsigned int maxNum = 128;

    return maxNum;
}
