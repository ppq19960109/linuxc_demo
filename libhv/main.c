#include "hloop.h"
#include "hbase.h"
#include "hlog.h"
#include "nlog.h"
#include "hsocket.h"
#include "hssl.h"
#include "hmain.h"
#include "UartCfg.h"
int tcp_client_reconnect_create(hloop_t *loop);
static void mlogger(int loglevel, const char *buf, int len)
{
    if (loglevel >= LOG_LEVEL_ERROR)
    {
        stderr_logger(loglevel, buf, len);
        if (loglevel >= LOG_LEVEL_INFO)
        {
            file_logger(loglevel, buf, len);
        }
    }
    else
        stdout_logger(loglevel, buf, len);
    // network_logger(loglevel, buf, len);
}
static void on_reload(void *userdata)
{
    hlogi("reload confile [%s]", g_main_ctx.confile);
}
void on_idle(hidle_t *idle)
{
    printf("on_idle: event_id=%llu\tpriority=%d\tuserdata=%ld now_ms:%ld\n",
           LLU(hevent_id(idle)), hevent_priority(idle), (long)(intptr_t)(hevent_userdata(idle)), hloop_now_ms(hevent_loop(idle)));
}

void on_uart_read(hio_t *io, void *buf, int readbytes)
{
    char *data = buf;
    hlogi("on_uart_read fd=%d readbytes=%d", hio_fd(io), readbytes);
    hlogi("on_uart_read buf:0x%x", data[0]);
}
static void reconnect_timer_cb(htimer_t* timer) {
    tcp_client_reconnect_create(hevent_loop(timer));
}
static void on_close(hio_t *io)
{
    printf("on_close fd=%d error=%d\n", hio_fd(io), hio_error(io));
    hloop_t* loop=hevent_loop(io);
    htimer_add(loop, reconnect_timer_cb, 2000, 1);
}

static void on_recv(hio_t *io, void *buf, int readbytes)
{
    printf("on_recv fd=%d readbytes=%d\n", hio_fd(io), readbytes);
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printf("[%s] <=> [%s]\n",
           SOCKADDR_STR(hio_localaddr(io), localaddrstr),
           SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));
    printf("< %.*s", readbytes, (char *)buf);
    // echo
    // printf("> %.*s", readbytes, (char *)buf);
    // hio_write(io, buf, readbytes);
}

static void on_accept(hio_t *io)
{
    printf("on_accept connfd=%d\n", hio_fd(io));
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printf("accept connfd=%d [%s] <= [%s]\n", hio_fd(io),
           SOCKADDR_STR(hio_localaddr(io), localaddrstr),
           SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));

    hio_setcb_close(io, on_close);
    hio_setcb_read(io, on_recv);
    hio_read_start(io);
}
static void on_connect(hio_t *io)
{
    printf("on_connect fd=%d\n", hio_fd(io));

    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printf("connect connfd=%d [%s] => [%s]\n", hio_fd(io),
           SOCKADDR_STR(hio_localaddr(io), localaddrstr),
           SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));

    hio_read_start(io);
    // uncomment to test heartbeat
    // hio_set_heartbeat(sockio, 3000, send_heartbeat);
}
int tcp_client_reconnect_create(hloop_t *loop)
{
    static char recvbuf[512];
    hio_t *sockio = hloop_create_tcp_client(loop, "/tmp/unix_server.domain", -1, on_connect, on_close);
    if (sockio == NULL)
    {
        return -20;
    }
    hio_set_connect_timeout(sockio, 20000);
    hio_setcb_read(sockio, on_recv);
    hio_set_readbuf(sockio, recvbuf, sizeof(recvbuf));
    return 0;
}
int main()
{
    hloop_t *loop = hloop_new(0);

    signal_init(on_reload, NULL);
    // hidle_t *idle = hidle_add(loop, on_idle, INFINITE);

    hlog_set_handler(mlogger);
    hlog_set_file("X8GCZ01.log");
    hlog_set_format(DEFAULT_LOG_FORMAT);
    hlog_set_level(LOG_LEVEL_DEBUG);
    logger_enable_color(hlog, 1);
    nlog_listen(loop, DEFAULT_LOG_PORT);

    int fd = uart_init("/dev/ttyS0", BAUDRATE_9600, DATABIT_8, PARITY_NONE, STOPBIT_1, FLOWCTRL_NONE, BLOCKING_BLOCK);
    if (fd < 0)
    {
        hloge("ecb_uart uart init error:%d,%s", errno, strerror(errno));
        return;
    }
    static char uart_buf[512];
    hio_t *uart_io = hread(loop, fd, uart_buf, sizeof(uart_buf), on_uart_read);

    // unlink("/tmp/unix_server.domain");
    // hio_t *listenio = hloop_create_tcp_server(loop, "/tmp/unix_server.domain", -1, on_accept);
    // if (listenio == NULL)
    // {
    //     return -20;
    // }
    tcp_client_reconnect_create(loop);

    hloop_run(loop);
    hloop_free(&loop);
    close(fd);
    return 0;
}
