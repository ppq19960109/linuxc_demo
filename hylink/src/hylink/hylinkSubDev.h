#ifndef _HYLINKSUBDEV_H_
#define _HYLINKSUBDEV_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hylink.h"

    typedef enum
    {
        SINGLESWITCHU2 = 0,
        DOUBLESWITCHU2,
        THREESWITCHU2,
        DLTDIMMINGU2,
        SINGLESWITCHMODULE,
        DOUBLESWITCHMODULE,
        THREESWITCHMODULE,
        DOORWINDOWSENSORB,
        TOUCHPANEL02U2,
    } HylinkSubDev;

    typedef struct
    {
        char Switch;
        char LedEnable;
        char PowerOffProtection;
    } SingleSwitchU2;

    typedef struct
    {
        char Switch[2];
        char LedEnable;
        char PowerOffProtection;
    } DoubleSwitchU2;
    typedef struct
    {
        char Switch[3];
        char LedEnable;
        char PowerOffProtection;
    } ThreeSwitchU2;
    typedef struct
    {
        char Switch;
        char Luminance;       //亮度
        int ColorTemperature; //色温
    } DLTDimmingU2;
    typedef struct
    {
        char Switch;
        char LedEnable;
        char PowerOffProtection;
        char KeyMode;
    } SingleSwitchModule;
    typedef struct
    {
        char Switch[2];
        char LedEnable;
        char PowerOffProtection;
        char KeyMode;
    } DoubleSwitchModule;
    typedef struct
    {
        char Switch[3];
        char LedEnable;
        char PowerOffProtection;
        char KeyMode;
    } ThreeSwitchModule;
    typedef struct
    {
        char ContactAlarm;
        char BatteryPercentage;
        char LowBatteryAlarm;
        char TamperAlarm;
        char doorEvent;
    } DoorWindowSensorB;
    typedef struct
    {
        char KeyFobValue;
        char SceName[18][18];
        char ScePhoto[18];
        char Enable[3];
        char Switch[3];
        char CurrentTemperature;
        char TargetTemperature[2];
        char WorkMode;
        char WindSpeed[2];

    } TouchPanel02U2;

    extern const SAttrInfo g_SLocalModel;
    extern const SAttrInfo g_SLocalAttr[];

    int hylinkSubDevAttrUpdate(HylinkDev *dev, cJSON *Data);

#ifdef __cplusplus
}
#endif
#endif