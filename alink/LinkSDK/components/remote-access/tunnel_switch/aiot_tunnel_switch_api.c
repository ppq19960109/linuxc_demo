/**
 * @file aiot_tunnel_switch_api.c
 * @brief 隧道开关接口实现
 * @date 2020-01-20
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 */

#include <stdio.h>
#include "aiot_sysdep_api.h"
#include "aiot_state_api.h"
#include "aiot_tunnel_switch_api.h"
#include "aiot_mqtt_api.h"
#include "core_mqtt.h"
#include "core_string.h"

static char* FMT_TOPIC_SWITCH = "/sys/%s/%s/secure_tunnel/notify";
static char* FMT_TOPIC_REQUEST = "/sys/%s/%s/secure_tunnel/proxy/request";
static char* FMT_TOPIC_REQUEST_REPLY = "/sys/%s/%s/secure_tunnel/proxy/request_reply";
static char* TAG = "TUNNEL";

typedef struct {
    aiot_sysdep_portfile_t *sysdep;
    void *mqtt_handle;

    aiot_tunnel_switch_recv_handler_t recv_handler;
    void *userdata;
} tunnel_switch_handler_t;

static char* _core_json_get_value_char(const char *payload, int32_t payload_len, const char *key)
{
    char *value = NULL;
    char *data = NULL;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    uint32_t data_len = 0;
    int32_t res = 0;

    sysdep = aiot_sysdep_get_portfile();
    if(sysdep == NULL) {
        return NULL;
    }

    res = core_json_value((const char *)payload, payload_len, key, strlen(key), &data, &data_len);
    if(res != STATE_SUCCESS) {
        return NULL;
    }

    value = sysdep->core_sysdep_malloc(data_len + 1, TAG);
    if(value == NULL) {
        return NULL;
    }
    memcpy(value, data, data_len);
    value[data_len] = 0;

    return value;
}

static void _tunnel_switch_notify_topic_handler(void *handle, const aiot_mqtt_recv_t *packet, void *userdata)
{
    tunnel_switch_handler_t *ts_handle = (tunnel_switch_handler_t *)userdata;
    aiot_tunnel_switch_recv_data_t recv;
    const char *payload = NULL;
    int32_t payload_len = 0;
    char *operation = NULL;
    char *expired_time = NULL;
    if(packet == NULL || userdata == NULL || packet->type != AIOT_MQTTRECV_PUB) {
        return;
    }

    memset(&recv, 0, sizeof(recv));
    payload = (const char *)packet->data.pub.payload;
    payload_len = packet->data.pub.payload_len;
    core_log2(aiot_sysdep_get_portfile(), 0, "tunnel_switch_topic_handler payload:%.*s\r\n", &payload_len, packet->data.pub.payload);

    /* 解析操作，打开or关闭 */
    operation = _core_json_get_value_char((const char *)payload, payload_len, "operation");
    if (operation == NULL) {
        /* 不打开也不关闭，只是更新隧道信息 */
        recv.operation = AIOT_TUNNEL_OPERATOPN_UPDATE;
    } else  if (0 == strncmp(operation, "connect", strlen("connect"))) {
        recv.operation = AIOT_TUNNEL_OPERATOPN_OPEN;
    } else if (0 == strncmp(operation, "close", strlen("close"))) {
        recv.operation = AIOT_TUNNEL_OPERATOPN_CLOSE;
    }

    if(operation != NULL) {
        ts_handle->sysdep->core_sysdep_free(operation);
    }

    /* 解析tunnel_id */
    recv.tunnel_id = _core_json_get_value_char((const char *)payload, payload_len, "tunnel_id");
    if(recv.tunnel_id == NULL) {
        return;
    }

    /* 打开隧道操作的额外参数 */
    if(recv.operation != AIOT_TUNNEL_OPERATOPN_CLOSE) {
        /* 解析host, 可选 */
        recv.host = _core_json_get_value_char((const char *)payload, payload_len, "host");
        /* 解析port */
        recv.port = _core_json_get_value_char((const char *)payload, payload_len, "port");
        /* 解析path */
        recv.path = _core_json_get_value_char((const char *)payload, payload_len, "path");
        /* 解析token */
        recv.token = _core_json_get_value_char((const char *)payload, payload_len, "token");
        /* 获取token过期时间 */
        expired_time = _core_json_get_value_char((const char *)payload, payload_len, "token_expire");
        if(expired_time != NULL) {
            core_str2uint64(expired_time, strlen(expired_time), &recv.expired_time);
            recv.created_time = ts_handle->sysdep->core_sysdep_time();
            ts_handle->sysdep->core_sysdep_free(expired_time);
        }
    } else {
        recv.close_reason = _core_json_get_value_char((const char *)payload, payload_len, "close_reason");
    }

    /* 执行回调函数 */
    ts_handle->recv_handler(ts_handle, &recv, ts_handle->userdata);

    /* 资源回收 */
    if(recv.tunnel_id != NULL) {
        ts_handle->sysdep->core_sysdep_free(recv.tunnel_id);
    }

    if(recv.host != NULL) {
        ts_handle->sysdep->core_sysdep_free(recv.host);
    }

    if(recv.port != NULL) {
        ts_handle->sysdep->core_sysdep_free(recv.port);
    }

    if(recv.path != NULL) {
        ts_handle->sysdep->core_sysdep_free(recv.path);
    }

    if(recv.token != NULL) {
        ts_handle->sysdep->core_sysdep_free(recv.token);
    }

    if(recv.close_reason != NULL) {
        ts_handle->sysdep->core_sysdep_free(recv.close_reason);
    }
}

static void _tunnel_switch_request_reply_topic_handler(void *handle, const aiot_mqtt_recv_t *packet, void *userdata)
{
    tunnel_switch_handler_t *ts_handle = (tunnel_switch_handler_t *)userdata;
    aiot_tunnel_switch_recv_data_t recv;
    const char *payload = NULL;
    char *data = NULL;
    uint32_t payload_len = 0, data_len = 0;
    char *expired_time = NULL;
    if(packet == NULL || userdata == NULL || packet->type != AIOT_MQTTRECV_PUB) {
        return;
    }

    memset(&recv, 0, sizeof(recv));
    payload = (const char *)packet->data.pub.payload;
    payload_len = packet->data.pub.payload_len;
    core_log2(aiot_sysdep_get_portfile(), 0, "tunnel_switch_topic_handler payload:%.*s\r\n", &payload_len, packet->data.pub.payload);

    if(STATE_SUCCESS != core_json_value((const char *)payload, payload_len, "data", strlen("data"), &data, &data_len)) {
        return;
    }

    recv.operation = AIOT_TUNNEL_OPERATOPN_UPDATE;

    /* 解析tunnel_id */
    recv.tunnel_id = _core_json_get_value_char((const char *)data, data_len, "tunnel_id");
    if(recv.tunnel_id == NULL) {
        return;
    }

    /* 打开隧道操作的额外参数 */
    if(recv.operation != AIOT_TUNNEL_OPERATOPN_CLOSE) {
        /* 解析host, 可选 */
        recv.host = _core_json_get_value_char((const char *)data, data_len, "host");
        /* 解析port */
        recv.port = _core_json_get_value_char((const char *)data, data_len, "port");
        /* 解析path */
        recv.path = _core_json_get_value_char((const char *)data, data_len, "path");
        /* 解析token */
        recv.token = _core_json_get_value_char((const char *)data, data_len, "token");
        /* 获取token过期时间 */
        expired_time = _core_json_get_value_char((const char *)data, data_len, "token_expire");
        if(expired_time != NULL) {
            core_str2uint64(expired_time, strlen(expired_time), &recv.expired_time);
            recv.created_time = ts_handle->sysdep->core_sysdep_time();
        }
    }

    /* 执行回调函数 */
    ts_handle->recv_handler(ts_handle, &recv, ts_handle->userdata);

    /* 资源回收 */
    if(expired_time != NULL) {
        ts_handle->sysdep->core_sysdep_free(expired_time);
    }

    if(recv.tunnel_id != NULL) {
        ts_handle->sysdep->core_sysdep_free(recv.tunnel_id);
    }

    if(recv.host != NULL) {
        ts_handle->sysdep->core_sysdep_free(recv.host);
    }

    if(recv.port != NULL) {
        ts_handle->sysdep->core_sysdep_free(recv.port);
    }

    if(recv.path != NULL) {
        ts_handle->sysdep->core_sysdep_free(recv.path);
    }

    if(recv.token != NULL) {
        ts_handle->sysdep->core_sysdep_free(recv.token);
    }
}

static int32_t _sub_switch_topic(tunnel_switch_handler_t *ts_handle)
{
    int32_t res = STATE_SUCCESS;
    char *topic = NULL;
    char *pk = core_mqtt_get_product_key(ts_handle->mqtt_handle);
    char *dn = core_mqtt_get_device_name(ts_handle->mqtt_handle);
    char *src[] = { NULL, NULL };
    if( pk == NULL || dn == NULL ) {
        return STATE_USER_INPUT_MISSING_PRODUCT_KEY;
    }

    src[0] = pk;
    src[1] = dn;

    /* 订阅开关的topic */
    res = core_sprintf(ts_handle->sysdep, &topic, FMT_TOPIC_SWITCH, src, sizeof(src) / sizeof(char *), TAG);
    if(res != STATE_SUCCESS) {
        return res;
    }
    res = aiot_mqtt_sub(ts_handle->mqtt_handle, topic, _tunnel_switch_notify_topic_handler, 1, ts_handle);
    if (res < 0) {
        core_log(ts_handle->sysdep, 0, "aiot_mqtt_sub failed\r\n");
    }

    ts_handle->sysdep->core_sysdep_free(topic);

    /* 订阅请求回复的topic */
    res = core_sprintf(ts_handle->sysdep, &topic, FMT_TOPIC_REQUEST_REPLY, src, sizeof(src) / sizeof(char *), TAG);
    if(res != STATE_SUCCESS) {
        return res;
    }
    res = aiot_mqtt_sub(ts_handle->mqtt_handle, topic, _tunnel_switch_request_reply_topic_handler, 1, ts_handle);
    if (res < 0) {
        core_log(ts_handle->sysdep, 0, "aiot_mqtt_sub failed\r\n");
    }

    ts_handle->sysdep->core_sysdep_free(topic);
    return res;
}

static int32_t _unsub_switch_topic(tunnel_switch_handler_t *ts_handle)
{
    int32_t res = STATE_SUCCESS;
    char *topic = NULL;
    char *pk = core_mqtt_get_product_key(ts_handle->mqtt_handle);
    char *dn = core_mqtt_get_device_name(ts_handle->mqtt_handle);
    char *src[] = { NULL, NULL };
    if( pk == NULL || dn == NULL ) {
        return STATE_USER_INPUT_MISSING_PRODUCT_KEY;
    }

    src[0] = pk;
    src[1] = dn;

    /* 取消订阅开关的topic */
    res = core_sprintf(ts_handle->sysdep, &topic, FMT_TOPIC_SWITCH, src, sizeof(src) / sizeof(char *), TAG);
    if(res != STATE_SUCCESS) {
        return res;
    }
    res = aiot_mqtt_unsub(ts_handle->mqtt_handle, topic);
    if (res < 0) {
        core_log(ts_handle->sysdep, 0, "aiot_mqtt_unsub failed\r\n");
        return res;
    }

    ts_handle->sysdep->core_sysdep_free(topic);

    /* 取消订阅开关的topic */
    res = core_sprintf(ts_handle->sysdep, &topic, FMT_TOPIC_REQUEST_REPLY, src, sizeof(src) / sizeof(char *), TAG);
    if(res != STATE_SUCCESS) {
        return res;
    }
    res = aiot_mqtt_unsub(ts_handle->mqtt_handle, topic);
    if (res < 0) {
        core_log(ts_handle->sysdep, 0, "aiot_mqtt_unsub failed\r\n");
    }

    ts_handle->sysdep->core_sysdep_free(topic);
    return res;
}

void *aiot_tunnel_switch_init()
{
    tunnel_switch_handler_t *handle = NULL;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if(sysdep == NULL) {
        return NULL;
    }

    handle = (tunnel_switch_handler_t *) sysdep->core_sysdep_malloc(sizeof(tunnel_switch_handler_t), TAG);
    if(handle == NULL) {
        return NULL;
    }

    handle->sysdep = sysdep;
    handle->mqtt_handle = NULL;
    handle->recv_handler = NULL;
    handle->userdata = NULL;

    return handle;
}

int32_t aiot_tunnel_switch_setopt(void *handle, aiot_tunnel_switch_option_t option, void *data)
{
    tunnel_switch_handler_t *ts_handle = (tunnel_switch_handler_t *)handle;
    int32_t res = STATE_SUCCESS;
    if (NULL == handle || NULL == data) {
        return STATE_USER_INPUT_NULL_POINTER;
    }
    if (option >= AIOT_TSOPT_MAX) {
        return STATE_USER_INPUT_OUT_RANGE;
    }

    switch(option) {
    case AIOT_TSOPT_MQTT_HANDLE:
        ts_handle->mqtt_handle = data;
        res = _sub_switch_topic(ts_handle);
        break;
    case AIOT_TSOPT_RECV_HANDLER:
        ts_handle->recv_handler = (aiot_tunnel_switch_recv_handler_t)data;
        break;
    case AIOT_TSOPT_USERDATA:
        ts_handle->userdata = data;
        break;
    default:
        break;
    }

    return res;
}

int32_t aiot_tunnel_switch_deinit(void **handle)
{
    tunnel_switch_handler_t *ts_handle = NULL;
    aiot_sysdep_portfile_t *sysdep = NULL;
    if(handle == NULL || *handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    ts_handle = *(tunnel_switch_handler_t **)handle;

    if(ts_handle->mqtt_handle != NULL) {
        _unsub_switch_topic(ts_handle);
    }

    sysdep = ts_handle->sysdep;
    sysdep->core_sysdep_free(ts_handle);

    *handle = NULL;

    return STATE_SUCCESS;
}

int32_t aiot_tunnel_switch_request(void *handle)
{
    tunnel_switch_handler_t *ts_handle = (tunnel_switch_handler_t *)handle;
    int32_t res = STATE_SUCCESS;
    char *topic = NULL, *pk = NULL, *dn = NULL;
    char *payload = "{\"id\":1000,\"params\":{}}";
    char *src[] = { NULL, NULL };
    if (NULL == handle) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    pk = core_mqtt_get_product_key(ts_handle->mqtt_handle);
    dn = core_mqtt_get_device_name(ts_handle->mqtt_handle);
    if( pk == NULL || dn == NULL ) {
        return STATE_USER_INPUT_MISSING_PRODUCT_KEY;
    }

    src[0] = pk;
    src[1] = dn;
    res = core_sprintf(ts_handle->sysdep, &topic, FMT_TOPIC_REQUEST, src, sizeof(src) / sizeof(char *), TAG);
    if(res != STATE_SUCCESS) {
        return res;
    }

    res = aiot_mqtt_pub(ts_handle->mqtt_handle, topic, (uint8_t *)payload, strlen(payload), CORE_MQTT_QOS0);
    if (res < 0) {
        core_log1(ts_handle->sysdep, 0, "aiot_mqtt_pub failed %x\r\n", &res);
    }

    ts_handle->sysdep->core_sysdep_free(topic);

    return res;
}