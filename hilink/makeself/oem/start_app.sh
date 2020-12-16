#!/bin/sh

echo "start app..."
# while true
# do
#     ping -c 1 114.114.114.114 > /dev/null 2>&1
#     if [ $? -eq 0 ];then
#         echo "network normal"
#         break
#     else
#         echo "network anomaly!"
#     fi  
#     sleep 1
# done

killall dnsmasq dhcpcd 
rfkill block wifi
dhclient -cf /userdata/app/dhclient.conf -nw

rmmod rkled
rmmod rkkeyasync

insmod /oem/rkled.ko
insmod /oem/rkkeyasync.ko


ALL_APP="runing.sh hyDaemon hilinkapp hy_server_iot hydevapp"
killall $ALL_APP

UPDATE_FILE="/userdata/update/upgrade.bin"

if [ -e "$UPDATE_FILE" ]; then
    echo Power on find upgrade
    cd /tmp
    chmod -R 777 $UPDATE_FILE
    $UPDATE_FILE
    # if [ $? != 0 ];then
    echo Power on rm upgrade
    rm $UPDATE_FILE
    # fi
fi

sleep 1

cd /userdata/hyapp
./hydevapp &

cd /userdata/iotapp
./hy_server_iot &

cd /userdata/app
./hilinkapp &

# /oem/runing.sh &
/userdata/app/hyDaemon

