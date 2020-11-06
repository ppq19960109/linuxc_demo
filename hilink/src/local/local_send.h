#ifndef _LOCAL_SEND_H_
#define _LOCAL_SEND_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include "local_receive.h"

#define HY_HEART "{\"Command\":\"TcpBeatHeart\",\"Period\":\"60\"}"

#define STR_ADD "Add"
#define STR_DEVSINFO "DevsInfo"
#define STR_DEVATTRI "DevAttri"
#define STR_REFACTORY "ReFactory"

#define STR_NET_OPEN "120"
#define STR_NET_CLOSE "0"

    int write_haryan(const char *data, int dataLen);
    int write_hanyar_cmd(char *cmd, char *DeviceId, char *Value);
    int write_to_local(void *ptr);
    int write_delete_dev(const char *sn);
    int write_heart(void);
#ifdef __cplusplus
}
#endif
#endif