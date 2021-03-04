#include "cloudLinkListFunc.h"
#include "cloudLink.h"
#include "cloudLinkCtrl.h"

#include "scene.h"

static int user_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    int res = -1;

    EXAMPLE_TRACE("Property Set Received, Devid: %d, Request: %s", devid, request);

    // EXAMPLE_TRACE("Post Property Message ID: %d", res);

    res = cloudLinkCtrl((void *)devid, request);
    return res;
}

static int user_property_get_event_handler(const int devid, const char *request, const int request_len, char **response, int *response_len)
{
    int res = 0, i, j, k;

    EXAMPLE_TRACE("Property Get Received, Devid: %d, Request: %s", devid, request);

    CloudLinkDev *cloudLinkDev = cloudLinkListGetBySn(devid);
    if (cloudLinkDev == NULL)
    {
        printf(" cloudLinkDev is null\n");
        return -1;
    }
    HyLinkDev *hyLinkDev = hylinkListGetById(cloudLinkDev->alinkInfo.device_name);
    if (hyLinkDev == NULL)
    {
        printf(" hyLinkDev is null\n");
        return -1;
    }

    cJSON *root = cJSON_Parse(request);
    // char *json = cJSON_Print(root);
    // EXAMPLE_TRACE("json:%s", json);
    // free(json);

    cJSON *send = cJSON_CreateObject();

    cJSON *array_sub;
    int array_size = cJSON_GetArraySize(root);

    for (i = 0; i < array_size; i++)
    {
        array_sub = cJSON_GetArrayItem(root, i);
        if (array_sub == NULL)
            continue;

        for (j = 0; j < cloudLinkDev->attrLen; j++)
        {
            if (strcmp(cloudLinkDev->attr[j].cloudKey, array_sub->valuestring) == 0)
            {
                for (k = 0; k < hyLinkDev->attrLen; ++k)
                {
                    if (strcmp(hyLinkDev->attr[k].hyKey, cloudLinkDev->attr[j].hyKey) == 0)
                    {
                        char *cloudKey = cloudLinkDev->attr[j].cloudKey;
                        char *hyValue = hyLinkDev->attr[k].value;
                        switch (hyLinkDev->attr[k].valueType)
                        {
                        case LINK_VALUE_TYPE_ENUM:
                            cJSON_AddNumberToObject(send, cloudKey, *hyValue);
                            break;
                        case LINK_VALUE_TYPE_NUM:
                            cJSON_AddNumberToObject(send, cloudKey, *(int *)hyValue);
                            break;
                        case LINK_VALUE_TYPE_STRING:
                            cJSON_AddStringToObject(send, cloudKey, hyValue);
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
    char *snedjson = cJSON_PrintUnformatted(send);
    EXAMPLE_TRACE("report json:%s", snedjson);
    *response_len = strlen(snedjson);
    *response = HAL_Malloc(*response_len);
    strncpy(*response, snedjson, *response_len);
    free(snedjson);
    cJSON_Delete(root);
    cJSON_Delete(send);
    return res;
}
static int user_permit_join_event_handler(const char *product_key, const int time)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("Product Key: %s, Time: %d", product_key, time);

    user_example_ctx->permit_join = 1;

    return 0;
}
static int user_trigger_event_reply_event_handler(const int devid, const int msgid, const int code, const char *eventid,
                                                  const int eventid_len, const char *message, const int message_len)
{
    EXAMPLE_TRACE("Trigger Event Reply Received, Message ID: %d, Code: %d, EventID: %.*s, Message: %.*s",
                  msgid, code,
                  eventid_len,
                  eventid, message_len, message);

    return 0;
}

static int user_report_reply_event_handler(const int devid, const int msgid, const int code, const char *reply,
                                           const int reply_len)
{
    const char *reply_value = (reply == NULL) ? ("NULL") : (reply);
    const int reply_value_len = (reply_len == 0) ? (strlen("NULL")) : (reply_len);

    EXAMPLE_TRACE("Message Post Reply Received, Devid: %d, Message ID: %d, Code: %d, Reply: %.*s", devid, msgid, code,
                  reply_value_len,
                  reply_value);
    return 0;
}

static int user_topolist_handler(const int devid, const int msgid, const int code, const char *payload, const int payload_len)
{
    EXAMPLE_TRACE("user_topolist_handler devid:%d,msgid:%d,code:%d,%s", devid, msgid, code, payload);
    return 0;
}

static int user_service_request_event_handler(const int devid, const char *serviceid, const int serviceid_len,
                                              const char *request, const int request_len,
                                              char **response, int *response_len)
{
    EXAMPLE_TRACE("Service Request Received,serviceid_len:%d,Service ID: %.*s, Payload: %s", serviceid_len, serviceid_len, serviceid, request);

    return cloudLinkServicCtrl(devid, serviceid, serviceid_len, request, response, response_len);
}

#ifdef DEV_BIND_ENABLED
static int user_dev_bind_handler(const char *detail)
{
    EXAMPLE_TRACE("get bind event:%s", detail);
    return 0;
}
#endif
void linkkit_subdev_register(void)
{
    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_GET, user_property_get_event_handler);
    IOT_RegisterCallback(ITE_PERMIT_JOIN, user_permit_join_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, user_service_request_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_TRIGGER_EVENT_REPLY, user_trigger_event_reply_event_handler);
    IOT_RegisterCallback(ITE_TOPOLIST_REPLY, user_topolist_handler);
#ifdef DEV_BIND_ENABLED
    IOT_RegisterCallback(ITE_BIND_EVENT, user_dev_bind_handler);
#endif
}

void linkkit_devrst_evt_handle(iotx_devrst_evt_type_t evt, void *msg)
{
    switch (evt)
    {
    case IOTX_DEVRST_EVT_RECEIVED:
    {
        iotx_devrst_evt_recv_msg_t *recv_msg = (iotx_devrst_evt_recv_msg_t *)msg;

        EXAMPLE_TRACE("Receive Reset Responst");
        EXAMPLE_TRACE("Msg ID: %d", recv_msg->msgid);
        EXAMPLE_TRACE("Payload: %.*s", recv_msg->payload_len, recv_msg->payload);
    }
    break;

    default:
        break;
    }
}

void linkkit_user_post_property(const int devid, const char *payload)
{
    int res = 0;

    EXAMPLE_TRACE("linkkit_user_post_property:%d,%s", devid, payload);
    if (payload == NULL)
        return;
    res = IOT_Linkkit_Report(devid, ITM_MSG_POST_PROPERTY, (unsigned char *)payload, strlen(payload));
    EXAMPLE_TRACE("Post Property Message ID: %d", res);
}

int linkkit_subdev_status(iotx_linkkit_dev_meta_info_t *meta_info, int *id, SubDevStatus status)
{
    EXAMPLE_TRACE("linkkit_subdev_status\n");
    int res = -1, devid = -1;
    if (id != NULL)
    {
        user_example_ctx_t *user_example_ctx = user_example_get_ctx();
        if (*id == user_example_ctx->master_devid)
        {
            EXAMPLE_TRACE("gw device online.......");
            return -1;
        }
    }
    switch (status)
    {
    case SUBDEV_OFFLINE:
        devid = *id;
        if (devid < 0)
        {
            EXAMPLE_TRACE("subdev already logout...\n");
            goto fail;
        }
        res = IOT_Linkkit_Report(devid, ITM_MSG_LOGOUT, NULL, 0);
        if (res == FAIL_RETURN)
        {
            EXAMPLE_TRACE("subdev logout Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev logout success: devid = %d,%d\n", devid, res);
        *id = -1;
        break;
    case SUBDEV_ONLINE:
    {
        EXAMPLE_TRACE("subdev meta_info->device_secret %s\n", meta_info->device_secret);

        devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_SLAVE, meta_info);
        if (devid == FAIL_RETURN)
        {
            EXAMPLE_TRACE("subdev open Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev open susseed, devid = %d\n", devid);
        *id = devid;

        res = IOT_Linkkit_Connect(devid);
        if (res == FAIL_RETURN)
        {
            EXAMPLE_TRACE("subdev connect Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev connect success: devid = %d,%d\n", devid, res);

        res = IOT_Linkkit_Report(devid, ITM_MSG_LOGIN, NULL, 0);
        if (res == FAIL_RETURN)
        {
            EXAMPLE_TRACE("subdev login Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev login success: devid = %d,%d,%s\n", devid, res, meta_info->device_secret);
        res = 0;
    }
    break;
    case SUBDEV_RESTORE:
    {
        // res = IOT_Linkkit_Report(devid, ITM_MSG_DELETE_TOPO, NULL, 0);
        // if (res == FAIL_RETURN)
        // {
        //     EXAMPLE_TRACE("subdev ITM_MSG_DELETE_TOPO Failed\n");
        //     goto fail;
        // }
        // EXAMPLE_TRACE("subdev ITM_MSG_DELETE_TOPO success: devid = %d\n", devid);

        iotx_dev_meta_info_t reset_meta_info;
        memset(&reset_meta_info, 0, sizeof(iotx_dev_meta_info_t));
        memcpy(reset_meta_info.product_key, meta_info->product_key, strlen(meta_info->product_key));
        memcpy(reset_meta_info.device_name, meta_info->device_name, strlen(meta_info->device_name));

        res = IOT_DevReset_Report(&reset_meta_info, linkkit_devrst_evt_handle, NULL);
        if (res == FAIL_RETURN)
        {
            EXAMPLE_TRACE("subdev IOT_DevReset_Report Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev IOT_DevReset_Report success: device_name = %s\n", meta_info->device_name);

        devid = *id;
        if (devid < 0)
        {
            EXAMPLE_TRACE("subdev already restore...\n");
            goto fail;
        }
        res = IOT_Linkkit_Close(devid);
        EXAMPLE_TRACE("subdev IOT_Linkkit_Close:%d\n", res);
        *id = -1;
    }
    break;
    default:
        break;
    }
fail:
    return res;
}

int user_post_event(int devid, char *event_id, char *event_payload)
{
    int res = -1;

    EXAMPLE_TRACE("Post Event %s,%s", event_id, event_payload);
    if (event_payload != NULL)
    {
        res = IOT_Linkkit_TriggerEvent(devid, event_id, strlen(event_id),
                                       event_payload, strlen(event_payload));
    }
    else
    {
        res = IOT_Linkkit_TriggerEvent(devid, event_id, strlen(event_id),
                                       "{}", strlen("{}"));
    }
    EXAMPLE_TRACE("Post Event Message ID: %d,%s", res, event_payload);
    return res;
}
