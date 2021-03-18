#ifndef __MAIN_H_
#define __MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/reboot.h>
#include <pthread.h>

#include "cJSON.h"

#include "logFunc.h"
#include "cmd_run.h"
#include "commonFunc.h"
#include "networkFunc.h"

#include "frameCb.h"

#include "hylink.h"
#include "hylinkListFunc.h"
#include "hylinkSubDev.h"
#include "hylinkSend.h"
#include "hylinkRecv.h"

#define ETH_NAME "eth0"

#define MQTT_ADDRESS "post-cn-oew22m4al1d.mqtt.aliyuncs.com"
#define MQTT_CLIENTID "GID_HONYAR@@@0001"
#define MQTT_USERNAME "Signature|JNKMaQHBiFiwGPQJ|post-cn-oew22m4al1d"
#define MQTT_PASSWORD "j4ihmBX0vZtWIMOdCHFtHDnOLIw="

#endif
