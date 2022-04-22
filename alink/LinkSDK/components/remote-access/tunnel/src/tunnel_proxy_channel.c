#include <core_stdinc.h>
#include "nopoll.h"
#include "core_log.h"
#include "tunnel_proxy_private.h"
#include "tunnel_proxy_channel.h"


static void websocket_log(noPollCtx * ctx, noPollDebugLevel level, char * log_msg, noPollPtr user_data)
{
    core_log1(aiot_sysdep_get_portfile(), 0, "%s\r\n",log_msg);
}

static int read_handle(noPollConn * conn, char *buffer, int buffer_size)
{
    int recv_len = STATE_TUNNEL_FAILED;
    void* network_handle = nopoll_conn_get_userdata(conn);
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return STATE_TUNNEL_FAILED;
    }

    if( (recv_len = sysdep->core_sysdep_network_recv(network_handle, (uint8_t *)buffer, buffer_size, 50, NULL)) < STATE_SUCCESS)
    {
        return STATE_TUNNEL_FAILED;
    }
    return recv_len;
}
static int write_handle(noPollConn * conn, char *buffer, int buffer_size)
{
    int send_len = STATE_TUNNEL_FAILED;
    void* network_handle = nopoll_conn_get_userdata(conn);
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return STATE_TUNNEL_FAILED;
    }

    if( (send_len = sysdep->core_sysdep_network_send(network_handle, (uint8_t *)buffer, buffer_size, 50, NULL)) < STATE_SUCCESS)
    {
        return STATE_TUNNEL_FAILED;
    }

    return send_len;
}

/*********************************************************
 * 接口名称：open_cloud_proxy_channel
 * 描       述：创建北向云端连接
 * 输入参数：
 * 输出参数：
 * 返 回 值：
 * 说    明：
 *********************************************************/
void *open_cloud_proxy_channel(char *host, char *port, char *path, char *token, void *userdata, int *code)
{
    void* network_handle = NULL;
    char exttunnel_header[256];
    int i = 0;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return NULL;
    }
    network_handle = sysdep->core_sysdep_network_init();
    if (network_handle == NULL) {
        return NULL;
    }

    int socket_type = CORE_SYSDEP_SOCKET_TCP_CLIENT;
    uint32_t tmp_port = 0;
    uint16_t u16_port = 0;
    core_str2uint(port, strlen(port), &tmp_port);
    u16_port = tmp_port;
    uint32_t timeout_ms = 5 * 1000;
    int32_t res = STATE_SUCCESS;
    if ((res = sysdep->core_sysdep_network_setopt(network_handle, CORE_SYSDEP_NETWORK_SOCKET_TYPE,
               &socket_type)) < STATE_SUCCESS ||
            (res = sysdep->core_sysdep_network_setopt(network_handle, CORE_SYSDEP_NETWORK_HOST,
                    host)) < STATE_SUCCESS ||
            (res = sysdep->core_sysdep_network_setopt(network_handle, CORE_SYSDEP_NETWORK_PORT,
                    &u16_port)) < STATE_SUCCESS ||
            (res = sysdep->core_sysdep_network_setopt(network_handle, CORE_SYSDEP_NETWORK_CONNECT_TIMEOUT_MS,
                    &timeout_ms)) < STATE_SUCCESS ||
            (res = sysdep->core_sysdep_network_setopt(network_handle, CORE_SYSDEP_NETWORK_CRED, userdata)) < STATE_SUCCESS) {
        sysdep->core_sysdep_network_deinit(&network_handle);
        return NULL;
    }

    core_log2(sysdep, STATE_TUNNEL_BASE, "connect remote service host %s, port %s", host, port);
    if((res = sysdep->core_sysdep_network_establish(network_handle)) < STATE_SUCCESS)
    {
        sysdep->core_sysdep_network_deinit(&network_handle);
        return NULL;
    }

    noPollCtx *ctx = NULL;
    noPollConn *conn = NULL;
    noPollConnOpts *opts = NULL;
    int socket_fd = -1;
    memcpy(&socket_fd, network_handle, sizeof(socket_fd));
    int ret_code = STATE_SUCCESS;

    /* create context */
    ctx = nopoll_ctx_new();
    if (ctx == NULL)
    {
        core_log(sysdep, STATE_PORT_MALLOC_FAILED, "nopoll new failed\r\n");
        goto end_label;
    }
    nopoll_ctx_set_read_write_handle(ctx, read_handle, write_handle);
    nopoll_ctx_set_userdata(ctx, network_handle);
    nopoll_log_set_handler(ctx,(noPollLogHandler)websocket_log,NULL);

    opts = nopoll_conn_opts_new();
    if (opts == NULL) {
        core_log(sysdep, STATE_PORT_MALLOC_FAILED, "nopoll new failed\r\n");
        ret_code = STATE_TUNNEL_FAILED;
        goto end_label;
    }
    memset(exttunnel_header, 0, sizeof(exttunnel_header));
    snprintf(exttunnel_header, sizeof(exttunnel_header), "\r\ntunnel-access-token: %s\r\nSec-WebSocket-Protocol: aliyun.iot.securetunnel-v1.1", token);
    nopoll_conn_opts_set_exttunnel_headers(opts, exttunnel_header);

    conn = nopoll_conn_new_with_socket(ctx, opts, socket_fd, host, port, NULL, path, NULL, NULL);
    if (!nopoll_conn_is_ok(conn))
    {
        ret_code = STATE_TUNNEL_FAILED;
        goto end_label;
    }
    /* wait for ready */

    while (nopoll_true)
    {
        if (nopoll_conn_wait_until_connection_ready (conn, 5))
        {
            //连接成功
            core_log(sysdep, STATE_PORT_NETWORK_CONNECT_FAILED, "open_cloud_proxy_channel success \r\n");
            return conn;
        }
        i++;
        if (i > 2)
        {
            core_log(sysdep, STATE_PORT_NETWORK_CONNECT_FAILED, "ERROR: connection not ready \r\n");
            ret_code = STATE_TUNNEL_FAILED;
            goto end_label;
        }
    }

end_label:

    if (STATE_SUCCESS != ret_code)
    {
        if (NULL != conn)
        {
            *code = nopoll_conn_get_close_status(conn);
            nopoll_conn_close(conn);
            sysdep->core_sysdep_network_deinit(&network_handle);
        } else if (NULL != ctx)
        {
            nopoll_ctx_unref(ctx);
        }

        return NULL;
    }

    return conn;
}

void close_cloud_proxy_channel(void *channel_handle)
{
    core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "close cloud channel!\r\n");
    noPollConn *conn = (noPollConn *) channel_handle;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return;
    }

    if (NULL == channel_handle)
    {
        return;
    }

    //释放云端连接资源

    /* finish connection */
    if (NULL != conn)
    {
        nopoll_conn_close(conn);
    }

    /* call to cleanup */
    nopoll_cleanup_library();
}


/*********************************************************
 * 接口名称：write_cloud_channel
 * 描       述：向云通道写数据
 * 输入参数：void* params
 * 输出参数：
 * 返  回 值： 发送的数据量
 * 说       明：发送数据前先尽力发送待发送数据
 *           然后发送本次需要发送数据
 *********************************************************/
int write_cloud_proxy_channel(void *channel_handle, const char *data, unsigned int len,unsigned int timeout)
{
    noPollConn *conn = (noPollConn *) channel_handle;

    int retry_times = 3;
    int ret = 0;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    // core_log_hexdump(-0X02,'>',(uint8_t *)data,(uint32_t)len);
    //首先尽力发送待发送数据
    if(nopoll_conn_pending_write_bytes(conn) > 0)
    {
        do
        {
            if (nopoll_conn_pending_write_bytes(conn) > 0)
            {
                if(0 == nopoll_conn_complete_pending_write(conn))
                {
                    //待发送数据已经发送完成
                    break;
                }
            }

            sysdep->core_sysdep_sleep(SEND_WAITING_PERIOD_MS);

        } while (--retry_times);

        if (nopoll_conn_pending_write_bytes(conn) > 0)
        {
            //用户指定数据没有发送
            return 0;
        }
    }

    //发送数据
    ret = nopoll_conn_send_binary(conn, data, len);
    if(ret == len)
    {
        //成功发送
        return len;
    }
    else if(ret == -1)
    {
        //异常返回
        return ret;
    }

    //部分成功或网络异常需要重试
    int total_send_len=0;
    int remain_tmp = 0;

    //获取剩余待发送数据
    if(ret > 0)
    {
        total_send_len = ret;
    }
    else
    {
        total_send_len = 0;
    }

    if(timeout > 10000)
    {
        timeout = 10000;
    }
    else if(timeout < 50)
    {
        timeout = 50;
    }

    retry_times = timeout/SEND_WAITING_PERIOD_MS;
    do
    {
        if (nopoll_conn_pending_write_bytes(conn) > 0)
        {
            remain_tmp = nopoll_conn_complete_pending_write(conn);
            if(remain_tmp == 0)
            {
                //发送完成则退出
                total_send_len = len;
                break;
            }
            else if(remain_tmp > 0)
            {
                total_send_len = len - remain_tmp;
            }
        }
        else
        {
            total_send_len = len;
            break;
        }

        sysdep->core_sysdep_sleep(SEND_WAITING_PERIOD_MS);
    } while (--retry_times);

    return (total_send_len);
}

/*********************************************************
 * 接口名称：read_cloud_proxy_channel
 * 描       述：创建北向云端连接
 * 输入参数：void *channel_handle
 *          int *is_fin
 *          call_back_func func
 *          void *args
 * 输出参数：
 * 返 回 值：
 * 说    明：
 *********************************************************/
int read_cloud_proxy_channel(void *channel_handle, RA_BUFFER_INFO_S *channel_buffer,int *is_fin)
{
    noPollConn *conn = (noPollConn *) channel_handle;
    noPollMsg *msg = NULL;
    int amount = 0;
    char *payload = NULL;
    int ret = 0;

    //接收msg数据,此接口为非阻塞接口
    msg = nopoll_conn_get_msg(conn);
    if (msg == NULL)
    {
        //没有数据则直接退出,有可读事件但是无完整数据此时适当等待
        return STATE_TUNNEL_TIMEOUT;
    }

    //读取当前报文大小
    amount = nopoll_msg_get_payload_size(msg);
    if (amount <= 0)
    {
        //没有数据则直接退出
        nopoll_msg_unref(msg);
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "can not get cloud message size!\r\n");
        return 0;
    }

    payload = (char*)nopoll_msg_get_payload (msg);
    if(NULL == payload)
    {
        nopoll_msg_unref(msg);
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "can not get cloud message payload!\r\n");
        return 0;
    }
    //将msg数据放入导入回调结构中,需要判断超出范围
    ret = write_tunnel_buffer(channel_buffer, payload, amount);
    if(ret < 0)
    {
        nopoll_msg_unref(msg);
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "write the cloud channel buffer error!\r\n");
        return STATE_TUNNEL_FAILED;
    }
    // core_log_hexdump(-0X01, '<',(uint8_t *)payload,(uint32_t)amount);
    //判断接收的msg的完整性
    if(nopoll_msg_is_final(msg))
    {
        //已经接收到完整数据包，则进行数据处理
        *is_fin = 1;
    }
    else
    {
        //没有接收到完整数据包则继续接收
        *is_fin = 0;
    }
    nopoll_msg_unref(msg);

    return amount;
}

