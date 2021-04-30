
pub/bin/pc/mkfs.jffs2 -d pub/rootfs_glibc -l -e 0x10000 -o rootfs_glibc_64k.jffs2
pub/bin/pc/mkfs.jffs2 -d pub/rootfs_uclibc -l -e 0x10000 -o rootfs_glibc_64k.jffs2

tools/pc/jffs2_tool/mkfs.jffs2 -d rootfs_scripts/rootfs -l -e 0x10000 -o rootfs_scripts/rootfs_uclibc_64k.jffs2
--------------------------------
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

--------------------------------
#!/bin/sh

ifconfig eth0 192.168.1.199
route add default gw 192.168.1.1
telnetd&
./load3516ev300 -i -sensor0 imx335 -osmem 32M
./load3516ev300 -i -sensor0 imx335 -osmem 64M
./load3516ev300 remove_ko

vi /etc/resolv.conf
nameserver 114.114.114.114
nameserver 223.5.5.5
--------------------------------
mount -o nolock,tcp -t nfs 192.168.0.166:/home/ppq/nfs /nfsroot
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/nfsroot/hisi/libffmpeg/lib:/nfsroot/hisi/libx264/lib
./app b.264 rtsp://192.168.1.177:8554/test

--------------------------------
mkfifo -m 777 stream_chn0.264


export PATH=$PATH:/home/ppq/libs/ffmpeg-4.3.1/_install/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ppq/libs/ffmpeg-4.3.1/_install/lib:/home/ppq/libs/x264-master/_install/lib

export PKG_CONFIG_PATH=/home/ppq/mnt/hisi/ffmpeg/ffmpeg-4.2.4/doc/examples/pc-uninstalled
echo $PKG_CONFIG_PATH
pkg-config --cflags --libs

static char av_error[AV_ERROR_MAX_STRING_SIZE] = {0};

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/nfsroot/libopenssl