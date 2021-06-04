#ifndef _WIFI_SERVER_H_
#define _WIFI_SERVER_H_

#ifdef __cplusplus
extern "C"
{
#endif
    int wifi_serverOpen(void);
    int wifi_serverClose(void);
    int wifi_server_send(void *data, unsigned int len);

    int thread_wifi_config(void);
    int wifi_client_open(void);
#ifdef __cplusplus
}
#endif
#endif