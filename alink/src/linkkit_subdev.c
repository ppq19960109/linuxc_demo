#include <string.h>

#include "linkkit_subdev.h"
#include "linkkit_app_gateway.h"

#include "infra_defs.h"
#include "dev_reset_api.h"
#include "infra_compat.h"

#include "cJSON.h"
#include "cloudLink.h"
#include "cloudLinkCtrl.h"
#include "frameCb.h"
#include "cloudLinkListFunc.h"

static int user_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    int res = -1;

    EXAMPLE_TRACE("Property Set Received, Devid: %d, Request: %s", devid, request);

    // EXAMPLE_TRACE("Post Property Message ID: %d", res);

    res = cloudLinkCtrl(devid, request);
    return res;
}

static int user_property_get_event_handler(const int devid, const char *request, const int request_len, char **response, int *response_len)
{
    int res = 0, i, j;

    EXAMPLE_TRACE("Property Get Received, Devid: %d, Request: %s", devid, request);
    cJSON *root = cJSON_Parse(request);
    // char *json = cJSON_Print(root);
    // EXAMPLE_TRACE("json:%s", json);
    // free(json);

    CloudLinkDev *cloudLinkDev = cloudLinkListGetBySn(devid);
    if (cloudLinkDev == NULL)
        return -1;
    cJSON *send = cJSON_CreateObject();

    cJSON *array_sub;
    int array_size = cJSON_GetArraySize(root);

    for (i = 0; i < array_size; i++)
    {
        array_sub = cJSON_GetArrayItem(root, i);
        if (array_sub == NULL)
            continue;

        for (j = 0; j < cloudLinkDev->devSvcNum; j++)
        {
            if (strcmp(cloudLinkDev->devSvc[j].svcId, array_sub->valuestring) == 0)
            {
                if (cloudLinkDev->devSvc[j].svcVal == NULL)
                {
                    EXAMPLE_TRACE("svcId:%s svcVal is null", cloudLinkDev->devSvc[j].svcId);
                    break;
                }
                // EXAMPLE_TRACE("svcId:%s,%s", cloudLinkDev->devSvc[j].svcId, cloudLinkDev->devSvc[j].svcVal);
                cJSON *buf = cJSON_Parse(cloudLinkDev->devSvc[j].svcVal);
                cJSON *val = cJSON_GetObjectItem(buf, cloudLinkDev->devSvc[j].svcId);
                if (val->valuestring != NULL)
                {
                    cJSON_AddStringToObject(send, cloudLinkDev->devSvc[j].svcId, val->valuestring);
                }
                else
                {
                    cJSON_AddNumberToObject(send, cloudLinkDev->devSvc[j].svcId, val->valueint);
                }

                cJSON_Delete(buf);
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
    // IOT_RegisterCallback(ITE_PROPERTY_GET, user_property_get_event_handler);
    IOT_RegisterCallback(ITE_PERMIT_JOIN, user_permit_join_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, user_service_request_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_TRIGGER_EVENT_REPLY, user_trigger_event_reply_event_handler);
    IOT_RegisterCallback(ITE_TOPOLIST_REPLY, user_topolist_handler);
#ifdef DEV_BIND_ENABLED
    IOT_RegisterCallback(ITE_BIND_EVENT, user_dev_bind_handler);
#endif
}

static void linkkit_devrst_evt_handle(iotx_devrst_evt_type_t evt, void *msg)
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

    switch (status)
    {
    case SUBDEV_OFFLINE:
        devid = *id;
        if (devid < 0)
        {
            goto fail;
        }
        res = IOT_Linkkit_Report(devid, ITM_MSG_LOGOUT, NULL, 0);
        if (res == FAIL_RETURN)
        {
            EXAMPLE_TRACE("subdev logout Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev logout success: devid = %d,%d\n", devid, res);

        break;
    case SUBDEV_ONLINE:
    {
        int online = 0;
        EXAMPLE_TRACE("subdev meta_info->device_secret %s\n", meta_info->device_secret);
        if (id != NULL && *id > 0)
        {
            EXAMPLE_TRACE("subdev already login: devid = %d\n", *id);
            online = 1;
        }
        else
        {
            // res = IOT_Dynamic_Register(IOTX_HTTP_REGION_SHANGHAI, meta_info);
            // if (res < 0)
            // {
            //     EXAMPLE_TRACE("IOT_Dynamic_Register:%d fail\n", res);
            //     goto fail;
            // }
            // EXAMPLE_TRACE("subdev IOT_Dynamic_Register %s\n", meta_info->device_secret);
        }

        devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_SLAVE, meta_info);
        if (devid < 0)
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
            if (online == 0)
            {
                goto fail;
            }
        }
        EXAMPLE_TRACE("subdev login success: devid = %d,%d,%s\n", devid, res, meta_info->device_secret);
        res = 0;
    }
    break;
    // case SUBDEV_LAST:
    case SUBDEV_RESTORE:
        devid = *id;
        // res = IOT_Linkkit_Report(devid, ITM_MSG_DELETE_TOPO, NULL, 0);
        // if (res == FAIL_RETURN)
        // {
        //     EXAMPLE_TRACE("subdev ITM_MSG_DELETE_TOPO Failed\n");
        //     goto fail;
        // }
        // EXAMPLE_TRACE("subdev ITM_MSG_DELETE_TOPO success: devid = %d\n", devid);

        res = IOT_DevReset_Report(meta_info, linkkit_devrst_evt_handle, NULL);
        if (res == FAIL_RETURN)
        {
            EXAMPLE_TRACE("subdev IOT_DevReset_Report Failed\n");
            goto fail;
        }
        EXAMPLE_TRACE("subdev IOT_DevReset_Report success: device_name = %s\n", meta_info->device_name);
        res = IOT_Linkkit_Close(devid);
        EXAMPLE_TRACE("subdev IOT_Linkkit_Close:%d\n", res);
        *id = -1;
        break;
    default:
        break;
    }
fail:
    return res;
}

int user_post_event(int devid, char *event_id, const char *event_payload)
{
    int res = -1;

    res = IOT_Linkkit_TriggerEvent(devid, event_id, strlen(event_id),
                                   event_payload, strlen(event_payload));
    EXAMPLE_TRACE("Post Event Message ID: %d", res);
    return res;
}
