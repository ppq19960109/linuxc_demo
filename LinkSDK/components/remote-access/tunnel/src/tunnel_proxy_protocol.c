
#include <stdio.h>
#include "core_stdinc.h"
#include "core_string.h"
#include "core_log.h"
#include "core_sha256.h"

#include "tunnel_proxy_private.h"
#include "tunnel_proxy_channel.h"
#include "tunnel_proxy_protocol.h"


int splice_proxy_protocol_header(char* buffer, int size, int msg_type, int payload_len, char *msg_id, char *token)
{
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return -1;
    }
    memset(buffer, 0, size);
    int ret = snprintf(&buffer[2], size - 3, MSG_HEAD_FMT, msg_type, msg_id == NULL ? rand_string_static() : msg_id, token == NULL ? "" : token);

    buffer[0] = (ret & 0xff00) >> 8;
    buffer[1] = ret & 0xff;
    ret += 2;
    return ret;
}

int splice_proxy_protocol_response_payload(char* buffer, int size, int code, char *data, char *msg)
{
    memset(buffer, 0, size);
    int ret = snprintf(buffer, size - 1, MSG_RESPONSE_FMT, code, !msg ? "null" : msg);

    return ret;
}

int parse_proxy_protocol_header(char *buf, int buf_len, PROXY_PROT_HEADER_S *hdr)
{
    int hdr_len = 0, frame_len = 0;
    uint32_t ret_len = 0, value = 0;
    char *ret = NULL, *hdr_start = NULL, *hdr_end = NULL;
    int32_t res = STATE_SUCCESS;
    if (!buf || !hdr)
    {
        return STATE_TUNNEL_FAILED;
    }

    frame_len = buf[0] << 8 | buf[1];
    if (buf[2] != '{')
    {
        goto _exit;
    }

    hdr_start = &buf[2];
    hdr_end = strchr(hdr_start, '}');
    if (hdr_end == NULL)
    {
        goto _exit;
    }
    hdr_len = frame_len;

    if ((res = core_json_value(hdr_start, hdr_len, "frame_type", strlen("frame_type"), &ret, &ret_len)) < STATE_SUCCESS) {
        goto _exit;
    }
    core_str2uint(ret, ret_len, &value);
    hdr->msg_type = value;

    if ((res = core_json_value(hdr_start, hdr_len, "service_type", strlen("service_type"), &ret, &ret_len)) < STATE_SUCCESS) {
        //goto _exit;
    } else {
        strncpy(hdr->srv_type, ret, ret_len < 63 ? ret_len : 63);
    }


    if ((res = core_json_value(hdr_start, hdr_len, "frame_id", strlen("frame_id"), &ret, &ret_len)) < STATE_SUCCESS) {
        goto _exit;
    }
    strncpy(hdr->msgID, ret, ret_len < 63 ? ret_len : 63);

    if ((res = core_json_value(hdr_start, hdr_len, "session_id", strlen("session_id"), &ret, &ret_len)) == STATE_SUCCESS) {
        strncpy(hdr->session_id, ret, ret_len < 63 ? ret_len : 63);
    }

    hdr->hdr_len = hdr_len + 2;
    hdr->payload_len = buf_len - hdr->hdr_len;
    return STATE_SUCCESS;
_exit:
    core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "header json formate error:  %s \r\n", buf);
    return STATE_TUNNEL_FAILED;
}

char *rand_string_static()
{
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    static char str[10];
    uint8_t rand_value[10];
    size_t size = sizeof(str);
    const char charset[] = "0123456789";
    sysdep->core_sysdep_rand(rand_value, size);
    memset(str, 0, sizeof(str));
    if (size) {
        --size;
        size_t n = 0;
        for (n = 0; n < size; n++) {
            int key = rand_value[n] % (int) (sizeof(charset) - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}
