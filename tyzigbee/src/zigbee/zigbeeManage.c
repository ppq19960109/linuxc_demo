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
        if (converDesc->hyKeyPrivate == 0)
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

void zigbeeAttrReportHy(ZigbeeAttr *attr, char *devId, char *modelId, unsigned char *in, int inLen)
{
    unsigned char converData[33] = {0};
    ConverDesc converDesc = {0};
    converDesc.modelId = modelId;
    strcpy(converDesc.hyKey, attr->hyKey);
    converDesc.hyKeyPrivate = attr->hyKeyPrivate;
    converDesc.dir = STR_UP;
    converDesc.in = in;
    converDesc.inLen = inLen;
    converDesc.out = converData;
    logDebug("converDesc.hyKey:%s", converDesc.hyKey);
    int converLen = hylinkValueConversion(&converDesc);

    runZigbeeCb(devId, converDesc.hyKey, converData, (void *)converLen, ZIGBEE_DEV_REPORT);
}

void zigbeeReportHy(zigbeeDev *zDev, ty_z3_aps_frame_s *frame, int attributeId, unsigned char *in, int inLen)
{
    int i;
    for (i = 0; i < zDev->attrLen; ++i)
    {
        if (zDev->attr[i].ClusterId == frame->cluster_id && (zDev->attr[i].AttributeId == attributeId || attributeId < 0) && (zDev->attr[i].dstEndpoint == frame->src_endpoint || frame->dst_endpoint == 255))
        {
            // unsigned char converData[33] = {0};
            // ConverDesc converDesc = {0};
            // converDesc.modelId = zDev->manuName;
            // strcpy(converDesc.hyKey, zDev->attr[i].hyKey);
            // converDesc.hyKeyPrivate = zDev->attr[i].hyKeyPrivate;
            // converDesc.dir = STR_UP;
            // converDesc.in = in;
            // converDesc.inLen = inLen;
            // converDesc.out = converData;
            // logDebug("converDesc.hyKey:%s", converDesc.hyKey);
            // int converLen = hylinkValueConversion(&converDesc);

            // runZigbeeCb(frame->id, converDesc.hyKey, converData, (void *)converLen, ZIGBEE_DEV_REPORT);

            if (strcmp(zDev->manuName, "_TYZB01_kw2okqc3") == 0)
            {
                unsigned char attr = (*in >> zDev->attr[i].z3CmdId) & 0x01;
                zigbeeAttrReportHy(&zDev->attr[i], frame->id, zDev->manuName, &attr, 1);
            }
            else
            {
                zigbeeAttrReportHy(&zDev->attr[i], frame->id, zDev->manuName, in, inLen);
            }
        }
    }
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
    if (frame->cmd_type == Z3_CMD_TYPE_PRIVATE)
    {
        if (frame->cluster_id == ZCL_PRIVATE_CLUSTER)
        {
            switch (frame->cmd_id)
            {
            case TY_DATA_RESPONE:
            case TY_DATA_REPORT:
            case TY_DATA_QUERY:
            case TY_DATA_MODULE_RSP:
            case TY_DATA_MODULE:
            {
                attributeId = frame->message[2];
                dataType = frame->message[3];
                dataTypeLen = (frame->message[4] << 8) + frame->message[5];
                value = &frame->message[6];
                zigbeeReportHy(zDev, frame, attributeId, value + shift, dataTypeLen);
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
            if (strcmp(zDev->manuName, "_TYZB01_kw2okqc3") == 0)
            {
                dataTypeLen = 1;
                value = frame->message;
                zigbeeReportHy(zDev, frame, -1, value + shift, dataTypeLen);
            }
        }
    }
    else
    {
        switch (frame->cmd_id)
        {
        case READ_ATTRIBUTES_RESPONSE:
        {
            int index = 0;
            while (index < frame->msg_length)
            {
                logInfo("READ_ATTRIBUTES_RESPONSE");
                attributeId = frame->message[index + 0] + (frame->message[index + 1] << 8);
                index += 2;
                status = frame->message[index];
                index += 1;
                if (status != 0)
                {
                    logError("READ_ATTRIBUTES_RESPONSE status error:%d", status);
                    break;
                }
                dataType = frame->message[index];
                index += 1;
                value = &frame->message[index];
                dataTypeLen = getZcl3DataType(dataType, value, &shift);
                index += dataTypeLen + shift;
                zigbeeReportHy(zDev, frame, attributeId, value + shift, dataTypeLen);
            }
        }
        break;
        case REPORT_ATTRIBUTES:
        {
            int index = 0;
            while (index < frame->msg_length)
            {
                logInfo("REPORT_ATTRIBUTES");
                attributeId = frame->message[index + 0] + (frame->message[index + 1] << 8);
                index += 2;
                dataType = frame->message[index];
                index += 1;
                value = &frame->message[index];
                dataTypeLen = getZcl3DataType(dataType, value, &shift);
                index += dataTypeLen + shift;
                zigbeeReportHy(zDev, frame, attributeId, value + shift, dataTypeLen);
            }
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
}

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

int zigbeeDevZclQuery(ZigbeeAttr *attr, ty_z3_aps_frame_s *frame)
{
    if (attr->ClusterId == ZCL_PRIVATE_CLUSTER)
    {
        frame->cmd_type = Z3_CMD_TYPE_PRIVATE;
        frame->cmd_id = attr->z3CmdId >> 4;
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
    return tuyaZclSend(frame);
}

static int zigbeeDevZclCtrl(ZigbeeAttr *attr, ty_z3_aps_frame_s *frame, char *modelId, void *key, void *value)
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
    cmdType = attr->z3CmdType;
    if (attr->ClusterId == ZCL_PRIVATE_CLUSTER)
    {
        cmdId = attr->z3CmdId & 0x0f;
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
    return tuyaZclSend(frame);
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

    unsigned char msg[33] = {0};
    ty_z3_aps_frame_s frame = {0};
    strcpy(frame.id, devId);
    frame.node_id = 0;
    frame.profile_id = Z3_PROFILE_ID_HA;
    frame.group_id = 0;
    frame.disable_ack = 0;
    frame.frame_type = Z3_FRAME_TYPE_UNICAST;
    frame.message = msg;

    if (key == NULL)
    {
        for (i = 0; i < zDev->attrLen; ++i)
        {
            frame.cluster_id = zDev->attr[i].ClusterId;
            frame.src_endpoint = zDev->attr[i].srcEndpoint;
            frame.dst_endpoint = zDev->attr[i].dstEndpoint;
            zigbeeDevZclQuery(&zDev->attr[i], &frame);
        }
        return 0;
    }
    int hyKeyLen;
    for (i = 0; i < zDev->attrLen; ++i)
    {
        hyKeyLen = strlen(zDev->attr[i].hyKey);
        if (strncmp(key, zDev->attr[i].hyKey, hyKeyLen) == 0)
        {
            break;
        }
    }
    if (i == zDev->attrLen)
        return -1;
    //------------------------
    frame.cluster_id = zDev->attr[i].ClusterId;
    frame.src_endpoint = zDev->attr[i].srcEndpoint;
    frame.dst_endpoint = zDev->attr[i].dstEndpoint;
    //-------------------------

    if (value == NULL)
    {
        return zigbeeDevZclQuery(&zDev->attr[i], &frame);
    }
    else
    {
        return zigbeeDevZclCtrl(&zDev->attr[i], &frame, zDev->manuName, key, value);
    }
}