#!/bin/sh

APP_PATH="/userdata/app"
APP="hilinkapp"
HYAPP_PATH="/userdata/hyapp"
HYAPP="hydevapp"
IOTAPP_PATH="/userdata/iotapp"
IOTAPP="hy_server_iot"


function check_process()
{
    count=`ps | grep $1 | grep -v "grep" | wc -l`
    if [ $count -eq 0 ];then
        # echo $1" process not exist"
        cd $2
        chmod 777 $1
        ./$1 &
    elif  [ $count -gt 1 ];then
        # echo $1" process exist > 1"
        killall $1
        cd $2
        chmod 777 $1
        ./$1 &
    else
        # echo $1" process exist"
        :
    fi
}

while true
do
    check_process $APP $APP_PATH
    check_process $HYAPP $HYAPP_PATH
    check_process $IOTAPP $IOTAPP_PATH
    sleep 60
done
