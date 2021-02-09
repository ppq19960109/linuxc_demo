#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "logFunc.h"
#include "commonFunc.h"
#include "frameCb.h"

#include "hylinkSubDev.h"
#include "hylink.h"
#include "hylinkRecv.h"

int hylinkSubDevAttrUpdate(HyLinkDev *hyLinkDev, cJSON *Data)
{
    int i = 0;

    if (Data == NULL)
    {
        logDebug("hylinkSubDevAttrUpdate Data NULL\n");
        goto success;
    }

    cJSON *Key = cJSON_GetObjectItem(Data, STR_KEY);
    if (Key == NULL)
        goto fail;

    for (i = 0; i < hyLinkDev->attrLen; ++i)
    {
        if (strcmp(hyLinkDev->attr[i].hyKey, Key->valuestring) == 0)
        {
            break;
        }
    }
    if (i == hyLinkDev->attrLen)
        goto fail;

    cJSON *value = cJSON_GetObjectItem(Data, STR_VALUE);
    if (value == NULL)
    {
        logError("value is NULL");
        goto fail;
    }
    if (strlen(value->valuestring) == 0)
    {
        logError("value string len is 0");
        goto fail;
    }
    switch (hyLinkDev->attr[i].valueType)
    {
    case LINK_VALUE_TYPE_ENUM:
    {
        char *hyVal = hyLinkDev->attr[i].value;
        int num = atoi(value->valuestring);
        if (num == *hyVal)
        {
            if (hyLinkDev->attr[i].repeat != REPEAT_REPORT)
                goto repeat;
        }
        else
            *hyVal = num;
    }
    break;
    case LINK_VALUE_TYPE_NUM:
    {
        int *hyVal = (int *)hyLinkDev->attr[i].value;
        int num = atoi(value->valuestring);
        if (num == *hyVal)
        {
            if (hyLinkDev->attr[i].repeat != REPEAT_REPORT)
                goto repeat;
        }
        else
            *hyVal = num;
    }
    break;
    case LINK_VALUE_TYPE_STRING:
    {
        char *hyVal = hyLinkDev->attr[i].value;

        if (strcmp(hyVal, value->valuestring) == 0)
        {
            if (hyLinkDev->attr[i].repeat != REPEAT_REPORT)
                goto repeat;
        }
        else
            strcpy(hyVal, value->valuestring);
    }
    break;
    default:
        break;
    }

success:
    return i;
repeat:
    return -2;
fail:
    return -1;
}
