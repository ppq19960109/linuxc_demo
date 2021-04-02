#include "main.h"
#include "hy_mqtt_client.h"

static int main_close(void)
{
  mqtt_client_close();
  hylinkClose();
  exit(0);
  return 0;
}
int main(void)
{
  registerSystemCb(main_close, SYSTEM_CLOSE);

  mqtt_client_open();

  hylinkOpen();

  mqtt_client_publish("honyar/Report/12345678","test publush!!!!!!!!!!!!!");
  while (1)
  {
    sleep(1);
  }
  main_close();
  return 0;
}