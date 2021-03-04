#include "cloudLinkListFunc.h"
#include "cloudLinkReport.h"
#include "cloudLinkCtrl.h"
#include "cloudLink.h"

#include "scene.h"
#include "scene_add.h"

static int time_key;
static char *weekDay[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char *cloudActive[] = {"==", ">", "<", "!="};
static char *localActive[] = {"Equ", "Gtr", "Lss", "Not"};

static int getSceneDevStatus(cJSON *params, cJSON *arrayItem)
{
    cJSON *deviceName, *propertyName, *propertyValue, *compareType;

    compareType = cJSON_GetObjectItem(params, "compareType");
    deviceName = cJSON_GetObjectItem(params, "deviceName");
    propertyName = cJSON_GetObjectItem(params, "propertyName");

    int active = findStrIndex(compareType->valuestring, cloudActive, sizeof(cloudActive));
    if (active == -1)
    {
        logError("compareType active is no exist\n");
        cJSON_AddStringToObject(arrayItem, "Active", "Equ");
    }
    else
    {
        cJSON_AddStringToObject(arrayItem, "Active", localActive[active]);
    }

    if (cJSON_HasObjectItem(params, "propertyValue"))
    {
        propertyValue = cJSON_GetObjectItem(params, "propertyValue");
    }
    else
    {
        propertyValue = cJSON_GetObjectItem(params, "compareValue");
    }

    CloudLinkDev *cloudLinkDev = cloudLinkListGetById(deviceName->valuestring);
    if (cloudLinkDev == NULL)
        return -1;
    int i;
    char *hyKey = NULL;
    for (i = 0; i < cloudLinkDev->attrLen; ++i)
    {
        if (strcmp(propertyName->valuestring, cloudLinkDev->attr[i].cloudKey) == 0)
        {
            break;
        }
    }
    if (i == cloudLinkDev->attrLen)
    {
        for (i = 0; i < cloudLinkDev->eventAttrLen; ++i)
        {
            if (strcmp(propertyName->valuestring, cloudLinkDev->eventAttr[i].key) == 0)
            {
                break;
            }
        }
        if (i == cloudLinkDev->eventAttrLen)
            return -1;
        hyKey = cloudLinkDev->eventAttr[i].hyKey;
    }
    else
    {
        hyKey = cloudLinkDev->attr[i].hyKey;
    }

    HyLinkDev *hylinkDev = hylinkListGetById(deviceName->valuestring);
    if (hylinkDev != NULL)
    {
        for (i = 0; i < hylinkDev->attrLen; ++i)
        {
            if (strcmp(hylinkDev->attr[i].hyKey, hyKey) == 0)
            {
                break;
            }
        }
        if (i != hylinkDev->attrLen)
        {
            if (hylinkDev->attr[i].repeat)
            {
                cJSON_AddStringToObject(arrayItem, "TriggerType", "InstantEvery");
            }
            else
            {
                cJSON_AddStringToObject(arrayItem, "TriggerType", "InstantOnce");
            }
        }
    }

    //-------------------------------
    cJSON_AddStringToObject(arrayItem, "KeyCoding", "Original");

    char strBuf[8];
    cJSON_AddStringToObject(arrayItem, "ActionId", "0");
    cJSON_AddStringToObject(arrayItem, "DevId", deviceName->valuestring);
    cJSON_AddStringToObject(arrayItem, "Key", hyKey);
    cjson_to_str(propertyValue, strBuf);
    cJSON_AddStringToObject(arrayItem, "Value", strBuf);
    return 0;
}

int addtriggers(const char *localSceneId, cJSON *triggersItems)
{
    time_key = 0;
    int triggersItemsSize = cJSON_GetArraySize(triggersItems);
    if (triggersItemsSize == 0)
        return 0;
    cJSON *triggersArray = cJSON_CreateArray();
    cJSON *triggersItemsSub;
    for (int i = 0; i < triggersItemsSize; ++i)
    {
        triggersItemsSub = cJSON_GetArrayItem(triggersItems, i);
        if (triggersItemsSub == NULL)
            continue;

        cJSON *arrayItem = cJSON_CreateObject();
        cJSON_AddItemToArray(triggersArray, arrayItem);
        cJSON_AddStringToObject(arrayItem, "Op", "AddCond");
        cJSON_AddStringToObject(arrayItem, "Id", localSceneId);

        cJSON_AddStringToObject(arrayItem, "Logic", "Or");

        cJSON *params = cJSON_GetObjectItem(triggersItemsSub, "params");
        if (cJSON_HasObjectItem(params, "cron"))
        {
            cJSON_AddStringToObject(arrayItem, "CondType", "Time");
            char key[8];
            sprintf(key, "Or_%d", time_key++);
            cJSON_AddStringToObject(arrayItem, "TimeKey", key);

            cJSON *cron = cJSON_GetObjectItem(params, "cron");

            char *ptimeStr = strdup(cron->valuestring);
            logInfo("ptimeStr addr %s", ptimeStr);
            char *token, *timeStr = ptimeStr;
            const char delim[] = " ,";

            token = strsep(&timeStr, delim);
            cJSON_AddStringToObject(arrayItem, "StartMinu", token);
            cJSON_AddStringToObject(arrayItem, "EndMinu", token);
            token = strsep(&timeStr, delim);
            cJSON_AddStringToObject(arrayItem, "StartHour", token);
            cJSON_AddStringToObject(arrayItem, "EndHour", token);

            token = strsep(&timeStr, delim);
            token = strsep(&timeStr, delim);

            if (strcmp("*", token) == 0)
                cJSON_AddStringToObject(arrayItem, "Repeat", "1");
            else
                cJSON_AddStringToObject(arrayItem, "Repeat", "0");

            char weekBuf[33] = {0};
            while ((token = strsep(&timeStr, delim)) != NULL)
            {
                int day = atoi(token);
                if (strlen(weekBuf) != 0)
                    strcat(weekBuf, ",");
                strcat(weekBuf, weekDay[day]);
            }
            cJSON_AddStringToObject(arrayItem, "Week", weekBuf);
            logWarn("timeStr addr %d", timeStr);
            free(ptimeStr);
        }
        else if (cJSON_HasObjectItem(params, "deviceName"))
        {
            cJSON_AddStringToObject(arrayItem, "CondType", "Event");
            getSceneDevStatus(params, arrayItem);
        }
        else
        {
            /* code */
        }
    }
    return sceneHyDispatch(triggersArray);
}

int addconditions(const char *localSceneId, cJSON *conditionsItems)
{
    time_key = 0;
    int conditionsItemsSize = cJSON_GetArraySize(conditionsItems);
    if (conditionsItemsSize == 0)
        return 0;
    cJSON *conditionsArray = cJSON_CreateArray();

    cJSON *conditionsItemsSub;
    for (int i = 0; i < conditionsItemsSize; ++i)
    {
        conditionsItemsSub = cJSON_GetArrayItem(conditionsItems, i);
        if (conditionsItemsSub == NULL)
            continue;
        cJSON *arrayItem = cJSON_CreateObject();
        cJSON_AddItemToArray(conditionsArray, arrayItem);
        cJSON_AddStringToObject(arrayItem, "Op", "AddCond");
        cJSON_AddStringToObject(arrayItem, "Id", localSceneId);

        cJSON_AddStringToObject(arrayItem, "Logic", "And");

        cJSON *params = cJSON_GetObjectItem(conditionsItemsSub, "params");
        if (cJSON_HasObjectItem(params, "startTime"))
        {
            cJSON_AddStringToObject(arrayItem, "CondType", "Time");
            char key[8];
            sprintf(key, "And_%d", time_key++);
            cJSON_AddStringToObject(arrayItem, "TimeKey", key);

            cJSON *startTime = cJSON_GetObjectItem(params, "startTime");
            cJSON *endTime = cJSON_GetObjectItem(params, "endTime");

            const char delim[] = ":";

            char *ptimeStr = strdup(startTime->valuestring);
            char *token, *timeStr = ptimeStr;
            token = strsep(&timeStr, delim);
            cJSON_AddStringToObject(arrayItem, "StartHour", token);
            token = strsep(&timeStr, delim);
            cJSON_AddStringToObject(arrayItem, "StartMinu", token);
            free(ptimeStr);

            ptimeStr = strdup(endTime->valuestring);
            timeStr = ptimeStr;
            token = strsep(&timeStr, delim);
            cJSON_AddStringToObject(arrayItem, "EndHour", token);
            token = strsep(&timeStr, delim);
            cJSON_AddStringToObject(arrayItem, "EndMinu", token);
            free(ptimeStr);
        }
        else if (cJSON_HasObjectItem(params, "deviceName"))
        {
            cJSON_AddStringToObject(arrayItem, "CondType", "Event");
            getSceneDevStatus(params, arrayItem);
        }
        else
        {
            /* code */
        }
    }
    return sceneHyDispatch(conditionsArray);
}

int addActionsForSceneId(const char *src_scene_id, const char *rule_id, const char *tar_scene_id)
{
    cJSON *actionsArray = cJSON_CreateArray();

    cJSON *arrayItem = cJSON_CreateObject();
    cJSON_AddItemToArray(actionsArray, arrayItem);
    cJSON_AddStringToObject(arrayItem, "Op", "AddAction");
    cJSON_AddStringToObject(arrayItem, "Id", src_scene_id);
    //-------------------------------
    cJSON_AddStringToObject(arrayItem, "KeyCoding", "Original");

    cJSON_AddStringToObject(arrayItem, "ActionId", rule_id);
    cJSON_AddStringToObject(arrayItem, "DevId", STR_GATEWAY_DEVID);
    cJSON_AddStringToObject(arrayItem, "Key", "SceneId");

    cJSON *Value = cJSON_CreateObject();
    cJSON_AddStringToObject(Value, "SceneId", rule_id);
    cJSON_AddStringToObject(Value, "Enable", "1");
    char *json = cJSON_PrintUnformatted(Value);
    cJSON_AddStringToObject(arrayItem, "Value", json);
    cJSON_Delete(Value);
    cJSON_free(json);
    sceneHyDispatch(actionsArray);
    return 0;
}

int addActions(const char *localSceneId, cJSON *actions)
{
    int actionsSize = cJSON_GetArraySize(actions);
    if (actionsSize == 0)
        return 0;
    int dispatchNum = 0, i = 0;
    do
    {
        if (actionsSize > dispatchNum + 10)
        {
            dispatchNum += 10;
        }
        else
        {
            dispatchNum = actionsSize;
        }
        cJSON *actionsArray = cJSON_CreateArray();
        cJSON *actionsSub;
        for (; i < dispatchNum; ++i)
        {
            actionsSub = cJSON_GetArrayItem(actions, i);
            if (actionsSub == NULL)
                continue;

            //--------------------------
            cJSON *params = cJSON_GetObjectItem(actionsSub, "params");
            if (cJSON_HasObjectItem(params, "sceneId"))
            {
                cJSON *sceneId = cJSON_GetObjectItem(params, "sceneId");
                selectDatabseBySceneId(localSceneId, sceneId->valuestring, addActionsForSceneId);
            }
            else
            {
                cJSON *arrayItem = cJSON_CreateObject();
                cJSON_AddItemToArray(actionsArray, arrayItem);
                cJSON_AddStringToObject(arrayItem, "Op", "AddAction");
                cJSON_AddStringToObject(arrayItem, "Id", localSceneId);
                getSceneDevStatus(params, arrayItem);
            }
        }

        sceneHyDispatch(actionsArray);
    } while (dispatchNum < actionsSize);

    return 0;
}
