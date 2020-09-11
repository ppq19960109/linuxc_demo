#!/bin/sh
#检测网络连接
echo "start app..."
while true
do
    ping -c 1 114.114.114.114 > /dev/null 2>&1
    if [ $? -eq 0 ];then
        echo "网络正常"
        break
    else
        echo "网络连接异常"
    fi  
    sleep 1
done

/userdata/iotapp/hy_server_iot > /dev/null &
/userdata/hyapp/hydevapp > /dev/null &
/userdata/app/hilinkapp &
