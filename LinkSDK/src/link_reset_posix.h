#ifndef __LINK_RESET_H_
#define __LINK_RESET_H_

int link_reset_init(void *mqtt_handle, const char *productkey, const char *devicename);
int link_reset_report(void *mqtt_handle);
void link_reset_deinit(void *mqtt_handle);
#endif
