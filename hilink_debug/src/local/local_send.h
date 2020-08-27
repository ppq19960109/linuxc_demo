#ifndef _LOCAL_SEND_H_
#define _LOCAL_SEND_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include "local_receive.h"

#define ADD "Add"
#define DEVSINFO "DevsInfo"
#define DEVATTRI "DevAttri"
#define REFACTORY "ReFactory"

    int write_haryan(const char *data, int socketfd, char *sendBuf, int bufLen);
    int write_hanyar_cmd(char *cmd, char *DeviceId, char *Value);
    int write_to_local(void *ptr, LocalControl_t *localControl);

    void local_restart_reFactory(int index);
#ifdef __cplusplus
}
#endif
#endif