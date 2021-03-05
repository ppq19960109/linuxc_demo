#include "main.h"
#include "hy_mqtt_client.h"

// #define TEST
#ifdef TEST
#define MQTT_ADDRESS "post-cn-oew22m4al1d.mqtt.aliyuncs.com"
#define MQTT_CLIENTID "GID_HONYAR@@@0001"
#else
#define MQTT_ADDRESS "183.230.40.96:1883"
#define MQTT_CLIENTID "demo1"
#endif
#define TOPIC "$sys/353614/demo1/dp/post/json"
#define PAYLOAD "{\"id\":1234,\"dp\":{\"name\":[{\"v\": 76}]}}"

static int main_close(void)
{
  mqtt_client_close();
  // hylinkClose();
  exit(0);
  return 0;
}
int main(void)
{
  registerSystemCb(main_close, SYSTEM_CLOSE);
#ifdef TEST
  mqtt_client_open(MQTT_ADDRESS, MQTT_CLIENTID, "Signature|JNKMaQHBiFiwGPQJ|post-cn-oew22m4al1d", "j4ihmBX0vZtWIMOdCHFtHDnOLIw=");
#else
  mqtt_client_open(MQTT_ADDRESS, MQTT_CLIENTID, "353614", "version=2018-10-31&res=products%2F353614%2Fdevices%2Fdemo1&et=1672735919&method=md5&sign=VZG9VVXwRsR%2BQWZTiIsyjA%3D%3D");
#endif
  mqtt_client_publish(TOPIC, PAYLOAD);
  if (mqtt_client_subscribe("$sys/353614/demo1/cmd/request/#") != 0)
    printf("mqtt_client_subscribe fail\n");
  else
    printf("mqtt_client_subscribe success\n");

  // hylinkOpen();
  while (1)
  {
    sleep(1);
  }

  return 0;
}