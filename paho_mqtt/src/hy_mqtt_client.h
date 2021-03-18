#ifndef __HY_MQTT_CLIENT_H_
#define __HY_MQTT_CLIENT_H_

int mqtt_client_open(void);
int mqtt_client_close(void);
int mqtt_client_reconnect(void);

int mqtt_client_publish(const char *topicName, char *payload);
int mqtt_client_subscribe(const char *topicName);
#endif
