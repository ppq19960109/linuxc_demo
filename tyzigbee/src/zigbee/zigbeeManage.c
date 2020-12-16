#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "tuya_gw_infra_api.h"
#include "tuya_gw_z3_api.h"
#include "tuya_gw_dp_api.h"

#include "tyZigbee.h"
#include "tyZigbeeZcl3.h"

#include "logFunc.h"
#include "commonFunc.h"
#include "frameCb.h"
#include "cpython.h"
#include "base64.h"

#include "zigbeeManage.h"
#include "zigbeeListFunc.h"

#include "hylinkRecv.h"
#include "hylinkReport.h"
#include "hylinkListFunc.h"

typedef struct
{
    char *modelId;
    char hyKey[33];
    char hyKeyPrivate;
    char *dir;
    unsigned char *in;
    int inLen;
    unsigned char *out;
    int outLen;
} ConverDesc;

int hylinkValueConversion(ConverDesc *converDesc)
{
    if (converDesc->inLen == 0)
        return -1;
    if (strcmp(converDesc->dir, STR_UP) == 0) //up
    {
        if (converDesc->hyKeyPrivate == 0)
        {
            sprintf((char *)converDesc->out, "%d", *converDesc->in);
            converDesc->outLen = strlen((char *)converDesc->out);
        }
        else
        {
            if (strcmp(converDesc->hyKey, "SceName_") == 0 || strcmp(converDesc->hyKey, "ScePhoto_") == 0)
            {
                int num = (converDesc->in[0] << 8) + converDesc->in[1];
                char buf[8] = {0};
                sprintf(buf, "%d", num);
                strcat(converDesc->hyKey, buf);
                converDesc->in += 2;
                converDesc->inLen -= 2;
            }

            void *encodeOut = malloc(BASE64_ENCODE_OUT_SIZE(converDesc->inLen));
            int encodeOutlen = base64_encode(converDesc->in, converDesc->inLen, encodeOut);
            logDebug("base64_encode encodeOut:%s", encodeOut);

            hyLinkConver(converDesc->modelId, converDesc->hyKey, converDesc->dir, (char *)encodeOut, encodeOutlen, (char *)converDesc->out, &converDesc->outLen);
            free(encodeOut);
        }
    }
    else //down
    {
        if (converDesc->hyKeyPrivate == 0 && converDesc->inLen == 1)
        {
            long num;
            strToNum((char *)converDesc->in, 10, &num);
            *converDesc->out = num;
            converDesc->outLen = 1;
        }
        else
        {
            char privatekeyLen = 0;
            if (strncmp(converDesc->hyKey, "SceName_", strlen("SceName_")) == 0 || strncmp(converDesc->hyKey, "ScePhoto_", strlen("ScePhoto_")) == 0)
            {
                char *pos = strchr(converDesc->hyKey, '_');
                long num;
                strToNum(pos + 1, 10, &num);
                converDesc->out[0] = num >> 8;
                converDesc->out[1] = num & 0xff;
                converDesc->out += 2;
                privatekeyLen = 2;
            }
            logDebug("in %s,%d", converDesc->in, converDesc->inLen);
            hyLinkConver(converDesc->modelId, converDesc->hyKey, converDesc->dir, (char *)converDesc->in, converDesc->inLen, (char *)converDesc->out, &converDesc->outLen);
            logDebug("out %s,%d", converDesc->out, converDesc->outLen);

            void *decodeOut = malloc(BASE64_DECODE_OUT_SIZE(converDesc->outLen));
            int decodeOutlen = base64_decode((char *)converDesc->out, converDesc->outLen, decodeOut);
            logPrintfHex("base64_decode:", decodeOut, decodeOutlen);
            memcpy(converDesc->out, decodeOut, decodeOutlen);
            converDesc->outLen = decodeOutlen + privatekeyLen;
            free(decodeOut);
        }
    }

    return converDesc->outLen;
}

int zigbeeDevZclReport(void *recv)
{
    if (recv == NULL)
        return -1;
    ty_z3_aps_frame_s *frame = (ty_z3_aps_frame_s *)recv;

    HylinkDev *hyDev = (HylinkDev *)hylinkListGet(frame->id);
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

    unsigned short attributeId;
    unsigned char status;
    unsigned short dataType;
    unsigned char *value;
    int shift = 0, i = 0;
    int dataTypeLen;

    if (frame->cluster_id == ZCL_PRIVATE_CLUSTER)
    {
        switch (frame->cmd_id)
        {
        case TY_DATA_RESPONE:
        case TY_DATA_REPORT:
        case TY_DATA_QUERY:
        {
            attributeId = frame->message[2];
            dataType = frame->message[3];
            dataTypeLen = (frame->message[4] << 8) + frame->message[5];
            value = &frame->message[6];
            goto report;
        }
        break;
        case TUYA_MCU_VERSION_RSP:
        {
            char ver_str[16] = {0};
            unsigned char version = frame->message[2];
            snprintf(ver_str, sizeof(ver_str), "%d.%d.%d",
                     (version >> 6) & 0x03, (version >> 4) & 0x03, version & 0x0f);
            logInfo("mcu version: %s", ver_str);
        }
        break;
        case TUYA_MCU_SYNC_TIME:
        {
            unsigned char msg[16] = {0};
            unsigned short sequence = 0;
            memcpy(msg, &sequence, 2);
            time_t stdTime = time(NULL);

            msg[2] = stdTime >> 24;
            msg[3] = stdTime >> 16;
            msg[4] = stdTime >> 8;
            msg[5] = stdTime & 0xff;

            stdTime = stdTime + 8 * 60 * 60;
            msg[6] = stdTime >> 24;
            msg[7] = stdTime >> 16;
            msg[8] = stdTime >> 8;
            msg[9] = stdTime & 0xff;

            frame->msg_length = 10;
            if (frame->msg_length > 0)
                frame->message = msg;

            return tuya_user_iot_z3_dev_send_zcl_cmd(frame);
        }
            logInfo("zigbee tuya sync time");
            break;
        default:
            logError("zigbee private cmd id not exist");
            break;
        }
    }
    else
    {
        switch (frame->cmd_id)
        {
        case READ_ATTRIBUTES_RESPONSE:
        {
            attributeId = frame->message[0] + (frame->message[1] << 8);
            status = frame->message[2];
            if (status != 0)
            {
                logError("READ_ATTRIBUTES_RESPONSE status error:%d", status);
                break;
            }
            dataType = frame->message[3];
            value = &frame->message[4];
            dataTypeLen = getZcl3DataType(dataType, value, &shift);
            goto report;
        }
        break;
        case REPORT_ATTRIBUTES:
        {
            attributeId = frame->message[0] + (frame->message[1] << 8);
            dataType = frame->message[2];
            value = &frame->message[3];
            dataTypeLen = getZcl3DataType(dataType, value, &shift);
            goto report;
        }
        break;
        case WRITE_ATTRIBUTES_RESPONSE:
        {
            logInfo("WRITE_ATTRIBUTES_RESPONSE:");
            for (i = 0; i < frame->msg_length; i++)
                printf("%02x ", frame->message[i]);
            printf("\n");
        }
        break;
        default:
            logError("zigbee cmd id not exist");
            break;
        }
    }
    return 0;
report:
    for (i = 0; i < zDev->attrLen; ++i)
    {
        if (zDev->attr[i].ClusterId == frame->cluster_id && zDev->attr[i].AttributeId == attributeId && zDev->attr[i].dstEndpoint == frame->src_endpoint)
        {

            unsigned char converData[33] = {0};
            ConverDesc converDesc = {0};
            converDesc.modelId = hyDev->ModelId;
            strcpy(converDesc.hyKey, zDev->attr[i].hyKey);
            converDesc.hyKeyPrivate = zDev->attr[i].hyKeyPrivate;
            converDesc.dir = STR_UP;
            converDesc.in = value + shift;
            converDesc.inLen = dataTypeLen;
            converDesc.out = converData;
            logDebug("converDesc.hyKey:%s", converDesc.hyKey);
            int converLen = hylinkValueConversion(&converDesc);

            runZigbeeCb(frame->id, converDesc.hyKey, converData, (void *)converLen, ZIGBEE_DEV_REPORT);
        }
    }
    return 0;
}

int zigbeeDevZclDispatch(void *devId, void *modelId, void *key, void *value)
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
    int i;
    int hyKeyLen;
    for (i = 0; i < zDev->attrLen; ++i)
    {
        hyKeyLen = strlen(zDev->attr[i].hyKey);
        if (strncmp(key, zDev->attr[i].hyKey, hyKeyLen) == 0)
        {
            break;
        }
    }
    unsigned char cmdId;
    unsigned char cmdType;
    unsigned char msg[33] = {0};
    unsigned short msgLen;

    if (value == NULL)
    {

        if (zDev->attr[i].ClusterId == ZCL_PRIVATE_CLUSTER)
        {
            cmdType = Z3_CMD_TYPE_PRIVATE;
            cmdId = TY_DATA_QUERY;
            msgLen = 0;
        }
        else
        {
            cmdType = Z3_CMD_TYPE_GLOBAL;
            cmdId = READ_ATTRIBUTE;
            memcpy(msg, &zDev->attr[i].AttributeId, 2);
            msgLen = 2;
        }
    }
    else
    {
        ConverDesc converDesc = {0};
        converDesc.modelId = hyDev->ModelId;
        strcpy(converDesc.hyKey, key);
        converDesc.hyKeyPrivate = zDev->attr[i].hyKeyPrivate;
        converDesc.dir = STR_DOWN;
        converDesc.in = value;

        int shift;
        int dataTypeLen = 0;
        cmdType = zDev->attr[i].z3CtrlCmdType;
        if (zDev->attr[i].ClusterId == ZCL_PRIVATE_CLUSTER)
        {
            cmdId = TY_DATA_REQUEST;
            unsigned short sequence = 0;
            memcpy(msg, &sequence, 2);
            msg[2] = zDev->attr[i].AttributeId;
            msg[3] = zDev->attr[i].dataType;
            dataTypeLen = getZcl3ProvateDataType(zDev->attr[i].dataType);

            converDesc.inLen = dataTypeLen;
            converDesc.out = &msg[6];
            dataTypeLen = hylinkValueConversion(&converDesc);
            msg[4] = dataTypeLen >> 8;
            msg[5] = dataTypeLen & 0xff;
            msgLen = dataTypeLen + 6;
        }
        else
        {
            dataTypeLen = getZcl3DataType(zDev->attr[i].dataType, NULL, &shift);
            switch (cmdType)
            {
            case Z3_CMD_TYPE_GLOBAL:
            {
                cmdId = WRITE_ATTRIBUTES;
                memcpy(msg, &zDev->attr[i].AttributeId, 2);
                msg[2] = zDev->attr[i].dataType;

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
                dataTypeLen = hylinkValueConversion(&converDesc);
                cmdId = msg[0];
                msgLen = 0;
            }
            break;
            case Z3_CMD_TYPE_ZDO:
                break;
            default:
                return -1;
            }
        }
    }
    ty_z3_aps_frame_s frame = {0};
    strcpy(frame.id, devId);
    frame.node_id = 0;
    frame.profile_id = Z3_PROFILE_ID_HA;
    frame.cluster_id = zDev->attr[i].ClusterId;
    frame.src_endpoint = zDev->attr[i].srcEndpoint;
    frame.dst_endpoint = zDev->attr[i].dstEndpoint;
    frame.group_id = 0;
    frame.cmd_type = cmdType;
    frame.cmd_id = cmdId;
    frame.frame_type = Z3_FRAME_TYPE_UNICAST;
    frame.disable_ack = 0;
    frame.msg_length = msgLen;
    if (frame.msg_length > 0)
        frame.message = msg;

    logDebug("device zcl send callback");
    logDebug("        id: %s", frame.id);
    logDebug("profile_id: 0x%04x", frame.profile_id);
    logDebug("cluster_id: 0x%04x", frame.cluster_id);
    logDebug("   node_id: 0x%04x", frame.node_id);
    logDebug("    src_ep: %d", frame.src_endpoint);
    logDebug("    dst_ep: %d", frame.dst_endpoint);
    logDebug("  group_id: %d", frame.group_id);
    logDebug("  cmd_type: %d", frame.cmd_type);
    logDebug("   command: 0x%02x", frame.cmd_id);
    logDebug("frame_type: %d", frame.frame_type);
    logDebug("   msg_len: %d", frame.msg_length);
    logDebug("       msg: ");
    for (i = 0; i < frame.msg_length; i++)
        printf("%02x ", frame.message[i]);
    printf("\n");
    return tuya_user_iot_z3_dev_send_zcl_cmd(&frame);
}