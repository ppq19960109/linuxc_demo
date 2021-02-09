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

def Luminanceup(inBase64,inLen):
    num=round(num_up(inBase64,inLen)*100/255)
    strVal=str(num)
    return strVal,len(strVal)

def Luminancedown(inStrVal,inLen):
    outNum=round(int(inStrVal)/100*255)
    outBase64=num_down(outNum,inLen)
    return outBase64,len(outBase64)

def ColorTemperatureup(inBase64,inLen):
    num=num_up(inBase64,inLen)
    num = round(10000/(217*(255-num)/255+153))
    if num>=65:
        num=65
    elif num<=27:
        num=27
    strVal=str(num*100)
    return strVal,len(strVal)

def ColorTemperaturedown(inStrVal,inLen):
    outNum=int(inStrVal)
    outNum = round(255 - 255 *(1000000 / outNum - 153)/ 217)
    outBase64=num_down(outNum,inLen)
    return outBase64,len(outBase64)