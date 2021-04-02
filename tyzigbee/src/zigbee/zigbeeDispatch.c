#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tuya_gw_infra_api.h"
#include "tuya_gw_z3_api.h"
#include "tuya_gw_dp_api.h"

#include "tyZigbee.h"
#include "tyZigbeeZcl3.h"

#include "logFunc.h"
#include "commonFunc.h"
#include "frameCb.h"

#include "zigbeeManage.h"
#include "zigbeeListFunc.h"

#include "hylinkRecv.h"
#include "hylinkListFunc.h"

static int tuyaZclSend(ty_z3_aps_frame_s *frame)
{
    logDebug("device zcl send callback");
    logDebug("        id: %s", frame->id);
    logDebug("profile_id: 0x%04x", frame->profile_id);
    logDebug("cluster_id: 0x%04x", frame->cluster_id);
    logDebug("   node_id: 0x%04x", frame->node_id);
    logDebug("    src_ep: %d", frame->src_endpoint);
    logDebug("    dst_ep: %d", frame->dst_endpoint);
    logDebug("  group_id: %d", frame->group_id);
    logDebug("  cmd_type: %d", frame->cmd_type);
    logDebug("   command: 0x%02x", frame->cmd_id);
    logDebug("frame_type: %d", frame->frame_type);
    logDebug("   msg_len: %d", frame->msg_length);
    logDebug("       msg: ");
    for (int i = 0; i < frame->msg_length; i++)
        printf("%02x ", frame->message[i]);
    printf("\n");
    return tuya_user_iot_z3_dev_send_zcl_cmd(frame);
}

static int zigbeeZclQuery(ZigbeeAttr *attr, ty_z3_aps_frame_s *frame)
{
    if (attr->ClusterId == ZCL_PRIVATE_CLUSTER)
    {
        frame->cmd_type = Z3_CMD_TYPE_PRIVATE;
        frame->cmd_id = attr->z3CmdId >> 4; //tuya common TY_DATA_QUERY:3,high 4 query
        // frame->msg_length = 0;
        unsigned short squ = 0;
        memcpy(frame->message, &squ, 2);
        frame->message[2] = attr->AttributeId;
        frame->msg_length = 3;
    }
    else
    {
        frame->cmd_type = Z3_CMD_TYPE_GLOBAL;
        frame->cmd_id = READ_ATTRIBUTE;
        memcpy(frame->message, &attr->AttributeId, 2);
        frame->msg_length = 2;
    }
    frame->cluster_id = attr->ClusterId;
    frame->src_endpoint = attr->srcEndpoint;
    frame->dst_endpoint = attr->dstEndpoint;
    return tuyaZclSend(frame);
}

static int zigbeeZclCtrl(ZigbeeAttr *attr, ty_z3_aps_frame_s *frame, char *modelId, void *key, void *value)
{
    unsigned char cmdId;
    unsigned char cmdType;
    unsigned short msgLen;
    unsigned char *msg = frame->message;

    ConverDesc converDesc = {0};
    converDesc.modelId = modelId;
    strcpy(converDesc.hyKey, key);
    converDesc.hyKeyPrivate = attr->hyKeyPrivate;
    converDesc.dir = STR_DOWN;
    converDesc.in = value;

    int shift;
    int dataTypeLen = 0;
    cmdType = attr->z3CmdType; //tuya zigbee cmdtype:Z3_CMD_TYPE_GLOBAL Z3_CMD_TYPE_PRIVATE Z3_CMD_TYPE_ZDO
    if (attr->ClusterId == ZCL_PRIVATE_CLUSTER)
    {
        cmdId = attr->z3CmdId & 0x0f; //tuya common TY_DATA_REQUEST:0,low 4 ctrl
        unsigned short sequence = 0;
        memcpy(msg, &sequence, 2);
        msg[2] = attr->AttributeId;
        msg[3] = attr->dataType;
        dataTypeLen = getZcl3ProvateDataType(attr->dataType);

        converDesc.inLen = dataTypeLen;
        converDesc.out = &msg[6];
        dataTypeLen = hylinkValueConversion(&converDesc);
        msg[4] = dataTypeLen >> 8;
        msg[5] = dataTypeLen & 0xff;
        msgLen = dataTypeLen + 6;
    }
    else
    {
        dataTypeLen = getZcl3DataType(attr->dataType, NULL, &shift);
        switch (cmdType)
        {
        case Z3_CMD_TYPE_GLOBAL:
        {
            cmdId = WRITE_ATTRIBUTES;
            memcpy(msg, &attr->AttributeId, 2);
            msg[2] = attr->dataType;

            converDesc.inLen = dataTypeLen;
            converDesc.out = &msg[3];
            dataTypeLen = hylinkValueConversion(&converDesc);
            msgLen = dataTypeLen + 3;
        }
        break;
        case Z3_CMD_TYPE_PRIVATE:
        {
            converDesc.inLen = dataTypeLen;
            converDesc.out = &msg[0];
            if (attr->z3CmdId == 0)
            {
                dataTypeLen = hylinkValueConversion(&converDesc);
                cmdId = msg[0];
                msgLen = 0;
            }
            else
            {
                cmdId = attr->z3CmdId & 0x0f;
                msgLen = hylinkValueConversion(&converDesc);
            }
        }
        break;
        case Z3_CMD_TYPE_ZDO:
            break;
        default:
            return -1;
        }
    }

    frame->cmd_type = cmdType;
    frame->cmd_id = cmdId;
    frame->msg_length = msgLen;

    frame->cluster_id = attr->ClusterId;
    frame->src_endpoint = attr->srcEndpoint;
    frame->dst_endpoint = attr->dstEndpoint;

    return tuyaZclSend(frame);
}

static int zigbeeDevZclDispatch(zigbeeDev *zDev, ty_z3_aps_frame_s *frame, void *key, void *value)
{
    int i = 0;
    if (key == NULL)
    {
        for (i = 0; i < zDev->attrLen; ++i)
        {
            frame->cluster_id = zDev->attr[i].ClusterId;
            frame->src_endpoint = zDev->attr[i].srcEndpoint;
            frame->dst_endpoint = zDev->attr[i].dstEndpoint;
            zigbeeZclQuery(&zDev->attr[i], frame);
        }
        return 0;
    }

    for (int hyKeyLen, i = 0; i < zDev->attrLen; ++i)
    {
        hyKeyLen = strlen(zDev->attr[i].hyKey);
        if (strncmp(key, zDev->attr[i].hyKey, hyKeyLen) == 0)
        {
            break;
        }
    }
    if (i == zDev->attrLen)
    {
        return -1;
    }
    //------------------------
    // frame->cluster_id = zDev->attr[i].ClusterId;
    // frame->src_endpoint = zDev->attr[i].srcEndpoint;
    // frame->dst_endpoint = zDev->attr[i].dstEndpoint;
    //-------------------------

    if (value == NULL)
    {
        return zigbeeZclQuery(&zDev->attr[i], frame);
    }
    else
    {
        return zigbeeZclCtrl(&zDev->attr[i], frame, zDev->manuName, key, value);
    }
}

int zigbeeZclDispatch(void *devId, void *modelId, void *key, void *value)
{
    HylinkDev *hyDev = (HylinkDev *)hylinkListGet(devId);
    if (hyDev == NULL)
    {
        logError("hyDev is null");
        return -1;
    }
    zigbeeDev *zDev = (zigbeeDev *)zigbeeListGet(hyDev->ModelId);
    if (zDev == NULL)
    {
        logError("zDev is null");
        return -1;
    }

    unsigned char msg[33] = {0};
    ty_z3_aps_frame_s frame = {0};
    strcpy(frame.id, devId);
    frame.node_id = 0;
    frame.profile_id = Z3_PROFILE_ID_HA;
    frame.group_id = 0;
    frame.disable_ack = 0;
    frame.frame_type = Z3_FRAME_TYPE_UNICAST;
    frame.message = msg;

    int res;
    res = zigbeeDevZclDispatch(zDev, &frame, key, value);
    if (res < 0)
    {
        int i;
        for (i = 0; i < zDev->sceneAttrLen; ++i)
        {
            if (strcmp(key, zDev->sceneAttr[i].key) == 0)
            {
                break;
            }
        }
        if (i == zDev->sceneAttrLen)
            return -1;
        for (int j = 0; j < zDev->sceneAttr[i].valueLen; ++j)
        {
            res = zigbeeDevZclDispatch(zDev, &frame, zDev->sceneAttr[i].value[j], value);
        }
    }
    return res;
}