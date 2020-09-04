

import struct


def Switch_1_up(s, len):
    print(type(s), s)
    if s == 0:
        s = '0'
        len = 2
    else:
        s = '3'
        len = 3
    print(type(s), s)
    return s, len


def Switch_1_down(s, len):
    print(s, len)
    if s == '0':
        s = 0
        len = 2
    else:
        s = 1
        len = 3
    print(type(s), s)
    return s, len


def strTobytes(inVal):
    inVal=inVal.encode('GB2312')
    print(inVal,len(inVal))
    inVal = struct.unpack(str(len(inVal))+'s',inVal )
    inVal = bytearray(inVal[0])
    print(inVal,list(inVal))
    return inVal

def up1(inVal, inLen):
    inVal=strTobytes(inVal)

    # -------------------------------
    print(inVal[1],-3)
    inVal[2] = 22

    # -------------------------------
    out1Val = inVal
    out1Str = str(len(out1Val))+'s'
    print(out1Val, out1Str)
    out1 = struct.pack(out1Str, out1Val)

    out2Val = inVal
    out2Str = str(len(out2Val))+'s'
    print(out2Val, out2Str)
    out2 = struct.pack(out2Str, out2Val)

    return out1, out2


def down(inVal1, inLen1, inVal2, inLen2):
    inVal1 = inVal1.encode('utf-8')
    sinLen = str(inLen1)+'s'
    inVal1 = struct.unpack(sinLen, inVal1)
    inVal1 = bytearray(inVal1[0])
    print(inVal1, inLen1, sinLen)

    inVal2 = inVal2.encode('utf-8')
    sinLen = str(inLen2)+'s'
    inVal2 = struct.unpack(sinLen, inVal2)
    inVal2 = bytearray(inVal2[0])
    print(inVal2, inLen2, sinLen)
    # -------------------------------
    inVal1[2] = inVal1[1]+inVal2[2]

    # -------------------------------
    out1Val = inVal1
    out1Str = str(len(out1Val))+'s'
    print(out1Val, out1Str)
    out1 = struct.pack(out1Str, out1Val)

    return out1

def up(inVal):
    outVal=[123,234,0xc6]

    return outVal,outVal
# print('test:',Switch_1_down("0",1))
# print('test:',Switch_1_up(1,1))
print('test:',up('\x00\x8e\x6c\x01\x00\x01\x00'))
