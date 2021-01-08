#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

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

void zigbeeAttrReport(ZigbeeAttr *attr, char *devId, char *modelId, unsigned char *in, int inLen)
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

void zigbeeReport(zigbeeDev *zDev, ty_z3_aps_frame_s *frame, int attributeId, unsigned char *in, int inLen)
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

            zigbeeAttrReport(&zDev->attr[i], frame->id, zDev->manuName, in, inLen);
        }
    }
}

int zigbeeZclReport(void *zclFrame)
{
    if (zclFrame == NULL)
        return -1;
    ty_z3_aps_frame_s *frame = (ty_z3_aps_frame_s *)zclFrame;

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
    unsigned char dataType;
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
                zigbeeReport(zDev, frame, attributeId, value + shift, dataTypeLen);
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
                zigbeeReport(zDev, frame, -1, value + shift, dataTypeLen);
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
                zigbeeReport(zDev, frame, attributeId, value + shift, dataTypeLen);
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
                zigbeeReport(zDev, frame, attributeId, value + shift, dataTypeLen);
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
