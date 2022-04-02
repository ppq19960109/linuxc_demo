/**
 * @file aiot_tunnel_switch_api.h
 * @brief 远程隧道的开关的实现文件，实现mqtt消息的订阅及解析
 * @date 2019-12-27
 *
 * @copyright Copyright (C) 2015-2018 Alibaba Group Holding Limited
 *
 */

#ifndef _AIOT_TUNNEL_SWTICH_API_H_
#define _AIOT_TUNNEL_SWTICH_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    /**
     * @brief 关闭隧道连接
     */
    AIOT_TUNNEL_OPERATOPN_CLOSE,
    /**
     * @brief 打开隧道连接
     */
    AIOT_TUNNEL_OPERATOPN_OPEN,
    /**
     * @brief 更新隧道连接信息
     */
    AIOT_TUNNEL_OPERATOPN_UPDATE,
} aiot_tunnel_switch_operation_t;

typedef struct {
    /**
     * @brief 字符串，隧道ID，用于标示指定隧道
     */
    char *tunnel_id;
    /**
     * @brief 隧道操作类型，【1，打开隧道】 【0，关闭隧道】
     */
    int8_t operation;
    /**
     * @brief 隧道连接的地址
     */
    char *host;
    /**
     * @brief 隧道连接的端口号
     */
    char *port;
    /**
     * @brief 隧道连接的具体路径
     */
    char *path;
    /**
     * @brief 隧道建连的token
     */
    char *token;
    /**
     * @brief 隧道token过期的剩余时间,单位s
     */
    uint64_t expired_time;
    /**
     * @brief 隧道token创建的时间,设备系统时间，单位ms
     */
    uint64_t created_time;
    /**
     * @brief 如果是关闭操作，反馈是隧道过期，还是用户关闭
     */
    char *close_reason;
} aiot_tunnel_switch_recv_data_t;

/**
 * @brief tunnel_swtich模块消息接收回调函数的函数原型定义, 当模块接收到服务器下行数据后将调用此回调函数, 并将消息数据通过<i>recv</i>参数输入给用户, \n
 * 同时将用户上下文数据指针通过<i>userdata</i>参数返回给用户
 *
 * @param[in] handle tunnel_switch实例句柄
 * @param[in] recv   服务下发的消息数据, <b>消息结构体中的所有数据指针在离开回调函数后将失效, 保存消息数据必须使用内存复制的方式</b>
 * @param[in] userdata 指向用户上下文数据的指针, 这个指针由用户通过调用@ref aiot_tunnel_switch_option_t 配置@ref AIOT_TSOPT_USERDATA 选项设置
 *
 * @return void
 */
typedef void (*aiot_tunnel_switch_recv_handler_t)(void *handle, const aiot_tunnel_switch_recv_data_t *recv, void *userdata);

typedef enum {
    /**
     * @brief 模块依赖的MQTT句柄
     *
     * @details
     *
     * tunnel_switch模块依赖底层的MQTT模块, 用户必需配置正确的MQTT句柄, 否则无法正常工作
     *
     * 数据类型: (void *)
     */
    AIOT_TSOPT_MQTT_HANDLE,

    /**
     * @brief 数据接收回调函数, tunnel_switch接收物联网平台的下行消息后调用此回调函数
     *
     * @details
     *
     * 数据类型: (aiot_tunnel_switch_recv_handler_t), 详细查看@ref aiot_tunnel_switch_recv_handler_t 回调函数原型
     */
    AIOT_TSOPT_RECV_HANDLER,

    /**
     * @brief 指向用户上下文数据的指针
     *
     * @details
     *
     * 在用户注册的@ref aiot_tunnel_switch_recv_handler_t 数据接收回调函数中会同过userdata参数将此指针返回给用户
     *
     * 数据类型: (void *)
     */
    AIOT_TSOPT_USERDATA,

    /**
     * @brief 配置选项数量最大值, 不可用作配置参数
     */
    AIOT_TSOPT_MAX,
} aiot_tunnel_switch_option_t;

/**
 * @brief 初始化tunnel_switch实例
 *
 * @return void*
 * @retval 非NULL tunnel_switch实例句柄
 * @retval NULL 初始化失败, 一般是内存分配失败导致
 */
void *aiot_tunnel_switch_init();

/**
 * @brief 设置tunnel_switch参数
 *
 * @param[in] handle tunnel_switch实例句柄
 * @param[in] option 配置选项, 更多信息请查看@ref aiot_tunnel_switch_option_t
 * @param[in] data   配置数据, 更多信息请查看@ref aiot_tunnel_switch_option_t
 *
 * @return int32_t
 * @retval STATE_SUCCESS 参数配置成功
 * @retval others 参考@ref aiot_state_api.h
 *
 */
int32_t aiot_tunnel_switch_setopt(void *handle, aiot_tunnel_switch_option_t option, void *data);
/**
 * @brief 销毁tunnel_switch实例, 释放资源
 *
 * @param[in] handle 指向tunnel_switch实例句柄的指针
 * @return int32_t
 * @retval STATE_SUCCESS 执行成功
 * @retval <STATE_SUCCESS 执行失败
 *
 */
int32_t aiot_tunnel_switch_deinit(void **handle);

/**
 * @brief 发送获取隧道信息
 *
 * @param[in] handle 指向tunnel_switch实例句柄
 * @return int32_t
 * @retval STATE_SUCCESS 执行成功
 * @retval <STATE_SUCCESS 执行失败
 *
 */
int32_t aiot_tunnel_switch_request(void *handle);

#if defined(__cplusplus)
}
#endif

#endif

