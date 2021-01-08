# -*- coding: UTF-8 -*-
STRING_Z3_SWITHC_ON= '01'
STRING_Z3_SWITHC_OFF = '00'
STRING_HY_SWITHC_OFF = '0'
STRING_HY_SWITHC_ON = '1'

STRING_Z3_LEVEL_CTRL_CMD_ID = '00'
STRING_Z3_LEVEL_CTRL_TRANSITION_TIME = 'FFFF'

STRING_Z3_COLOR_TEMPERATURE_CTRL_CMD_ID = '0A'
STRING_Z3_COLOR_TEMPERATURE_CTRL_TRANSITION_TIME = 'FFFF'


def Switch_up(inValue, valueLen):
    if inValue == STRING_Z3_SWITHC_OFF:
        inValue = STRING_HY_SWITHC_OFF
        valueLen = 1
    else:
        inValue = STRING_HY_SWITHC_ON
        valueLen = 1
    return inValue, valueLen


def Switch_down(inValue, valueLen):
    cmdId = '-1'
    payloadLen = 0
    if inValue == STRING_HY_SWITHC_OFF:
        cmdId = STRING_Z3_SWITHC_OFF  # 关闭
        payloadLen = 2
    else:
        cmdId = STRING_Z3_SWITHC_ON  # 打开
        payloadLen = 2
    return cmdId, payloadLen


def Luminance_up(inValue, valueLen):
    if inValue=='E6':
        outValue='90'
    elif inValue=='EB':
        outValue = '92'
    else:
        outValue = str(round(100 * int(inValue, 16) / 254))
    return outValue, len(outValue)


def Luminance_down(inValue, valueLen):

    if inValue == '72':
        outValue = 'B8'
    elif inValue == '74':
        outValue = 'BD'
    elif inValue == '83':
        outValue = 'D4'
    elif inValue == '85':
        outValue = 'D9'
    elif inValue == '87':
        outValue = 'DE'
    elif inValue == '92':
        outValue = 'EB'
    elif inValue == '94':
        outValue = 'F0'
    elif inValue == '96':
        outValue = 'F5'
    elif inValue == '98':
        outValue = 'FA'
    else:
        outValue = str(hex(int(254 * int(inValue) / 100)))
        if len(outValue)==3:
            outValue='0'+outValue[2:]
        else:
            outValue=outValue[2:]
    outbuff = STRING_Z3_LEVEL_CTRL_CMD_ID+outValue+STRING_Z3_LEVEL_CTRL_TRANSITION_TIME
    return outbuff, len(outbuff)


def ColorTemperature_up(inValue, valueLen):
    tmp = round(10000/(217*(255-int(inValue[0:2], 16))/255+153))
    if tmp>=65:
        tmp=65
    elif tmp<=27:
        tmp=27
    outValue=str(tmp*100)
    valueLen = len(outValue)
    return outValue, valueLen


def ColorTemperature_down(inValue, valueLen):
    outValue = str(hex(int(255 - 255 * int(1000000 / int(inValue) - 153) / 217)))
    if 3 == len(outValue):
        outValue = '0'+outValue[2:]+'00'
    else:
        outValue=outValue[2:]+'00'
    outbuff = STRING_Z3_COLOR_TEMPERATURE_CTRL_CMD_ID+outValue+STRING_Z3_COLOR_TEMPERATURE_CTRL_TRANSITION_TIME
    return outbuff, len(outbuff)


if __name__ == '__main__':
    outValue = round(10000/(217*(255-int('76', 16))/255+153))
    print(outValue)
