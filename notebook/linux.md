#mount
mount -t nfs -o nolock -o tcp -o rsize=32768,wsize=32768 192.168.20.188:/home/ppq/linux/nfs /var/nfs
mount -o nolock,tcp -t nfs 192.168.1.188:/home/ppq/nfs /userdata/nfs

#uboot
setenv netbootcmd 'tftp 80800000 zImage; tftp 83000000 imx6ull-alientek-evk.dtb; bootz 80800000 - 83000000'
setenv ipaddr 192.168.0.88
setenv ethaddr 00:04:9f:04:d2:35 
setenv gatewayip 192.168.0.1
setenv netmask 255.255.255.0
setenv serverip 192.168.0.188 
saveenv

# bootargs
setenv bootargs mem=80M console=ttyAMA0,115200 root=/dev/mtdblock2 rootfstype=cramfs mtdparts=xm_sfc:256K(boot),1536K(kernel),1280K(romfs),4544K(user),256K(custom),8512k(mtd)
setenv bootargs mem=40M console=ttyAMA0,115200 root=/dev/mtdblock2 rootfstype=cramfs mtdparts=xm_sfc:256K(boot),1536K(kernel),1280K(romfs),4544K(user),256K(custom),320K(mtd)
setenv bootargs 'console=ttymxc0,115200 root=/dev/nfs rootwait rw nfsroot=192.168.0.188:/home/ppq/linux/nfs/rootfs ip=192.168.0.88:192.168.0.188:192.168.0.1:255.255.255.0::eth0:off'


#printk
echo 5 > /proc/sys/kernel/printk

#compile
./configure --prefix=/home/ppq/gcc-linaro-4.9 CFLAGS+=-mfloat-abi=softfp
./config --prefix=`pwd`/_install
./configure --prefix=`pwd`/_install -with-ssl=/home/ppq/tmp/openssl-1.1.1g/_install
./configure --prefix=`pwd`/_install --host=armv8l-linux-gnueabihf CC=armv8l-linux-gnueabihf-gcc


#LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/pulse-11.1/modules
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/userdata:/userdata/lib
echo $LD_LIBRARY_PATH

#网络抓包
tcpdump -i eth0 tcp port 5683 -w log.cap

#filesystem
mount -t ext2fs /dev/mmcblk0 /var/sd
mount -t ext2 /dev/mmcblk0 /var/sd
mount -t f2fs /dev/mmcblk0 /var/sd
./busybox mkfs.vfat /dev/mmcblk0
./busybox mkfs.ext2 /dev/mmcblk0
./busybox mkfs.f2fs /dev/mmcblk0





