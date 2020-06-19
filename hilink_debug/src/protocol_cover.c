#include <string.h>
#include "cJSON.h"
// #include "sqlite3.h"
#include "log.h"

// const char Command[] = {"Dispatch", "Report"};
const char test_json[] = {"{\
                            \"Command\" : \"Dispatch\", \
                            \"FrameNumber\" : \"00\",\
                            \"Type\" : \"Ctrl\",\
                            \"Data\" : [\
                                {\
                                    \"DeviceId\" : \"123456787654310\",\
                                    \"Key\" : \"Switch\",\
                                    \"Value\" : \"1\"\
                                }\
                            ]} "};

void protlcol_init()
{
}

int read_from_bottom(const char *json)
{
    cJSON *root = cJSON_Parse(test_json);
    if (root == NULL)
    {
        log_error("root is NULL\n");
        goto fail;
    }
    // log_debug("%s\n",cJSON_Print(root));
    cJSON *Command = cJSON_GetObjectItem(root, "Command");
    if (Command == NULL)
    {
        log_error("Command is NULL\n");
        goto fail;
    }
    cJSON *Type = cJSON_GetObjectItem(root, "Type");
    if (Type == NULL)
    {
        log_error("Type is NULL\n");
        goto fail;
    }

    cJSON *Data = cJSON_GetObjectItem(root, "Data");
    if (Data == NULL)
    {
        log_error("Data is NULL\n");
        goto fail;
    }
    int array_size = cJSON_GetArraySize(Data);
    for (int cnt = 0; cnt < array_size; cnt++)
    {
        cJSON *array_sub = cJSON_GetArrayItem(Data, cnt);
        cJSON *DeviceId = cJSON_GetObjectItem(array_sub, "DeviceId");
        log_debug("%s\n", DeviceId->valuestring);
    }
    cJSON *GatewayId = cJSON_GetObjectItem(root, "GatewayId");
    if (GatewayId == NULL)
    {
        log_error("GatewayId is NULL\n");
    }
    if (strcmp("Dispatch", Command->valuestring) == 0)
    {
    }
    else if (strcmp("Report", Command->valuestring) == 0)
    {
    }
    else
    {
        /* code */
        log_error("Command is value invaild\n");
        goto fail;
    }
    return 0;
fail:
    log_error("error\n");
    return -1;
}

int write_to_bottom(void *ptr)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "Command", "Dispatch");
    cJSON_AddStringToObject(root, "Type", "DevAttri");

    cJSON *DataArr = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "Data", DataArr);

    cJSON *arrayItem = cJSON_CreateObject();
    cJSON_AddStringToObject(arrayItem, "DeviceId", "123456787654310");
    cJSON_AddStringToObject(arrayItem, "Key", "All");
    cJSON_AddItemToArray(DataArr, arrayItem);

    char *json = cJSON_Print(root);
    log_debug("%s\n", json);
    return 0;
}

int read_from_cloud(void *ptr)
{

    return 0;
}

int write_to_cloud(void *ptr)
{

    return 0;
}

int cloud_to_bottom()
{
    return 0;
}

int bottom_to_cloud()
{
    return 0;
}