# -*- coding: UTF-8 -*-
import json


def up(commandId, iBuff,inLen):
    outbuff=''
    if commandId==0x0500:
        ContactAlarm={"Key":"ContactAlarm","Value":"0"}
        TamperAlarm = {"Key": "TamperAlarm", "Value": "0"}
        LowBatteryAlarm = {"Key": "LowBatteryAlarm", "Value": "0"}
        alarmType=int(iBuff[0:2])
        jsonDict=''
        if alarmType == 1 or alarmType == 3:
            ContactAlarm["Value"]="1"
            jsonDict=[ContactAlarm,TamperAlarm,LowBatteryAlarm]
            outbuff = json.dumps(jsonDict)
        elif alarmType == 4 :
            TamperAlarm["Value"]="1"
            jsonDict=[ContactAlarm,TamperAlarm,LowBatteryAlarm]
            outbuff = json.dumps(jsonDict)
        elif alarmType == 5 :
            ContactAlarm["Value"] = "1"
            TamperAlarm["Value"]="1"
            jsonDict = [ContactAlarm, TamperAlarm, LowBatteryAlarm]
            outbuff = json.dumps(jsonDict)
        else:
            jsonDict = [ContactAlarm, TamperAlarm, LowBatteryAlarm]
            outbuff = json.dumps(jsonDict)
    else:
        outbuff=""
    return outbuff,len(outbuff)


def BatteryPercentage_up(inValue, valueLen):
    percent=int(inValue, 16)/2
    valuebuff=str(percent)
    valueLen=len(valuebuff)
    return valuebuff, valueLen


if __name__ == '__main__':
    [outbuff, len1] = up(0x0500, '0100', 4)
    print (outbuff)
    print (len1)

    [outbuff, len2] = BatteryPercentage_up("9A", 2)
    print (outbuff)
    print (len2)

