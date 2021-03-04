import base64

def Temperature_up(inBase64,inLen):
    outBytes=base64.b64decode(inBase64.encode("utf-8"))
    sum=0
    for element in outBytes:
        sum=sum*0xff+element
    return str(sum)

def Temperature_down(inStrVal,inLen):
    outNum=int(inStrVal)
    outList = [0]*inLen
    for i in range(inLen):
        outList[i]=(outNum >> 8*(inLen-i-1)) & 0xff
    enc=base64.b64encode(bytes(outList)).decode('utf-8')
    return enc
###########################################
def CurrentTemperature_up(inBase64,inLen):
    strVal=Temperature_up(inBase64,inLen)
    return strVal,len(strVal)

def TargetTemperature_up(inBase64,inLen):
    strVal=Temperature_up(inBase64,inLen)
    return strVal,len(strVal)

def TargetTemperature_down(inStrVal,inLen):
    outBase64=Temperature_down(inStrVal,inLen)
    return outBase64,len(outBase64)

def ScePhoto_up(inBase64,inLen):
    outBytes=base64.b64decode(inBase64.encode("utf-8"))
    sum=0
    for element in outBytes:
        sum=sum*0xff+element
    return str(sum),1

def ScePhoto_down(inStrVal,inLen):
    inLen=2
    outNum=int(inStrVal)
    outList = [0]*inLen
    for i in range(inLen):
        outList[i]=(outNum >> 8*(inLen-i-1)) & 0xff
    enc=base64.b64encode(bytes(outList)).decode('utf-8')
    return enc,len(enc)

def SceName_up(inBase64,inLen):
    dec=base64.b64decode(inBase64.encode("utf-8"))
    print(dec)
    enc=dec.decode('utf-16be')
    enc=enc.strip('\x00')
    return enc,len(enc)

def SceName_down(inStrVal,inLen):
    enc=base64.b64encode(inStrVal.encode("utf-16be"))
    dec=enc.decode('utf-8')
    return dec,len(dec)

def WorkMode_up(inBase64,inLen):
    outBytes=base64.b64decode(inBase64.encode("utf-8"))
    mode=int.from_bytes(outBytes,'little')
    if mode == 2:
        mode = 3
    elif mode == 3:
        mode = 2 
    mode+=1
    strVal= str(mode)
    return strVal,len(strVal)

def WorkMode_down(inStrVal,inLen):
    mode=int(inStrVal)
    if mode == 3:
        mode = 4
    elif mode == 4:
        mode = 3 
    mode-=1
    outBase64=base64.b64encode(mode.to_bytes(1,'little')).decode('utf-8')
    return outBase64,len(outBase64)

def WindSpeed_up(inBase64,inLen):
    outBytes=base64.b64decode(inBase64.encode("utf-8"))
    speed=int.from_bytes(outBytes,'little') 
    speed+=2
    strVal= str(speed)
    return strVal,len(strVal)

def WindSpeed_down(inStrVal,inLen):
    speed=int(inStrVal)
    speed-=2
    outBase64=base64.b64encode(speed.to_bytes(1,'little')).decode('utf-8')
    return outBase64,len(outBase64)

def KeyFobValueup(inBase64,inLen):
    outBytes=base64.b64decode(inBase64.encode("utf-8"))
    key=int.from_bytes(outBytes,'little') 
    key+=1
    strVal= str(key)
    return strVal,len(strVal)

# print(WindSpeed_up('AQ==',1))
# print(WorkMode_down('2',1))
# print(ScePhoto_up('ABg=',2))
# print(SceName_up('ebtbtgAA',1))
# print(ScePhoto_down('5',2))
# print(ScePhoto_up('AAA=',1))
# print(CurrentTemperature_up('MQ==',1))
# print(TargetTemperature_down('1155',4))