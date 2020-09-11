# -*- coding: UTF-8 -*-
STRING_Z3_SWITHC_ON= '01'
STRING_Z3_SWITHC_OFF = '00'
STRING_HY_SWITHC_OFF = '0'
STRING_HY_SWITHC_ON = '1'

STRING_Z3_LEVEL_CTRL_CMD_ID = '05'
STRING_Z3_LEVEL_CTRL_TRANSITION_TIME = 'FFFF'

STRING_Z3_COLOR_TEMPERATURE_CTRL_CMD_ID = '0A'
STRING_Z3_COLOR_TEMPERATURE_CTRL_TRANSITION_TIME = '000A'


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
    outValue = str(int(100 * int(inValue, 16) / 254))
    return outValue, len(outValue)


def Luminance_down(inValue, valueLen):
    outValue = str(hex(int(254 * int(inValue) / 100))[2:])
    outbuff = STRING_Z3_LEVEL_CTRL_CMD_ID+outValue+STRING_Z3_LEVEL_CTRL_TRANSITION_TIME
    return outbuff, len(outbuff)


def ColorTemperature_up(inValue, valueLen):
    tmp = str(100*inValue/254)
    valueLen = len(tmp)
    return tmp, valueLen


def ColorTemperature_down(inValue, valueLen):
    outValue = str(hex(int(254 * int(inValue) / 100))[2:])
    outbuff = STRING_Z3_COLOR_TEMPERATURE_CTRL_CMD_ID+outValue+STRING_Z3_COLOR_TEMPERATURE_CTRL_TRANSITION_TIME
    return outbuff, len(outbuff)


if __name__ == '__main__':
    inValue = '50'
    outValue = str(int(100 * int(inValue) / 254))
    print(outValue)