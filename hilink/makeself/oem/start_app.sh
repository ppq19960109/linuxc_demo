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
rmmod rkled
rmmod rkkeyasync

insmod /oem/rkled.ko
insmod /oem/rkkeyasync.ko

killall dnsmasq
rfkill block all

killall hilinkapp
killall hydevapp
killall hy_server_iot
killall runing.sh

sleep 1

cd /userdata/iotapp
./hy_server_iot > /dev/null &
cd /userdata/hyapp
./hydevapp > /dev/null &
cd /userdata/app
./hilinkapp &
sleep 1
/oem/runing.sh &
