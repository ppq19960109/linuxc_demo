import base64

def num_up(inBase64,inLen):
    outBytes=base64.b64decode(inBase64.encode("utf-8"))
    sum=0
    for element in outBytes[::-1]:
        sum=sum*0xff+element
    return sum

def num_down(inNum,inLen):
    outList = [0]*inLen
    for i in range(inLen):
        outList[i]=(inNum >> 8*(inLen-i-1)) & 0xff
    enc=base64.b64encode(bytes(outList[::-1])).decode('utf-8')
    return enc
###########################################

def KeyModeup(inBase64,inLen):
    num=num_up(inBase64,inLen)+1
    strVal=str(num)
    return strVal,len(strVal)

def KeyModedown(inStrVal,inLen):
    outNum=int(inStrVal)-1
    outBase64=num_down(outNum,inLen)
    return outBase64,len(outBase64)

def PowerOffProtectionup(inBase64,inLen):
    num=num_up(inBase64,inLen)
    if num < 2:
        num = 1
    else:
        num = 0
    strVal=str(num)
    return strVal,len(strVal)

def PowerOffProtectiondown(inStrVal,inLen):
    outNum=int(inStrVal)
    if outNum > 0:
        outNum = 0
    else:
        outNum = 2
    outBase64=num_down(outNum,inLen)
    return outBase64,len(outBase64)
