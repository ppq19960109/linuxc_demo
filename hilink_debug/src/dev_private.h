#ifndef _DEV_PRIVATE_H
#define _DEV_PRIVATE_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "protocol_cover.h"

    typedef struct
    {
        int ColorTemperature;
        char Luminance;
        char Switch;
    } dev_09223f_t; //U2/天际系列：DLT液晶调光器（09223f，型号U86KTGS150-ZXP）

    typedef struct
    {
        char Switch_1;
        char Switch_2;
        char Switch_3;
        char LedEnable;
        char PowerOffProtection;
        char KeyMode;
    } dev_HY0107_t; //3路智能开关模块（HY0107，型号IHC1240）

    typedef struct
    {
        char Switch_1;
        char Switch_2;
        char Switch_3;
        char LedEnable;
        char PowerOffProtection;
    } dev_500c33_t; //U2/天际系列：三键智能开关（HY0097）
    //----------------------------------
    typedef struct
    {
        char ContactAlarm;
        char BatteryPercentage;
        char LowBatteryAlarm;
        char TamperAlarm;
    } dev_HY0093_t; //门磁传感器（HY0093，型号IHG5201）

    typedef struct
    {
        char KeyFobValue;
        char SceName[12][18];
        char Enable[3];
        char Switch[3];
        char CurrentTemperature_1;
        char TargetTemperature_1;
        char WorkMode_1;
        char WindSpeed[2];
        char TargetTemperature_3;
    } dev_HY0134_t; //U2/天际系列：智镜/全面屏/触控屏（HY0134）

    void dev_private_attribute(dev_data_t *dev, cJSON *Data);
    void dev_private_event(dev_data_t *dev, cJSON *Data);
#ifdef __cplusplus
}
#endif
#endif