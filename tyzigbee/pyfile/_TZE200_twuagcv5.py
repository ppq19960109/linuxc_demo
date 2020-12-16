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

# print(ScePhoto_up('ABg=',2))
# print(SceName_up('ebtbtgAA',1))
# print(ScePhoto_down('5',2))
# print(ScePhoto_up('AAA=',1))
# print(CurrentTemperature_up('MQ==',1))
# print(TargetTemperature_down('1155',4))