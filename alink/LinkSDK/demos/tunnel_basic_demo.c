/*
 * 这个例程用于演示远程登录/安全隧道功能的使用，
 *
 * + 该例程将该功能分为两个部分：隧道开关模块，隧道功能模块，可将两个模块用于不同的进程。
 * + 隧道开关模块[aiot_tunnel_switch_***] 用于请求/接收/解析隧道的消息
 * + 隧道功能模块[aiot_tunnel_***] 用于隧道的建连/数据收发及管理
 *
 * 需要用户关注或修改的部分, 已经用 TODO 在注释中标明
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_tunnel_api.h"

/* 位于portfiles/aiot_port文件夹下的系统适配函数集合 */
extern aiot_sysdep_portfile_t g_aiot_sysdep_portfile;

/* 位于external/ali_ca_cert.c中的服务器证书 */
extern const char *ali_ca_cert;
static pthread_t g_tunnel_process_thread;
static pthread_t g_tunnel_switch_thread;
static uint8_t g_tunnel_switch_thread_running = 0;

aiot_tunnel_service_t services[] = {
    {
        .type = "_SSH",
        .ip = "127.0.0.1",
        .port = 22,
    },
};

/* TODO: 如果要关闭日志, 就把这个函数实现为空, 如果要减少日志, 可根据code选择不打印
 *
 * 例如: [1577589489.033][LK-0317] mqtt_basic_demo&a13FN5TplKq
 *
 * 上面这条日志的code就是0317(十六进制), code值的定义见core/aiot_state_api.h
 *
 */

/* 日志回调函数, SDK的日志会从这里输出 */
int32_t demo_state_logcb(int32_t code, char *message)
{
    printf( "%s", message);
    return 0;
}


void _demo_tunnel_event_cb(void *handle, const aiot_tunnel_event_t *event, void *userdata)
{
    switch(event->type)
    {
    case AIOT_TUNNEL_EVT_CONNECT:
        printf( "ra_event_cb AIOT_TUNNEL_EVT_CONNECT %s \r\n", event->tunnel_id);
        /* TODO: 告知websocket建连成功, 不可以在这里调用耗时较长的阻塞函数 */
        break;
    case AIOT_TUNNEL_EVT_DISCONNECT:
        printf( "ra_event_cb AIOT_TUNNEL_EVT_DISCONNECT %s \r\n", event->tunnel_id);
        /* TODO: 告知websocket掉线, 不可以在这里调用耗时较长的阻塞函数 */
        break;
    case AIOT_TUNNEL_EVT_EXPIRED:
        printf( "ra_event_cb AIOT_TUNNEL_EVT_EXPIRED %s \r\n", event->tunnel_id);
        /* TODO: 隧道认证信息过期，需要更新隧道验证信息或者重新打开隧道 */
        break;
    }
}

typedef struct {
    int32_t operation;
    char tunnel_id[128];
    char host[128];
    char port[128];
    char path[128];
    char token[128];
} demo_tunnel_info_t;

void demo_tunnel_do_operation(void *tunnel_handle, char *data, int32_t len)
{
    demo_tunnel_info_t *info = (demo_tunnel_info_t *)data;
    aiot_tunnel_connect_param_t params;
    if(len != sizeof(demo_tunnel_info_t)) {
        return;
    }

    memset(&params, 0, sizeof(params));
    params.host = info->host;
    params.port = info->port;
    params.path = info->path;
    params.token = info->token;

    if (1 == info->operation ) {
        /* 新增隧道建连 */
        aiot_tunnel_add(tunnel_handle, info->tunnel_id, &params);
    } else if(0 == info->operation) {
        /* 关闭隧道 */
        aiot_tunnel_delete(tunnel_handle, info->tunnel_id);
    } else if(2 == info->operation) {
        aiot_tunnel_update(tunnel_handle, info->tunnel_id, &params);
    }
}


void* demo_tunnel_listen_task(void *tunnel_handle)
{
    char msg_buf[1024];
    int32_t len = 0;
    char *data = NULL;
    int msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
    if (msgid < 0) {
        printf( "demo_tunnel_listen_task msgget error\r\n");
    }
    printf("demo_tunnel_listen_task Listen \r\n");
    
    while(g_tunnel_switch_thread_running) {
        memset(msg_buf, 0, sizeof(msg_buf));
        len = msgrcv(msgid, msg_buf, sizeof(msg_buf) - sizeof(long), 1, IPC_NOWAIT);
        if(len > 0) {
            /* 消息队列通信头部long表示type，忽略 */
            data = msg_buf + sizeof(long);
            
            demo_tunnel_do_operation(tunnel_handle, data, len);
            printf( "msg recv data len %d, %s\r\n", len, data);
        }
        usleep(100 * 1000);
    }

    return NULL;
}


int main(int argc, char *argv[])
{
    aiot_sysdep_network_cred_t cred; /* 安全凭据结构体, 如果要用TLS, 这个结构体中配置CA证书等参数 */

    /* 配置SDK的底层依赖 */
    aiot_sysdep_set_portfile(&g_aiot_sysdep_portfile);
    /* 配置SDK的日志输出 */
    aiot_state_set_logcb(demo_state_logcb);

    /* 创建SDK的安全凭据, 用于建立TLS连接 */
    memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
    #if 1
    cred.option = AIOT_SYSDEP_NETWORK_CRED_SVRCERT_CA;  /* 使用RSA证书校验MQTT服务端 */
    cred.max_tls_fragment = 16384; /* 最大的分片长度为16K, 其它可选值还有4K, 2K, 1K, 0.5K */
    cred.sni_enabled = 1;                               /* TLS建连时, 支持Server Name Indicator */
    cred.x509_server_cert = ali_ca_cert;                 /* 用来验证MQTT服务端的RSA根证书 */
    cred.x509_server_cert_len = strlen(ali_ca_cert);     /* 用来验证MQTT服务端的RSA根证书长度 */
    #endif

    /* 创建1个RA实例并内部初始化默认参数 */
    void *tunnel_handle = aiot_tunnel_init();
    if (tunnel_handle == NULL) {
        printf( "aiot_tunnel_init failed\n");
        return -1;
    }

    /* 配置网络连接的安全凭据, 上面已经创建好了 */
    aiot_tunnel_setopt(tunnel_handle, AIOT_TUNNELOPT_NETWORK_CRED, (void *)&cred);
    /* 配置RA内部事件回调函数， 可选*/
    aiot_tunnel_setopt(tunnel_handle, AIOT_TUNNELOPT_EVENT_HANDLER, (void *)_demo_tunnel_event_cb);
    /* 配置本地可支持的服务 */
    for(int i = 0; i < sizeof(services) / sizeof(aiot_tunnel_service_t); i++) {
        aiot_tunnel_setopt(tunnel_handle, AIOT_TUNNELOPT_ADD_SERVICE, (void *)&services[i]);
    }

    /*开启线程，运行RA服务*/
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (0 != pthread_create(&g_tunnel_process_thread, &attr, aiot_tunnel_start, (void*) tunnel_handle))
    {
        printf( "creat remote_proxy_thread error!");
        return -1;
    }

    g_tunnel_switch_thread_running = 1;
    if (0 != pthread_create(&g_tunnel_switch_thread, &attr, demo_tunnel_listen_task, (void*) tunnel_handle))
    {
        printf( "creat aiot_tunnel_start error!");
        return -1;
    }

    while(1) {
        sleep(1);
        /* TODO: 业务逻辑 */
    }

    /* 关闭监听线程，等待线程退出 */
    g_tunnel_switch_thread_running = 0;
    pthread_join(g_tunnel_switch_thread, NULL);

    /* 关闭RA服务 */
    aiot_tunnel_stop(tunnel_handle);

    /* 等待隧道管理线程退出 */
    void *result = NULL;
    pthread_join(g_tunnel_process_thread, &result);
    if(NULL != result) {
        /* 打印出线程退出的状态码 */
        printf("pthread exit state -0x%04X\n", *(int32_t *)result * -1);
    }

    /* 删除隧道模块内的所有资源 */
    aiot_tunnel_deinit(&tunnel_handle);

    return 0;
}

