#ifndef _LOCAL_SEND_H_
#define _LOCAL_SEND_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include "local_receive.h"

#define STR_ADD "Add"
#define STR_DEVSINFO "DevsInfo"
#define STR_DEVATTRI "DevAttri"
#define STR_REFACTORY "ReFactory"

#define STR_NET_OPEN "120"
#define STR_NET_CLOSE "0"

#define INT_REFACTORY 1
#define INT_RESTART 0

    int write_haryan(const char *data, int socketfd, char *sendBuf, int bufLen);
    int write_hanyar_cmd(char *cmd, char *DeviceId, char *Value);
    int write_to_local(void *ptr, LocalControl_t *localControl);

    void local_restart_reFactory(int index);
#ifdef __cplusplus
}
#endif
#endif