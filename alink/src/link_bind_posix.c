
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"
#include "core_mqtt.h"
#include "core_global.h"

#define BIND_TOKEN_LEN (16)
#define BIND_TOKEN_LIFE (90 * 1000)
static uint32_t token_exp_time = 0;
static char token_topic_fmt_buf[128];

static int g_token_state = 1;
void (*token_state_cb)(int);
void register_token_state_cb(void (*cb)(int))
{
    token_state_cb = cb;
}
int get_token_state(void)
{
    return g_token_state;
}

static void LITE_hexbuf_convert(unsigned char *digest, char *out, int in_len, int uppercase)
{
    static char *zEncode[] = {"0123456789abcdef", "0123456789ABCDEF"};
    int j = 0;
    int i = 0;
    int idx = uppercase ? 1 : 0;

    for (i = 0; i < in_len; i++)
    {
        int a = digest[i];

        out[j++] = zEncode[idx][(a >> 4) & 0xf];
        out[j++] = zEncode[idx][a & 0xf];
    }
}
static uint64_t HAL_UptimeMs(void)
{
    uint64_t time_ms;
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    time_ms = ((uint64_t)ts.tv_sec * (uint64_t)1000) + (ts.tv_nsec / 1000 / 1000);

    return time_ms;
}

static uint32_t bind_time_is_expired(uint32_t time)
{
    uint32_t cur_time;

    cur_time = HAL_UptimeMs();
    /*
     *  WARNING: Do NOT change the following code until you know exactly what it do!
     *
     *  check whether it reach destination time or not.
     */
    if ((cur_time - time) < (UINT32_MAX / 2))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static int bind_time_countdown_ms(uint32_t *time, uint32_t millisecond)
{
    if (time == NULL)
    {
        return STATE_USER_INPUT_NULL_POINTER;
    }
    *time = HAL_UptimeMs() + millisecond;
    return 0;
}

static int bind_refresh_token(char *token)
{
    int i = 0;
    uint32_t time;

    time = HAL_UptimeMs();
    srandom(time);
    for (i = 0; i < BIND_TOKEN_LEN; i++)
    {
        token[i] = random() % 0xFF;
    }
    bind_time_countdown_ms(&token_exp_time, BIND_TOKEN_LIFE);
    return 0;
}

static void link_token_recv_handler(void *handle, const aiot_mqtt_recv_t *packet, void *userdata)
{
    g_token_state = 1;
    printf("%s\n", __func__);
    printf("type:%d\n", packet->type);
    printf("topic:%s\n", packet->data.pub.topic);
    printf("payload:%s\n", packet->data.pub.payload);
}

int link_bind_token_init(void *mqtt_handle, const char *productkey, const char *devicename)
{
#define BIND_TOPIC_TOKEN_FMT "/sys/%s/%s/thing/awss/enrollee/match"
#define BIND_TOPIC_TOKEN_REPY_FMT "/sys/%s/%s/thing/awss/enrollee/match_reply"
    sprintf(token_topic_fmt_buf, BIND_TOPIC_TOKEN_FMT, productkey, devicename);
    char fmt_buf[128];
    sprintf(fmt_buf, BIND_TOPIC_TOKEN_REPY_FMT, productkey, devicename);
    aiot_mqtt_sub(mqtt_handle, fmt_buf, link_token_recv_handler, 1, NULL);
    return 0;
}

int link_bind_token_report(void *mqtt_handle)
{
#define BIND_REPORT_TOKEN_FMT "{\"id\":\"%d\",\"version\":\"1.0\",\"method\":\"thing.awss.enrollee.match\",\"params\":{\"token\":\"%s\"}}"

    if (!bind_time_is_expired(token_exp_time))
    {
        return -1;
    }
    int32_t alink_id;

    core_mqtt_handle_t *handle = (core_mqtt_handle_t *)mqtt_handle;
    int res = core_global_alink_id_next(handle->sysdep, &alink_id);
    if (res != STATE_SUCCESS)
    {
        printf("link_reset_report error:%d\n", res);
        return -1;
    }
    char token[BIND_TOKEN_LEN];
    bind_refresh_token(token);
    char rand_str[(BIND_TOKEN_LEN << 1) + 1] = {0};
    LITE_hexbuf_convert((unsigned char *)token, rand_str, BIND_TOKEN_LEN, 1);

    char payload_fmt_buf[256];
    sprintf(payload_fmt_buf, BIND_REPORT_TOKEN_FMT, alink_id, rand_str);
    printf("%s:%s\n", __func__, payload_fmt_buf);
    aiot_mqtt_pub(mqtt_handle, token_topic_fmt_buf, (unsigned char *)payload_fmt_buf, strlen(payload_fmt_buf), 1);
    return 0;
}

void link_bind_token_deinit(void *mqtt_handle)
{
    aiot_mqtt_unsub(mqtt_handle, token_topic_fmt_buf);
}
