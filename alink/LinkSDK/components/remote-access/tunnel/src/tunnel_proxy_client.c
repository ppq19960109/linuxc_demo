
#include "core_stdinc.h"
#include "tunnel_proxy_private.h"
#include "tunnel_buffer_mgr.h"
#include "tunnel_proxy_channel.h"
#include "tunnel_proxy_protocol.h"
#include "tunnel_proxy_trans.h"
#include "tunnel_session_mgr.h"
#include "nopoll.h"
#include "core_log.h"


#define CLOUD_CHANNEL_CONNECTED_TIMEOUT 3
#define CLOUD_CHANNEL_KEEPALIVE_CNT_MAX 3
// #define CLOUD_CHANNEL_FD_SET

#define UPDATE_PROXY_THREAD_RUNNING_CNT(a) do{a->thread_running_cnt = (a->thread_running_cnt + 1) % 0xFFFF; }while(0)


/*********************************************************
 * 接口名称：update_retry_params
 * 描       述：更新重连参数
 * 输入参数：REMOTE_PROXY_INFO_S *remote_proxy_info
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
static void update_retry_params(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    remote_proxy_info->retry_info.retry_times++;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    remote_proxy_info->retry_info.connect_time = sysdep->core_sysdep_time();
    return ;
}

/*********************************************************
 * 接口名称：clean_retry_params
 * 描       述：清除重连参数
 * 输入参数：REMOTE_PROXY_INFO_S *remote_proxy_info
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
static void clean_retry_params(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    remote_proxy_info->retry_info.retry_times = 0;
    remote_proxy_info->retry_info.connect_time = 0;
    return ;
}

nopoll_bool recv_nopoll_pong_cb(noPollCtx * ctx, noPollConn * conn, noPollPtr user_data)
{
    REMOTE_PROXY_INFO_S *proxy = (REMOTE_PROXY_INFO_S *)user_data;

    if(proxy != NULL) {
        core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "websocket %s pong\r\n", proxy->tunnel_id);
        proxy->pong_time = proxy->sysdep->core_sysdep_time();
        return nopoll_true;
    } else {
        return nopoll_false;
    }
}

static int cloud_channel_keepalive(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    if(remote_proxy_info->keepalive_cnt > CLOUD_CHANNEL_KEEPALIVE_CNT_MAX)
    {
        return STATE_TUNNEL_FAILED;
    }

    uint64_t tp_now;
    tp_now = remote_proxy_info->sysdep->core_sysdep_time();
    if(tp_now - remote_proxy_info->pong_time >= 60 * 1000) {
        return STATE_TUNNEL_FAILED;
    }
    if(tp_now - remote_proxy_info->ping_time >= 20 * 1000)
    {
        //发送通道保活信号
        core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "websocket %s ping\r\n", remote_proxy_info->tunnel_id);
        nopoll_conn_send_ping(remote_proxy_info->cloud_channel_params.cloud_connection);
        aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
        remote_proxy_info->ping_time = sysdep->core_sysdep_time();
    }
    return STATE_SUCCESS;
}

static void close_nopoll_conn_cb(noPollCtx  * ctx,noPollConn * conn, noPollPtr user_data)
{
    void* network_handle = nopoll_conn_get_userdata(conn);
    REMOTE_PROXY_INFO_S *remote_proxy_info = (REMOTE_PROXY_INFO_S *)user_data;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return;
    }

    remote_proxy_info->remote_proxy_channel_switch = 0;
    remote_proxy_info->close_code = nopoll_conn_get_close_status(conn);
    if(NULL != network_handle)
    {
        core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "websocket %s closed\r\n", remote_proxy_info->tunnel_id);
        nopoll_ctx_set_userdata(ctx, NULL);
        sysdep->core_sysdep_network_deinit(&network_handle);
    }
}

/*********************************************************
 * 接口名称：create_remote_proxy
 * 描       述：创建北向连接云端的通道资源
 * 输入参数：void* params
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
static int create_remote_proxy(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    CLOUD_CHANNEL_PARAMS_S *cloud_channel_params = &remote_proxy_info->cloud_channel_params;
    void *cloud_connection = NULL;
    int ret_code = STATE_TUNNEL_FAILED;

    //如果资源已经建立，则返回成功
    if (CLOUD_CHANNEL_CLOSED != remote_proxy_info->cloud_channel_state)
    {
        ret_code = STATE_SUCCESS;
        goto end_label;
    }

    core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "start to create cloud channel\r\n");

    //创建云端通道连接资源
    cloud_connection = open_cloud_proxy_channel(cloud_channel_params->cloud_host, cloud_channel_params->cloud_port,
                       cloud_channel_params->cloud_path, cloud_channel_params->token, cloud_channel_params->cred, &remote_proxy_info->close_code);
    if (NULL == cloud_connection)
    {
        //连接失败则1s后重连接
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "cloud channel open failed");
        goto end_label;
    }
    nopoll_conn_set_on_pong(cloud_connection, recv_nopoll_pong_cb, remote_proxy_info);
    nopoll_conn_set_on_close(cloud_connection, close_nopoll_conn_cb, remote_proxy_info);
    core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "websocket %s opened\r\n", remote_proxy_info->tunnel_id);
    cloud_channel_params->cloud_connection = cloud_connection;

    //创建资源后赋值通道资源
    cloud_channel_params->origin = nopoll_conn_get_origin(cloud_connection);
    cloud_channel_params->get_uRL = nopoll_conn_get_requested_url(cloud_connection);

    ret_code = STATE_SUCCESS;

    //创建成功则清除buffer
    reset_tunnel_buffer(&remote_proxy_info->cloud_write_buffer);
    reset_tunnel_buffer(&remote_proxy_info->cloud_read_buffer);
    remote_proxy_info->cloud_channel_state = CLOUD_CHANNEL_CONNECTED;
    remote_proxy_info->ping_time = remote_proxy_info->sysdep->core_sysdep_time();
    remote_proxy_info->pong_time = remote_proxy_info->sysdep->core_sysdep_time();
end_label:

    if (STATE_SUCCESS != ret_code)
    {
        close_cloud_proxy_channel(cloud_connection);
        release_all_session_from_list(&remote_proxy_info->session_list);
        reset_tunnel_buffer(&remote_proxy_info->cloud_write_buffer);
        reset_tunnel_buffer(&remote_proxy_info->cloud_read_buffer);
    }

    return ret_code;
}

static LOCAL_SERVICE_NODE_S *get_service_node(LOCAL_SERVICES_S *services, PROXY_PROT_SESSION_PARAMS_S *session_params)
{
    LOCAL_SERVICE_NODE_S *item = NULL, *next = NULL;

    core_list_for_each_entry_safe(item, next, &services->service_list, node, LOCAL_SERVICE_NODE_S)
    {
        if (strcmp(session_params->type, item->type) == 0) {
            memset(session_params->ip, 0, sizeof(session_params->ip));
            strncpy(session_params->ip, item->ip, strlen(item->ip));
            session_params->port = item->port;
            return item;
        }
    }

    return NULL;
}

static void *connect2Local_service(char *host_addr, int port)
{
    void* network_handle = NULL;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return NULL;
    }
    network_handle = sysdep->core_sysdep_network_init();
    if (network_handle == NULL) {
        return NULL;
    }
    printf("local %s, %d\r\n", host_addr, port);
    int socket_type = CORE_SYSDEP_SOCKET_TCP_CLIENT;
    uint16_t tmp_port = port;
    uint32_t timeout_ms = 5 * 1000;
    int32_t res = STATE_SUCCESS;
    if ((res = sysdep->core_sysdep_network_setopt(network_handle, CORE_SYSDEP_NETWORK_SOCKET_TYPE,
               &socket_type)) < STATE_SUCCESS ||
            (res = sysdep->core_sysdep_network_setopt(network_handle, CORE_SYSDEP_NETWORK_HOST,
                    host_addr)) < STATE_SUCCESS ||
            (res = sysdep->core_sysdep_network_setopt(network_handle, CORE_SYSDEP_NETWORK_PORT,
                    &tmp_port)) < STATE_SUCCESS ||
            (res = sysdep->core_sysdep_network_setopt(network_handle,
                    CORE_SYSDEP_NETWORK_CONNECT_TIMEOUT_MS,
                    &timeout_ms)) < STATE_SUCCESS) {
        sysdep->core_sysdep_network_deinit(&network_handle);
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "core_sysdep_network_setopt error\r\n");
        return NULL;
    }

    if((res = sysdep->core_sysdep_network_establish(network_handle)) < STATE_SUCCESS)
    {
        sysdep->core_sysdep_network_deinit(&network_handle);
        return NULL;
    }
    return network_handle;
}

/*********************************************************
 * 接口名称：new_session
 * 描       述：创建南向的本地服务的会话
 * 输入参数：void* params
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
static void new_session(REMOTE_PROXY_INFO_S *remote_proxy_info, PROXY_PROT_HEADER_S *head_info)
{
    if(CLOUD_CHANNEL_CONNECTED != remote_proxy_info->cloud_channel_state)
    {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "cloud channel does not finish handshake!\r\n");
        return;
    }
    //云端指令是创建一个新的session
    void *network_handle = NULL;
    char *session_id = head_info->session_id;
    LOCAL_SERVICES_S *local_services = remote_proxy_info->cloud_channel_params.local_services;
    PROXY_PROT_SESSION_PARAMS_S session_params;
    memset(&session_params, 0, sizeof(PROXY_PROT_SESSION_PARAMS_S));
    strncpy(session_params.type, head_info->srv_type, strlen(head_info->srv_type));
    core_log2(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "new session session_id:%s, type %s\r\n",head_info->session_id,head_info->srv_type);
    LOCAL_SERVICE_NODE_S *service = get_service_node(local_services, &session_params);
    if(service == NULL) {
        return;
    }
    //判断session List 是否已经上限
    if (get_session_num_from_list(&remote_proxy_info->session_list) >= DEFAULT_SESSION_COUNT)
    {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "session List is limited\r\n");
        cloud_channel_response_with_error(remote_proxy_info, ERR_SESSION_CREATE_FAILED, "socketfd insert error", head_info->msgID, NULL);
        return;
    }

    //根据session的信息链接本地服务
    network_handle = connect2Local_service(session_params.ip, session_params.port);
    if (network_handle == NULL)
    {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "failed to connect to local service\r\n");
        cloud_channel_response_with_error(remote_proxy_info, ERR_BACKEND_SERVICE_UNAVALIBE, "local service is not available", head_info->msgID, NULL);
        return;
    }

    if (STATE_SUCCESS != add_one_session_to_list(&remote_proxy_info->session_list, session_id, network_handle, service))
    {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "failed to alloc new session\r\n");
        cloud_channel_response_with_error(remote_proxy_info, ERR_SESSION_CREATE_FAILED, "memory error", head_info->msgID, NULL);
        return;
    }

    if(0 != cloud_channel_response_new_session(remote_proxy_info, head_info->msgID, session_id))
    {
        core_log2(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "cloud channel response to new session error!session_id:%s, msgID:%s",session_id,head_info->msgID);
        return;
    }
}

/*********************************************************
 * 接口名称：release_session
 * 描       述：释放南向的本地服务的会话
 * 输入参数：void* params
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
static void release_session(REMOTE_PROXY_INFO_S *remote_proxy_info, PROXY_PROT_HEADER_S *head_info)
{
    if(CLOUD_CHANNEL_CONNECTED != remote_proxy_info->cloud_channel_state)
    {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "cloud channel does not finish handshake!\r\n");
        return;
    }
    core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "release the session: %s\r\n", head_info->session_id);
    (void)release_one_session_from_list(&remote_proxy_info->session_list, head_info->session_id);
    if(0 != cloud_channel_response_release_session(remote_proxy_info, head_info->msgID, head_info->session_id))
    {
        core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "release the session failed! sessionID:%s", head_info->session_id);
    }

    return;
}

/*********************************************************
 * 接口名称：send_raw_data2Local_service
 * 描       述：转发数据到本地服务
 * 输入参数：void* params
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
static int send_raw_data2Local_service(RA_BUFFER_INFO_S  *channel_buffer,SESSION_INFO_NODE_S *session_node)
{
    int total_len = get_tunnel_buffer_read_len(channel_buffer);
    char *buffer = get_tunnel_buffer_read_pointer(channel_buffer);
    int res = STATE_SUCCESS;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return STATE_TUNNEL_FAILED;
    }
    // core_log_hexdump(-0X02,'s',(uint8_t *)buffer,(uint32_t)total_len);
    if( (res = sysdep->core_sysdep_network_send(session_node->network_handle, (uint8_t *)buffer, total_len, 500, NULL)) < STATE_SUCCESS)
    {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "core_sysdep_network_send error\r\n");
        return STATE_TUNNEL_FAILED;
    }

    return STATE_SUCCESS;
}

/*********************************************************
 * 接口名称：proc_cloud_channel_data
 * 描       述：云端下行数据的处理
 * 输入参数：REMOTE_PROXY_INFO_S *remote_proxy_info
 *           PROXY_PROT_HEADER_S *head_info
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
static void proc_cloud_channel_data(REMOTE_PROXY_INFO_S *remote_proxy_info, PROXY_PROT_HEADER_S *head_info)
{
    RA_BUFFER_INFO_S *cloud_read_buffer = &remote_proxy_info->cloud_read_buffer;

    if(CLOUD_CHANNEL_CONNECTED != remote_proxy_info->cloud_channel_state)
    {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "cloud channel does not finish handshake!\r\n");
        return;
    }

    //数据：则转发到本地相应的服务中，保证发送数据的完整性
    SESSION_INFO_NODE_S *session_node = get_one_session_from_list(&remote_proxy_info->session_list, head_info->session_id);
    if (NULL == session_node)
    {
        //session异常则退出
        core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "session id invalid: %s", head_info->session_id);
        reset_tunnel_buffer(cloud_read_buffer);
        return;
    }

    if(0 != send_raw_data2Local_service(cloud_read_buffer, session_node))
    {
        //session异常则退出
        core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "release the session: %s", head_info->session_id);
        send_cloud_channel_release_session_request(remote_proxy_info, head_info->msgID, head_info->session_id);
        release_one_session_from_list(&remote_proxy_info->session_list, head_info->session_id);
        reset_tunnel_buffer(cloud_read_buffer);
        return;
    }
    return;
}

/*********************************************************
 * 接口名称：release_cloud_channel_resource
 * 描       述：释放北向连接云端的通道资源
 * 输入参数：void* params
 * 输出参数：
 * 返  回 值：
 * 说       明：接口可重入
 *********************************************************/
static int release_cloud_channel_resource(CLOUD_CHANNEL_PARAMS_S* cloud_channel_params)
{
    void *cloud_connection = cloud_channel_params->cloud_connection;

    //释放云端连接资源
    if(cloud_connection != NULL)
    {
        close_cloud_proxy_channel(cloud_connection);
        cloud_channel_params->cloud_connection = NULL;
    }

    return STATE_SUCCESS;
}

/*********************************************************
 * 接口名称：release_remote_proxy
 * 描       述：释放远程代理通道的所有资源
 * 输入参数：REMOTE_PROXY_INFO_S *remote_proxy_info
 * 输出参数：
 * 返  回 值：
 * 说       明：此接口释放所有连接资源。
 *          包括：北向的云端连接资源，南向的本地服务的链接资源
 *********************************************************/
static int release_remote_proxy(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    if(CLOUD_CHANNEL_CLOSED == remote_proxy_info->cloud_channel_state)
    {
        return STATE_SUCCESS;
    }

    core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "end the cloud channel.\r\n");

    //释放云端websocket连接资源
    release_cloud_channel_resource(&remote_proxy_info->cloud_channel_params);
    release_all_session_from_list(&remote_proxy_info->session_list);

    //清理其它相关资源
    reset_tunnel_buffer(&remote_proxy_info->cloud_write_buffer);
    reset_tunnel_buffer(&remote_proxy_info->cloud_read_buffer);
    remote_proxy_info->cloud_channel_state = CLOUD_CHANNEL_CLOSED;
    remote_proxy_info->keepalive_cnt = 0;

    return STATE_SUCCESS;
}

/*********************************************************
 * 接口名称：cloud_data_proc
 * 描       述：云端数据的处理
 * 输入参数：REMOTE_PROXY_INFO_S *remote_proxy_info
 * 输出参数：
 * 返  回 值：
 * 说       明：cloud_data有一个接收buffer，确保接收数据的完整性
 *          每一个本地服务有一个sendbuff，确保发送给本地服务的数据完整性
 *********************************************************/
static void cloud_data_proc(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    void *cloud_connection = remote_proxy_info->cloud_channel_params.cloud_connection;
    RA_BUFFER_INFO_S *cloud_read_buffer = &remote_proxy_info->cloud_read_buffer;
    int ret_code = STATE_SUCCESS;
    int is_fin = 0;

    //nopoll尽力处理完所有已经接收的数据（数据已经从socket缓存放入nopoll的缓存中）
    for(;;)
    {
        //取到一个_fin==1的包（之后需要清除接收buffer）
        //取到一个_fin==0的包
        //取不到完整的包
        //取到一个错误包（之后需要清除buffer）
        ret_code = read_cloud_proxy_channel(cloud_connection, cloud_read_buffer, &is_fin);
        if(ret_code == STATE_TUNNEL_FAILED)
        {
            //异常退出，需要清理buffer,一般是buffer开辟的不够大
            reset_tunnel_buffer(cloud_read_buffer);
            return;
        }
        else if(ret_code == STATE_TUNNEL_TIMEOUT)
        {
            //确认已经无包可取时退出
            break;
        }

        //不是一个完整的包,则等待下次接收,保证数据完整性
        if (is_fin == 0)
        {
            core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "this websocket message not have Fin flag!\r\n");
            continue;
        }

        //读取自定义的proxy协议的数据头，判断数据类型，分别如下：
        PROXY_PROT_HEADER_S header_info;
        memset(&header_info, 0, sizeof(PROXY_PROT_HEADER_S));
        ret_code = get_remote_access_protocol_header(cloud_read_buffer, &header_info);
        if (STATE_SUCCESS != ret_code)
        {
            //数据包异常，则丢弃，处理下一个包
            reset_tunnel_buffer(cloud_read_buffer);
            continue;
        }

        move_tunnel_buffer_read_pointer(cloud_read_buffer,header_info.hdr_len);

        //如果协议头中描述的负载长度不等于得到的完整包的负载长度，则作为异常包丢弃
        if(header_info.payload_len != get_tunnel_buffer_read_len(cloud_read_buffer))
        {
            //数据包异常，则丢弃，处理下一个包
            core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "payload length info is error!\r\n");
            reset_tunnel_buffer(cloud_read_buffer);
            continue;
        }

        //云端下行数据和命令的处理
        if (header_info.msg_type == MSG_SERVICE_CONSUMER_NEW_SESSION)
        {
            //命令：本地服务session的开启
            new_session(remote_proxy_info, &header_info);
        }
        else if (header_info.msg_type == MSG_SERVICE_CONSUMER_RELEASE_SESSION)
        {
            //命令：则做本地服务session的关闭
            release_session(remote_proxy_info, &header_info);
        }
        else if (header_info.msg_type == MSG_SERVICE_RAW_PROTOCOL)
        {
            //数据：云端传输给本地服务的数据
            proc_cloud_channel_data(remote_proxy_info, &header_info);
        }
        else
        {
            //error
            core_log2(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "recv error websocket package! header %.*s ", &header_info.hdr_len, cloud_read_buffer);
        }

        //处理成功则清除缓存
        reset_tunnel_buffer(cloud_read_buffer);
    }
}

/*********************************************************
 * 接口名称：local_service_data_proc
 * 描       述：本地服务的数据的处理
 * 输入参数：REMOTE_PROXY_INFO_S *remote_proxy_info
 *          SESSION_INFO_NODE_S  *session_info
 * 输出参数：
 * 返  回 值：
 * 说       明：发送给云通道的数据完整性通过检测nopoll_conn_send_ping完成
 *********************************************************/
static int local_service_data_proc(SESSION_INFO_NODE_S *session_info, void *remote_proxy_info_in)
{
    int recv_len = 0;
    REMOTE_PROXY_INFO_S *remote_proxy_info = (REMOTE_PROXY_INFO_S *)remote_proxy_info_in;
    void *cloud_connection = remote_proxy_info->cloud_channel_params.cloud_connection;
    char common_buffer[DEFAULT_MSG_HDR_LEN] = {0};
    //重置writebuffer
    RA_BUFFER_INFO_S *channel_buffer = &remote_proxy_info->cloud_write_buffer;
    reset_tunnel_buffer(channel_buffer);

    char *buffer = get_tunnel_buffer_write_pointer(channel_buffer);
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (NULL == sysdep) {
        return STATE_TUNNEL_FAILED;
    }
    recv_len = sysdep->core_sysdep_network_recv(session_info->network_handle, (uint8_t *)buffer, (channel_buffer->size - DEFAULT_MSG_HDR_LEN - 4), 10, NULL);
    //接收本地服务的数据
    if (recv_len < 0)
    {
        //接收异常退出
        if((errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
        {
            //重试
            return STATE_TUNNEL_FAILED;
        }
        else
        {
            core_log2(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "system error:%s. local service exit, release session: %s", strerror(errno),session_info->sessionID);

            send_cloud_channel_release_session_request(remote_proxy_info,NULL,session_info->sessionID);
            release_one_session_from_list(&remote_proxy_info->session_list, session_info->sessionID);
            return STATE_TUNNEL_FAILED;
        }
    }
    else if (recv_len == 0)
    {
        return STATE_TUNNEL_FAILED;
    }
    if(-1 == move_tunnel_buffer_write_pointer(channel_buffer, recv_len))
    {
        return STATE_TUNNEL_FAILED;
    }
    // core_log_hexdump(-0X02,'r',(uint8_t *)buffer,(uint32_t)recv_len);
    //获取报文头
    int header_len = splice_proxy_protocol_header(common_buffer,DEFAULT_MSG_HDR_LEN,MSG_SERVICE_RAW_PROTOCOL, recv_len, NULL, session_info->sessionID);
    if(header_len <= 0)
    {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "splice local services packets header error!\r\n");
        return STATE_TUNNEL_FAILED;
    }

    //拼接报文
    if( 0 != join_content_before_tunnel_buffer(common_buffer,header_len,channel_buffer))
    {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "join content before buffer error\r\n");
        return STATE_TUNNEL_FAILED;
    }

    /*core_log_hexdump(-0X2,2,(uint8_t *)(channel_buffer->buffer+header_len),(uint32_t)recv_len);*/
    /*发送至云端，如果消息阻塞，则整包丢弃*/
    int writen_len = write_cloud_proxy_channel(cloud_connection, channel_buffer->buffer, get_tunnel_buffer_read_len(channel_buffer),remote_proxy_info->cloud_channel_params.trans_timeout);
    if (writen_len != get_tunnel_buffer_read_len(channel_buffer))
    {
        return STATE_TUNNEL_FAILED;
    }

    return STATE_SUCCESS;
}

#ifdef  CLOUD_CHANNEL_FD_SET

#include <sys/select.h>
typedef struct
{
    fd_set               *rfds;                                      //可读fd数组，用于记录可读的所有fds
    int                   fd_num;                                     //可读的fds的数目
    SESSION_INFO_NODE_S  *session_array[DEFAULT_SESSION_COUNT];       //可读的fd对应的session，所有session按顺序放入此数组
} ACTIVE_FD_CALLBACK_PARAMS;

typedef struct
{
    fd_set        *rfds;                                      //可读fd数组，用于记录可读的所有fds
    int            max_fd;                                     //最大的fd
} SET_FD_CALLBACK_PARAMS;

/*********************************************************
 * 接口名称：set_local_service_sockets_to_rfds
 * 描       述：记录本地服务中发生可读事件的socket
 * 输入参数：SESSION_INFO_NODE_S *session_info
 *           void *data
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
static int set_local_service_sockets_to_rfds(SESSION_INFO_NODE_S *session_info, void *data)
{
    SET_FD_CALLBACK_PARAMS *param = (SET_FD_CALLBACK_PARAMS*) data;
    int fd = -1;
    memcpy((void *)&fd, session_info->network_handle, sizeof(fd));
    if (fd > 0)
    {
        FD_SET(fd, param->rfds);

        if (fd > param->max_fd)
        {
            param->max_fd = fd;
        }
    }

    return 0;
}

/*********************************************************
 * 接口名称：init_select_io_array
 * 描       述：初始化多路复用的IO列表
 * 输入参数：REMOTE_PROXY_INFO_S *remote_proxy_info
 * 输出参数：int *maxfd
 * 返  回 值：
 * 说       明：返回最大的fd
 *********************************************************/
static inline int init_select_io_array(REMOTE_PROXY_INFO_S *remote_proxy_info, fd_set *rfds_obj, int *maxfd)
{
    int cloudfd = (int)nopoll_conn_socket(remote_proxy_info->cloud_channel_params.cloud_connection);
    SESSION_LIST_S *session_list = &remote_proxy_info->session_list;

    FD_ZERO(rfds_obj);

    if (cloudfd >= 0)
    {
        FD_SET(cloudfd, rfds_obj);
    }
    else
    {
        //云端通道socket异常需要释放
        return STATE_TUNNEL_RESET;
    }

    SET_FD_CALLBACK_PARAMS params;
    memset(&params, 0x0, sizeof(SET_FD_CALLBACK_PARAMS));
    params.max_fd = cloudfd;
    params.rfds = rfds_obj;

    iterate_each_session(session_list, set_local_service_sockets_to_rfds, &params);

    *maxfd = params.max_fd;

    return STATE_SUCCESS;
}

/*********************************************************
 * 接口名称：record_active_local_service_sockets
 * 描       述：记录本地服务中发生可读事件的socket
 * 输入参数：SESSION_INFO_NODE_S *session_info
 *           void *data
 * 输出参数：
 * 返  回 值：
 * 说       明：
 *********************************************************/
static int record_active_local_service_sockets(SESSION_INFO_NODE_S *session_info, void *data)
{
    ACTIVE_FD_CALLBACK_PARAMS *param = (ACTIVE_FD_CALLBACK_PARAMS*) data;
    int fd = -1;
    memcpy((void *)&fd, session_info->network_handle, sizeof(fd));
    if (FD_ISSET(fd, param->rfds) > 0)
    {
        param->session_array[param->fd_num] = session_info;
        param->fd_num++;
    }
    return 0;
}

/*********************************************************
 * 接口名称：proxy_data_proc
 * 描       述：代理的南北向数据的处理
 * 输入参数：void* params
 * 输出参数：
 * 返  回 值：
 * 说       明：接收南北向数据，并向对端进行转发，所有网络IO非阻塞
 *********************************************************/
static int proxy_data_proc(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    SESSION_LIST_S *session_list = &remote_proxy_info->session_list;
    int maxfd = 0;
    int ret = 0;
    fd_set rfds;
    ACTIVE_FD_CALLBACK_PARAMS params;
    memset(&params, 0x0, sizeof(ACTIVE_FD_CALLBACK_PARAMS));
    struct timeval tv = { .tv_sec = 1, .tv_usec = 0 };
    FD_ZERO(&rfds);

    do
    {
        //初始化多路复用的IO列表
        if(STATE_TUNNEL_RESET == init_select_io_array(remote_proxy_info, &rfds, &maxfd))
        {
            return STATE_TUNNEL_RESET;
        }

        //判断多路复用IO是否有可读数据
        ret = select(maxfd + 1, &rfds, NULL, NULL, &tv);
        if (0 == ret)
        {
            //超时没有任何可读事件
            break;
        }
        else if (ret < 0)
        {
            //系统error, 由以下本地和云端通道处理对通道异常进行判断，根据情况对资源进行释放回收
            core_log1(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "failed to select: %s", strerror(errno));
        }

        //ret > 0的处理部分
        //判断fd是否有可读状态
        //判断是否是本地服务可读
        params.rfds = &rfds;
        iterate_each_session(session_list, record_active_local_service_sockets, &params);
        if (params.fd_num > 0)
        {
            //依次读取可读的本地服务socket
            int j = 0;
            for (j = 0; j < params.fd_num; j++)
            {
                local_service_data_proc(params.session_array[j], remote_proxy_info);
            }
        }

        //判断是否是云端服务可读
        if (FD_ISSET(nopoll_conn_socket(remote_proxy_info->cloud_channel_params.cloud_connection), &rfds) > 0)
        {
            //接收处理云端数据
            cloud_data_proc(remote_proxy_info);
        }
    } while (0);

    return STATE_SUCCESS;
}

#else
typedef struct
{
    int                   fd_num;                                     //可读的fds的数目
    SESSION_INFO_NODE_S  *session_array[DEFAULT_SESSION_COUNT];       //可读的fd对应的session，所有session按顺序放入此数组
} ACTIVE_FD_CALLBACK_PARAMS;

static int record_active_local_service_sockets(SESSION_INFO_NODE_S *session_info, void *data)
{
    ACTIVE_FD_CALLBACK_PARAMS *param = (ACTIVE_FD_CALLBACK_PARAMS*) data;
    param->session_array[param->fd_num] = session_info;
    param->fd_num++;
    return 0;
}

static int proxy_data_proc(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    SESSION_LIST_S *session_list = &remote_proxy_info->session_list;
    ACTIVE_FD_CALLBACK_PARAMS params = { .fd_num = 0 };
    iterate_each_session(session_list, record_active_local_service_sockets, &params);
    if (params.fd_num > 0)
    {
        //依次读取可读的本地服务socket
        int j = 0;
        for (j = 0; j < params.fd_num; j++)
        {
            local_service_data_proc(params.session_array[j], remote_proxy_info);
        }
    }
    cloud_data_proc(remote_proxy_info);
    return STATE_SUCCESS;
}
#endif

int init_remote_proxy_resource(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (sysdep == NULL) {
        return -1;
    }

    memset(remote_proxy_info, 0x0, sizeof(REMOTE_PROXY_INFO_S));
    //初始化资源
    if (0 != create_tunnel_buffer(&remote_proxy_info->cloud_read_buffer,DEFAULT_MSG_BUFFER_LEN))
    {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "cloud channel buffer malloc failed\r\n");
        return STATE_TUNNEL_FAILED;
    }

    if (0 != create_tunnel_buffer(&remote_proxy_info->cloud_write_buffer,DEFAULT_SEND_MSG_BUFFER_LEN))
    {
        core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "cloud channel buffer malloc failed\r\n");
        return STATE_TUNNEL_FAILED;
    }
    remote_proxy_info->lock = sysdep->core_sysdep_mutex_init();
    init_session_list(&remote_proxy_info->session_list);
    remote_proxy_info->sysdep = sysdep;
    //初始化重连时间
    remote_proxy_info->retry_info.connect_time = 0;
    remote_proxy_info->status = 0;
    return STATE_SUCCESS;
}

void update_cloud_channel_params(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    //用户开关事件发生,且用户操作的是开操作，即用户触发开关时立即发起重连
    if(0 != remote_proxy_info->has_switch_event && 0 != remote_proxy_info->remote_proxy_channel_switch)
    {
        clean_retry_params(remote_proxy_info);
    }

    //清除事件标记
    remote_proxy_info->has_switch_event = 0;
}

#define RETRY_TIMES_CYCLE 5

/*********************************************************
 * 接口名称：get_wait_time_exp
 * 描       述：获取等待时间的指数
 * 输入参数：int retry_times 重连次数
 * 输出参数：
 * 返  回 值：0~6
 * 说       明：
 *********************************************************/
int get_wait_time_exp(int retry_times)
{
    int exp = retry_times/RETRY_TIMES_CYCLE;
    if(exp > 6)
    {
        exp = 6;
    }
    return exp;
}

/*********************************************************
 * 接口名称：get_wait_time_period
 * 描       述：获取等待时间周期
 * 输入参数：int exp 重连次数
 * 输出参数：
 * 返  回 值：等待周期时间s
 * 说       明：
 *********************************************************/
int get_wait_time_period(int exp)
{
    int i= 1;
    i = i << exp;
    return i;
}

int whether_connect_time_up(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    uint64_t timeout = get_wait_time_period(get_wait_time_exp(remote_proxy_info->retry_info.retry_times));

    uint64_t tp_now;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    tp_now = sysdep->core_sysdep_time();
    if(tp_now - remote_proxy_info->retry_info.connect_time >= timeout * 1000) {
        return STATE_SUCCESS;
    }
    else {
        return STATE_TUNNEL_FAILED;
    }
}

static void cloud_channel_error_state_proc(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    //错误状态
    core_log(aiot_sysdep_get_portfile(), STATE_TUNNEL_BASE, "cloud channel state error! state\r\n");
    release_remote_proxy(remote_proxy_info);

    //用户关闭时，主动清除重连参数
    clean_retry_params(remote_proxy_info);
}

static void cloud_channel_closed_state_proc(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    if(0 != remote_proxy_info->remote_proxy_channel_switch) {
        //判断是否连接时间到
        if(0 != whether_connect_time_up(remote_proxy_info)) {
            return;
        }

        //开关标记为开，则创建云端连接
        if(STATE_SUCCESS != create_remote_proxy(remote_proxy_info)) {
            //更新连接时间和连接次数,等待下次重连
            update_retry_params(remote_proxy_info);
        }
    }

    return;
}

static void cloud_channel_connected_state_proc(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    if(0 != remote_proxy_info->remote_proxy_channel_switch) {
        //开关标记为开，则做数据的处理
        proxy_data_proc(remote_proxy_info);
        //通道保活
        if(0 != cloud_channel_keepalive(remote_proxy_info)) {
            //开关标记为关，释放资源，状态转移到closed状态
            release_remote_proxy(remote_proxy_info);

            //用户关闭时，主动清除重连参数
            clean_retry_params(remote_proxy_info);
        }
    }
    else {
        //开关标记为关，释放资源，状态转移到closed状态
        release_remote_proxy(remote_proxy_info);

        //用户关闭时，主动清除重连参数
        clean_retry_params(remote_proxy_info);
        remote_proxy_info->remote_proxy_channel_switch = 1;
    }

    return;
}

void remote_proxy_process(REMOTE_PROXY_INFO_S *remote_proxy_info)
{
    remote_proxy_info->sysdep->core_sysdep_mutex_lock(remote_proxy_info->lock);
    update_cloud_channel_params(remote_proxy_info);

    if(CLOUD_CHANNEL_CLOSED == remote_proxy_info->cloud_channel_state) {
        cloud_channel_closed_state_proc(remote_proxy_info);
    }
    else if(CLOUD_CHANNEL_CONNECTED == remote_proxy_info->cloud_channel_state) {
        cloud_channel_connected_state_proc(remote_proxy_info);
    }
    else {
        cloud_channel_error_state_proc(remote_proxy_info);
    }
    // 更新线程计数器
    UPDATE_PROXY_THREAD_RUNNING_CNT(remote_proxy_info);
    remote_proxy_info->sysdep->core_sysdep_mutex_unlock(remote_proxy_info->lock);
}

int deinit_remote_proxy_resource(REMOTE_PROXY_INFO_S **remote_proxy)
{
    REMOTE_PROXY_INFO_S *remote_proxy_info = *remote_proxy;
    aiot_sysdep_portfile_t *sysdep = aiot_sysdep_get_portfile();
    if (sysdep == NULL) {
        return -1;
    }
    sysdep->core_sysdep_mutex_lock(remote_proxy_info->lock);

    release_remote_proxy(remote_proxy_info);
    release_tunnel_buffer(&remote_proxy_info->cloud_read_buffer);
    release_tunnel_buffer(&remote_proxy_info->cloud_write_buffer);
    release_all_session_from_list(&remote_proxy_info->session_list);
    sysdep->core_sysdep_mutex_deinit(&remote_proxy_info->session_list.list_lock);
    sysdep->core_sysdep_mutex_unlock(remote_proxy_info->lock);

    sysdep->core_sysdep_mutex_deinit(&remote_proxy_info->lock);
    sysdep->core_sysdep_free(remote_proxy_info);
    *remote_proxy = NULL;

    return STATE_SUCCESS;
}