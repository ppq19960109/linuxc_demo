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

insmod /oem/rkled.ko
insmod /oem/rkkeyasync.ko

killall dnsmasq
rfkill block all

sleep 2

cd /userdata/iotapp
./hy_server_iot > /dev/null &
cd /userdata/hyapp
./hydevapp > /dev/null &
cd /userdata/app
./hilinkapp &
