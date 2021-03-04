#! /bin/sh
#检测网络连接
killall dnsmasq dhcpcd 
wait_net_connect=0
while true
do
ping -c 2 114.114.114.114 > /dev/null 2>&1
if [ $? -ne 0 ];then
    echo "network disconnection..."
    if ((wait_net_connect==0));then
        echo "wait_net_connect.."
        wait_net_connect=1
        dhclient -cf /userdata/app/dhclient.conf -r
        dhclient -cf /userdata/app/dhclient.conf -nw
    fi
else
    wait_net_connect=0
fi

sleep 2
done