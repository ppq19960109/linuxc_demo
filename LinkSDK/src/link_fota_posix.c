
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_ota_api.h"

#define OTA_FILE "/tmp/upgrade.bin"
static FILE *ota_fp = NULL;
static void *ota_handle = NULL;
static pthread_t g_download_thread; /* 用于HTTP的固件下载线程 */

void (*reboot_cb)();
void register_reboot_cb(void (*cb)())
{
    reboot_cb = cb;
}

/* 下载收包回调, 用户调用 aiot_download_recv() 后, SDK收到数据会进入这个函数, 把下载到的数据交给用户 */
/* TODO: 一般来说, 设备升级时, 会在这个回调中, 把下载到的数据写到Flash上 */
void demo_download_recv_handler(void *handle, const aiot_download_recv_t *packet, void *userdata)
{
    uint32_t data_buffer_len = 0;
    uint32_t last_percent = 0;
    int32_t percent = 0;

    /* 目前只支持 packet->type 为 AIOT_DLRECV_HTTPBODY 的情况 */
    if (!packet || AIOT_DLRECV_HTTPBODY != packet->type)
    {
        return;
    }
    percent = packet->data.percent;

    /* userdata可以存放 demo_download_recv_handler() 的不同次进入之间, 需要共享的数据 */
    /* 这里用来存放上一次进入本回调函数时, 下载的固件进度百分比 */
    if (userdata)
    {
        last_percent = *((uint32_t *)(userdata));
    }

    data_buffer_len = packet->data.len;

    /* 如果 percent 为负数, 说明发生了收包异常或者digest校验错误 */
    if (percent < 0)
    {
        printf("exception: percent = %d\r\n", percent);
        if (userdata)
        {
            free(userdata);
        }
        return;
    }

    /*
     * TODO: 下载一段固件成功, 这个时候, 用户应该将
     *       起始地址为 packet->data.buffer, 长度为 packet->data.len 的内存, 保存到flash上
     *
     *       如果烧写flash失败, 还应该调用 aiot_download_report_progress(handle, -4) 将失败上报给云平台
     *       备注:协议中, 与云平台商定的错误码在 aiot_ota_protocol_errcode_t 类型中, 例如
     *           -1: 表示升级失败
     *           -2: 表示下载失败
     *           -3: 表示校验失败
     *           -4: 表示烧写失败
     *
     *       详情可见 https://help.aliyun.com/document_detail/85700.html
     */
    fwrite(packet->data.buffer, packet->data.len, 1, ota_fp);

    /* 简化输出, 只有距离上次的下载进度增加5%以上时, 才会打印进度, 并向服务器上报进度 */
    if (percent - last_percent >= 5 || percent == 100)
    {
        printf("download %03d%% done, +%d bytes\r\n", percent, data_buffer_len);
        aiot_download_report_progress(handle, percent);

        if (userdata)
        {
            *((uint32_t *)(userdata)) = percent;
        }
        if (percent == 100 && userdata)
        {
            free(userdata);
        }
    }

    /* percent 入参的值为 100 时, 说明SDK已经下载固件内容全部完成 */
    if (percent == 100)
    {
        /*
         * TODO: 这个时候, 一般用户就应该完成所有的固件烧录, 保存当前工作, 重启设备, 切换到新的固件上启动了
                 并且, 新的固件必须要以

                 aiot_ota_report_version(ota_handle, new_version);

                 这样的操作, 将升级后的新版本号(比如1.0.0升到1.1.0, 则new_version的值是"1.1.0")上报给云平台
                 云平台收到了新的版本号上报后, 才会判定升级成功, 否则会认为本次升级失败了
                 如果下载成功后升级失败, 还应该调用 aiot_download_report_progress(handle, -1) 将失败上报给云平台
         */
        printf("ota end,start install...\n");
        if (ota_fp != NULL)
        {
            fclose(ota_fp);
            ota_fp = NULL;
        }
        system("chmod 777 " OTA_FILE);
        system("cd /tmp &&" OTA_FILE);
        sync();
        if (reboot_cb != NULL)
            reboot_cb();
    }
}
/* 执行aiot_download_recv的线程, 实现固件内容的请求和接收 */
void *demo_ota_download_thread(void *dl_handle)
{
    int32_t ret = 0;

    printf("starting download thread in 2 seconds ......\r\n");
    sleep(2);

    /* 向固件服务器请求下载 */
    /*
     * TODO: 下面这样的写法, 就是以1个请求, 获取全部的固件内容
     *       设备资源比较少, 或者网络较差时, 也可以分段下载, 需要组合
     *
     *       aiot_download_setopt(dl_handle, AIOT_DLOPT_RANGE_START, ...);
     *       aiot_download_setopt(dl_handle, AIOT_DLOPT_RANGE_END, ...);
     *       aiot_download_send_request(dl_handle);
     *
     *       实现, 这种情况下, 需要把以上组合语句放置到循环中, 多次 send_request 和 recv
     *
     */
    ota_fp = fopen(OTA_FILE, "w+");
    aiot_download_send_request(dl_handle);
    while (1)
    {
        /* 从网络收取服务器回应的固件内容 */
        ret = aiot_download_recv(dl_handle);

        /* 固件全部下载完时, aiot_download_recv() 的返回值会等于 STATE_DOWNLOAD_FINISHED, 否则是当次获取的字节数 */
        if (STATE_DOWNLOAD_FINISHED == ret)
        {
            printf("download completed\r\n");
            break;
        }
        if (STATE_DOWNLOAD_RENEWAL_REQUEST_SENT == ret)
        {
            printf("download renewal request has been sent successfully\r\n");
            continue;
        }
        if (ret <= STATE_SUCCESS)
        {
            printf("download failed, error code is %d, try to send renewal request\r\n", ret);
            continue;
        }
    }
    /* 下载所有固件内容完成, 销毁下载会话, 线程自行退出 */
    aiot_download_deinit(&dl_handle);
    printf("download thread exit\r\n");
    if (ota_fp != NULL)
    {
        fclose(ota_fp);
        ota_fp = NULL;
    }
    return NULL;
}

/* 用户通过 aiot_ota_setopt() 注册的OTA消息处理回调, 如果SDK收到了OTA相关的MQTT消息, 会自动识别, 调用这个回调函数 */
void demo_ota_recv_handler(void *ota_handle, aiot_ota_recv_t *ota_msg, void *userdata)
{
    switch (ota_msg->type)
    {
    case AIOT_OTARECV_FOTA:
    {
        uint32_t res = 0;
        uint16_t port = 443;
        uint32_t max_buffer_len = (8 * 1024);
        aiot_sysdep_network_cred_t cred;
        void *dl_handle = NULL;
        void *last_percent = NULL;

        if (NULL == ota_msg->task_desc || ota_msg->task_desc->protocol_type != AIOT_OTA_PROTOCOL_HTTPS)
        {
            break;
        }

        dl_handle = aiot_download_init();
        if (NULL == dl_handle)
        {
            break;
        }
        printf("OTA target firmware version: %s, size: %u Bytes \r\n", ota_msg->task_desc->version,
               ota_msg->task_desc->size_total);

        if (NULL != ota_msg->task_desc->extra_data)
        {
            printf("extra data: %s\r\n", ota_msg->task_desc->extra_data);
        }

        memset(&cred, 0, sizeof(aiot_sysdep_network_cred_t));
        cred.option = AIOT_SYSDEP_NETWORK_CRED_NONE;
        cred.max_tls_fragment = 16384;

        /* 设置下载时为TLS下载 */
        aiot_download_setopt(dl_handle, AIOT_DLOPT_NETWORK_CRED, (void *)(&cred));
        /* 设置下载时访问的服务器端口号 */
        aiot_download_setopt(dl_handle, AIOT_DLOPT_NETWORK_PORT, (void *)(&port));
        /* 设置下载的任务信息, 通过输入参数 ota_msg 中的 task_desc 成员得到, 内含下载地址, 固件大小, 固件签名等 */
        aiot_download_setopt(dl_handle, AIOT_DLOPT_TASK_DESC, (void *)(ota_msg->task_desc));
        /* 设置下载内容到达时, SDK将调用的回调函数 */
        aiot_download_setopt(dl_handle, AIOT_DLOPT_RECV_HANDLER, (void *)(demo_download_recv_handler));
        /* 设置单次下载最大的buffer长度, 每当这个长度的内存读满了后会通知用户 */
        aiot_download_setopt(dl_handle, AIOT_DLOPT_BODY_BUFFER_MAX_LEN, (void *)(&max_buffer_len));
        /* 设置 AIOT_DLOPT_RECV_HANDLER 的不同次调用之间共享的数据, 比如例程把进度存在这里 */
        last_percent = malloc(sizeof(uint32_t));
        if (NULL == last_percent)
        {
            aiot_download_deinit(&dl_handle);
            break;
        }
        memset(last_percent, 0, sizeof(uint32_t));
        aiot_download_setopt(dl_handle, AIOT_DLOPT_USERDATA, (void *)last_percent);

        /* 启动专用的下载线程, 去完成固件内容的下载 */
        res = pthread_create(&g_download_thread, NULL, demo_ota_download_thread, dl_handle);
        if (res != 0)
        {
            printf("pthread_create demo_ota_download_thread failed: %d\r\n", res);
            aiot_download_deinit(&dl_handle);
            free(last_percent);
        }
        else
        {
            /* 下载线程被设置为 detach 类型, 固件内容获取完毕后可自主退出 */
            pthread_detach(g_download_thread);
        }
        break;
    }

    default:
        break;
    }
}

int link_fota_start(void *mqtt_handle)
{
    /* 与MQTT例程不同的是, 这里需要增加创建OTA会话实例的语句 */
    ota_handle = aiot_ota_init();
    if (NULL == ota_handle)
    {
        printf("aiot_ota_init failed\r\n");
        return -2;
    }

    /* 用以下语句, 把OTA会话和MQTT会话关联起来 */
    aiot_ota_setopt(ota_handle, AIOT_OTAOPT_MQTT_HANDLE, mqtt_handle);
    /* 用以下语句, 设置OTA会话的数据接收回调, SDK收到OTA相关推送时, 会进入这个回调函数 */
    aiot_ota_setopt(ota_handle, AIOT_OTAOPT_RECV_HANDLER, demo_ota_recv_handler);
    return 0;
}

int link_fota_report_version(char *cur_version)
{
    int32_t res = STATE_SUCCESS;
    /*   TODO: 非常重要!!!
     *
     *   cur_version 要根据用户实际情况, 改成从设备的配置区获取, 要反映真实的版本号, 而不能像示例这样写为固定值
     *
     *   1. 如果设备从未上报过版本号, 在控制台网页将无法部署升级任务
     *   2. 如果设备升级完成后, 上报的不是新的版本号, 在控制台网页将会显示升级失败
     *
     */

    /* 演示MQTT连接建立起来之后, 就可以上报当前设备的版本号了 */
    cur_version = "1.0.0";
    res = aiot_ota_report_version(ota_handle, cur_version);
    if (res < STATE_SUCCESS)
    {
        printf("aiot_ota_report_version failed: -0x%04X\r\n", -res);
    }
    return res;
}

void link_fota_stop(void)
{
    /* 销毁OTA实例, 一般不会运行到这里 */
    aiot_ota_deinit(&ota_handle);
}
