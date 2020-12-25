import base64

def num_up(inBase64,inLen):
    outBytes=base64.b64decode(inBase64.encode("utf-8"))
    sum=0
    for element in outBytes:
        sum=sum*0xff+element
    return str(sum)

###########################################

def ContactAlarm_up(inBase64,inLen):
    strVal=num_up(inBase64,inLen)
    return strVal,len(strVal)

def TamperAlarm_up(inBase64,inLen):
    strVal=num_up(inBase64,inLen)
    return strVal,len(strVal)

def LowBatteryAlarm_up(inBase64,inLen):
    strVal=num_up(inBase64,inLen)
    return strVal,len(strVal)
