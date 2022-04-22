#ifndef __LINK_BIND_H_
#define __LINK_BIND_H_

int link_bind_token_init(void *mqtt_handle, const char *productkey, const char *devicename);
int link_bind_token_report(void *mqtt_handle);
void link_bind_token_deinit(void *mqtt_handle);

void register_token_state_cb(void (*cb)(int));
int get_token_state(void);
#endif
