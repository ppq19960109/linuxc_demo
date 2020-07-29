/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HiLink产品适配头文件
 * Create: 2019-04-20
 * Notes: 该文件中的接口需要对外提供给第三方厂商使用，为了前向兼容，部分老接口暂不按最新编码规范整改.
 */
#ifndef __HILINK_PROFILE_ADAPTER_H__
#define __HILINK_PROFILE_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 设备版本信息 */
#define FIRMWARE_VER  "20000"
#define SOFTWARE_VER  "1.0.3"
#define HARDWARE_VER  "20000"

/* 设备产品信息 */
#define PRODUCT_ID    "2AO8"
#define DEVICE_TYPE   "020"
#define MANUAFACTURER "312"
#define DEVICE_MODEL  "IHA1221HW"
#define HILINK_VER    "1.0.0"

/* 请确保设备类型英文名和厂商英文名长度之和不超过17字节 */
#define DEVICE_TYPE_NAME   "Bridge"
#define MANUAFACTURER_NAME "HONYAR"

/* 通信协议类型: WiFi */
#define PROTOCOL_TYPE 1

/* 获取BI 参数接口函数 */
char *hilink_get_auto_bi_rsa_cipher(void);

/* 获取AC 参数接口函数 */
unsigned char *hilink_get_auto_ac(void);

/*
 * 修改服务当前字段值
 * svcId为服务的ID，payload为接收到需要修改的Json格式的字段与其值，len为payload的长度
 * 返回0表示服务状态值修改成功，不需要底层设备主动上报，由Hilink Device SDK上报；
 * 返回-101表示获得报文不符合要求；
 * 返回-111表示服务状态值正在修改中，修改成功后底层设备必须主动上报；
 */
int hilink_put_char_state(const char *svcId, const char *payload, unsigned int len);

/*
 * 获取服务字段值
 * svcId表示服务ID。厂商实现该函数时，需要对svcId进行判断；
 * in表示接收到的Json格式的字段与其值；
 * inLen表示接收到的in的长度；
 * out表示保存服务字段值内容的指针,内存由厂商开辟，使用完成后，由Hilink Device SDK释放；
 * outLen表示读取到的payload的长度；
 * 返回0表示服务状态字段值获取成功，返回非0表示获取服务状态字段值不成功。
 */
int hilink_get_char_state(const char *svcId, const char *in, unsigned int inLen, char **out, unsigned int *outLen);

/*
 * 获取设备sn号
 * 注意: sn指向的字符串长度为0时将使用设备mac地址作为sn
 */
void HilinkGetDeviceSn(unsigned int len, char *sn);

/* 获取当前设备版本号 */
int getDeviceVersion(char **firmwareVer, char **softwareVer, char **hardwareVer);

#ifdef __cplusplus
}
#endif
#endif
