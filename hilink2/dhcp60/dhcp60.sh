#! /bin/sh
#检测网络连接
killall dnsmasq dhcpcd 
while true
do

ping -c 2 114.114.114.114 > /dev/null 2>&1
if [ $? -ne 0 ];then
    echo "network disconnection..."
    dhclient -cf /userdata/app/dhclient.conf -r
    dhclient -cf /userdata/app/dhclient.conf -nw
fi

sleep 4
done