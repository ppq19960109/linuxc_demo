/**
 * @file tunnel_private.h
 * @brief 安全隧道内部结构体定义
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 * @details
 *
 */
#ifndef _TUNNEL_PRIVATE_H_
#define _TUNNEL_PRIVATE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "tunnel_proxy_private.h"
#include "aiot_sysdep_api.h"
#include "aiot_tunnel_api.h"
#include "core_list.h"

typedef enum {
    TUNNEL_CMD_ADD,
    TUNNEL_CMD_DELETE,
    TUNNEL_CMD_UPDATE
} tunnel_cmd_type_t;

typedef struct {
    tunnel_cmd_type_t           type;
    char                        *tunnel_id;
    aiot_tunnel_connect_param_t params;
    struct core_list_head       node;
} tunnel_cmd_node_t;

typedef struct
{
    void                        *mutex;
    struct core_list_head       cmd_list_head;
} tunnel_cmd_list_t;

typedef struct {
    aiot_sysdep_portfile_t    *sysdep;
    aiot_tunnel_event_handler_t   event_handle;
    void*                     userdata;                          /* 组件调用入参之一 */
    LOCAL_SERVICES_S          local_services;                     //远程服务信息
    char                      version[VERSION_NAME_LEN_MAX];      //版本号
    void                      *cred;
    unsigned int              pthread_exit_flag;                 //线程退出需求标志
    REMOTE_PROXY_INFO_S       *proxy_info[MAX_PROXY_NUM];
    int                       channel_num;
    tunnel_cmd_list_t         cmd_list;
} tunnel_manager_handle_t;


#endif /* __AIOT_tunnel_API_H_ */
