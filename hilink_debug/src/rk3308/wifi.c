#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <DeviceIo/Rk_wifi.h>
#include <cJSON.h>
#include "wifi.h"

int _RK_wifi_state_callback(RK_WIFI_RUNNING_State_e state)
{
    printf("_RK_wifi_state_callback state:%d\n", state);
    return 0;
}

int getWiFiSsid(char *ssid, unsigned int *ssidLen)
{
    RK_WIFI_INFO_Connection_s WIFI_INFO_Connection;
    int ret = RK_wifi_running_getConnectionInfo(&WIFI_INFO_Connection);
    printf("ret:%d,ssid:%s\n", ret, WIFI_INFO_Connection.ssid);
    strcpy(ssid, WIFI_INFO_Connection.ssid);
    *ssidLen = strlen(WIFI_INFO_Connection.ssid);
    return ret;
}

void reconnectWiFi(void)
{
    RK_wifi_restart_network();
}

int connectWiFi(const char *ssid, const char *psk)
{
    RK_WIFI_RUNNING_State_e WIFI_RUNNING_State;
    RK_wifi_running_getState(&WIFI_RUNNING_State);
    if (WIFI_RUNNING_State == 3)
    {
        RK_WIFI_INFO_Connection_s WIFI_INFO_Connection;
        RK_wifi_running_getConnectionInfo(&WIFI_INFO_Connection);

        if (strcmp(WIFI_INFO_Connection.ssid, ssid) == 0)
        {
            printf("connectWiFi connected\n");
            return 0;
        }
    }
    // RK_wifi_enable(0);
    // sleep(1);
    RK_wifi_enable(1);
    return RK_wifi_connect(ssid, psk);
}

int getWiFiBssid(char *bssid, unsigned char *bssidLen)
{
    RK_WIFI_INFO_Connection_s WIFI_INFO_Connection;
    int ret = RK_wifi_running_getConnectionInfo(&WIFI_INFO_Connection);

    strcpy(bssid, WIFI_INFO_Connection.bssid);
    *bssidLen = strlen(WIFI_INFO_Connection.bssid);
    printf("bssidLen:%d,bssid:%s\n", *bssidLen, bssid);
    return ret;
}

int getWiFiRssi(signed char *rssi, char *ssid)
{
    char *json = RK_wifi_scan_r_sec(0x14);
    printf("RK_wifi_scan_r_sec:%s\n", json);
    cJSON *root = cJSON_Parse(json);
    int array_size = cJSON_GetArraySize(root);
    cJSON *array_sub;
    for (int cnt = 0; cnt < array_size; cnt++)
    {
        array_sub = cJSON_GetArrayItem(root, cnt);
        cJSON *scan_ssid = cJSON_GetObjectItem(array_sub, "ssid");
        if (strcmp(scan_ssid->valuestring, ssid) == 0)
        {
            *rssi = cJSON_GetObjectItem(array_sub, "rssi")->valueint;
            printf("rssi:%d\n", *rssi);
            return 0;
        }
    }
    return -1;
}

int getWiFiState(void)
{
    RK_WIFI_RUNNING_State_e WIFI_RUNNING_State;
    RK_wifi_running_getState(&WIFI_RUNNING_State);
    return WIFI_RUNNING_State;
}

int getWiFiIp(char *ip, unsigned char len)
{
    RK_WIFI_INFO_Connection_s WIFI_INFO_Connection;
    int ret = RK_wifi_running_getConnectionInfo(&WIFI_INFO_Connection);

    strncpy(ip, WIFI_INFO_Connection.ip_address, len);
    // printf("getWiFiIp:%s\n", ip);
    return ret;
}

void delete_char(char str[], char target)
{
    int i, j;
    for (i = j = 0; str[i] != '\0'; i++)
    {
        if (str[i] != target)
        {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

int getWiFiMac(char *mac, unsigned char len)
{
    char wifi_mac[20];
    int ret = RK_wifi_get_mac(wifi_mac);
    delete_char(wifi_mac, ':');
    strncpy(mac, wifi_mac, len - 1);
    mac[len - 1] = '\0';
    printf("getWiFiMac:%s %s %d\n", wifi_mac, mac, ret);
    return ret;
}

int file_size(char *filename)
{
    struct stat statbuf;
    stat(filename, &statbuf);
    int size = statbuf.st_size;
    return size;
}

int save_ssid_psk(const char *ssid, const char *pwd)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "ssid", ssid);
    cJSON_AddStringToObject(root, "psk", pwd);
    char *json = cJSON_Print(root);

    FILE *pFile = fopen(WIFI_CONFIG_FILE, "w");
    fwrite(json, strlen(json) + 1, 1, pFile);
    fclose(pFile);
    printf("json len:%d\n", strlen(json) + 1);
    free(json);
    free(root);
    return 0;
}

int read_ssid_psk(char *ssid, char *pwd)
{
    FILE *pFile = fopen(WIFI_CONFIG_FILE, "r");
    if (pFile == NULL)
    {
        perror("fopen read fail:");
        return -1;
    }
    int size = file_size(WIFI_CONFIG_FILE);
    printf("file_size:%d\n", size);
    if (size == 0)
        return -1;
    char *json = (char *)malloc(size);

    fread(json, size, 1, pFile);
    fclose(pFile);

    printf("read_ssid_psk json:%s\n", json);
    cJSON *root = cJSON_Parse(json);
    cJSON *rssid = cJSON_GetObjectItem(root, "ssid");
    cJSON *rpsk = cJSON_GetObjectItem(root, "psk");
    strcpy(ssid, rssid->valuestring);
    strcpy(pwd, rpsk->valuestring);
    free(root);
    free(json);
    return 0;
}

int create_hostapd_file(const char *softap_name, const char *name, const char *password)
{
    FILE *fp;
    char cmdline[256] = {0};

    fp = fopen(HOSTAPD_CONF_DIR, "w+");

    if (fp != 0)
    {
        sprintf(cmdline, "interface=%s\n", softap_name);
        fputs(cmdline, fp);
        fputs("ctrl_interface=/var/run/hostapd\n", fp);
        fputs("driver=nl80211\n", fp);
        fputs("ssid=", fp);
        fputs(name, fp);
        fputs("\n", fp);
        fputs("channel=6\n", fp);
        fputs("hw_mode=g\n", fp);
        fputs("ieee80211n=1\n", fp);
        fputs("ignore_broadcast_ssid=0\n", fp);

        fputs("max_num_sta=2\n", fp);
        fputs("wpa=2\n", fp);
        fputs("wpa_passphrase=", fp);
        fputs(password, fp);
        fputs("\n", fp);
        fputs("wpa_key_mgmt=WPA-PSK\n", fp);
        fputs("wpa_pairwise=CCMP\n", fp);

        fclose(fp);
        return 0;
    }
    return -1;
}