/**
 * @file tunnel_proxy_private.h
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 * @details
 *
 */

#ifndef _TUNNEL_PROXY_PRIVATE_H_
#define _TUNNEL_PROXY_PRIVATE_H_
#include "core_stdinc.h"
#include "core_sysdep.h"
#include "core_list.h"
#include "tunnel_buffer_mgr.h"
#include "tunnel_session_mgr.h"

#define VERSION "2.1"

#define REMOTE_ACCESS_MODULE_NAME "REMOTE-ACCESS"

#define IIOT_DEVICE_NAME_LEN_MAX (32 + 8)
#define IIOT_PRODUCT_KEY_LEN_MAX (32 + 8)
#define IIOT_DEVICE_SECRET_MAX (64 + 8)
#define HOST_LEN_MAX             (128+8)
#define PORT_LEN_MAX             (8+8)
#define PATH_NAME_LEN_MAX        (64+8)
#define VERSION_NAME_LEN_MAX     (32+8)
#define IIOT_TUNNEL_TOKEN_LEN_MAX (64 + 8)

#define CLOUD_TOPIC_LEN_MAX 128
#define MAX_PROXY_NUM            20


#define DEFAULT_LEN_PORT         5
#define DEFAULT_MSG_HDR_LEN      1024
#define DEFAULT_LEN_USER_NAME    256
#define DEFAULT_LEN_PASSWORD     256

#define DEFAULT_SEND_MSG_BUFFER_LEN (5 * DEFAULT_MSG_HDR_LEN)
#define DEFAULT_MSG_BUFFER_LEN (5 * DEFAULT_MSG_HDR_LEN)

#define KEEP_ALIVE_INTERVAL  5
#define KEEP_ALIVE_COUNT     3

#define SEND_WAITING_PERIOD_MS 50

#define SERVICE_LIST_MAX_CNT  20

#define DEFAULT_LEN_SERVICE_NAME 32
#define DEFAULT_LEN_SERVICE_TYPE 128
#define DEFAULT_LEN_IP           128

/**
 * @brief -0x1C80~-0x1CFF表达SDK在ra模块内的状态码
 */
#define  STATE_TUNNEL_BASE                                              (-0x1C80)
/**
 * @brief RA执行失败返回码
 */
#define  STATE_TUNNEL_FAILED                                            (-0x1C81)
/**
 * @brief RA链接超时，发送超时等
 */
#define  STATE_TUNNEL_TIMEOUT                                           (-0x1C82)
/**
 * @brief RA内部I/O出错，执行复位
 */
#define  STATE_TUNNEL_RESET                                             (-0x1C83)

/**
 * @brief 本地服务类型的抽象描述
 *
 */
typedef struct REMOTE_SERVICE_NODE {
    /**
     * @brief 服务类型
     */
    char                        type[DEFAULT_LEN_SERVICE_TYPE];
    /**
     * @brief 服务IP地址
     */
    char                        ip[DEFAULT_LEN_IP];
    /**
     * @brief 服务端口号
     */
    unsigned int                port;
    /**
     * @brief 服务链表，用户不用关心
     */
    struct core_list_head       node;
} LOCAL_SERVICE_NODE_S;

typedef struct
{
    unsigned int        		service_count;
    struct core_list_head       service_list;   //远程服务信息链表，其node为_lOCAL_SERVICE_NODE_S
} LOCAL_SERVICES_S;

/*与云端建联配置*/
typedef struct CLOUD_CHANNEL_PARAMS
{
    char                      cloud_host[HOST_LEN_MAX];           //远程连接通道云端服务地址，可以是域名
    char                      cloud_port[PORT_LEN_MAX];           //远程连接通道云端服务端口
    char                      cloud_path[128];                    //远程服务的路径
    char                      token[IIOT_TUNNEL_TOKEN_LEN_MAX];
    char                      *private_key;
    unsigned int              trans_timeout;
    const char                *get_uRL;
    const char                *origin;
    void                      *cloud_connection;
    int                       flag;
    LOCAL_SERVICES_S          *local_services;
    char                      *services_list;                         //服务列表
    void                      *cred;
} CLOUD_CHANNEL_PARAMS_S;

/*远程通道状态*/
typedef enum REMOTE_PROXY_STATE
{
    CLOUD_CHANNEL_CLOSED = 0,
    CLOUD_CHANNEL_CONNECTED,
} REMOTE_PROXY_STATE_E;

typedef struct RETRY_CONNECT_INFO
{
    int                    retry_times;
    uint64_t               connect_time;
} RETRY_CONNECT_INFO_S;

typedef struct REMOTE_PROXY_INFO
{
    aiot_sysdep_portfile_t*   sysdep;                            /*底层依赖回调合集的引用指针 */
    CLOUD_CHANNEL_PARAMS_S   cloud_channel_params;                   //连接云端通道的参数
    REMOTE_PROXY_STATE_E     cloud_channel_state;                    //云通道的连接状态
    char                     tunnel_id[128];                       //隧道ID
    SESSION_LIST_S           session_list;                         //建立的session的管理hash表
    RA_BUFFER_INFO_S         cloud_read_buffer;                      //接收云端数据使用的buffer，用于向本地服务转发
    RA_BUFFER_INFO_S         cloud_write_buffer;                     //接收本地数据，向云端转发时使用的buffer
    RETRY_CONNECT_INFO_S     retry_info;
    int                      thread_running_cnt;                     // proxy线程运行计数器,用于判断线程是否正常运行
    int                      keepalive_cnt;                         // proxy线程运行计数器,用于判断线程是否正常运行
    unsigned int             remote_proxy_channel_switch;        //远程代理通道的开关
    unsigned int             has_switch_event;                   //是否有开关事件
    void*                    lock;                               //锁
    uint64_t                 ping_time;                         //发送心跳的时间
    uint64_t                 pong_time;                         //发送心跳的时间
    int                      status;                             // 0 close , 1 open
    int                      close_code;
} REMOTE_PROXY_INFO_S;

int init_remote_proxy_resource(REMOTE_PROXY_INFO_S *remote_proxy_info);
void remote_proxy_process(REMOTE_PROXY_INFO_S *remote_proxy_info);
int deinit_remote_proxy_resource(REMOTE_PROXY_INFO_S **remote_proxy);

#endif /* ADVANCED_SERVICES_REMOTE_ACCESS_DAEMON_REMOTE_ACCESS_PARAMS_H_ */

