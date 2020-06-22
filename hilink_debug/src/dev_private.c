#include "dev_private.h"

typedef struct
{
    char Switch_1;
    char Switch_2;
    char Switch_3;
} Switch_dev_t;

typedef struct
{
    char Switch_1;
    char Switch_2;
    char Switch_3;
} HY0098_dev_t;

void dev_private_attribute(dev_data_t *dev, cJSON *Data)
{
    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);
    cJSON *Key = cJSON_GetObjectItem(array_sub, "Key");

    if (strcmp(dev->ModelId, "500c32") == 0)
    {
        if (dev->private == NULL)
        {
            dev->private = malloc(2);
        }

        if (Key != NULL && strcmp(Key->valuestring, "Switch"))
        {
            str_copy_from_json(array_sub, "Value", dev->private);
        }
    }
}