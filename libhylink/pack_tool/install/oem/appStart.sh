#!/bin/sh

echo "start app..."

rmmod rkled
rmmod rkkeyasync

insmod /oem/rkled.ko
insmod /oem/rkkeyasync.ko

killall dnsmasq
rfkill block all

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
./hydevapp > /dev/null &

cd /userdata/iotapp
./hy_server_iot > /dev/null &

cd /userdata/app

if [ -e "/userdata/app/alinkapp" ];then
    echo "run alink"
    killall alinkapp
    ./alinkapp &
fi

if [ -e "/userdata/app/hlinkapp" ];then
    echo "run hilink"
    killall hilinkapp
    ./hilinkapp &
fi

/userdata/app/hydaemon
