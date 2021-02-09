#ifndef __MAIN_H_
#define __MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/reboot.h>

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

#include "hilink.h"
#include "hilink_interface.h"
#include "hilink_log_manage.h"
#include "hilink_netconfig_mode_mgt.h"

#include "hilink_network_adapter.h"
#include "hilink_softap_adapter.h"

#include "hilink_profile_adapter.h"
#include "hilink_profile_bridge.h"

#define PROFILE_PATH "hilinkprofile"
#define ETH_NAME "eth0"

// #define HILINK_REPORT_ASYNC
#define HILINK_OTA_PATH "/userdata/update/upgrade.bin"
#define HILINK_OTA_MAX_FILE_SIZE (8 * 1024 * 1024)
#endif
