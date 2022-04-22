/*
 * 这个例程适用于`Linux`这类支持pthread的POSIX设备, 它演示了用SDK配置MQTT参数并建立连接, 之后创建2个线程
 *
 * + 一个线程用于保活长连接
 * + 一个线程用于接收消息, 并在有消息到达时进入默认的数据回调, 在连接状态变化时进入事件回调
 *
 * 需要用户关注或修改的部分, 已经用 TODO 在注释中标明
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"
#include "aiot_ra_api.h"

/* TODO: 替换为自己设备的三元组 */
char *product_key       = "${YourProductKey}";
char *device_name       = "${YourDeviceName}";
char *device_secret     = "${YourDeviceSecret}";

/*
    TODO: 替换为自己实例的接入点

    对于企业实例, 或者2021年07月30日之后（含当日）开通的物联网平台服务下公共实例
    mqtt_host的格式为"${YourInstanceId}.mqtt.iothub.aliyuncs.com"
    其中${YourInstanceId}: 请替换为您企业/公共实例的Id

    对于2021年07月30日之前（不含当日）开通的物联网平台服务下公共实例
    需要将mqtt_host修改为: mqtt_host = "${YourProductKey}.iot-as-mqtt.${YourRegionId}.aliyuncs.com"
    其中, ${YourProductKey}：请替换为设备所属产品的ProductKey。可登录物联网平台控制台，在对应实例的设备详情页获取。
    ${YourRegionId}：请替换为您的物联网平台设备所在地域代码, 比如cn-shanghai等
    该情况下完整mqtt_host举例: a1TTmBPIChA.iot-as-mqtt.cn-shanghai.aliyuncs.com

    详情请见: https://help.aliyun.com/document_detail/147356.html
*/
char  *mqtt_host = "${YourInstanceId}.mqtt.iothub.aliyuncs.com";
uint16_t    port = 443;      /* 无论设备是否使用TLS连接阿里云平台, 目的端口都是443 */

/* 位于portfiles/aiot_port文件夹下的系统适配函数集合 */
extern aiot_sysdep_portfile_t g_aiot_sysdep_portfile;

/* 位于external/ali_ca_cert.c中的服务器证书 */
extern const char *ali_ca_cert;
static pthread_t g_ra_process_thread;
static pthread_t g_mqtt_process_thread;
static pthread_t g_mqtt_recv_thread;
static uint8_t g_mqtt_process_thread_running = 0;
static uint8_t g_mqtt_recv_thread_running = 0;

aiot_ra_service_t services[] = {
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

/* MQTT事件回调函数, 当网络连接/重连/断开时被触发, 事件定义见core/aiot_mqtt_api.h */
void demo_mqtt_event_handler(void *handle, const aiot_mqtt_event_t *event, void *userdata)
{
    switch (event->type) {
    /* SDK因为用户调用了aiot_mqtt_connect()接口, 与mqtt服务器建立连接已成功 */
    case AIOT_MQTTEVT_CONNECT: {
        printf( "AIOT_MQTTEVT_CONNECT\n");
        /* TODO: 处理SDK建连成功, 不可以在这里调用耗时较长的阻塞函数 */
    }
    break;

    /* SDK因为网络状况被动断连后, 自动发起重连已成功 */
    case AIOT_MQTTEVT_RECONNECT: {
        printf( "AIOT_MQTTEVT_RECONNECT\n");
        /* TODO: 处理SDK重连成功, 不可以在这里调用耗时较长的阻塞函数 */
    }
    break;

    /* SDK因为网络的状况而被动断开了连接, network是底层读写失败, heartbeat是没有按预期得到服务端心跳应答 */
    case AIOT_MQTTEVT_DISCONNECT: {
        char *cause = (event->data.disconnect == AIOT_MQTTDISCONNEVT_NETWORK_DISCONNECT) ? ("network disconnect") :
                      ("heartbeat disconnect");
        printf( "AIOT_MQTTEVT_DISCONNECT: %s\n", cause);
        /* TODO: 处理SDK被动断连, 不可以在这里调用耗时较长的阻塞函数 */
    }
    break;

    default: {

    }
    }
}

/* MQTT默认消息处理回调, 当SDK从服务器收到MQTT消息时, 且无对应用户回调处理时被调用 */
void demo_mqtt_default_recv_handler(void *handle, const aiot_mqtt_recv_t *packet, void *userdata)
{
    switch (packet->type) {
    case AIOT_MQTTRECV_HEARTBEAT_RESPONSE: {
        printf( "heartbeat response\n");
        /* TODO: 处理服务器对心跳的回应, 一般不处理 */
    }
    break;

    case AIOT_MQTTRECV_SUB_ACK: {
        printf( "suback, res: -0x%04X, packet id: %d, max qos: %d\n",
                packet->data.sub_ack.res, packet->data.sub_ack.packet_id, packet->data.sub_ack.max_qos);
        /* TODO: 处理服务器对订阅请求的回应, 一般不处理 */
    }
    break;

    case AIOT_MQTTRECV_PUB: {
        printf( "pub, qos: %d, topic: %.*s\n", packet->data.pub.qos, packet->data.pub.topic_len, packet->data.pub.topic);
        printf( "pub, payload: %.*s\n", packet->data.pub.payload_len, packet->data.pub.payload);
        /* TODO: 处理服务器下发的业务报文 */
    }
    break;

    case AIOT_MQTTRECV_PUB_ACK: {
        printf( "puback, packet id: %d\n", packet->data.pub_ack.packet_id);
        /* TODO: 处理服务器对QoS1上报消息的回应, 一般不处理 */
    }
    break;

    default: {

    }
    }
}

/* 执行aiot_mqtt_process的线程, 包含心跳发送和QoS1消息重发 */
void *demo_mqtt_process_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_process_thread_running) {
        res = aiot_mqtt_process(args);
        if (res == STATE_USER_INPUT_EXEC_DISABLED) {
            break;
        }
        sleep(1);
    }
    return NULL;
}

/* 执行aiot_mqtt_recv的线程, 包含网络自动重连和从服务器收取MQTT消息 */
void *demo_mqtt_recv_thread(void *args)
{
    int32_t res = STATE_SUCCESS;

    while (g_mqtt_recv_thread_running) {
        res = aiot_mqtt_recv(args);
        if (res < STATE_SUCCESS) {
            if (res == STATE_USER_INPUT_EXEC_DISABLED) {
                break;
            }
            sleep(1);
        }
    }
    return NULL;
}
void ra_event_cb(void *handle, const aiot_ra_event_t *event, void *userdata)
{
    switch(event->type)
    {
    case AIOT_RA_EVT_CONNECT:
        printf( "ra_event_cb AIOT_RA_EVT_CONNECT %s \r\n", event->tunnel_id);
        /* TODO: 告知websocket建连成功, 不可以在这里调用耗时较长的阻塞函数 */
        break;
    case AIOT_RA_EVT_DISCONNECT:
        printf( "ra_event_cb AIOT_RA_EVT_DISCONNECT %s \r\n", event->tunnel_id);
        /* TODO: 告知websocket掉线, 不可以在这里调用耗时较长的阻塞函数 */
        break;
    case AIOT_RA_EVT_OPEN_WEBSOCKET:
        printf( "ra_event_cb AIOT_RA_EVT_OPEN_WEBSOCKET %s \r\n", event->tunnel_id);
        /* TODO: 告知RA接收到打开websocket链接命令, 不可以在这里调用耗时较长的阻塞函数 */
        break;
    case AIOT_RA_EVT_CLOSE_WEBSOCKET:
        printf( "ra_event_cb AIOT_RA_EVT_CLOSE_WEBSOCKET %s \r\n", event->tunnel_id);
        /* TODO: 告知RA接收到关闭websocket链接命令, 不可以在这里调用耗时较长的阻塞函数 */
        break;
    }
}


int main(int argc, char *argv[])
{
    int32_t     res = STATE_SUCCESS;
    void       *mqtt_handle = NULL;
    aiot_sysdep_network_cred_t cred; /* 安全凭据结构体, 如果要用TLS, 这个结构体中配置CA证书等参数 */

    /* 配置SDK的底层依赖 */
    aiot_sysdep_set_portfile(&g_aiot_sysdep_portfile);
    /* 配置SDK的日志输出 */
    aiot_state_set_logcb(demo_state_logcb);

    /* 创建SDK的安全凭据, 用于建立TLS连接 */
    memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
    cred.option = AIOT_SYSDEP_NETWORK_CRED_SVRCERT_CA;  /* 使用RSA证书校验MQTT服务端 */
    cred.max_tls_fragment = 16384; /* 最大的分片长度为16K, 其它可选值还有4K, 2K, 1K, 0.5K */
    cred.sni_enabled = 1;                               /* TLS建连时, 支持Server Name Indicator */
    cred.x509_server_cert = ali_ca_cert;                 /* 用来验证MQTT服务端的RSA根证书 */
    cred.x509_server_cert_len = strlen(ali_ca_cert);     /* 用来验证MQTT服务端的RSA根证书长度 */

    /* 创建1个MQTT客户端实例并内部初始化默认参数 */
    mqtt_handle = aiot_mqtt_init();
    if (mqtt_handle == NULL) {
        printf( "aiot_mqtt_init failed\n");
        return -1;
    }

    /* TODO: 如果以下代码不被注释, 则例程会用TCP而不是TLS连接云平台 */
    /*
    {
        memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
        cred.option = AIOT_SYSDEP_NETWORK_CRED_NONE;
    }
    */

    /* 配置MQTT服务器地址 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_HOST, (void *)mqtt_host);
    /* 配置MQTT服务器端口 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PORT, (void *)&port);
    /* 配置设备productKey */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_PRODUCT_KEY, (void *)product_key);
    /* 配置设备deviceName */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_NAME, (void *)device_name);
    /* 配置设备deviceSecret */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_DEVICE_SECRET, (void *)device_secret);
    /* 配置网络连接的安全凭据, 上面已经创建好了 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_NETWORK_CRED, (void *)&cred);
    /* 配置MQTT默认消息接收回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_RECV_HANDLER, (void *)demo_mqtt_default_recv_handler);
    /* 配置MQTT事件回调函数 */
    aiot_mqtt_setopt(mqtt_handle, AIOT_MQTTOPT_EVENT_HANDLER, (void *)demo_mqtt_event_handler);

    /* 与服务器建立MQTT连接 */
    res = aiot_mqtt_connect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        /* 尝试建立连接失败, 销毁MQTT实例, 回收资源 */
        aiot_mqtt_deinit(&mqtt_handle);
        printf( "aiot_mqtt_connect failed: -0x%04X\n", -res);
        return -1;
    }

    /* 创建一个单独的线程, 专用于执行aiot_mqtt_process, 它会自动发送心跳保活, 以及重发QoS1的未应答报文 */
    g_mqtt_process_thread_running = 1;
    res = pthread_create(&g_mqtt_process_thread, NULL, demo_mqtt_process_thread, mqtt_handle);
    if (res < 0) {
        printf( "pthread_create demo_mqtt_process_thread failed: %d\n", res);
        return -1;
    }

    /* 创建一个单独的线程用于执行aiot_mqtt_recv, 它会循环收取服务器下发的MQTT消息, 并在断线时自动重连 */
    g_mqtt_recv_thread_running = 1;
    res = pthread_create(&g_mqtt_recv_thread, NULL, demo_mqtt_recv_thread, mqtt_handle);
    if (res < 0) {
        printf( "pthread_create demo_mqtt_recv_thread failed: %d\n", res);
        return -1;
    }

    /* 创建1个RA实例并内部初始化默认参数 */
    void *ra_handle = aiot_ra_init();
    if (ra_handle == NULL) {
        printf( "aiot_ra_init failed\n");
        return -1;
    }

    /* 配置MQTT句柄，ra内部会订阅MQTT的消息 */
    aiot_ra_setopt(ra_handle, AIOT_RAOPT_MQTT_HANDLE, mqtt_handle);
    /* 配置网络连接的安全凭据, 上面已经创建好了 */
    aiot_ra_setopt(ra_handle, AIOT_RAOPT_NETWORK_CRED, (void *)&cred);
    /* 配置RA内部事件回调函数， 可选*/
    aiot_ra_setopt(ra_handle, AIOT_RAOPT_EVENT_HANDLER, (void *)ra_event_cb);
    /* 配置本地可支持的服务 */
    for(int i = 0; i < sizeof(services) / sizeof(aiot_ra_service_t); i++) {
        aiot_ra_setopt(ra_handle, AIOT_RAOPT_ADD_SERVICE, (void *)&services[i]);
    }
    /*开启线程，运行RA服务*/
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (0 != pthread_create(&g_ra_process_thread, &attr, aiot_ra_start, (void*) ra_handle))
    {
        printf( "creat remote_proxy_thread error!");
        return -1;
    }

    /* 设备主动请求隧道建连，该隧道只用于远程登录 */
    aiot_ra_request(ra_handle); 
    int i = 60000;
    while(1)
    {
        sleep(1);
        /* TODO: 业务逻辑 */
        i--;
        /*根据业务逻辑需求，设置条件，需要退出线程可以调用aiot_ra_stop*/
        if(i == 0)
        {
            /*退出线程，关闭RA服务*/
            aiot_ra_stop(ra_handle);
            break;
        }
    }

    /* 主循环进入休眠 */
    void *result = NULL;
    pthread_join(g_ra_process_thread, &result);
    if(NULL != result)
    {
        /* 打印出线程退出的状态码 */
        printf("pthread exit state -0x%04X\n", *(int32_t *)result * -1);
    }

    aiot_ra_deinit(&ra_handle);

    g_mqtt_process_thread_running = 0;
    g_mqtt_recv_thread_running = 0;
    sleep(1);
    pthread_join(g_mqtt_process_thread, NULL);
    pthread_join(g_mqtt_recv_thread, NULL);

    /* 断开MQTT连接, 一般不会运行到这里 */
    res = aiot_mqtt_disconnect(mqtt_handle);
    if (res < STATE_SUCCESS) {
        aiot_mqtt_deinit(&mqtt_handle);
        printf( "aiot_mqtt_disconnect failed: -0x%04X\n", -res);
        return -1;
    }

    /* 销毁MQTT实例, 一般不会运行到这里 */
    res = aiot_mqtt_deinit(&mqtt_handle);
    if (res < STATE_SUCCESS) {
        printf( "aiot_mqtt_deinit failed: -0x%04X\n", -res);
        return -1;
    }

    return 0;
}

