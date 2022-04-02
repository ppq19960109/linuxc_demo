
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "aiot_state_api.h"
#include "aiot_ntp_api.h"

static void *ntp_handle = NULL;
/* 事件处理回调,  */
void demo_ntp_event_handler(void *handle, const aiot_ntp_event_t *event, void *userdata)
{
    switch (event->type)
    {
    case AIOT_NTPEVT_INVALID_RESPONSE:
    {
        printf("AIOT_NTPEVT_INVALID_RESPONSE\n");
    }
    break;
    case AIOT_NTPEVT_INVALID_TIME_FORMAT:
    {
        printf("AIOT_NTPEVT_INVALID_TIME_FORMAT\n");
    }
    break;
    default:
    {
    }
    }
}

/* TODO: 数据处理回调, 当SDK从网络上收到ntp消息时被调用 */
void demo_ntp_recv_handler(void *handle, const aiot_ntp_recv_t *packet, void *userdata)
{
    switch (packet->type)
    {
    /* TODO: 结构体 aiot_ntp_recv_t{} 中包含当前时区下, 年月日时分秒的数值, 可在这里把它们解析储存起来 */
    case AIOT_NTPRECV_LOCAL_TIME:
    {
        printf("local time: %llu, %02d/%02d/%02d-%02d:%02d:%02d:%d\n",
               (long long unsigned int)packet->data.local_time.timestamp,
               packet->data.local_time.year,
               packet->data.local_time.mon, packet->data.local_time.day, packet->data.local_time.hour, packet->data.local_time.min,
               packet->data.local_time.sec, packet->data.local_time.msec);
    }
    break;

    default:
    {
    }
    }
}

int link_ntp_start(void *mqtt_handle)
{
    int32_t res = STATE_SUCCESS;
    int8_t time_zone = 8;

    /* 创建1个ntp客户端实例并内部初始化默认参数 */
    ntp_handle = aiot_ntp_init();
    if (ntp_handle == NULL)
    {
        printf("aiot_ntp_init failed\n");
        return -1;
    }

    res = aiot_ntp_setopt(ntp_handle, AIOT_NTPOPT_MQTT_HANDLE, mqtt_handle);
    if (res < STATE_SUCCESS)
    {
        printf("aiot_ntp_setopt AIOT_NTPOPT_MQTT_HANDLE failed, res: -0x%04X\n", -res);
        aiot_ntp_deinit(&ntp_handle);
        return -1;
    }

    res = aiot_ntp_setopt(ntp_handle, AIOT_NTPOPT_TIME_ZONE, (int8_t *)&time_zone);
    if (res < STATE_SUCCESS)
    {
        printf("aiot_ntp_setopt AIOT_NTPOPT_TIME_ZONE failed, res: -0x%04X\n", -res);
        aiot_ntp_deinit(&ntp_handle);
        return -1;
    }

    /* TODO: NTP消息回应从云端到达设备时, 会进入此处设置的回调函数 */
    res = aiot_ntp_setopt(ntp_handle, AIOT_NTPOPT_RECV_HANDLER, (void *)demo_ntp_recv_handler);
    if (res < STATE_SUCCESS)
    {
        printf("aiot_ntp_setopt AIOT_NTPOPT_RECV_HANDLER failed, res: -0x%04X\n", -res);
        aiot_ntp_deinit(&ntp_handle);
        return -1;
    }

    res = aiot_ntp_setopt(ntp_handle, AIOT_NTPOPT_EVENT_HANDLER, (void *)demo_ntp_event_handler);
    if (res < STATE_SUCCESS)
    {
        printf("aiot_ntp_setopt AIOT_NTPOPT_EVENT_HANDLER failed, res: -0x%04X\n", -res);
        aiot_ntp_deinit(&ntp_handle);
        return -1;
    }

    /* 发送NTP查询请求给云平台 */
    res = aiot_ntp_send_request(ntp_handle);
    if (res < STATE_SUCCESS)
    {
        aiot_ntp_deinit(&ntp_handle);
        return -1;
    }
    return 0;
}
int link_ntp_stop(void)
{
    if (ntp_handle == NULL)
        return -1;
    int32_t res = STATE_SUCCESS;

    /* 销毁NTP实例, 一般不会运行到这里 */
    res = aiot_ntp_deinit(&ntp_handle);
    if (res < STATE_SUCCESS)
    {
        printf("aiot_ntp_deinit failed: -0x%04X\n", -res);
        return -1;
    }

    return 0;
}
