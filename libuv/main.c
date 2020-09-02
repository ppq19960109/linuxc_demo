#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <uv.h>

#include "fs.h"
#include "net.h"

void uv_idle_task(uv_idle_t *handle)
{
    // printf("uv_idle_task\n");
    // sleep(10);
}
void my_prep_cb(uv_prepare_t *handle)
{
    // printf("prep callback\n");
}
int main(int argc, char **argv)
{
    // uv_loop_t *loop = malloc(sizeof(uv_loop_t));
    // uv_loop_init(loop);

    uv_idle_t idler;
    uv_idle_init(uv_default_loop(), &idler);
    uv_idle_start(&idler, uv_idle_task);

    uv_prepare_t prep;
    uv_prepare_init(uv_default_loop(), &prep);
    uv_prepare_start(&prep, my_prep_cb);
    printf("libuv start\n");
    // fs_open();
    if (argc > 1 && atoi(argv[1]) == 1)
    {
        printf("net_client\n");
        net_client_open();
    }
    else
    {
        printf("net_sever\n");
        net_sever_open();
    }

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    // fs_close();
    // uv_loop_close(loop);
    // free(loop);

    return 0;
}