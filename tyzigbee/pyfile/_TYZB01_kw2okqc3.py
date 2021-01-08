import base64

def num_up(inBase64,inLen):
    outBytes=base64.b64decode(inBase64.encode("utf-8"))
    sum=0
    for element in outBytes:
        sum=sum*0xff+element
    return sum

###########################################
def BatteryPercentageup(inBase64,inLen):
    num=num_up(inBase64,inLen)
    num=round(num/2)
    strVal=str(num)
    return strVal,len(strVal)

def ContactAlarmup(inBase64,inLen):
    num=num_up(inBase64,inLen)
    num=(num >> 0)& 0x01
    strVal=str(num)
    return strVal,len(strVal)

def TamperAlarmup(inBase64,inLen):
    num=num_up(inBase64,inLen)
    num=(num >> 2)& 0x01
    strVal=str(num)
    return strVal,len(strVal)

def LowBatteryAlarmup(inBase64,inLen):
    num=num_up(inBase64,inLen)
    num=(num >> 3)& 0x01
    strVal=str(num)
    return strVal,len(strVal)
    