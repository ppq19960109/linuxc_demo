#ifndef _UCI_WIFI_H_
#define _UCI_WIFI_H_

#ifdef __cplusplus
extern "C"
{
#endif
    int uci_wifi_set(const char *ssid, const char *key, const char *bssid);
    int wifi_config(char *data, unsigned int len);
#ifdef __cplusplus
}
#endif
#endif