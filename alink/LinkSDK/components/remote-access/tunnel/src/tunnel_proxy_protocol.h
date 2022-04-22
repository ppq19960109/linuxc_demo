/**
 * @file tunnel_proxy_protocol.h
 *
 * @copyright Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 * @details
 *
 */

#ifndef _TUNNEL_PROXY_PROTOCOL_H_
#define _TUNNEL_PROXY_PROTOCOL_H_

#include "tunnel_proxy_private.h"

/*消息报文头格式-->通用*/
#define MSG_HEAD_FMT "{\"frame_type\":%d,\"frame_id\":%s,\"session_id\":\"%s\"}"

/*以下皆为包体结构*/
/*MSG_RESP_OK响应报文格式*/
#define MSG_RESPONSE_FMT "{\"code\":%d,\"msg\":\"%s\"}"
/*MSG_SERVICE_PROVIDER_CONN_REQ握手报文格式*/
#define MSG_HDSK_FMT "{\"uuid\": \"%s\",\"product_key\": \"%s\",\"device_name\": \"%s\",\"version\":\"%s\",\"IP\":\"%s\",\"MAC\":\"%s\",\"token\":\"\", \"service_meta\": %s,\"signmethod\": \"hmacsha256\", \"sign\": \"%s\"}"
/*MSG_SERVICE_CONSUMER_CONN_REQ 未使用*/
/*MSG_SERVICE_CONSUMER_NEW_SESSION新增session报文格式，数组形式*/
#define FORMAT_SERVICE_INFO "{\"service_type\":\"%s\",\"service_name\":\"%s\",\"service_ip\":\"%s\",\"service_port\":%d}"
/*MSG_SERVICE_CONSUMER_RELEASE_SESSION释放session报文格式*/
//空，无包体
/*MSG_SERVICE_VERIFY_ACCOUNT*/
// “username”, "password"
/*MSG_SERVICE_PROVIDER_RAW_PROTOCOL*/
// payload为包体内容
/*MSG_SERVICE_CONSUMER_RAW_PROTOCOL*/
// payload为包体内容
/*MSG_KEEPALIVE_PING*/
/*MSG_KEEPALIVE_PONG*/
//消息缓存大小
//DEFAULT_MSG_BUFFER_LEN: must <= 60K

typedef enum PROXY_PROT_MSG_TYPE
{
    MSG_RESP_OK                         = 1,           //消息的response
    MSG_SERVICE_CONSUMER_NEW_SESSION    = 2,           //新增session
    MSG_SERVICE_CONSUMER_RELEASE_SESSION = 3,          //释放session
    MSG_SERVICE_RAW_PROTOCOL             = 4,          //服务发送的原始服务协议.
} PROXY_PROT_MSG_TYPE_E;

typedef enum PROXY_PROT_SERVICE_TYPE
{
    SERVICE_TYPE_SSH                = 1,                //SSH 服务,端口默认 22
    SERVICE_TYPE_HTTP               = 2,                //HTTP 服务，端口默认 80
    SERVICE_TYPE_REMOTE_DESKTOP     = 3,                //windows远程桌面服务，端口默认 3389
    SERVICE_TYPE_FTP                = 4,                //FTP服务， 默认端口为21
    SERVICE_TYPE_OPENAPI            = 5,                //eweb和open_api 服务， 默认端口 9999
    SERVICE_TYPE_TELNET             = 6,                //telnet 服务， 默认端口 23
    SERVICE_TYPE_NONE               = 255               //无服务
} PROXY_PROT_SERVICE_TYPE_E;

typedef enum PROXY_PROT_ERROR_TYPE
{
    ERR_TYPE_SUCCESS                = 0,                //成功
    ERR_SIGNATURE_INVALID           = 101600,           //签名验证失败
    ERR_PARAM_INVALID               = 101601,           //入参不合法
    ERR_SESSION_LIMIT               = 101602,           //Session已达最大值
    ERR_SESSION_NONEXISTENT         = 101603,           //Session不存在
    ERR_SESSION_CREATE_FAILED       = 101604,           //Session创建失败
    ERR_SERVICE_UNAVALIBE           = 101604,           //服务不可达
    ERR_SERVICE_EXIT                = 101605,           //服务异常退出
    ERR_CONNECTION_CLOSE            = 101606,           //连接异常退出
    ERR_VERIFY_ACCOUT               = 101607,           //校验账号失败
    ERR_BACKEND_SERVICE_UNAVALIBE   = 101671            //backend service not available
} PROXY_PROT_ERROR_TYPE_E;

typedef struct PROXY_PROT_HEADER
{
    PROXY_PROT_MSG_TYPE_E       msg_type;
    char                        srv_type[64];
    unsigned int                payload_len;
    char                        msgID[64];
    unsigned long long          timestamp;
    char                        session_id[64];
    unsigned int                hdr_len;
} PROXY_PROT_HEADER_S;

typedef struct PROXY_PROT_SESSION_PARAMS
{
    char       type[DEFAULT_LEN_SERVICE_TYPE];
    char       name[DEFAULT_LEN_SERVICE_NAME];
    char       ip[DEFAULT_LEN_IP];
    unsigned   port;
} PROXY_PROT_SESSION_PARAMS_S;


int splice_proxy_protocol_header(char* buffer, int size, int msg_type, int payload_len, char *msg_id, char *token);
int splice_proxy_protocol_response_payload(char* buffer, int size, int code, char *data, char *msg);

int  parse_proxy_protocol_header(char *buf, int buf_len, PROXY_PROT_HEADER_S *hdr);
char *rand_string_static();

#endif /* ADVANCED_SERVICES_REMOTE_ACCESS_DAEMON_REMOTE_ACCESS_PROXY_PROTOCOL_PACKET_H_ */
