#include "dev_private.h"

typedef struct
{
    char Switch_1;
    char Switch_2;
    char Switch_3;
    char Switch_All;
} dev_500c33_t;

typedef struct
{
    char KeyFobValue;
} dev_5f0cf1_t;

void dev_private_attribute(dev_data_t *dev, cJSON *Data)
{
    log_debug("dev_private_attribute\n");
    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);
    cJSON *Key = cJSON_GetObjectItem(array_sub, "Key");
    if (Key == NULL)
    {
        return;
    }
    if (strcmp(dev->ModelId, "500c32") == 0)
    {
        if (dev->private == NULL)
        {
            dev->private = malloc(1);
        }

        if (strcmp(Key->valuestring, "Switch"))
        {
            char_copy_from_json(array_sub, "Value", dev->private);
        }
        log_debug("500c32:%d\n", *(char *)dev->private);
    }
    else if (strcmp(dev->ModelId, "500c33") == 0)
    {
        if (dev->private == NULL)
        {
            dev->private = malloc(sizeof(dev_500c33_t));
        }

        dev_500c33_t *dev_500c33 = (dev_500c33_t *)dev->private;
        char *target;
        if (strcmp(Key->valuestring, "Switch_1"))
        {
            target = &dev_500c33->Switch_1;
        }
        else if (strcmp(Key->valuestring, "Switch_2"))
        {
            target = &dev_500c33->Switch_2;
        }
        else if (strcmp(Key->valuestring, "Switch_3"))
        {
            target = &dev_500c33->Switch_3;
        }
        else
        {
            return;
        }
        char_copy_from_json(array_sub, "Value", target);
    }
    else
    {
    }
}

void dev_private_event(dev_data_t *dev, cJSON *Data)
{
    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);
    cJSON *Key = cJSON_GetObjectItem(array_sub, "Key");
    if (Key == NULL)
    {
        return;
    }
    if (strcmp(dev->ModelId, "5f0cf1") == 0)
    {
        if (dev->private == NULL)
        {
            dev->private = malloc(1);
        }

        if (strcmp(Key->valuestring, "KeyFobValue"))
        {
            char_copy_from_json(array_sub, "Value", dev->private);
        }
    }
    else if (strcmp(dev->ModelId, "5f0c3b") == 0)
    {
        if (dev->private == NULL)
        {
            dev->private = malloc(1);
        }

        if (strcmp(Key->valuestring, "KeyFobValue"))
        {
            char_copy_from_json(array_sub, "Value", dev->private);
        }
    }
}