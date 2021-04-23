#ifndef _ONVIF_SERVER_H_
#define _ONVIF_SERVER_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "networkFunc.h"

#define ONVIF_PORT 6000
#define RTSP_PORT 8554

#define ETH "eth0" //"enp0s3"
#define XADDR_LEN 64
    const char *onvif_get_xaddrs(char *addr, int len);

#ifdef __cplusplus
}
#endif
#endif
