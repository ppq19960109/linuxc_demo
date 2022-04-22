#ifndef __LINK_FOTA_H_
#define __LINK_FOTA_H_

enum OTA_TYPE
{
    OTA_IDLE = 0x00,
    OTA_NO_FIRMWARE,
    OTA_NEW_FIRMWARE,

    OTA_DOWNLOAD_START,
    OTA_DOWNLOAD_FAIL,
    OTA_DOWNLOAD_SUCCESS,
    OTA_INSTALL_START,
    OTA_INSTALL_FAIL,
    OTA_INSTALL_SUCCESS,
};

void register_reboot_cb(void (*cb)());
int link_fota_start(void *mqtt_handle);
int link_fota_report_version(char *cur_version);
void link_fota_stop(void);
int link_download_firmware(void);
int link_query_firmware(void);

int get_ota_state(void);
void register_ota_progress_cb(void (*cb)(const int));
void register_ota_state_cb(int (*cb)(const int, void *));
void register_ota_query_timer_start_cb(int (*cb)(void));
void register_ota_query_timer_stop_cb(int (*cb)(void));
void ota_query_timer_cb(void);
void register_ota_complete_cb(void (*cb)(void));
#endif
