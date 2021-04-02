docker run --net="host" -it ppq19960109/ubuntu:18.04

echo "deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic main restricted universe multiverse" > /etc/apt/sources.list &&
echo "deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-updates main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-updates main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-backports main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-backports main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-security main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-security main restricted universe multiverse" >> /etc/apt/sources.list && 
echo "deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-proposed main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-proposed main restricted universe multiverse" >> /etc/apt/sources.list 

echo "deb http://mirrors.163.com/ubuntu/  bionic main restricted universe multiverse" > /etc/apt/sources.list &&
echo "deb-src http://mirrors.163.com/ubuntu/  bionic main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb http://mirrors.163.com/ubuntu/  bionic-updates main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.163.com/ubuntu/  bionic-updates main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb http://mirrors.163.com/ubuntu/  bionic-backports main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.163.com/ubuntu/  bionic-backports main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb http://mirrors.163.com/ubuntu/  bionic-security main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.163.com/ubuntu/  bionic-security main restricted universe multiverse" >> /etc/apt/sources.list && 
echo "deb http://mirrors.163.com/ubuntu/  bionic-proposed main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.163.com/ubuntu/  bionic-proposed main restricted universe multiverse" >> /etc/apt/sources.list 

echo "deb http://mirrors.aliyun.com/ubuntu/ bionic main restricted universe multiverse" > /etc/apt/sources.list &&
echo "deb-src http://mirrors.aliyun.com/ubuntu/ bionic main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb http://mirrors.aliyun.com/ubuntu/ bionic-updates main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.aliyun.com/ubuntu/ bionic-updates main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb http://mirrors.aliyun.com/ubuntu/ bionic-backports main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.aliyun.com/ubuntu/ bionic-backports main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb http://mirrors.aliyun.com/ubuntu/ bionic-security main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.aliyun.com/ubuntu/ bionic-security main restricted universe multiverse" >> /etc/apt/sources.list && 
echo "deb http://mirrors.aliyun.com/ubuntu/ bionic-proposed main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.aliyun.com/ubuntu/ bionic-proposed main restricted universe multiverse" >> /etc/apt/sources.list 

echo "deb http://mirrors.aliyun.com/ubuntu/ trusty main multiverse restricted universe" > /etc/apt/sources.list &&
echo "deb http://mirrors.aliyun.com/ubuntu/ trusty-backports main multiverse restricted universe" >> /etc/apt/sources.list &&
echo "deb http://mirrors.aliyun.com/ubuntu/ trusty-proposed main multiverse restricted universe" >> /etc/apt/sources.list &&
echo "deb http://mirrors.aliyun.com/ubuntu/ trusty-security main multiverse restricted universe" >> /etc/apt/sources.list &&
echo "deb http://mirrors.aliyun.com/ubuntu/ trusty-updates main multiverse restricted universe" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.aliyun.com/ubuntu/ trusty main multiverse restricted universe" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.aliyun.com/ubuntu/ trusty-backports main multiverse restricted universe" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.aliyun.com/ubuntu/ trusty-proposed main multiverse restricted universe" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.aliyun.com/ubuntu/ trusty-security main multiverse restricted universe" >> /etc/apt/sources.list &&
echo "deb-src http://mirrors.aliyun.com/ubuntu/ trusty-updates main multiverse restricted universe" >> /etc/apt/sources.list
 
echo "deb https://mirrors.ustc.edu.cn/ubuntu/ bionic main restricted universe multiverse" > /etc/apt/sources.list &&
echo "deb https://mirrors.ustc.edu.cn/ubuntu/ bionic-updates main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb https://mirrors.ustc.edu.cn/ubuntu/ bionic-backports main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb https://mirrors.ustc.edu.cn/ubuntu/ bionic-security main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb https://mirrors.ustc.edu.cn/ubuntu/ bionic-proposed main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ bionic main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ bionic-updates main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ bionic-backports main restricted universe multiverse" >> /etc/apt/sources.list &&
echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ bionic-security main restricted universe multiverse" >> /etc/apt/sources.list && 
echo "deb-src https://mirrors.ustc.edu.cn/ubuntu/ bionic-proposed main restricted universe multiverse" >> /etc/apt/sources.list 

tools/pc/jffs2_tool/mkfs.jffs2 -d rootfs_scripts/rootfs -l -e 0x10000 -o rootfs_scripts/rootfs_uclibc_64k.jffs2
setenv bootargs 'mem=32M console=ttyAMA0,115200 root=/dev/mtdblock2 rootfstype=jffs2 rw mtdparts=hi_sfc:1M(boot),4M(kernel),25M(rootfs)'
setenv bootargs 'mem=64M console=ttyAMA0,115200 root=/dev/mtdblock2 rootfstype=jffs2 rw mtdparts=hi_sfc:1M(boot),4M(kernel),25M(rootfs)'
setenv bootargs 'mem=128M console=ttyAMA0,115200 root=/dev/mtdblock2 rootfstype=jffs2 rw mtdparts=hi_sfc:1M(boot),4M(kernel),25M(rootfs)'
setenv bootcmd 'sf probe 0;sf read 0x42000000 0x100000 0x400000;bootm 0x42000000'
setenv serverip 192.168.1.45
setenv ipaddr 192.168.1.99
setenv ethaddr 5E:6A:F9:E6:8B:28
setenv netmask 255.255.255.0
setenv gatewayip 192.168.1.1

ifconfig eth0 hw ether xx:xx:xx:xx:xx:xx;
ifconfig eth0 192.168.1.199 netmask xx.xx.xx.xx;
route add default gw 192.168.1.1

mount -o nolock,tcp -t nfs 192.168.1.177:/home/ppq/nfs /nfsroot
#!/bin/sh

ifconfig eth0 192.168.1.199
route add default gw 192.168.1.1
telnetd&
./load3516ev300 -i -sensor0 imx335 -osmem 32M

remove_ko

export PATH="/opt/hisi-linux/x86-arm/arm-himix100-linux/bin:$PATH"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/hisi-linux/x86-arm/arm-himix100-linux/lib


export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/nfsroot/hisi/libffmpeg/lib:/nfsroot/hisi/libx264/lib
./app b.264 rtsp://192.168.1.177:8554/test

