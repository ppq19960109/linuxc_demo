#ifndef _HYLINKRECV_H_
#define _HYLINKRECV_H_

#ifdef __cplusplus
extern "C"
{
#endif
#define STR_GATEWAY_DEVID "0000000000000000"
#define STR_GATEWAY_MODELID "000000"
#define STR_TIME "Time"
#define STR_ADD "Add"

#define STR_REFACTORY "ReFactory"

#define STR_COMMAND "Command"
#define STR_REPORT "Report"
#define STR_DISPATCH "Dispatch"

#define STR_BEATHEARTRESPONSE "TcpBeatHeart"
#define STR_FRAMENUMBER "FrameNumber"
#define STR_TYPE "Type"
#define STR_DATA "Data"
#define STR_DEVICEID "DeviceId"
#define STR_MODELID "ModelId"
#define STR_GATEWAYID "GatewayId"
#define STR_ONLINE "Online"
#define STR_VERSION "Version"
#define STR_PARAMS "Params"

#define STR_REGISTER "Register"
#define STR_UNREGISTER "UnRegister"
#define STR_ONOFF "OnOff"
#define STR_CTRL "Ctrl"
#define STR_ATTRIBUTE "Attribute"
#define STR_DELETE "Delete"

#define STR_KEY "Key"
#define STR_VALUE "Value"

#define STR_UP "up"
#define STR_DOWN "down"

#define STR_DEVSINFO "DevsInfo"
#define STR_DEVATTRI "DevAttri"
    typedef struct
    {
        char *const *attr;
        unsigned short attrLen;
        char *const *attrCtrl;
        unsigned short attrCtrlLen;
    } AttrDesc;

    int hylinkRecvManage(void *data, unsigned int len);

#ifdef __cplusplus
}
#endif
#endif