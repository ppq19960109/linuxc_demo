#!/bin/sh

APP_PATH="/userdata/app"
APP="hilinkapp"
HYAPP_PATH="/userdata/hyapp"
HYAPP="hydevapp"
IOTAPP_PATH="/userdata/iotapp"
IOTAPP="hy_server_iot"
UPGRADE_BACKUP="/userdata/update/upgrade_backup.bin"

function check_process()
{
    count=`ps | grep $1 | grep -v "grep" | wc -l`
    if [ $count -eq 0 ];then
        # echo $1" process not exist"
        cd $2
        if [ ! -e $1 ]; then
            cd /tmp
            chmod 777 $UPGRADE_BACKUP
            $UPGRADE_BACKUP
        fi 
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
    check_process $HYAPP $HYAPP_PATH
    check_process $IOTAPP $IOTAPP_PATH
    check_process $APP $APP_PATH
    sleep 60
done
