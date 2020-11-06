#if USE_LIBUV
#include "uv_main.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <uv.h>

#include "local_send.h"
#include "local_callback.h"
#include "local_device.h"

struct uv_app_t
{
    uv_tcp_t *tcp_client;
    uv_connect_t *g_connect;
    uv_timer_t timer_client;
    uv_write_t writereq;
    uv_buf_t w_buf;

    uv_signal_t g_signal;
    uv_timer_t timer_req;
    volatile int s_isConnect;
#define RECVLEN 16384
    char tcpBuf[RECVLEN + 1];
};

static struct uv_app_t uv_app;

static int timer_open();
static void timer_close();
static void client_timer();

static void client_close(uv_handle_t *handle)
{
    printf("client_close\n");
    uv_app.s_isConnect = 0;
    timer_close();
    uv_tcp_init(uv_default_loop(), uv_app.tcp_client);
    client_timer();
}

static void client_write_cb(uv_write_t *req, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "client write error %s\n", uv_strerror(status));
        uv_close((uv_handle_t *)req->handle, client_close);
        return;
    }
    printf("client write succeed!\n");
}

static int client_write(char *data, unsigned int len)
{
    if (!uv_app.s_isConnect)
    {
        fprintf(stderr, "client not connect\n");
        return -1;
    }
    uv_app.w_buf = uv_buf_init(data, len);

    printf("uv_app.w_buf:%p,%d\n", data, len);
    uv_write(&uv_app.writereq, (uv_stream_t *)uv_app.tcp_client, &uv_app.w_buf, 1, client_write_cb);
    return uv_app.writereq.error;
}

static void client_alloc_buf(uv_handle_t *handle,
                             size_t suggested_size,
                             uv_buf_t *buf)
{
    // *buf = uv_buf_init((char *)malloc(suggested_size), suggested_size);
    *buf = uv_buf_init(uv_app.tcpBuf, RECVLEN);
}

static void client_read(uv_stream_t *stream,
                        ssize_t nread,
                        const uv_buf_t *buf)
{
    if (nread < 0)
    {
        // UV_EINTR
        // UV_ECONNRESET

        fprintf(stderr, "client read error: %s\n", uv_strerror(nread));
        uv_read_stop(stream);
        uv_close((uv_handle_t *)stream, client_close);

        // if (buf->base)
        //     free(buf->base);
        return;
    }

    buf->base[nread] = 0;
    char *tcpBuf = buf->base;
    // printf("recv %d\n", nread);
    // printf("%s\n", buf->base);
    recv_toLocal(buf->base, nread);
    // if (buf->base)
    //     free(buf->base);
}

static void on_connect(uv_connect_t *req, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "connect error %s\n", uv_strerror(status));
        // error!
        return;
    }
    uv_timer_stop(&uv_app.timer_client);
    uv_app.s_isConnect = 1;
    write_hanyar_cmd(STR_ADD, NULL, STR_NET_CLOSE);
    write_hanyar_cmd(STR_DEVSINFO, NULL, NULL);

    struct sockaddr addrpeer;
    struct sockaddr_in addrinpeer;
    int addrlenpeer = sizeof(addrpeer);
    char socknamepeer[18] = {0};

    if (0 == uv_tcp_getpeername((uv_tcp_t *)req->handle, &addrpeer, &addrlenpeer))
    {
        addrinpeer = *((struct sockaddr_in *)&addrpeer);
        uv_ip4_name(&addrinpeer, socknamepeer, addrlenpeer);
        printf("connect to:%s:%d\n", socknamepeer, ntohs(addrinpeer.sin_port));
    }
    else
        printf("get socket fail!\n");
    //-----------------------------------------------
    uv_read_start(req->handle, client_alloc_buf, client_read);
    timer_open();
}

static void client_timer_callback(uv_timer_t *timer)
{
    // printf("client_timer_callback\n");

    struct sockaddr_in dest;
    uv_ip4_addr("127.0.0.1", SERVER_PORT, &dest);

    uv_tcp_connect(uv_app.g_connect, uv_app.tcp_client, (struct sockaddr *)&dest, on_connect);

    // uv_timer_stop(timer);
}
//------------------------------------------

static void client_timer()
{
    uv_timer_again(&uv_app.timer_client);
}
static int net_client_open()
{

    uv_app.tcp_client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));

    uv_app.g_connect = (uv_connect_t *)malloc(sizeof(uv_connect_t));

    // struct sockaddr_in dest;
    // uv_ip4_addr("127.0.0.1", SERVER_PORT, &dest);
    // if (uv_tcp_connect(uv_app.g_connect, uv_app.tcp_client, (struct sockaddr *)&dest, on_connect) == 0)
    // {
    //     printf("uv_tcp_connect\n");
    // }
    uv_tcp_init(uv_default_loop(), uv_app.tcp_client);

    uv_timer_init(uv_default_loop(), &uv_app.timer_client);
    uv_timer_start(&uv_app.timer_client, client_timer_callback, 100, 2000);
    return 0;
}

static void net_client_close()
{
    uv_read_stop(uv_app.g_connect->handle);

    uv_close((uv_handle_t *)uv_app.tcp_client, NULL);
    uv_app.s_isConnect = 0;
    free(uv_app.tcp_client);
    free(uv_app.g_connect);
}
//------------------------------

static void timer_callback(uv_timer_t *timer)
{
    printf("HY_HEART timer_callback\n");
    write_heart();
}

static int timer_open()
{
    uv_timer_init(uv_default_loop(), &uv_app.timer_req);
    uv_timer_start(&uv_app.timer_req, timer_callback, 5000, 60000);
    return 0;
}
static void timer_close()
{
    uv_timer_stop(&uv_app.timer_req);
}
//---------------------------------------------

static void signal_handler(uv_signal_t *handle, int signum)
{
    printf("signal received: %d\n", signum);
    if (signum == SIGINT || signum == SIGQUIT || signum == SIGKILL)
    {
        local_system_restartOrReFactory(INT_OFFLINE);
    }
}

static void signal_open()
{
    uv_signal_init(uv_default_loop(), &uv_app.g_signal);
    uv_signal_start_oneshot(&uv_app.g_signal, signal_handler, SIGINT);
    // uv_signal_start(&uv_app.g_signal, signal_handler, SIGQUIT);
    // uv_signal_start(&uv_app.g_signal, signal_handler, SIGKILL);
}
static void signal_close()
{
    uv_signal_stop(&uv_app.g_signal);
}
//--------------------------------------------------
void uv_main_close()
{
    timer_close();
    net_client_close();
    signal_close();
}

void uv_idle_task(uv_idle_t *handle)
{
    // printf("uv_idle_task\n");
}
void uv_main_open()
{
    register_closeCallback(uv_main_close);
    register_writeCallback(client_write);

    signal_open();
    net_client_open();

    // uv_idle_t idler;
    // uv_idle_init(uv_default_loop(), &idler);
    // uv_idle_start(&idler, uv_idle_task);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    // uv_idle_stop(&idler);
    uv_main_close();
  
}
#endif