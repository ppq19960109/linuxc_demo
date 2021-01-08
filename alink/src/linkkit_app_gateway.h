#ifndef _LINKKIT_APP_GATEWAY_H_
#define _LINKKIT_APP_GATEWAY_H_

#include "wrappers.h"

#define ALINKGATEWAYFILE "alinkscret.json"

#define EXAMPLE_TRACE(...)                                      \
    do                                                          \
    {                                                           \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__); \
        HAL_Printf(__VA_ARGS__);                                \
        HAL_Printf("\033[0m\r\n");                              \
    } while (0)

typedef struct
{
    int auto_add_subdev;
    int master_devid;
    int cloud_connected;
    int master_initialized;
    int subdev_index;
    int permit_join;
    void *g_user_dispatch_thread;
    int g_user_dispatch_thread_running;
    void *mutex;
} user_example_ctx_t;

user_example_ctx_t *user_example_get_ctx(void);

#endif