/**
 * @file aiot_tunnel_api.h
 * @brief 安全隧道能力连接的功能实现
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 * @details
 *
 */
#ifndef AIOT_TUNNEL_API_H_
#define AIOT_TUNNEL_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief tunnel内部事件类型
 */
typedef enum {
    /**
     * @brief 当tunnel实例连接代理通道成功, 触发此事件
     */
    AIOT_TUNNEL_EVT_CONNECT,
    /**
     * @brief 当tunnel实例从代理通道断开, 触发此事件
     */
    AIOT_TUNNEL_EVT_DISCONNECT,
    /**
     * @brief 隧道认证信息已经过期，需要重新连接
     */
    AIOT_TUNNEL_EVT_EXPIRED,
} aiot_tunnel_event_type;

/**
 * @brief 本地服务信息
 */
typedef struct {
    /**
     * @brief 服务类型
     */
    char                        type[128];
    /**
     * @brief 服务IP地址/host
     */
    char                        ip[128];
    /**
     * @brief 服务端口号
     */
    unsigned int                port;
} aiot_tunnel_service_t;

/**
 * @brief tunnel内部事件
 */
typedef struct {
    aiot_tunnel_event_type type;
    char               tunnel_id[128];
} aiot_tunnel_event_t;

/**
 * @brief 隧道建连需要的参数
 */
typedef struct {
    char *host;
    char *port;
    char *path;
    char *token;
} aiot_tunnel_connect_param_t;

/**
 * @brief tunnel内部事件回调函数接口定义
 *
 * @details
 *
 * 当tunnel内部事件被触发时, 调用此函数. 如连接成功/断开连接/
 * @patunnelm[in] handle tunnel管理模块句柄
 * @patunnelm[in] event tunnel消息结构体, 存放内部事件
 * @patunnelm[in] userdata 用户上下文
 */
typedef void (*aiot_tunnel_event_handler_t)(void *handle, const aiot_tunnel_event_t *event, void *userdata);


typedef enum {
    /**
     * @brief 设置内部事件回调, 它在内部事件触发, 告知用户
     *
     * @details
     *
     * 数据类型: ( @ref aiot_tunnel_event_handler_t )
     */
    AIOT_TUNNELOPT_EVENT_HANDLER,
    /**
     * @brief 用户需要SDK暂存的上下文
     *
     * @details 这个上下文指针会在 AIOT_TUNNELOPT_EVENT_HANDLER 设置的回调被调用时, 由SDK传给用户
     *
     * 数据类型: (void *)
     */
    AIOT_TUNNELOPT_USERDATA,

    /**
     * @brief tunnel建联时, 网络使用的安全凭据
     *
     * @details
     *
     * 该配置项用于为底层网络配置@ref aiot_sysdep_network_cred_t 安全凭据数据
     *
     * 1. 若该选项不配置, 那么tunnel将以tcp方式直接建联
     *
     * 2. 若@ref aiot_sysdep_network_cred_t 中option配置为@ref AIOT_SYSDEP_NETWORK_CRED_NONE , tunnel将以tcp方式直接建联
     *
     * 3. 若@ref aiot_sysdep_network_cred_t 中option配置为@ref AIOT_SYSDEP_NETWORK_CRED_SVRCERT_CA , tunnel将以tls方式建联
     *
     * 数据类型: (aiot_sysdep_network_cred_t *)
     */
    AIOT_TUNNELOPT_NETWORK_CRED,
    /**
     * @brief 新增本地可提供的远程服务
     */
    AIOT_TUNNELOPT_ADD_SERVICE,
    AIOT_TUNNELOPT_MAX,
} aiot_tunnel_option_t;

/**
 * @brief 创建tunnel管理模块实例, 并以默认值配置会话参数
 *
 * @return void *
 * @retval 非NULL tunnel实例的句柄
 * @retval NULL   初始化失败, 一般是内存分配失败导致
 *
 */
void *aiot_tunnel_init(void);

/**
 * @brief 配置tunnel管理模块
 *
 * @details
 *
 * @patunnelm[in] handle tunnel管理模块句柄
 * @patunnelm[in] option 配置选项, 更多信息请参考@ref aiot_tunnel_option_t
 * @patunnelm[in] data   配置选项数据, 更多信息请参考@ref aiot_tunnel_option_t
 *
 * @return int32_t
 * @retval <STATE_SUCCESS  参数配置失败
 * @retval >=STATE_SUCCESS 参数配置成功
 *
 */
int32_t aiot_tunnel_setopt(void *handle, aiot_tunnel_option_t option, void *data);

/**
 * @brief 结束tunnel管理模块, 销毁实例并回收资源
 *
 * @patunnelm[in] handle 指向tunnel管理模块句柄的指针
 *
 * @return int32_t
 * @retval <STATE_SUCCESS  执行失败
 * @retval >=STATE_SUCCESS 执行成功
 *
 */
int32_t aiot_tunnel_deinit(void **handle);

/**
 * @brief 开始tunnel管理服务，作为后台线程开始运行，会一直阻塞，直至退出
 *
 * @patunnelm[in] handle 指向tunnel管理模块句柄的指针
 *
 * @return int32_t*
 * @retval <STATE_SUCCESS 执行失败, 更多信息请参考 STATE_TUNNEL_* 定义
 * @retval >=STATE_SUCCESS 执行成功
 */
void*   aiot_tunnel_start(void *handle);

/**
 * @brief 关闭tunnel管理服务，作为线程开始运行
 *
 * @patunnelm[in] handle 指向tunnel管理模块句柄的指针
 *
 * @return int32_t*
 * @retval <STATE_SUCCESS 执行失败, 更多信息请参考 STATE_TUNNEL_* 定义
 * @retval >=STATE_SUCCESS 执行成功
 */
int32_t  aiot_tunnel_stop(void *handle);


/**
 * @brief 向隧道管理模块增加隧道，并建连
 *
 * @patunnelm[in] handle 指向tunnel管理模块句柄的指针
 * @patunnelm[in] tunnel 隧道id
 * @patunnelm[in] params 隧道的建连信息
 *
 * @return int32_t*
 * @retval <STATE_SUCCESS 执行失败, 更多信息请参考 STATE_TUNNEL_* 定义
 * @retval >=STATE_SUCCESS 执行成功
 */
int32_t aiot_tunnel_add(void *handle, char *tunnel_id, aiot_tunnel_connect_param_t *params);

/**
 * @brief 更新隧道建来呢信息
 *
 * @patunnelm[in] handle 指向tunnel管理模块句柄的指针
 * @patunnelm[in] tunnel 隧道id
 * @patunnelm[in] params 隧道的建连信息
 *
 * @details
 *     当该隧道id已经存在，且在线时，会断连后使用新的建连信息建连
 *     当该隧道id已经存在，且不在线时，会使用新的建连信息建连
 *     当该隧道id不存在，会直接使用新的建连信息建连
 * @return int32_t*
 * @retval <STATE_SUCCESS 执行失败, 更多信息请参考 STATE_TUNNEL_* 定义
 * @retval >=STATE_SUCCESS 执行成功
 */
int32_t aiot_tunnel_update(void *handle, char *tunnel_id, aiot_tunnel_connect_param_t *params);

/**
 * @brief 从隧道管理模块删除隧道，并断连
 *
 * @patunnelm[in] handle 指向tunnel管理模块句柄的指针
 * @patunnelm[in] tunnel 隧道的信息
 *
 * @return int32_t*
 * @retval <STATE_SUCCESS 执行失败, 更多信息请参考 STATE_TUNNEL_* 定义
 * @retval >=STATE_SUCCESS 执行成功
 */
int32_t aiot_tunnel_delete(void *handle, char *tunnel_id);





#endif /* __AIOT_tunnel_API_H_ */
