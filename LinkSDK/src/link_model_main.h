#ifndef __LINK_MODEL_MAIN_H_
#define __LINK_MODEL_MAIN_H_

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

void register_connected_cb(void (*cb)(int));
void register_property_set_event_cb(int (*cb)(const int, const char *, const int));
void register_reboot_cb(void (*cb)());

int link_send_property_post(char *params);
int link_send_event_post(char *event_id, char *params);
int link_model_main(const char *productkey, const char *productsecret, const char *devicename, const char *devicesecret, const char *version);

#endif
