/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: 此文件提供了集成网桥功能所需开发者实现的接口的说明，HiLink SDK会在适时调用对应接口;
 *              需开发者调用的接口详见hilink_profile_bridge.h;
 *              请开发者仔细阅读文件中的注释说明，参考或修改实现。
 * Create: 2019-03-27
 */
#include "hilink_profile_bridge.h"

#ifndef NULL
#define NULL 0
#endif

/*
 * 获取设备的设备信息
 * 该函数由设备开发者或厂商实现
 * sn: Bridge下挂设备唯一标识
 * devInfo: sdk开辟空间，由厂商填充设备信息
 * 返回0: 设备信息获取成功，devInfo指向的设备信息正确有效
 * 返回非0: 设备信息获取失败，devInfo指向的设备信息无效
 */
int HilinkGetBrgDevInfo(const char* sn, BrgDevInfo* devInfo)
{
    /* 厂商实现此接口 */
    if ((sn == NULL) || (devInfo == NULL)) {
        return -1;
    }
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
int HilinkGetBrgSvcInfo(const char* sn, BrgDevSvcInfo* svcInfo, unsigned int* svcNum)
{
    /* 厂商实现此接口 */
    if ((sn == NULL) || (svcInfo == NULL) || (svcNum == NULL)) {
        return -1;
    }
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
int HilinkPutBrgDevCharState(const char* sn, const char* svcId, const char* payload, unsigned int len)
{
    /* 厂商实现此接口 */
    if ((sn == NULL) || (svcId == NULL) || (payload == NULL)) {
        return -1;
    }
    return 0;
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
int HilinkGetBrgDevCharState(const char* sn, GetBrgDevCharState* in, char** out, unsigned int* outLen)
{
    /* 厂商实现此接口 */
    if ((sn == NULL) || (in == NULL) || (out == NULL) || (outLen == NULL)) {
        return -1;
    }
    return 0;
}

/*
 * 删除bridge下挂设备
 * 该函数由设备开发者或厂商实现
 * sn: Bridge桥下挂设备唯一标识
 */
int HilinkDelBrgDev(const char* sn)
{
    /* 厂商实现此接口 */
    if (sn == NULL) {
        return -1;
    }
    return 0;
}
