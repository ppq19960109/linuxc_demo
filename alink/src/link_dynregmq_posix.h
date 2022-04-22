#ifndef __LINK_DYREGMQ_H_
#define __LINK_DYREGMQ_H_

void register_dynreg_device_secret_cb(int (*cb)(const char *));
int link_dynregmq_start(const char *mqtt_host, const char *product_key, const char *product_secret, const char *device_name, char *device_secret);
#endif
