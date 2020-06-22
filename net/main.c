#include "log.h"
#include "server.h"
#include "client.h"

#include <math.h>

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        int arg = atoi(argv[1]);
        if (arg == 1)
        {
            log_debug("net_server\n");
            net_server_input();
        }
        else
        {
            log_debug("arg err\n");
        }
    }
    else
    {
        log_debug("net_client\n");
        net_client();
    }

    log_debug("main end\n");
    return 0;
}