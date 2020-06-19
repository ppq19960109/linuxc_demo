/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: HiLink SDK网桥相关头文件
 */
#ifndef HILINK_PROFILE_BRIDGE_H
#define HILINK_PROFILE_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HILINK_M2M_MAX_SVC_NUM
#define HILINK_M2M_MAX_SVC_NUM 20
#endif

#define ST_STR_MAX_LEN  32
#define SVCID_STR_MAX_LEN    64
#ifndef BRG_MAX_SUB_DEV_NUM
#define BRG_MAX_SUB_DEV_NUM  64
#endif

/* Hilink Device SDK 对外开放接口 */
typedef struct {
    char sn[40];     /* 设备唯一标识，比如sn号，长度范围（0,40] */
    char prodId[5];  /* 设备HiLink认证号，长度范围（0,5] */
    char model[32];  /* 设备型号，长度范围（0,32] */
    char devType[4]; /* 设备类型，长度范围（0,4] */
    char manu[4];    /* 设备制造商，长度范围（0,4] */
    char mac[32];    /* 设备MAC地址，固定32字节 */
    char hiv[32];    /* 设备Hilink协议版本，长度范围（0,32] */
    char fwv[64];    /* 设备固件版本，长度范围[0,64] */
    char hwv[64];    /* 设备硬件版本，长度范围[0,64] */
    char swv[64];    /* 设备软件版本，长度范围[0,64] */
    int protType;    /* 设备协议类型，取值范围[1,3] */
} BrgDevInfo;

typedef struct {
    char st[HILINK_M2M_MAX_SVC_NUM][ST_STR_MAX_LEN];
    char svcId[HILINK_M2M_MAX_SVC_NUM][SVCID_STR_MAX_LEN];
} BrgDevSvcInfo;

typedef enum {
    DEV_OFFLINE = 0,    /* 设备下线 */
    DEV_ONLINE  = 1,    /* 设备上线 */
    DEV_RESTORE = 2,    /* 设备恢复出厂，删除云端信息 */
    DEV_ADD     = 3     /* 设备恢复出厂之后重新注册 */
} DevOnlineStatus;

typedef struct {
    const char *svcId;    /* 待获取的服务ID */
    const char *json;     /* Json格式的属性字段，为NULL时获取svcId服务下的所有属性 */
    unsigned int jsonLen; /* json不为NULL时，Json格式字符串长度 */
} GetBrgDevCharState;

/*
 * Bridge上报下挂设备状态
 * 该函数由设备开发者或厂商调用
 * sn: Bridge桥下挂设备唯一标识
 * status: 下挂设备状态
 * 注意：DEV_RESTORE、DEV_ADD只能在网桥处于直连云模式下使用
 */
int HilinkSyncBrgDevStatus(const char *sn, DevOnlineStatus status);

/*
 * 获取设备的设备信息
 * 该函数由设备开发者或厂商实现
 * sn: Bridge下挂设备唯一标识
 * devInfo: sdk开辟空间，由厂商填充设备信息
 * 返回0: 设备信息获取成功，devInfo指向的设备信息正确有效
 * 返回非0: 设备信息获取失败，devInfo指向的设备信息无效
 */
int HilinkGetBrgDevInfo(const char *sn, BrgDevInfo *devInfo);

/*
 * 获取设备的服务信息。
 * 该函数由设备开发者或厂商实现
 * sn: Bridge下挂设备唯一标识。
 * svcInfo: sdk开辟空间，由厂商填充
 * svcNum: 由厂商根据实际填充的服务个数填充该值
 * 返回0: 服务信息获取成功，svcInfo指向的服务信息正确有效
 * 返回非0: 服务信息获取失败，svcInfo指向的服务信息无效
 */
int HilinkGetBrgSvcInfo(const char *sn, BrgDevSvcInfo *svcInfo, unsigned int *svcNum);

/*
 * 上报桥下设备具有report属性的状态
 * 该函数由设备开发者或厂商调用
 * 注意: 只具有report能力的属性字段需使用此接口上报事件，只允许与hilink_m2m_process在一个任务中调用
 * sn: 桥下挂设备SN
 * svcId: 桥下挂设备服务ID
 * payload: Json格式的字段与其值
 * len: payload的长度
 * tid: 接口调用所在taskid 需同m2m taskid, 用于保证同一任务中调用
 * 返回0: 服务状态上报成功
 * 返回非0: 服务状态上报失败
 */
int HilinkReportBrgDevCharState(const char *sn, const char *svcId,
    char *payload, unsigned int len, unsigned long tid);

/*
 * 上报Bridge桥下挂设备服务状态
 * 该函数由设备开发者或厂商调用
 * 注意: 此接口发送上报事件通知，服务字段状态由Hilink Device SDK统一查询后上报，大概延时200ms
 * sn: Bridge桥下挂设备唯一标识
 * svcid: Bridge桥下挂设备服务ID
 * 返回0: 服务状态上报成功
 * 返回非0: 服务状态上报失败
 */
int HilinkUploadBrgDevCharState(const char *sn, const char *svcId);

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
int HilinkPutBrgDevCharState(const char *sn, const char *svcId, const char *payload, unsigned int len);

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
int HilinkGetBrgDevCharState(const char *sn, GetBrgDevCharState *in, char **out, unsigned int *outLen);

/*
 * 删除bridge下挂设备
 * 该函数由设备开发者或厂商实现
 * sn: Bridge桥下挂设备唯一标识
 */
int HilinkDelBrgDev(const char *sn);

/*
 * bride下挂设备恢复出厂
 * 该函数由设备开发者或厂商调用
 * sn: Bridge桥下挂设备唯一标识
 */
int HilinkResetBrgDev(const char *sn);

/*
 * bride下挂设备升级完成通知SDK
 * 该函数由设备开发者或厂商调用
 * sn表示桥下挂设备唯一标识，code表示升级结果
 */
void HiLinkBrgDevOtaFinish(const char *sn, int code);

/*
 * 子设备devinfo更新后可调用此接口同步
 * sn表示桥下挂设备唯一标识，taskId表示接口调用所在任务id
 */
int HiLinkSyncBrgDevInfo(const char *sn, unsigned long taskId);

/*
 * 设备开发者或者厂商调用此接口实现网桥下挂子设备服务/属性的批量上报
 * payload为json格式的数据，注意是数组形式
    [{
      "devId": "2389294",
      "sn": "00E0FC018009",
      "services": [{
          "sid": "light1",
          "data": {
            "characteristicName1": "value1",
            "characteristicName2": "value2"
        }
      }]
    }]
 * 返回0表示上报成功，其他表示失败
 * 注意:payload必须以'\0'为结束符，否则可能导致异常
 */
int HILINK_ReportBrgAllSubDevState(const char *payload);

#ifdef __cplusplus
}
#endif

#endif  /* HILINK_PROFILE_BRIDGE_H */
