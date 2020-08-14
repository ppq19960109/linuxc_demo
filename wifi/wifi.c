#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <cJSON.h>
#include <DeviceIo/Rk_wifi.h>

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
    // RK_wifi_enable(0);
    sleep(1);
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

int getWiFiState()
{
    RK_WIFI_RUNNING_State_e WIFI_RUNNING_State;
    RK_wifi_running_getState(&WIFI_RUNNING_State);
    return WIFI_RUNNING_State;
}

int _RK_wifi_state_callback(RK_WIFI_RUNNING_State_e state)
{
    printf("_RK_wifi_state_callback state:%d\n", state);
    return 0;
}

int _RK_wifi_ble_state_callback(RK_WIFI_RUNNING_State_e state)
{
    printf("_RK_wifi_ble_state_callback state:%d\n", state);
    return 0;
}

int test()
{

    // 注册WIFI状态回调
    RK_wifi_register_callback(_RK_wifi_state_callback);
    // RK_wifi_ble_register_callback(_RK_wifi_ble_state_callback);
    // 获取MAC地址并打印
    char mac[32];
    memset(mac, 0, sizeof(mac));
    RK_wifi_get_mac(mac);
    printf("mac:%s\n", mac);
    //
    RK_WIFI_RUNNING_State_e WIFI_RUNNING_State;
    RK_wifi_running_getState(&WIFI_RUNNING_State);
    printf("RK_wifi_running_getState:%d\n", WIFI_RUNNING_State);
    //
    RK_WIFI_INFO_Connection_s WIFI_INFO_Connection;
    RK_wifi_running_getConnectionInfo(&WIFI_INFO_Connection);
    //
    char *json = RK_wifi_scan_r_sec(0x1f);
    printf("RK_wifi_scan_r_sec:%s\n", json);

    // 如果有配置过WIFI，enable wifi自动连接到配置的WIFI
    // 否则连接到指定WIFI
    // if (RK_wifi_has_config())
    // {
    //     RK_wifi_enable(1);
    // }
    // else
    // {
    RK_wifi_enable(1);
    RK_wifi_connect("HUAWEI-WDNJ4L", "1234567890");
    // }
    // RK_wifi_disconnect_network();
    // RK_wifi_disable_ap();

    // RK_wifi_enable_ap("Rkchip-123", "1234567890", "10.201.126.1");
    for (;;)
        ;
    // 断开WIFI并关闭WIFI模块
    RK_wifi_enable(0);
    return 0;
}

int main(int argc, char **argv)
{
    // RK_wifi_register_callback(_RK_wifi_state_callback);
    // int ret = connectWiFi("HUAWEI-WDNJ4L", "1234567890");
    // printf("connectWiFi:%d\n", ret);
    // RK_WIFI_RUNNING_State_e WIFI_RUNNING_State;
    // while (1)
    // {
    //     sleep(1);
    //     RK_wifi_running_getState(&WIFI_RUNNING_State);
    //     if (WIFI_RUNNING_State == RK_WIFI_State_CONNECTED)
    //         break;
    // }
    char ssid[64];
    unsigned int ssidLen = 0;
    getWiFiSsid(ssid, &ssidLen);
    char bssid[20];
    unsigned char bssidLen = 0;
    getWiFiBssid(bssid, &bssidLen);
    signed char rssi;
    getWiFiRssi(&rssi, ssid);
    // test();

    return 0;
}