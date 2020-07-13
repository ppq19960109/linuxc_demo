/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: HiLink主流程框架集成头文件
 * Create: 2018-06-22
 */
#ifndef _HILINK_H_
#define _HILINK_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * HiLink SDK属性结构体，开发者可以通过HILINK_GetSdkAttr查看当前系统的属性值，通过HILINK_SetSdkAttr设置新的属性值；
 * 注意: 1) 普通设备形态和网桥设备形态的主线程任务栈大小，开发者根据产品形态，仅需设置对应产品形态的属性值即可；
 *       2) 使用HiLink SDK升级架构时，需要设置升级检查任务的栈大小和升级任务的栈大小
 *       3) 如果开发者未注册软重启接口rebootSoftware和硬重启接口rebootHardware，使用HiLink SDK默认实现接口(硬重启)；
 */
typedef struct {
    unsigned long monitorTaskStackSize;    /* 监控任务栈大小，开发者根据具体情况调整，默认为1024字节 */
    unsigned long deviceMainTaskStackSize; /* 普通设备形态，HiLink SDK运行主任务栈大小，开发者根据具体情况调整 */
    unsigned long bridgeMainTaskStackSize; /* 网桥设备形态，HiLink SDK运行主任务栈大小，开发者根据具体情况调整 */
    unsigned long otaCheckTaskStackSize;   /* HiLink OTA检查升级版本任务栈大小，开发者根据具体情况调整 */
    unsigned long otaUpdateTaskStackSize;  /* HiLink OTA升级任务栈大小，开发者根据具体情况调整 */
    int (*rebootSoftware)(void);           /* 异常场景软重启接口，不影响硬件状态，如果用户注册，首先使用此接口 */
    int (*rebootHardware)(void);           /* 异常场景硬重启接口，影响硬件状态，如果用户没有注册软重启，使用此接口重启 */
} HILINK_SdkAttr;

/*
 * HiLink SDK入口函数
 * 返回0表示成功，返回-1表示失败
 */
int hilink_main(void);

/* 获取当前设备状态，如配网状态、连接云端、在线、离线等 */
int hilink_get_devstatus(void);

/*
 * 开发者直接调用该接口完成设备恢复出厂设置
 * 返回0表示恢复出厂成功,返回-1表示恢复出厂失败
 * 该接口会设置恢复出厂标志，设置成功后会清理掉ssid账号信息，并重启模组
 */
int hilink_restore_factory_settings(void);

/*
 * 获取存储在Flash的用户配置信息
 * 参数len表示用户需要获取的配置信息长度
 * 参数config表示用户获取的配置信息缓存
 * 返回0表示获取成功，返回-1表示获取失败
 * config的内存分配和初始化由调用者完成;用户可获取的配置信息最大长度为32字节
 */
int HilinkGetUserConfig(unsigned short len, char *config);

/*
 * 设置用户配置信息到flash
 * 参数config表示用户配置信息
 * 参数len表示用户配置信息长度
 * 返回0表示设置成功，返回-1表示设置失败
 * 信息写入为覆盖更新，每次写入前会清空旧的数据;用户可存储的配置信息最大长度为32字节
 */
int HilinkSetUserConfig(const char *config, unsigned short len);

typedef struct {
    unsigned short year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
    unsigned short ms;
    /* bit0置位表示星期一, 以此类推, 星期日为bit6置位, bit7无意义 */
    unsigned char w_day;
} stHILINK_TIME_INFO, *pstHILINK_TIME_INFO;

/* 时区字符串最大长度 */
#define TIME_ZONE_STR_MAX_LEN 64

/*
 * 时间同步模块获取本地日期信息
 * 返回0表示成功，返回非0表示失败
 */
int hilink_get_local_time_info(pstHILINK_TIME_INFO p_time_info);

/*
 * 时间同步模块获取当前UTC时间和时区信息
 * 返回0表示成功，返回非0表示失败
 */
int hilink_get_utc_time_ms(unsigned long long *p_time_ms, char *time_zone, unsigned int len);

/*
 * 时间同步模块获取当前UTC日期信息
 * 返回0表示成功，返回非0表示失败
 */
int hilink_get_utc_time_info(pstHILINK_TIME_INFO p_time_info);

/*
 * 时间同步模块转换ms为日期
 * 返回0表示成功，返回非0表示失败
 */
int hilink_convert_time(unsigned long long time_ms, pstHILINK_TIME_INFO p_time_info);

/*
 * 设置HiLink组网打开关闭
 * 参数flag为0表示关闭(保留HiLink注册),为1表示打开,为2表示完全关闭
 * 返回0表示设置成功，返回-1表示设置失败
 * 此函数由设备开发者调用;SDK默认为1打开
 */
int HiLinkSetGatewayMode(int flag);

/*
 * 设备离线时，如果在App上删除了设备，设备再次上线时云端会给设备下发Errcode=5或Errcode=6错误码。
 * 该接口用于使能SDK处理云端下发的Errcode=5或Errcode=6错误码。
 * enable为0表示SDK不处理云端下发的Errcode=5或Errcode=6错误码，此时SDK不会清除设备端注册信息，
 * 需要用户手动硬件恢复出厂设置，设备才能重新进行配网状态。
 * enable为非0表示SDK处理云端下发的Errcode=5或Errcode=6错误码，此时SDK会清除设备端注册信息，重新进行配网状态
 * 默认enable为0
 */
void HILINK_EnableProcessDelErrCode(int enable);

/*
 * 获取SoftAp配网PIN码
 * 返回值为8位数字PIN码,返回-1表示使用HiLink SDK的默认PIN码
 * 该接口需设备开发者实现
 */
int HiLinkGetPinCode(void);

/*
 * 设置配网信息
 * configInfo为APP发送过来的加密后的WiFi信息的buff,len为WiFi信息的长度
 * userData为接受扩展信息的buff，userDataLen为接受扩展信息buff的长度
 * 返回0表示设置成功，其他表示失败
 */
int HILINK_SetNetConfigInfo(const unsigned char *configInfo, unsigned int len,
                            char *userData, unsigned int userDataLen);
/*
 * HiLink SDK外部诊断信息记录接口
 * 该接口已经对外提供给第三方厂商使用，为了前向兼容，暂不按最新编码规范整改
 */
void hilink_diagnosis_record_ex(int errCode);

/*
 * 上报report能力属性状态，该接口为同步接口
 * 服务字段状态发生改变主动上报到云平台(连接云平台时)或者HiLink网关(连接HiLink网关时)
 * svcId表示服务ID
 * payload表示json格式数据
 * len表示payload长度
 * taskId表示调用该接口的线程的id
 * 返回0表示服务状态上报成功，返回-1表示服务状态上报失败
 * 该接口已经对外提供给第三方厂商使用，为了前向兼容，暂不按最新编码规范整改
 */
int hilink_report_char_state(const char *svcId, const char *payload, unsigned int len, int taskId);

/*
 * 上报服务状态，该接口为异步接口
 * 服务字段状态发生改变主动上报到云平台(连接云平台时)或者HiLink网关(连接HiLink网关时)
 * svcId表示服务ID
 * payload为json格式数据
 * len表示payload长度
 * 返回0表示服务状态上报成功，返回-1表示服务状态上报失败
 * 该接口已经对外提供给第三方厂商使用，为了前向兼容，暂不按最新编码规范整改
 */
int hilink_upload_char_state(const char *svcId, const char *payload, unsigned int len);

/* 设置HiLink SDK属性，返回0表示设置成功，否则，设置失败 */
int HILINK_SetSdkAttr(HILINK_SdkAttr sdkAttr);

/* 查询HiLink SDK属性 */
HILINK_SdkAttr *HILINK_GetSdkAttr(void);

/*
 * 厂家需要实现此接口实现license的写入，写入flash位置或者写入文件由厂家决定。
 * 厂家需要保证备份机制，防止突然断电导致license信息丢失，如果license信息丢失，将
 * 无法继续绑定设备，设备将不能再使用。
 * 执行成功返回0，执行失败返回-1
 */
int HILINK_WriteLicense(const unsigned char *license, unsigned int len);

/*
 * 厂家需要实现此接口实现license读取，读取flash位置或者写入文件由厂家决定。
 * 执行成功返回0，执行失败返回-1
 */
int HILINK_ReadLicense(unsigned char *license, unsigned int len);

/*
 * 查询设备是否已被注册
 * 返回非0，已注册；返回0，未注册；
 */
int HILINK_IsRegister(void);

/*
 * 设置HiLink SDK配置信息保存路径，仅linux和android版本适用
 * path为路径信息，绝对路径长度不超过127；
 * 返回0，设置成功；返回非0，设置失败。
 * 注意: 非linux和android系统无此接口实现
 */
int HILINK_SetConfigInfoPath(const char *path);

/*
 * 查询HiLink SDK配置信息保存路径，仅linux和android版本适用
 * path为保存路径的缓冲区；len为缓冲区长度；
 * 返回0，获取成功；返回非0，获取失败。
 * 注意: 非linux和android系统无此接口实现
 */
int HILINK_GetConfigInfoPath(char *path, unsigned int len);

/*
 * 查询当前设备敏感性标识。
 * 返回0为非敏感设备，返回1为敏感设备
 */
int HILINK_IsSensitiveDevice(void);

/* 厂商调用该接口获取匿名结构体 */
const void *HILINK_GetVoiceContext(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _HILINK_H_ */