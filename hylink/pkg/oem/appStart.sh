#!/bin/sh

echo "start app..."

rmmod rkled
rmmod rkkeyasync

insmod /oem/rkled.ko
insmod /oem/rkkeyasync.ko

killall dnsmasq
rfkill block all

ALL_APP="hydaemon hy_server_iot hydevapp"
killall $ALL_APP

UPDATE_FILE="/userdata/update/upgrade.bin"

if [ -e "$UPDATE_FILE" ]; then
    echo Power on find upgrade
    cd /tmp
    chmod -R 777 $UPDATE_FILE
    $UPDATE_FILE
    # reboot
    echo Power on rm upgrade
    rm $UPDATE_FILE
 
fi

cd /userdata/hyapp
./hydevapp &

cd /userdata/iotapp
./hy_server_iot &

cd /userdata/app

if [ -e "/userdata/app/alinkapp" ];then
    echo "run alink"
    killall ailinkapp
    ./ailinkapp &
else if [ -e "/userdata/app/hlinkapp" ];then
    echo "run hilink"
    killall hilinkapp
    ./hilinkapp &
else
    echo "app not exist"
fi

/userdata/app/hydaemon
