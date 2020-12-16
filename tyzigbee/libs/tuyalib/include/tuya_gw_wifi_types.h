#ifndef TUYA_GW_WIFI_TYPES_H
#define TUYA_GW_WIFI_TYPES_H

#include <stdint.h>

#define MAC_MAX_LEN     6
#define ADDR_MAX_LEN    16

#define BSSID_MAX_LEN   6
#define SSID_MAX_LEN    32
#define KEY_MAX_LEN     64

typedef void (*sniffer_callback)(uint8_t *buf, uint16_t len);

typedef enum {
	STATION = 0,
	AP      = 1,
} wifi_type_t;

typedef enum {
	WMODE_LOWPOWER    = 0, /* wifi work in lowpower mode */
	WMODE_SNIFFER,         /* wifi work in sniffer mode */
	WMODE_STATION,         /* wifi work in station mode */
	WMODE_SOFTAP,          /* wifi work in ap mode */
	WMODE_STATIONAP,       /* wifi work in station+ap mode */
} wifi_work_mode_t;

typedef enum {
	STAT_IDLE         = 0, /* disconnected */
	STAT_CONNECTING,       /* connecting */
	STAT_PASSWD_WRONG,     /* passwd do not match */
	STAT_NO_AP_FOUND,      /* ap not found */
	STAT_CONN_FAIL,        /* connect ap fail */
	STAT_CONN_SUCCESS,     /* connect ap success */
	STAT_GOT_IP,           /* connected and got ip success */
} station_conn_stat_t;

typedef enum {
	TYPE_OPEN         = 0, /* open */
	TYPE_WEP,              /* WEP */
	TYPE_WPA_PSK,          /* WPA—PSK  */
	TYPE_WPA2_PSK,         /* WPA2—PSK */
	TYPE_WPA_WPA2_PSK,     /* WPA/WPA2 */
} ap_encryption_type_t;

typedef struct {
	uint8_t channel;
	char rssi;
	uint8_t bssid[BSSID_MAX_LEN];
	uint8_t ssid[SSID_MAX_LEN+1];
	uint8_t s_len;
} ap_scan_info_s;

typedef struct {
	uint8_t ssid[SSID_MAX_LEN+1];
	uint8_t s_len;                  /* ssid length */
	uint8_t key[KEY_MAX_LEN+1];
	uint8_t p_len;                  /* security key length */
	uint8_t channel;
	ap_encryption_type_t type;
	uint8_t hidden;
	uint8_t max_conn;
	uint16_t ms_interval;           /* broadcast interval default:0 */
} ap_cfg_info_s;

typedef struct {
	char ip[ADDR_MAX_LEN];
	char mask[ADDR_MAX_LEN];
	char gateway[ADDR_MAX_LEN];
} ip_info_s;

typedef struct {
	uint8_t mac[MAC_MAX_LEN];
} mac_info_s;

#endif
