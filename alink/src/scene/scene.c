#include "cloudLinkListFunc.h"
#include "cloudLinkReport.h"
#include "cloudLinkCtrl.h"
#include "cloudLink.h"

#include "scene.h"

int sceneReport(void *req, unsigned int len)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "SceneId", req);

    char *json = cJSON_PrintUnformatted(root);
    logInfo("cJSON_free(json); send json:%s\n", json);
    cJSON_Delete(root);

    CloudLinkDev *cloudLinkDev = cloudLinkListGetById(STR_GATEWAY_DEVID);
    if (cloudLinkDev == NULL)
    {
        logError("gw cloudLinkDev is NULL");
        goto fail;
    }
    user_post_event(cloudLinkDev->id, "ReportScene", json);
    cJSON_free(json);
    return 0;
fail:
    cJSON_free(json);
    return -1;
}
//-------------------------------
int sceneHyDispatch(cJSON *DataArray)
{
    if (cJSON_GetArraySize(DataArray) == 0)
    {
        cJSON_Delete(DataArray);
        return -1;
    }
    cJSON *dispatch = cJSON_CreateObject();
    cJSON_AddStringToObject(dispatch, STR_COMMAND, STR_DISPATCH);
    cJSON_AddStringToObject(dispatch, STR_FRAMENUMBER, "00");
    cJSON_AddStringToObject(dispatch, STR_TYPE, "LocalScene");

    cJSON_AddItemToObject(dispatch, STR_DATA, DataArray);

    char *json = cJSON_PrintUnformatted(dispatch);
    logInfo("scene send json:%s\n", json);

    int res = hylinkDispatch(json);
    cJSON_free(json);
    cJSON_Delete(dispatch);
    return res;
}

static int delete_single_scene(const char *ruleId)
{
    cJSON *delArray = cJSON_CreateArray();

    cJSON *arrayItem = cJSON_CreateObject();
    cJSON_AddItemToArray(delArray, arrayItem);
    cJSON_AddStringToObject(arrayItem, "Op", "DelScene");
    cJSON_AddStringToObject(arrayItem, "Id", ruleId);

    int res = sceneHyDispatch(delArray);
    if (res >= 0)
    {
        //delete scene id and rule id for database
        deleteDatabse(ruleId);
    }
    return res;
}

static int delScene(cJSON *ruleIds)
{
    int res;
    if (cJSON_IsArray(ruleIds))
    {
        int ruleIdsSize = cJSON_GetArraySize(ruleIds);
        if (ruleIdsSize == 0)
            return 0;
        cJSON *ruleIdsSub;
        for (int i = 0; i < ruleIdsSize; ++i)
        {
            ruleIdsSub = cJSON_GetArrayItem(ruleIds, i);
            if (ruleIdsSub == NULL)
                continue;
            res = delete_single_scene(ruleIdsSub->valuestring);
        }
    }
    else
    {
        res = delete_single_scene(ruleIds->valuestring);
    }
    return res;
}

static int triggerScene(cJSON *ruleIds)
{
    int ruleIdsSize = cJSON_GetArraySize(ruleIds);
    if (ruleIdsSize == 0)
        return 0;

    cJSON *delArray = cJSON_CreateArray();

    cJSON *ruleIdsSub;
    for (int i = 0; i < ruleIdsSize; ++i)
    {
        ruleIdsSub = cJSON_GetArrayItem(ruleIds, i);
        if (ruleIdsSub == NULL)
            continue;
        cJSON *arrayItem = cJSON_CreateObject();
        cJSON_AddItemToArray(delArray, arrayItem);
        cJSON_AddStringToObject(arrayItem, "Op", "ExecScene");
        cJSON_AddStringToObject(arrayItem, "Id", ruleIdsSub->valuestring);
        cJSON_AddStringToObject(arrayItem, "Enable", "1");
    }
    return sceneHyDispatch(delArray);
}
static int addScene(const char *sceneId, cJSON *rules, const int isUpdate)
{
    int delayTime = 0, res;

    cJSON *ruleId = cJSON_GetObjectItem(rules, "ruleId");
    cJSON *enable = cJSON_GetObjectItem(rules, "enable");

    //---------------------------------
    cJSON *additionsSub, *params;
    cJSON *additions = cJSON_GetObjectItem(rules, "additions");
    int additionsSize = cJSON_GetArraySize(additions);
    if (additionsSize)
    {
        additionsSub = cJSON_GetArrayItem(additions, 0);
        params = cJSON_GetObjectItem(additionsSub, "params");
        if (params != NULL)
        {
            delayTime = cJSON_GetObjectItem(params, "delayTime")->valueint;
        }
    }
    //-----------------------------------
    if (isUpdate == 1)
    {
        delScene(ruleId);
    }
    cJSON *updateArray = cJSON_CreateArray();
    cJSON *arrayItem = cJSON_CreateObject();
    cJSON_AddItemToArray(updateArray, arrayItem);

    cJSON_AddStringToObject(arrayItem, "Op", "AddScene");
    char buf[8] = {0};
    cJSON_AddStringToObject(arrayItem, "Id", ruleId->valuestring);
    sprintf(buf, "%d", enable->valueint);
    cJSON_AddStringToObject(arrayItem, "Enable", buf);
    sprintf(buf, "%d", delayTime);
    cJSON_AddStringToObject(arrayItem, "ExecDelayed", buf);
    res = sceneHyDispatch(updateArray);
    if (res >= 0)
    {
        //add scene id and rule id for database
        insertDatabse(ruleId->valuestring, sceneId);
    }
    //---------------------------------
    cJSON *triggers = cJSON_GetObjectItem(rules, "triggers");
    cJSON *triggersItems = cJSON_GetObjectItem(triggers, "items");
    addtriggers(ruleId->valuestring, triggersItems);

    //---------------------------------
    cJSON *conditions = cJSON_GetObjectItem(rules, "conditions");
    cJSON *conditionsItems = cJSON_GetObjectItem(conditions, "items");
    addconditions(ruleId->valuestring, conditionsItems);

    //---------------------------------
    cJSON *actions = cJSON_GetObjectItem(rules, "actions");
    addActions(ruleId->valuestring, actions);

    return 0;
}

int sceneDispatch(const char *str, char **response, int *response_len)
{
    int res = 0;
    /* Parse Root */
    cJSON *root = cJSON_Parse(str);
    if (root == NULL || !cJSON_IsObject(root))
    {
        logError("JSON Parse Error");
        return -1;
    }
    // char *rootjson = cJSON_PrintUnformatted(root);
    // logInfo("rootjson:%s\n", rootjson);
    // cJSON_free(rootjson);

    cJSON *operationType = cJSON_GetObjectItem(root, "operationType");
    if (operationType == NULL)
    {
        logError("operationType is NULL\n");
        goto fail;
    }

    cJSON *sceneId = cJSON_GetObjectItem(root, "sceneId");
    if (sceneId == NULL)
    {
        logError("sceneId is NULL\n");
        goto fail;
    }

    //-----------------------------------
    cJSON *rules = cJSON_GetObjectItem(root, "rules");
    if (rules != NULL)
    {
        int rulesSize = cJSON_GetArraySize(rules);
        cJSON *rulesSub;
        for (int i = 0; i < rulesSize; ++i)
        {
            rulesSub = cJSON_GetArrayItem(rules, i);
            if (rulesSub == NULL)
                continue;
            if (strcmp("createScene", operationType->valuestring) == 0)
            {
                res = addScene(sceneId->valuestring, rulesSub, 0);
            }
            else if (strcmp("updateScene", operationType->valuestring) == 0)
            {
                res = addScene(sceneId->valuestring, rulesSub, 1);
            }
            else
            {
                goto fail;
            }
        }
    }
    cJSON *ruleIds = cJSON_GetObjectItem(root, "ruleIds");
    if (ruleIds != NULL)
    {
        if (strcmp("deleteScene", operationType->valuestring) == 0)
        {
            res = delScene(ruleIds);
        }
    }
    cJSON *ruleId = cJSON_GetObjectItem(root, "ruleId");
    if (ruleId != NULL)
    {
        if (strcmp("fire", operationType->valuestring) == 0)
        {
            res = triggerScene(ruleId);
        }
    }
    //-----------------------------------
    int code = 200;
    if (res < 0)
    {
        code = 400;
    }
    cJSON *rsp = cJSON_CreateObject();
    cJSON_AddNumberToObject(rsp, "code", code);
    cJSON_AddStringToObject(rsp, "data", sceneId->valuestring);
    cJSON_AddStringToObject(rsp, "message", "successed");
    char *rspJson = cJSON_PrintUnformatted(rsp);
    cJSON_Delete(rsp);
    //-----------------------------------
    cJSON_Delete(root);
    //-----------------------------------
    /* Send Service Response To Cloud */
    *response_len = strlen(rspJson) + 1;
    *response = (char *)malloc(*response_len);
    if (*response == NULL)
    {
        logInfo("Memory Not Enough");
        return -1;
    }
    memset(*response, 0, *response_len);
    strncpy(*response, rspJson, *response_len);
    logInfo("scene response:%s\n", *response);

    cJSON_free(rspJson);
    //-----------------------------------
    return res;

fail:
    logError("operationType not exist");
    cJSON_Delete(root);
    return -1;
}
int localScene(const int devid, const char *serviceid, const int serviceid_len, const char *request, char **response, int *response_len)
{
    int i;
    /* Parse Root */
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        logError("JSON Parse Error");
        return -1;
    }
    //Type字段
    cJSON *Type = cJSON_GetObjectItem(root, STR_TYPE);
    if (Type == NULL)
    {
        logError("Type is NULL\n");
        goto fail;
    }
    cJSON *sceneRequest;
    char buf[24] = {0};
    for (i = 1; i <= 9; ++i)
    {
        sprintf(buf, "Request_%d", i);
        sceneRequest = cJSON_GetObjectItem(root, buf);
        if (sceneRequest == NULL)
        {
            logError("sceneRequest is NULL\n");
            continue;
        }
        int encLen = strlen(sceneRequest->valuestring);
        if (encLen != 0)
        {
            void *decodeOut = malloc(BASE64_DECODE_OUT_SIZE(encLen) + 1);
            memset(decodeOut, 0, BASE64_DECODE_OUT_SIZE(encLen) + 1);
            int decodeOutlen = base64_decode(sceneRequest->valuestring, encLen, decodeOut);
            logDebug("sceneRequest:%s,%d", decodeOut, decodeOutlen);
            sceneDispatch(decodeOut, response, response_len);
            free(decodeOut);
        }
    }
    cJSON_Delete(root);

    return 0;

fail:
    cJSON_Delete(root);
    return -1;
}