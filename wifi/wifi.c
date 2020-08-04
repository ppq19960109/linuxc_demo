#include <stdio.h>
#include <string.h>
#include <Rk_wifi.h>

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

int main(int argc, char **argv)
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
    for (;;)
        ;
    // 断开WIFI并关闭WIFI模块
    RK_wifi_enable(0);
    return 0;
}