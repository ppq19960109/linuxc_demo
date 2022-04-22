/**
 * @file aiot_ra_api.h
 * @brief remote-access模块头文件, 提供远程登录调试的能力
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 * @details
 *
 * RA模块作用与开启一个通道，供前端访问设备内部, API的使用流程如下:
 *
 * 1. 首先参考 @ref aiot_mqtt_api.h 的说明, 保证成功建立与物联网平台的`MQTT`连接
 *
 * 2. 调用 @ref aiot_ra_init 初始化远程通道, 获取通道句柄
 *
 * 3. 调用 @ref aiot_ra_setopt 配置RA会话的参数, 常用配置项见 @ref aiot_ra_setopt 的说明
 *
 * 4. 启动一个线程，调用 @ref aiot_ra_start 开启远程代理服务, 会一直阻塞
 *
 * 5. RA状态发生变化，会通过 @ref aiot_ra_setopt 配置的 @ref AIOT_RAOPT_EVENT_HANDLER 回调函数, 通知用户当前的状态变化
 *
 */
#ifndef AIOT_RA_API_H_
#define AIOT_RA_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief -0x1C00~-0x1C7F表达SDK在ra模块内的状态码
 */
#define  STATE_REMOTE_ACCESS_BASE                                              (-0x1C00)
/**
 * @brief RA执行失败返回码
 */
#define  STATE_REMOTE_ACCESS_FAILED                                            (-0x1C01)
/**
 * @brief RA链接超时，发送超时等
 */
#define  STATE_REMOTE_ACCESS_TIMEOUT                                           (-0x1C02)
/**
 * @brief RA内部I/O出错，执行复位
 */
#define  STATE_REMOTE_ACCESS_RESET                                             (-0x1C03)
/**
 * @brief MQTT会话句柄未设置, 请通过 @ref aiot_ra_setopt 设置MQTT会话句柄
 */
#define  STATE_REMOTE_ACCESS_MISSING_MQTT_HADNL                                (-0x1C04)
/**
 * @brief 用户的操作系统不是linux
 */
#define  STATE_REMOTE_ACCESS_SYSTEM_NOT_LINUX                                  (-0x1C0A)


/**
 * @brief RA内部事件类型
 */
typedef enum {
    /**
     * @brief 当RA实例连接代理通道成功, 触发此事件
     */
    AIOT_RA_EVT_CONNECT,
    /**
     * @brief 当RA实例从代理通道断开, 触发此事件
     */
    AIOT_RA_EVT_DISCONNECT,
    /**
     * @brief 接收到topic信息，打开代理通道
     */
    AIOT_RA_EVT_OPEN_WEBSOCKET,
    /**
     * @brief 接收到topic信息，关闭代理通道
     */
    AIOT_RA_EVT_CLOSE_WEBSOCKET,
} aiot_ra_event_type;

/**
 * @brief RA内部事件
 */
typedef struct {
    aiot_ra_event_type type;
    char               tunnel_id[128];
} aiot_ra_event_t;

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
} aiot_ra_service_t;

/**
 * @brief RA内部事件回调函数接口定义
 *
 * @details
 *
 * 当RA内部事件被触发时, 调用此函数. 如连接成功/断开连接/
 * @param[in] handle ra会话句柄
 * @param[in] event ra消息结构体, 存放内部事件
 * @param[in] userdata 用户上下文
 */
typedef void (*aiot_ra_event_handler_t)(void *handle, const aiot_ra_event_t *event, void *userdata);


/**
 * @brief @ref aiot_mqtt_setopt 函数的option参数. 对于下文每一个选项中的数据类型, 指的是@ref aiot_mqtt_setopt 中的data参数的数据类型
 *
 * @details
 *
 * 1. data的数据类型是char *时, 以配置@ref AIOT_MQTTOPT_HOST 为例:
 *
 *    char *host = "xxx";
 *
 *    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_HOST, host);
 *
 * 2. data的数据类型是其他数据类型时, 以配置@ref AIOT_MQTTOPT_PORT 为例:
 *
 *    uint16_t port = 443;
 *
 *    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PORT, (void *)&port);
 */
typedef enum {
    /**
     * @brief RA会话 需要的MQTT句柄, 需要先建立MQTT连接, 再设置MQTT句柄
     *
     * @details
     *
     * 数据类型: (void *)
     */
    AIOT_RAOPT_MQTT_HANDLE,
    /**
     * @brief 设置内部事件回调, 它在内部事件触发, 告知用户
     *
     * @details
     *
     * 数据类型: ( @ref aiot_ra_event_handler_t )
     */
    AIOT_RAOPT_EVENT_HANDLER,
    /**
     * @brief 用户需要SDK暂存的上下文
     *
     * @details 这个上下文指针会在 AIOT_RAOPT_EVENT_HANDLER 设置的回调被调用时, 由SDK传给用户
     *
     * 数据类型: (void *)
     */
    AIOT_RAOPT_USERDATA,

    /**
     * @brief RA建联时, 网络使用的安全凭据
     *
     * @details
     *
     * 该配置项用于为底层网络配置@ref aiot_sysdep_network_cred_t 安全凭据数据
     *
     * 1. 若该选项不配置, 那么RA将以tcp方式直接建联
     *
     * 2. 若@ref aiot_sysdep_network_cred_t 中option配置为@ref AIOT_SYSDEP_NETWORK_CRED_NONE , RA将以tcp方式直接建联
     *
     * 3. 若@ref aiot_sysdep_network_cred_t 中option配置为@ref AIOT_SYSDEP_NETWORK_CRED_SVRCERT_CA , RA将以tls方式建联
     *
     * 4. 若@ref aiot_sysdep_network_cred_t 中option配置为@ref AIOT_SYSDEP_NETWORK_CRED_SVRCERT_PSK , RA将以tls psk方式建联
     *
     * 数据类型: (aiot_sysdep_network_cred_t *)
     */
    AIOT_RAOPT_NETWORK_CRED,
    /**
     * @brief 新增本地可提供的远程服务
     */
    AIOT_RAOPT_ADD_SERVICE,
    AIOT_RAOPT_MAX,
} aiot_ra_option_t;

/**
 * @brief 创建ra会话实例, 并以默认值配置会话参数
 *
 * @return void *
 * @retval 非NULL ra实例的句柄
 * @retval NULL   初始化失败, 一般是内存分配失败导致
 *
 */
void *aiot_ra_init(void);

/**
 * @brief 配置ra会话
 *
 * @details
 *
 * @param[in] handle ra会话句柄
 * @param[in] option 配置选项, 更多信息请参考@ref aiot_ra_option_t
 * @param[in] data   配置选项数据, 更多信息请参考@ref aiot_ra_option_t
 *
 * @return int32_t
 * @retval <STATE_SUCCESS  参数配置失败
 * @retval >=STATE_SUCCESS 参数配置成功
 *
 */
int32_t aiot_ra_setopt(void *handle, aiot_ra_option_t option, void *data);

/**
 * @brief 结束ra会话, 销毁实例并回收资源
 *
 * @param[in] handle 指向ra会话句柄的指针
 *
 * @return int32_t
 * @retval <STATE_SUCCESS  执行失败
 * @retval >=STATE_SUCCESS 执行成功
 *
 */
int32_t aiot_ra_deinit(void **handle);

/**
 * @brief 开始ra服务，作为线程开始运行
 *
 * @param[in] handle 指向ra会话句柄的指针
 *
 * @return int32_t*
 * @retval <STATE_SUCCESS 执行失败, 更多信息请参考 STATE_REMOTE_* 定义
 * @retval >=STATE_SUCCESS 执行成功
 */
void*   aiot_ra_start(void *handle);

/**
 * @brief 停止ra服务，aiot_ra_start线程退出
 *
 * @param[in] handle 指向ra会话句柄的指针
 *
 * @return int32_t*
 * @retval <STATE_SUCCESS 执行失败, 更多信息请参考 STATE_REMOTE_* 定义
 * @retval >=STATE_SUCCESS 执行成功
 */
int32_t  aiot_ra_stop(void *handle);

/**
 * @brief 主动请求建立通道
 *
 * @param[in] handle 指向ra会话句柄的指针
 *
 * @return int32_t*
 * @retval <STATE_SUCCESS 执行失败, 更多信息请参考 STATE_REMOTE_* 定义
 * @retval >=STATE_SUCCESS 执行成功
 */
int32_t  aiot_ra_request(void *handle);

#endif /* __AIOT_RA_API_H_ */
