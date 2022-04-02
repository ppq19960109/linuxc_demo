/**
 * @file aiot_tunnel_api.c
 * @brief 隧道管理模块，管理的隧道的连接及消息收发
 * @date 2020-01-20
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 */

#include <stdio.h>
#include "core_stdinc.h"
#include "core_log.h"
#include "tunnel_proxy_private.h"
#include "tunnel_private.h"
#include "aiot_state_api.h"
#include "aiot_tunnel_api.h"

static char *TAG = "TUNNEL";

void *aiot_tunnel_init(void)
{
    aiot_sysdep_portfile_t *sysdep = NULL;
    tunnel_manager_handle_t *tunnel_handle = NULL;
    sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return NULL;
    }

    tunnel_handle = sysdep->core_sysdep_malloc(sizeof(tunnel_manager_handle_t), TAG);
    if (tunnel_handle == NULL) {
        return NULL;
    }

    memset(tunnel_handle, 0, sizeof(tunnel_manager_handle_t));
    tunnel_handle->sysdep = sysdep;
    CORE_INIT_LIST_HEAD(&tunnel_handle->local_services.service_list);

    tunnel_handle->cmd_list.mutex = sysdep->core_sysdep_mutex_init();
    CORE_INIT_LIST_HEAD(&tunnel_handle->cmd_list.cmd_list_head);

    return tunnel_handle;
}

int32_t aiot_tunnel_setopt(void *handle, aiot_tunnel_option_t option, void *data)
{
    tunnel_manager_handle_t *tunnel_handle = (tunnel_manager_handle_t*)handle;

    if (tunnel_handle == NULL || data == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }
    if (option >= AIOT_TUNNELOPT_MAX) {
        return STATE_USER_INPUT_OUT_RANGE;
    }

    switch(option)
    {
    case AIOT_TUNNELOPT_EVENT_HANDLER:
        tunnel_handle->event_handle = (aiot_tunnel_event_handler_t)data;
        break;
    case AIOT_TUNNELOPT_USERDATA:
        tunnel_handle->userdata = data;
        break;
    case AIOT_TUNNELOPT_NETWORK_CRED: {
        aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
        if (tunnel_handle->cred != NULL) {
            sysdep->core_sysdep_free(tunnel_handle->cred);
            tunnel_handle->cred = NULL;
        }
        tunnel_handle->cred = sysdep->core_sysdep_malloc(sizeof(aiot_sysdep_network_cred_t), REMOTE_ACCESS_MODULE_NAME);
        if (tunnel_handle->cred != NULL) {
            memset(tunnel_handle->cred, 0, sizeof(aiot_sysdep_network_cred_t));
            memcpy(tunnel_handle->cred, data, sizeof(aiot_sysdep_network_cred_t));
        } else {
            return STATE_SYS_DEPEND_MALLOC_FAILED;
        }
    }
    break;
    case AIOT_TUNNELOPT_ADD_SERVICE: {
        aiot_tunnel_service_t *service = (aiot_tunnel_service_t *)data;
        LOCAL_SERVICE_NODE_S *service_node = tunnel_handle->sysdep->core_sysdep_malloc(sizeof(LOCAL_SERVICE_NODE_S), REMOTE_ACCESS_MODULE_NAME);
        memset(service_node, 0, sizeof(LOCAL_SERVICE_NODE_S));
        memcpy(service_node->type, service->type, strlen(service->type));
        memcpy(service_node->ip, service->ip, strlen(service->ip));
        service_node->port = service->port;

        core_list_add(&service_node->node, &tunnel_handle->local_services.service_list);
    }
    break;
    default:
        break;
    }

    return STATE_SUCCESS;
}

static void _release_all_service_info(LOCAL_SERVICES_S *local_services)
{
    aiot_sysdep_portfile_t *sysdep = NULL;
    sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return;
    }
    LOCAL_SERVICE_NODE_S *item = NULL, *next = NULL;
    core_list_for_each_entry_safe(item, next, &local_services->service_list, node, LOCAL_SERVICE_NODE_S)
    {
        core_list_del(&item->node);
        sysdep->core_sysdep_free(item);
    }
}

int32_t aiot_tunnel_deinit(void **handle)
{
    int i = 0;
    tunnel_cmd_node_t *item = NULL, *next = NULL;
    if(NULL == handle || NULL == *handle)
    {
        return STATE_USER_INPUT_OUT_RANGE;
    }

    tunnel_manager_handle_t *tunnel_handle = *(tunnel_manager_handle_t**)handle;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return STATE_TUNNEL_FAILED;
    }

    if (tunnel_handle->cred != NULL) {
        sysdep->core_sysdep_free(tunnel_handle->cred);
        tunnel_handle->cred = NULL;
    }

    _release_all_service_info(&tunnel_handle->local_services);
    for(i = 0; i < MAX_PROXY_NUM; i++) {
        if(tunnel_handle->proxy_info[i] != NULL) {
            deinit_remote_proxy_resource(&tunnel_handle->proxy_info[i]);
        }
    }

    /* 删除用户命令列表 */
    sysdep->core_sysdep_mutex_lock(tunnel_handle->cmd_list.mutex);
    core_list_for_each_entry_safe(item, next, &tunnel_handle->cmd_list.cmd_list_head, node, tunnel_cmd_node_t)
    {
        core_list_del(&item->node);
        if(item->tunnel_id != NULL) {
            tunnel_handle->sysdep->core_sysdep_free(item->tunnel_id);
        }

        if(item->params.host != NULL) {
            tunnel_handle->sysdep->core_sysdep_free(item->params.host);
        }
        if(item->params.port != NULL) {
            tunnel_handle->sysdep->core_sysdep_free(item->params.port);
        }
        if(item->params.path != NULL) {
            tunnel_handle->sysdep->core_sysdep_free(item->params.path);
        }
        if(item->params.token != NULL) {
            tunnel_handle->sysdep->core_sysdep_free(item->params.token);
        }

        tunnel_handle->sysdep->core_sysdep_free(item);
    }
    sysdep->core_sysdep_mutex_unlock(tunnel_handle->cmd_list.mutex);
    sysdep->core_sysdep_mutex_deinit(&tunnel_handle->cmd_list.mutex);

    sysdep->core_sysdep_free(*handle);
    return STATE_SUCCESS;
}

static void _tunnel_event_callack(tunnel_manager_handle_t *tunnel_handle, aiot_tunnel_event_type type, char *tunnel_id)
{
    aiot_tunnel_event_t event;
    memset(&event, 0, sizeof(event));
    event.type = type;
    memset(event.tunnel_id, 0, sizeof(event.tunnel_id));
    strcpy(event.tunnel_id, tunnel_id);

    if(tunnel_handle->event_handle != NULL) {
        tunnel_handle->event_handle(tunnel_handle, &event, tunnel_handle->userdata);
    }
}

static REMOTE_PROXY_INFO_S *_tunnel_create_new_tunnel(char *tunnel_id, aiot_tunnel_connect_param_t *params)
{
    REMOTE_PROXY_INFO_S *remote_proxy_info = NULL;
    aiot_sysdep_portfile_t *sysdep;
    if(tunnel_id == NULL || params == NULL) {
        return NULL;
    }

    sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return NULL;
    }

    //初始化资源
    remote_proxy_info = sysdep->core_sysdep_malloc(sizeof(REMOTE_PROXY_INFO_S), "RA");
    if(remote_proxy_info == NULL) {
        return NULL;
    }

    if(0 != init_remote_proxy_resource(remote_proxy_info)) {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "init remote proxy resource error!\r\n");
    }

    memcpy(remote_proxy_info->tunnel_id, tunnel_id, strlen(tunnel_id));

    if (params->host != NULL) {
        memset(remote_proxy_info->cloud_channel_params.cloud_host, 0, sizeof(remote_proxy_info->cloud_channel_params.cloud_host));
        memcpy(remote_proxy_info->cloud_channel_params.cloud_host, params->host, strlen(params->host));
    }

    if (params->port != NULL) {
        memset(remote_proxy_info->cloud_channel_params.cloud_port, 0, sizeof(remote_proxy_info->cloud_channel_params.cloud_port));
        memcpy(remote_proxy_info->cloud_channel_params.cloud_port, params->port, strlen(params->port));
    }

    if (params->path != NULL) {
        memset(remote_proxy_info->cloud_channel_params.cloud_path, 0, sizeof(remote_proxy_info->cloud_channel_params.cloud_path));
        memcpy(remote_proxy_info->cloud_channel_params.cloud_path, params->path, strlen(params->path));
    }

    if (params->token != NULL) {
        memset(remote_proxy_info->cloud_channel_params.token, 0, sizeof(remote_proxy_info->cloud_channel_params.token));
        memcpy(remote_proxy_info->cloud_channel_params.token, params->token, strlen(params->token));
    }

    remote_proxy_info->remote_proxy_channel_switch = 1;
    remote_proxy_info->has_switch_event = 1;
    remote_proxy_info->status = 1;
    remote_proxy_info->cloud_channel_state = CLOUD_CHANNEL_CLOSED;

    return remote_proxy_info;
}

static int32_t _tunnel_execute_cmd(tunnel_manager_handle_t *tunnel_handle, tunnel_cmd_node_t *cmd)
{
    REMOTE_PROXY_INFO_S *remote_proxy_info = NULL;
    int i = 0, unused = -1, used = -1;
    /* 查看隧道是否已经打开了 */
    for(i = 0; i < MAX_PROXY_NUM; i++) {
        if(tunnel_handle->proxy_info[i] != NULL) {
            if(0 == strncmp(tunnel_handle->proxy_info[i]->tunnel_id, cmd->tunnel_id, strlen(cmd->tunnel_id))) {
                remote_proxy_info = tunnel_handle->proxy_info[i];
                used = i;
            }
        } else if(unused == -1) {
            unused = i;
        }
    }

    switch(cmd->type) {
    case TUNNEL_CMD_ADD: {
        if(remote_proxy_info != NULL) {
            core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "reopen tunnel_id %s !\r\n", cmd->tunnel_id);
        } else {
            remote_proxy_info = _tunnel_create_new_tunnel(cmd->tunnel_id, &cmd->params);
            if(remote_proxy_info == NULL || unused == -1) {
                break;
            }
            remote_proxy_info->cloud_channel_params.cred = tunnel_handle->cred;
            remote_proxy_info->cloud_channel_params.local_services = &tunnel_handle->local_services;
            tunnel_handle->channel_num++;
            tunnel_handle->proxy_info[unused] = remote_proxy_info;
        }
    }
    break;
    case TUNNEL_CMD_DELETE: {
        if(remote_proxy_info != NULL) {
            if(remote_proxy_info->cloud_channel_state == CLOUD_CHANNEL_CONNECTED) {
                _tunnel_event_callack(tunnel_handle, AIOT_TUNNEL_EVT_DISCONNECT, remote_proxy_info->tunnel_id);
            }
            deinit_remote_proxy_resource(&tunnel_handle->proxy_info[used]);
            tunnel_handle->proxy_info[used] = NULL;
            tunnel_handle->channel_num--;
        } else {
            core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "close tunnel_id %s not found!\r\n", cmd->tunnel_id);
        }
    }
    break;
    case TUNNEL_CMD_UPDATE: {
        if(remote_proxy_info != NULL) {
            if(remote_proxy_info->cloud_channel_state == CLOUD_CHANNEL_CONNECTED) {
                _tunnel_event_callack(tunnel_handle, AIOT_TUNNEL_EVT_DISCONNECT, remote_proxy_info->tunnel_id);
            }
            deinit_remote_proxy_resource(&tunnel_handle->proxy_info[used]);
            tunnel_handle->proxy_info[used] = NULL;
            tunnel_handle->channel_num--;
        }
        //生成新的隧道
        remote_proxy_info = _tunnel_create_new_tunnel(cmd->tunnel_id, &cmd->params);
        if(remote_proxy_info == NULL || unused == -1) {
            break;
        }
        remote_proxy_info->cloud_channel_params.cred = tunnel_handle->cred;
        remote_proxy_info->cloud_channel_params.local_services = &tunnel_handle->local_services;
        tunnel_handle->channel_num++;
        tunnel_handle->proxy_info[unused] = remote_proxy_info;
    }
    break;
    }

    return STATE_SUCCESS;
}

static void _tunnel_check_cmd_list(tunnel_manager_handle_t *tunnel_handle)
{
    tunnel_cmd_node_t *item = NULL, *next = NULL;

    tunnel_handle->sysdep->core_sysdep_mutex_lock(tunnel_handle->cmd_list.mutex);
    core_list_for_each_entry_safe(item, next, &tunnel_handle->cmd_list.cmd_list_head, node, tunnel_cmd_node_t)
    {
        if(item->tunnel_id != NULL) {
            _tunnel_execute_cmd(tunnel_handle, item);
        }
        
        core_list_del(&item->node);
        if(item->tunnel_id != NULL) {
            tunnel_handle->sysdep->core_sysdep_free(item->tunnel_id);
        }

        if(item->params.host != NULL) {
            tunnel_handle->sysdep->core_sysdep_free(item->params.host);
        }
        if(item->params.port != NULL) {
            tunnel_handle->sysdep->core_sysdep_free(item->params.port);
        }
        if(item->params.path != NULL) {
            tunnel_handle->sysdep->core_sysdep_free(item->params.path);
        }
        if(item->params.token != NULL) {
            tunnel_handle->sysdep->core_sysdep_free(item->params.token);
        }

        tunnel_handle->sysdep->core_sysdep_free(item);
    }

    tunnel_handle->sysdep->core_sysdep_mutex_unlock(tunnel_handle->cmd_list.mutex);
}

static int32_t _tunnel_push_to_cmd_list(tunnel_manager_handle_t *tunnel_handle, char *tunnel_id, tunnel_cmd_type_t type, aiot_tunnel_connect_param_t *params)
{
    tunnel_cmd_node_t *cmd_node = tunnel_handle->sysdep->core_sysdep_malloc(sizeof(LOCAL_SERVICE_NODE_S), REMOTE_ACCESS_MODULE_NAME);
    memset(cmd_node, 0, sizeof(tunnel_cmd_node_t));
    cmd_node->type = type;
    core_strdup(tunnel_handle->sysdep, &cmd_node->tunnel_id, tunnel_id, TAG);
    if(params != NULL) {
        core_strdup(tunnel_handle->sysdep, &cmd_node->params.host, params->host, TAG);
        core_strdup(tunnel_handle->sysdep, &cmd_node->params.port, params->port, TAG);
        core_strdup(tunnel_handle->sysdep, &cmd_node->params.path, params->path, TAG);
        core_strdup(tunnel_handle->sysdep, &cmd_node->params.token, params->token, TAG);
    }

    tunnel_handle->sysdep->core_sysdep_mutex_lock(tunnel_handle->cmd_list.mutex);
    core_list_add(&cmd_node->node, &tunnel_handle->cmd_list.cmd_list_head);
    tunnel_handle->sysdep->core_sysdep_mutex_unlock(tunnel_handle->cmd_list.mutex);

    return STATE_SUCCESS;
}

static void* remote_proxy_thread(void* params)
{
    int i = 0;
    tunnel_manager_handle_t *tunnel_handle = (tunnel_manager_handle_t*)params;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (sysdep == NULL) {
        return NULL;
    }

    core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "remote proxy thread start!\r\n ");
    //作为代理处理南北向数据，以下为状态机处理内容
    while (1) {
        for(i = 0; i < MAX_PROXY_NUM; i++) {
            REMOTE_PROXY_INFO_S *remote_proxy_info = tunnel_handle->proxy_info[i];
            if(remote_proxy_info != NULL) {
                REMOTE_PROXY_STATE_E last_state = remote_proxy_info->cloud_channel_state;
                remote_proxy_process(remote_proxy_info);

                /* 云端通知设备token过期 */
                if(remote_proxy_info->cloud_channel_state == CLOUD_CHANNEL_CLOSED
                        && (remote_proxy_info->close_code == 401 || remote_proxy_info->close_code == 4001)) {
                    _tunnel_event_callack(tunnel_handle, AIOT_TUNNEL_EVT_EXPIRED, remote_proxy_info->tunnel_id);
                    deinit_remote_proxy_resource(&tunnel_handle->proxy_info[i]);
                    continue;
                }

                /* 连接状态变化 */
                if(last_state != remote_proxy_info->cloud_channel_state)
                {
                    if(last_state == CLOUD_CHANNEL_CLOSED) {
                        _tunnel_event_callack(tunnel_handle, AIOT_TUNNEL_EVT_CONNECT, remote_proxy_info->tunnel_id);
                    }
                    else if(last_state == CLOUD_CHANNEL_CONNECTED) {
                        _tunnel_event_callack(tunnel_handle, AIOT_TUNNEL_EVT_DISCONNECT, remote_proxy_info->tunnel_id);
                    }
                }
            }
        }

        _tunnel_check_cmd_list(tunnel_handle);

        if(tunnel_handle->channel_num == 0) {
            sysdep->core_sysdep_sleep(500);
        } else {
            sysdep->core_sysdep_sleep(50);
        }

        //线程退出，回收所有资源
        if(1 == tunnel_handle->pthread_exit_flag) {
            for(i = 0; i < MAX_PROXY_NUM; i++) {
                if(tunnel_handle->proxy_info[i] != NULL) {
                    deinit_remote_proxy_resource(&tunnel_handle->proxy_info[i]);
                }
            }
            break;
        }
    }

    core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "remote proxy thread exit!\r\n");

    return NULL;
}

void* aiot_tunnel_start(void *handle)
{
    tunnel_manager_handle_t *tunnel_handle = (tunnel_manager_handle_t*)handle;

    if(NULL == tunnel_handle) {
        return NULL;
    }

    return remote_proxy_thread(handle);
}
int32_t aiot_tunnel_stop(void *handle)
{
    tunnel_manager_handle_t *tunnel_handle = (tunnel_manager_handle_t*)handle;
    if(NULL == tunnel_handle) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    tunnel_handle->pthread_exit_flag = 1;
    return STATE_SUCCESS;
}
int32_t aiot_tunnel_add(void *handle, char *tunnel_id, aiot_tunnel_connect_param_t *params)
{
    tunnel_manager_handle_t *tunnel_handle = (tunnel_manager_handle_t*)handle;
    if(handle == NULL || tunnel_id == NULL || params == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    if(params->host == NULL || params->port == NULL || params->path == NULL || params->token == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    /* 超过建连个数限制 */
    if(tunnel_handle->channel_num >= MAX_PROXY_NUM) {
        return -1;
    }

    return _tunnel_push_to_cmd_list(tunnel_handle, tunnel_id, TUNNEL_CMD_ADD, params);
}

int32_t aiot_tunnel_update(void *handle, char *tunnel_id, aiot_tunnel_connect_param_t *params)
{
    tunnel_manager_handle_t *tunnel_handle = (tunnel_manager_handle_t*)handle;
    if(handle == NULL || tunnel_id == NULL || params == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    if(params->host == NULL || params->port == NULL || params->path == NULL || params->token == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    return _tunnel_push_to_cmd_list(tunnel_handle, tunnel_id, TUNNEL_CMD_UPDATE, params);
}
int32_t aiot_tunnel_delete(void *handle, char *tunnel_id)
{
    tunnel_manager_handle_t *tunnel_handle = (tunnel_manager_handle_t*)handle;
    if(handle == NULL || tunnel_id == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    return _tunnel_push_to_cmd_list(tunnel_handle, tunnel_id, TUNNEL_CMD_DELETE, NULL);
}
