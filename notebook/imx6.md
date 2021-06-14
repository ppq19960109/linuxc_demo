#!/bin/sh
ifconfig eth0 up
udhcpc -b
telnetd&


setenv ipaddr 192.168.0.199
setenv ethaddr 00:04:9f:04:d2:35 
setenv gatewayip 192.168.0.1
setenv netmask 255.255.255.0
setenv serverip 192.168.0.166
saveenv

setenv bootargs 'console=ttymxc0,115200 root=/dev/nfs nfsroot=192.168.0.166:/home/ppq/nfs/rootfs,proto=tcp rw ip=192.168.0.199:192.168.0.166:192.168.0.1:255.255.255.0::eth0:off'

setenv bootargs 'console=ttymxc0,115200 root=/dev/mmcblk1p2 rootwait rw'
setenv bootcmd 'tftp 80800000 zImage; tftp 83000000 imx6ull-ppq-evk.dtb; bootz 80800000 - 83000000'
saveenv

setenv bootargs 'console=ttymxc0,115200 root=/dev/mmcblk1p2 rootwait rw'
setenv bootcmd 'mmc dev 1; fatload mmc 1:1 80800000 zImage; fatload mmc 1:1 83000000 imx6ull-14x14-emmc-4.3-480x272.dtb; bootz 80800000 - 83000000;'
saveenv
tar -vcjf rootfs.tar.bz2 *

mount -o nolock,tcp -t nfs 192.168.0.166:/home/ppq/nfs /mnt

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib:/usr/lib

export TSLIB_ROOT=/arm-tslib
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0
export TSLIB_TSDEVICE=/dev/input/event1
export TSLIB_CONFFILE=$TSLIB_ROOT/etc/ts.conf
export TSLIB_PLUGINDIR=$TSLIB_ROOT/lib/ts
export LD_PRELOAD=$TSLIB_ROOT/lib/libts.so
export QT_QPA_FB_TSLIB=1
export TSLIB_CALIBFILE=/etc/pointercal
