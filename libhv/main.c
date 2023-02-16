#include "hloop.h"
#include "hbase.h"
#include "hlog.h"
#include "nlog.h"
#include "UartCfg.h"

void mylogger(int loglevel, const char *buf, int len)
{
    if (loglevel >= LOG_LEVEL_ERROR)
    {
        stderr_logger(loglevel, buf, len);
    }

    if (loglevel >= LOG_LEVEL_INFO)
    {
        file_logger(loglevel, buf, len);
    }

    network_logger(loglevel, buf, len);
}

void on_idle(hidle_t *idle)
{
    printf("on_idle: event_id=%llu\tpriority=%d\tuserdata=%ld\n",
           LLU(hevent_id(idle)), hevent_priority(idle), (long)(intptr_t)(hevent_userdata(idle)));
}

void on_timer(htimer_t *timer)
{
    hloop_t *loop = hevent_loop(timer);
    printf("on_timer: event_id=%llu\tpriority=%d\tuserdata=%ld\ttime=%llus\thrtime=%lluus\n",
           LLU(hevent_id(timer)), hevent_priority(timer), (long)(intptr_t)(hevent_userdata(timer)),
           LLU(hloop_now(loop)), LLU(hloop_now_hrtime(loop)));
}

void on_stdin(hio_t *io, void *buf, int readbytes)
{
    printf("on_stdin fd=%d readbytes=%d\n", hio_fd(io), readbytes);
    printf("> %s\n", (char *)buf);
    if (strncmp((char *)buf, "quit", 4) == 0)
    {
        hloop_stop(hevent_loop(io));
    }
}

void on_file_read(hio_t *io, void *buf, int readbytes)
{
    char *data = buf;
    printf("on_file_read fd=%d readbytes=%d\n", hio_fd(io), readbytes);
    printf("on_file_read buf:0x%x\n", data[0]);
}
int main()
{
    hloop_t *loop = hloop_new(0);

    // hidle_t *idle = hidle_add(loop, on_idle, INFINITE);

    hlog_set_handler(mylogger);
    hlog_set_file("loop.log");
    hlog_set_format(DEFAULT_LOG_FORMAT);
#ifndef _MSC_VER
    logger_enable_color(hlog, 1);
#endif
    nlog_listen(loop, DEFAULT_LOG_PORT);

    int fd = uart_init("/dev/ttyS0", BAUDRATE_9600, DATABIT_8, PARITY_NONE, STOPBIT_1, FLOWCTRL_NONE, BLOCKING_BLOCK);
    if (fd < 0)
    {
        printf("ecb_uart uart init error:%d,%s\n", errno, strerror(errno));
        return;
    }
    char buf[64];
    hread(loop, fd, buf, sizeof(buf), on_file_read);

    hloop_run(loop);
    hloop_free(&loop);
    close(fd);
    return 0;
}
