#ifndef __RK_WIFI_H_
#define __RK_WIFI_H_

#define WIFI_CONFIG_FILE "config_wifi.json"
#define HOSTAPD_CONF_DIR "/userdata/hostapd.conf"
#define IPADDR "10.201.126"
#define AP_NAME "p2p0"

int getWiFiSsid(char *ssid, unsigned int *ssidLen);
void reconnectWiFi(void);
int connectWiFi(const char *ssid, const char *psk);
int getWiFiBssid(char *bssid, unsigned char *bssidLen);
int getWiFiRssi(signed char *rssi, char *ssid);
int getWiFiState(void);
int getWiFiIp(char *ip, unsigned char len);
int getWiFiMac(char *mac, unsigned char len);
int save_ssid_psk(const char *ssid, const char *pwd);
int read_ssid_psk(char *ssid, char *pwd);
int create_hostapd_file(const char *softap_name, const char *name, const char *password);
#endif