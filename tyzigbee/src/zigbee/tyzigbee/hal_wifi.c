#include "tuya_gw_wifi_types.h"

int hal_wifi_scan_all_ap(ap_scan_info_s **aps, uint32_t *num)
{
    return 0;
}

int hal_wifi_scan_assigned_ap(char *ssid, ap_scan_info_s **ap)
{
    return 0;
}

int hal_wifi_release_ap(ap_scan_info_s *ap)
{
    return 0;
}

int hal_wifi_set_cur_channel(uint8_t channel)
{
    return 0;
}

int hal_wifi_get_cur_channel(uint8_t *channel)
{
    return 0;
}

int hal_wifi_set_sniffer(int enable, sniffer_callback cb)
{
    return 0;
}

int hal_wifi_get_ip(wifi_type_t type, ip_info_s *ip)
{
    return 0;
}

int hal_wifi_get_mac(wifi_type_t type, mac_info_s *mac)
{
    return 0;
}

int hal_wifi_set_mac(wifi_type_t type, mac_info_s *mac)
{
    return 0;
}

int hal_wifi_set_work_mode(wifi_work_mode_t mode)
{
    return 0;
}

int hal_wifi_get_work_mode(wifi_work_mode_t *mode)
{
    return 0;
}

int hal_wifi_connect_station(char *ssid, char *passwd)
{
    return 0;
}

int hal_wifi_disconnect_station(void)
{
    return 0;
}

int hal_wifi_get_station_rssi(char *rssi)
{
    return 0;
}

int hal_wifi_get_station_mac(mac_info_s *mac)
{
    return 0;
}

int hal_wifi_get_station_conn_stat(station_conn_stat_t *stat)
{
    return 0;
}

int hal_wifi_ap_start(ap_cfg_info_s *cfg)
{
    return 0;
}

int hal_wifi_ap_stop(void)
{
    return 0;
}

int hal_wifi_set_country_code(int code)
{
    return 0;
}