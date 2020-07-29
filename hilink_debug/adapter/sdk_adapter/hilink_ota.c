/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: 此文件提供了升级相关的接口实现示例;
 *              其中升级相关功能的接口需要开发者或厂商进行实现，HiLink SDK在收到升级相关命令时会调用对应接口;
 *              请开发者仔细阅读文件中的注释说明，参考或修改实现。
 */
#include "hilink_ota.h"
#include "hilink.h"
#include "protocol_cover.h"
#include <time.h>


#define UPDATE_PATH "/userdata/update/"
#define UPDATE_FILE "upgrade.bin"
#define UPDATE_PATH_ALL "/userdata/update/upgrade.bin"
#define OTA_VERSION "1.0.0"
/*
 * 获取MCU当前版本
 * version表示版本字符串
 * inLen表示输入字符串长度
 * outLen表示输出字符串长度
 * 返回值是RETURN_OK时，表示获取成功
 * 返回值是RETURN_ERROR_NO_MCU时，表示没有MCU
 * 返回值是RETURN_ERROR时，表示获取失败
 * 注意：
 * 如果获取不到MCU的版本，则不对MCU进行升级。
 * 建议厂商在MCU正常启动后，或升级启动后，就将MCU的版本号传递给模组，确保模组可以获取到MCU的版本。
 */

int HilinkGetMcuVersion(char *version, unsigned int inLen, unsigned int *outLen)
{

    strncpy(version, OTA_VERSION, inLen);
    *outLen = strlen(OTA_VERSION) + 1;
    log_debug("HilinkGetMcuVersion %s ,%d", version, *outLen);
    /* 厂商实现此接口 */
    return RETURN_OK;
}

/*
 * 开始模组升级
 * type表示升级类型
 * 当type是UPDATE_TYPE_MANUAL时，表示本次升级流程是由用户主动发起的手动升级
 * 当type是UPDATE_TYPE_AUTO时，表示本次升级流程是经过用户同意，由设备在凌晨2:00至4:00间业务空闲时主动发起的自动升级
 * 返回值是RETURN_OK时，表示处理成功，HiLink SDK将开始启动升级流程
 * 返回值是RETURN_ERROR时，表示处理不成功，HiLink SDK将终止本次升级流程
 * 注意：
 * 在手动场景场景下，HiLink SDK在接收到用户发出的升级指令后，将直接调用此接口；
 * 在自动升级场景下，当HiLink SDK在调用HilinkGetRebootFlag接口返回值是MODULE_CAN_REBOOT时，HiLink SDK将调用此接口。
 * 厂商可在此接口中完成和升级流程相关的处理。
 * 自动升级流程在凌晨进行，因此厂商在实现升级流程相关功能时，确保在升级的下载安装固件和重启设备时避免对用户产生影响，
 * 比如发出声音，光亮等。
 */
int HilinkOtaStartProcess(int type)
{

    if (type == UPDATE_TYPE_MANUAL)
    {
        log_debug("HilinkOtaStartProcess UPDATE_TYPE_MANUAL %d", type);
    }
    else
    {
        log_debug("HilinkOtaStartProcess UPDATE_TYPE_AUTO %d", type);
    }

    /* 厂商实现此接口 */
    return RETURN_OK;
}

/*
 * 模组升级结束
 * status表示升级结果
 * 当status是100时，表示升级成功
 * 当status不是100时，表示升级失败
 * 返回值是RETURN_OK时，表示处理成功，HiLink SDK将置升级标志或切换运行区标志
 * 返回值不是RETURN_OK时，表示处理不成功，HiLink SDK将终止本次升级流程
 * 注意：
 * HiLink SDK在将固件写入到OTA升级区后，且完整性校验通过后，将调用厂商适配的此接口；
 * 厂商可在此接口中完成和升级流程相关的处理。
 * 自动升级流程在凌晨进行，因此厂商在实现升级流程相关功能时，确保在升级的下载安装固件和重启设备时避免对用户产生影响，
 * 比如发出声音，光亮等；升级类型是否为自动升级可参考接口HilinkOtaStartProcess的参数type的描述。
 */
int HilinkOtaEndProcess(int status)
{
    log_debug("HilinkOtaEndProcess status:%d", status);
    /* 厂商实现此接口 */
    if (status != 100)
        return -1;
    return RETURN_OK;
}

/*
 * 判断模组是否能立即升级并重启
 * 返回值是MODULE_CAN_REBOOT时，表示模组可以立即升级并重启，HiLink SDK将开始自动升级流程。
 * 返回值是MODULE_CANNOT_REBOOT时，表示模组不能立即升级并重启，HiLink SDK将不进行本次自动升级流程。
 * 注意：
 * 在用户同意设备可以自动升级的情况下，HiLink SDK调用此接口获取设备当前业务状态下，模组是否可以立即升级并重启的标志。
 * 只有当设备处于业务空闲状态时，接口才可以返回MODULE_CAN_REBOOT。
 * 当设备处于业务非空闲状态时，接口返回MODULE_CANNOT_REBOOT。
 */

int HilinkGetRebootFlag(void)
{
    time_t tm = time(NULL);
    struct tm *localtm = localtime(&tm);
    log_debug("HilinkGetRebootFlag time:%s %d", asctime(localtm),localtm->tm_hour);
    if (localtm->tm_hour < 2 || localtm->tm_hour > 4)
        return MODULE_CANNOT_REBOOT;
    /* 厂商实现此接口 */
    return MODULE_CAN_REBOOT;
}

/*
 * HiLink SDK调用厂商适配的此接口通知MCU固件传输的状态
 * flag表示升级流程标志
 * 当flag是START_SEND_DATA时，表示通知模组即将开始发送MCU固件数据包
 * 当flag是STOP_SEND_DATA时，表示通知模组完整的MCU固件包已发送完成
 * 当flag是SEND_DATA_ERROR时，表示通知模组本次MCU固件升级异常终止
 * len表示MCU固件包的大小
 * type表示升级类型
 * 当type是UPDATE_TYPE_MANUAL时，表示本次升级流程是由用户主动发起的手动升级
 * 当type是UPDATE_TYPE_AUTO时，表示本次升级流程是经过用户同意，由设备在凌晨2:00至4:00间业务空闲时主动发起的自动升级
 * 返回值是RETURN_OK时，表示处理成功，HiLink SDK继续正常处理后续流程
 * 返回值是RETURN_ERROR时，表示处理失败，HiLink SDK将终止本次MCU升级流程
 * 注意：
 * 当flag是STOP_SEND_DATA时，此接口需返回MCU侧固件升级的结果；当flag是其它值时，需返回接口接收到此消息后的处理结果。
 * 自动升级流程在凌晨进行，因此厂商在实现升级流程相关功能时，确保在升级的下载安装固件和重启设备时避免对用户产生影响，
 * 比如发出声音，光亮等。
 */
int HilinkNotifyOtaStatus(int flag, unsigned int len, unsigned int type)
{
    log_debug("HilinkNotifyOtaStatus flag:%d len:%d type:%d", flag, len, type);
    /* 厂商实现此接口 */
    return RETURN_OK;
}

/*
 * HiLink SDK调用厂商适配的此接口通知厂商发送MCU固件文件数据
 * data表示发送的数据
 * len表示发送的数据的长度
 * offset表示发送的数据起始位置相对于完整固件包的偏移量
 * 此接口需要返回MCU接收这部分数据的处理结果
 * 返回值是RETURN_OK时，模组将通知的数据正确发送给MCU，且MCU正确处理发送的数据，HiLink SDK将继续正常处理后续流程
 * 返回值是RETURN_ERROR时，模组或MCU未能正常处理通知的MCU的固件文件数据，HiLink SDK将终止本次MCU固件升级流程
 */
int HilinkNotifyOtaData(const unsigned char *data, unsigned int len, unsigned int offset)
{
    log_debug("HilinkNotifyOtaData len:%d", len);
    /* 厂商实现此接口 */
    return RETURN_OK;
}

/*
 * 获取设备升级文件保存路径
 * filePath表示设备升级文件保存路径
 * len表示设备升级文件保存路径的长度
 * 返回值是RETURN_OK时，表示获取成功
 * 返回值是RETURN_ERROR时，表示获取失败
 * 注意：
 * Linux系统和安卓系统需要适配此接口，其他操作系统不用适配此接口。
 * HiLink SDK下载的设备升级文件需保存在设备上的某个文件路径下；
 * 此文件路径，需由厂商适配提供，例如“/var/update.bin”。
 */
int HILINK_GetUpdateFilePath(char *filePath, unsigned int len)
{

    strcpy(filePath, UPDATE_PATH_ALL);
    log_debug("HILINK_GetUpdateFilePath:%s len:%d", filePath, len);
    /* 厂商实现此接口 */
    return RETURN_OK;
}

/*
 * 厂商适配此接口提供设备上能够保存的升级文件的最大大小，
 * 避免当HiLink SDK下载的升级文件大小大于分区大小时，导致升级失败。
 * 注意：Linux系统和安卓系统需要适配此接口，其他操作系统不用适配此接口。
 */
unsigned int HILINK_GetMaxUpdateFileSize(void)
{
    log_debug("HILINK_GetMaxUpdateFileSize");
    /* 厂商实现此接口 */
    return 5 * 1024 * 1024;
}

/*
 * 启动软件安装
 * 返回值是RETURN_OK时，表示启动成功
 * 返回值是RETURN_ERROR时，表示启动失败
 * 注意：
 * Linux系统和安卓系统需要适配此接口，其他操作系统不用适配此接口。
 * HiLink SDK完成新版本的检测和下载流程后，由厂商实现软件安装流程。
 * HiLink SDK调用此接口后便会退出升级流程，交由厂商的安装程序来完成软件的安装。
 */
int HILINK_StartSoftwareIntall(void)
{
    log_debug("HILINK_StartSoftwareIntall");
    system("chmod 777 " UPDATE_PATH_ALL);
    //test
    system("cp -rf /userdata/nfs/upgrade.bin " UPDATE_PATH);
    system("cd " UPDATE_PATH ";./" UPDATE_FILE " &");

    /* 厂商实现此接口 */
    return RETURN_OK;
}

/*
 * 获取桥下子设备的当前版本
 * version返回同一proId品类的最低版本号
 * 返回值是RETURN_OK时，表示获取成功，反之失败
 */
int HILINK_GetBrgSubDevVersion(const char *proId, unsigned int proIdLen,
                               char *version, unsigned int versionLen)
{
    log_debug("HILINK_GetBrgSubDevVersion proId:%s,version:%s", proId, version);
    /* 厂商实现此接口 */
    return RETURN_OK;
}

/*
 * HiLink SDK调用厂商适配的此接口通知厂商发送桥下子设备固件数据
 * data表示发送的数据
 * len表示发送的数据的长度
 * offset表示发送的数据起始位置相对于完整固件包的偏移量
 * 返回值是RETURN_OK时，表示接受成功，反之失败
 */
int HILINK_NotifyBrgSubDevOtaData(const char *proId, unsigned int proIdLen,
                                  const unsigned char *data, unsigned int len, unsigned int offset)
{
    log_debug("HILINK_NotifyBrgSubDevOtaData proId:%s,data len:%d", proId, len);
    /* 厂商实现此接口 */
    return RETURN_OK;
}

/*
 * HiLink SDK调用厂商适配的此接口通知桥下子设备固件传输的状态
 * flag表示升级流程标志
 * 当flag是START_SEND_DATA时，表示通知模组即将开始发送桥下子设备固件数据包
 * 当flag是STOP_SEND_DATA时，表示通知模组完整的桥下子设备固件包已发送完成
 * 当flag是SEND_DATA_ERROR时，表示通知模组本次桥下子设备固件升级异常终止
 * len表示MCU固件包的大小
 * type表示升级类型
 * 当type是UPDATE_TYPE_MANUAL时，表示本次升级流程是由用户主动发起的手动升级
 * 当type是UPDATE_TYPE_AUTO时，表示本次升级流程是经过用户同意，由设备在凌晨2:00至4:00间业务空闲时主动发起的自动升级
 * 返回值是RETURN_OK时，表示处理成功，HiLink SDK继续正常处理后续流程
 * 返回值是RETURN_ERROR时，表示处理失败，HiLink SDK将终止本次桥下子设备升级流程
 * 注意
 * 当flag是STOP_SEND_DATA时，此接口需返回桥下子设备侧固件升级的结果；当flag是其它值时，需返回接口接收到此消息后的处理结果。
 * 自动升级流程在凌晨进行，因此厂商在实现升级流程相关功能时，确保在升级的下载安装固件和重启设备时避免对用户产生影响，
 * 比如发出声音，光亮等。
 */
int HILINK_NotifyBrgSubDevOtaStatus(const char *proId, unsigned int proIdLen, int flag,
                                    unsigned int len, unsigned int type)
{
    log_debug("HILINK_NotifyBrgSubDevOtaStatus proId:%s,data len:%d", proId, len);
    /* 厂商实现此接口 */
    return RETURN_OK;
}
