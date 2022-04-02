/**
 * @file aiot_ra_api.c
 * @brief remote-access模块的API接口实现, 提供远程隧道的能力
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 */
#include <stdio.h>
#include "core_stdinc.h"
#include "core_log.h"
#include "aiot_sysdep_api.h"
#include "aiot_state_api.h"
#include "aiot_ra_api.h"
#include "aiot_tunnel_api.h"
#include "aiot_tunnel_switch_api.h"
#include "ra_private.h"

static char *TAG = "RA";
static void remote_proxy_event_handle(ra_handle_t *ra_handle, aiot_ra_event_type type, char *tunnel_id);

static void _tunnel_event_cb(void *handle, const aiot_tunnel_event_t *event, void *userdata)
{
    ra_handle_t *ra_handle = (ra_handle_t *)userdata;
    aiot_ra_event_t ra_event;

    if(ra_handle->event_handle != NULL) {
        if(event->type == AIOT_TUNNEL_EVT_CONNECT) {
            ra_event.type = AIOT_RA_EVT_CONNECT;
        } else if(event->type == AIOT_TUNNEL_EVT_DISCONNECT) {
            ra_event.type = AIOT_RA_EVT_DISCONNECT;
        } else {
            return;
        }
        memcpy(ra_event.tunnel_id, event->tunnel_id, sizeof(ra_event.tunnel_id));
        ra_handle->event_handle(ra_handle, &ra_event, ra_handle->userdata);
    }
}
void *aiot_ra_init(void)
{
    aiot_sysdep_portfile_t *sysdep = NULL;
    ra_handle_t *ra_handle = NULL;
    sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return NULL;
    }

    ra_handle = sysdep->core_sysdep_malloc(sizeof(ra_handle_t), TAG);
    if (ra_handle == NULL) {
        return NULL;
    }

    memset(ra_handle, 0, sizeof(ra_handle_t));
    ra_handle->tunnel = aiot_tunnel_init();
    if(ra_handle->tunnel == NULL) {
        sysdep->core_sysdep_free(ra_handle);
        return NULL;
    }

    aiot_tunnel_setopt(ra_handle->tunnel, AIOT_TUNNELOPT_EVENT_HANDLER, _tunnel_event_cb);
    aiot_tunnel_setopt(ra_handle->tunnel, AIOT_TUNNELOPT_USERDATA, ra_handle);

    return ra_handle;
}



static void _ra_tunnel_switch_recv_handle(void *handle, const aiot_tunnel_switch_recv_data_t *recv, void *userdata)
{
    ra_handle_t *ra_handle = (ra_handle_t *)userdata;
    aiot_tunnel_connect_param_t params;
    if(handle == NULL || recv == NULL || userdata == NULL || ra_handle->tunnel == NULL) {
        return;
    }

    memset(&params, 0, sizeof(params));
    params.host = recv->host;
    params.port = recv->port;
    params.path = recv->path;
    params.token = recv->token;

    if (AIOT_TUNNEL_OPERATOPN_OPEN == recv->operation ) {
        if(STATE_SUCCESS == aiot_tunnel_add(ra_handle->tunnel, recv->tunnel_id, &params)) {
            remote_proxy_event_handle(ra_handle, AIOT_RA_EVT_OPEN_WEBSOCKET, recv->tunnel_id);
        }
    } else if(AIOT_TUNNEL_OPERATOPN_CLOSE == recv->operation) {
        if(STATE_SUCCESS == aiot_tunnel_delete(ra_handle->tunnel, recv->tunnel_id)) {
            remote_proxy_event_handle(ra_handle, AIOT_RA_EVT_CLOSE_WEBSOCKET, recv->tunnel_id);
        }
    } else if(AIOT_TUNNEL_OPERATOPN_UPDATE == recv->operation) {
        aiot_tunnel_update(ra_handle->tunnel, recv->tunnel_id, &params);
    }
}

int32_t aiot_ra_setopt(void *handle, aiot_ra_option_t option, void *data)
{
    ra_handle_t *ra_handle = (ra_handle_t*)handle;

    if (ra_handle == NULL || data == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }
    if (option >= AIOT_RAOPT_MAX) {
        return STATE_USER_INPUT_OUT_RANGE;
    }

    switch(option)
    {
    case AIOT_RAOPT_MQTT_HANDLE:
        ra_handle->tunnel_switch = aiot_tunnel_switch_init();
        aiot_tunnel_switch_setopt(ra_handle->tunnel_switch, AIOT_TSOPT_MQTT_HANDLE, data);
        aiot_tunnel_switch_setopt(ra_handle->tunnel_switch, AIOT_TSOPT_RECV_HANDLER, _ra_tunnel_switch_recv_handle);
        aiot_tunnel_switch_setopt(ra_handle->tunnel_switch, AIOT_TSOPT_USERDATA, ra_handle);
        break;
    case AIOT_RAOPT_EVENT_HANDLER:
        ra_handle->event_handle = (aiot_ra_event_handler_t)data;
        break;
    case AIOT_RAOPT_USERDATA:
        ra_handle->userdata = data;
        break;
    case AIOT_RAOPT_NETWORK_CRED: {
        if(ra_handle->tunnel != NULL) {
            return aiot_tunnel_setopt(ra_handle->tunnel, AIOT_TUNNELOPT_NETWORK_CRED, data);
        } else {
            return STATE_SYS_DEPEND_MALLOC_FAILED;
        }
    }
    break;
    case AIOT_RAOPT_ADD_SERVICE: {
        if(ra_handle->tunnel != NULL) {
            return aiot_tunnel_setopt(ra_handle->tunnel, AIOT_TUNNELOPT_ADD_SERVICE, data);
        } else {
            return STATE_SYS_DEPEND_MALLOC_FAILED;
        }
    }
    break;
    default:
        break;
    }

    return STATE_SUCCESS;
}
int32_t aiot_ra_deinit(void **handle)
{
    if(NULL == handle || NULL == *handle)
    {
        return STATE_USER_INPUT_OUT_RANGE;
    }

    ra_handle_t *ra_handle = *(ra_handle_t**)handle;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return STATE_REMOTE_ACCESS_FAILED;
    }

    if(ra_handle->tunnel_switch != NULL) {
        aiot_tunnel_switch_deinit(&ra_handle->tunnel_switch);
    }

    if(ra_handle->tunnel != NULL) {
        aiot_tunnel_deinit(&ra_handle->tunnel);
    }

    sysdep->core_sysdep_free(*handle);
    *handle = NULL;

    return STATE_SUCCESS;
}

static void remote_proxy_event_handle(ra_handle_t *ra_handle, aiot_ra_event_type type, char *tunnel_id)
{
    aiot_ra_event_t event;
    memset(&event, 0, sizeof(event));
    event.type = type;
    strcpy(event.tunnel_id, tunnel_id);
    if(ra_handle->event_handle != NULL) {
        ra_handle->event_handle(ra_handle, &event, ra_handle->userdata);
    }
}


void* aiot_ra_start(void *handle)
{
    ra_handle_t *ra_handle = (ra_handle_t*)handle;

    if(NULL == ra_handle) {
        return NULL;
    }

    ra_handle->result = STATE_SUCCESS;
    if(NULL == ra_handle->tunnel_switch) {
        ra_handle->result = STATE_REMOTE_ACCESS_MISSING_MQTT_HADNL;
    }
    else {
        aiot_tunnel_start(ra_handle->tunnel);
        ra_handle->result = STATE_SUCCESS;
    }

    return &ra_handle->result;
}

int32_t  aiot_ra_stop(void *handle)
{
    ra_handle_t *ra_handle = (ra_handle_t*)handle;
    if(NULL == ra_handle)
    {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    aiot_tunnel_stop(ra_handle->tunnel);
    aiot_tunnel_switch_deinit(&ra_handle->tunnel_switch);
    return STATE_SUCCESS;
}

int32_t  aiot_ra_request(void *handle)
{
    ra_handle_t *ra_handle = (ra_handle_t*)handle;
    if(NULL == ra_handle || ra_handle->tunnel_switch == NULL)
    {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    return aiot_tunnel_switch_request(ra_handle->tunnel_switch);
}

