#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cJSON.h"
#include "wifi_server.h"

const char *uci_wifi_command[] = {
    "uci delete wireless.@wifi-iface[1]",
    "uci add wireless wifi-iface",
    "uci set wireless.@wifi-iface[1].network=wwan",
    "uci set wireless.@wifi-iface[1].device=radio0",
    "uci set wireless.@wifi-iface[1].mode=sta",
    "uci set wireless.@wifi-iface[1].encryption=psk2",
    "uci set wireless.@wifi-iface[1].ssid=%s",
    "uci set wireless.@wifi-iface[1].key=%s",
    "uci set wireless.@wifi-iface[1].bssid=%s",
    "uci commit wireless && wifi"};

int system_run(const char *cmdline)
{
    printf("system_run cmdline = %s\n", cmdline);

    int ret = system(cmdline);
    if (ret < 0)
    {
        printf("system_run cmdline failed: %s\n", cmdline);
    }
    return ret;
}

int uci_wifi_set(const char *ssid, const char *key, const char *bssid)
{
    if (ssid == NULL)
        return -1;
    char cmdline[128];
    system_run(uci_wifi_command[0]);
    system_run(uci_wifi_command[1]);
    system_run(uci_wifi_command[2]);
    system_run(uci_wifi_command[3]);
    system_run(uci_wifi_command[4]);

    snprintf(cmdline, sizeof(cmdline), uci_wifi_command[6], ssid);
    system_run(cmdline);

    if (key != NULL && strlen(key) != 0)
    {
        system_run(uci_wifi_command[5]);

        snprintf(cmdline, sizeof(cmdline), uci_wifi_command[7], key);
        system_run(cmdline);
    }

    if (bssid != NULL && strlen(bssid) != 0)
    {
        snprintf(cmdline, sizeof(cmdline), uci_wifi_command[8], bssid);
        system_run(cmdline);
    }
    system_run(uci_wifi_command[9]);
    return 0;
}

int response_wifi_config(const int code)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "action", "wifi_config_report");
    cJSON_AddStringToObject(root, "requestId", "");
    cJSON_AddStringToObject(root, "apiVersion", "1.0");

    cJSON *params = cJSON_CreateObject();
    cJSON_AddItemToObject(root,"params",params);
    // cJSON *params = cJSON_AddObjectToObject(root, "params");
    cJSON_AddNumberToObject(params, "code", code);
    cJSON_AddStringToObject(params, "sid", "abedkkqiqq");
    cJSON_AddStringToObject(params, "type", "gateway");
    cJSON_AddStringToObject(params, "model", "");
    cJSON_AddStringToObject(params, "brand", "");

    char *json = cJSON_PrintUnformatted(root);
    printf("response_wifi_config json:%s\n", json);
    //send
    wifi_server_send(json, strlen(json));

    cJSON_Delete(root);
    return 0;
}

int wifi_config(char *data, unsigned int len)
{
    cJSON *root = cJSON_Parse(data);
    if (root == NULL)
    {
        printf("root is NULL\n");
        goto fail;
    }
    cJSON *action = cJSON_GetObjectItem(root, "action");
    if (action == NULL)
    {
        printf("action is NULL\n");
        goto fail;
    }
    cJSON *params = cJSON_GetObjectItem(root, "params");
    if (params == NULL)
    {
        printf("params is NULL\n");
        goto fail;
    }

    cJSON *ssid = cJSON_GetObjectItem(params, "ssid");
    if (ssid == NULL)
    {
        printf("ssid is NULL\n");
        goto fail;
    }

    cJSON *bssid = cJSON_GetObjectItem(params, "bssid");
    if (bssid == NULL)
    {
        printf("bssid is NULL\n");
        goto fail;
    }

    cJSON *pwd = cJSON_GetObjectItem(params, "pwd");
    if (pwd == NULL)
    {
        printf("pwd is NULL\n");
        goto fail;
    }
    uci_wifi_set(ssid->valuestring, pwd->valuestring, bssid->valuestring);
    cJSON_Delete(root);
    response_wifi_config(100);
    return 0;
fail:
    cJSON_Delete(root);
    response_wifi_config(105);
    return -1;
}