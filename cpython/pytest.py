
import struct


def add(a, b):
    print("agr1 %d agr2 %s" % (a, b))
    return a, "hi", (1, 23)

# print('add:',add(1,"good"))
# print('test',hex(12))


def ascii_arithmetic(inStr, addVal):
    out = bytes(inStr, 'ascii')[0]+addVal
    return out.to_bytes(1, 'little').decode()


def hy_down(strVal, strLen):
    # 要注意把从C传过来的string转化为bytes，以便struct.unpack解析
    strVal = strVal.encode('utf-8')
    sLen = str(strLen)+'s'
    inVal = struct.unpack(sLen, strVal)
    print(inVal, strLen)

    lstr=[3,4,5,0,0,9]
    outVal=bytes(lstr)
    out1Str=str(len(outVal))+'s'
    print(outVal, out1Str)
    out1 = struct.pack(out1Str, outVal)

    # if strVal[0] == '\x03':
    #     print("==")
    # else:
    #     print("!=")

    # out=ascii_arithmetic('\x77',1)
    # print(out)

    # bstr = bytes(strVal, 'ascii')
    # print(bstr)
    # lstr = list(bstr)
    # lstr[0] = lstr[0]+1
    # print(lstr, bytes(lstr).decode())
    print("----------------end")
    return out1, out1


print(hy_down('\x01\x02\x03\x04', 4))
