#ifndef __LINK_NTP_H_
#define __LINK_NTP_H_

int link_ntp_start(void *mqtt_handle);
int link_ntp_stop(void);
int link_ntp_request(void);
void register_link_timestamp_cb(void (*cb)(const unsigned int));
#endif
