#ifndef __LINK_SOLO_H_
#define __LINK_SOLO_H_

#define PRODUCT_KEY "ProductKey"
#define PRODUCT_SECRET "ProductSecret"
#define DEVICE_NAME "DeviceName"
#define DEVICE_SECRET "DeviceSecret"

/* 位于external/ali_ca_cert.c中的服务器证书 */
extern const char *ali_ca_cert;

void register_connected_cb(void (*cb)(int));
void register_property_set_event_cb(int (*cb)(const int, const char *, const int));
void register_reboot_cb(void (*cb)());

int link_send_property_post(char *params);
int link_send_event_post(char *event_id, char *params);
int link_main(const char *productkey, const char *productsecret, const char *devicename, const char *devicesecret, const char *version);
void link_model_close();

int get_link_connected_state(void);
void *get_mqtt_handle(void);
#endif
