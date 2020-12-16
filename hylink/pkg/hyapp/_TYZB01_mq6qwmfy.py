# -*- coding: UTF-8 -*-
STRING_Z3_SWITHC_ON= '01'
STRING_Z3_SWITHC_OFF = '00'
STRING_HY_SWITHC_OFF = '0'
STRING_HY_SWITHC_ON = '1'

STRING_Z3_LEN_ENABLE_CLOSE = '00'
STRING_Z3_LEN_ENABLE_ON = '01'
STRING_Z3_LEN_ENABLE_OFF = '02'

STRING_HY_LEN_ENABLE_CLOSE = '0'
STRING_HY_LEN_ENABLE_ON = '1'
STRING_HY_LEN_ENABLE_OFF = '2'


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


def LedEnable_up(inValue, valueLen):
    if inValue == STRING_Z3_LEN_ENABLE_CLOSE:
        inValue =STRING_HY_LEN_ENABLE_CLOSE
        valueLen = 1
    elif inValue ==STRING_Z3_LEN_ENABLE_ON:
        inValue = STRING_HY_LEN_ENABLE_ON
        valueLen = 1
    else:
        inValue = STRING_HY_LEN_ENABLE_OFF
        valueLen = 1
    return inValue, valueLen


def LedEnable_down(inValue, valueLen):
    outValue = '-1'
    payloadLen = 0
    if inValue == STRING_HY_LEN_ENABLE_CLOSE:
        outValue = STRING_Z3_LEN_ENABLE_CLOSE
        payloadLen = 2
    elif inValue == STRING_HY_LEN_ENABLE_ON:
        outValue = STRING_Z3_LEN_ENABLE_ON
        payloadLen = 2
    else:
        outValue = STRING_Z3_LEN_ENABLE_OFF
        payloadLen = 2
    return outValue, payloadLen


if __name__ == '__main__':
    str1=1
    leng=1
    print(isinstance(str1,int))
    print(isinstance(leng, int))

    (str1, leng) = Switch_1_up(1, 1)

    print (isinstance(str1,str))
    print(isinstance(leng, int))

    print("value= %s" %  str1)
    print("len=%d" % leng)
