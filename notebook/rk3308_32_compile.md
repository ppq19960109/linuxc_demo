uboot:
./make.sh rk3308-aarch32 
./make.sh evb-aarch32-rk3308
kernel:
make rk3308_linux_aarch32_defconfig
make ARCH=arm rk3308_linux_aarch32_debug_defconfig

make ARCH=arm rk3308-voice-module-board-v10-aarch32.img -j8

//Kernel hacking
CONFIG_DEBUG_INFO:n
//File systems
CONFIG_NFS_FS:y
CONFIGFS_FS
//Device Drivers
CONFIG_SCSI:y

	CONFIG_USB_EHCI_HCD: y
	CONFIG_USB20_HOST:   
	CONFIG_USB_GADGET: 
	USB_CONFIGFS
	USB_CONFIGFS_F_FS

CONFIG_MMC:
CONFIG_PHY_ROCKCHIP_INNO_USB2:
	
	CONFIG_NET_CORE:y
	CONFIG_ETHERNET: y
	CONFIG_USB_NET_CDCETHER: y
	CONFIG_RTL8723DS:y
	CONFIG_PHYLIB: 
//Networking support
CONFIG_NF_CONNTRACK:
CONFIG_NF_CONNTRACK_IPV4:
CONFIG_IP_NF_IPTABLES: 
CONFIG_IP_NF_NAT:

rootfs:

1、usrdata
	ln -s userdata usrdata
	mkdir app hyapp iotapp usrlibs
2、passwd
   root::
3、profile
	export PS1='[\u@\h \w]\$ '
	export PS1=’[\u@\h $PWD]$ '
	export PS1='[\u@\h /`pwd/`]$'
	
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/userdata:/userdata/app:/userdata/hyapp:/userdata/iotapp

4、python3 curl paho-mqtt-c lua 
5、hostapd 
	ACS
6、busybox telnet
7、CONFIG delete connman nfs rpcbind adbd 

	
misc.img:/device/rockchip/rockimg/blank-misc.img
rootfs.img:buildroot
parameter.txt:/device/rockchip/
recovery.img:buildroot
oem.img:/device/rockchip/
userdata.img:/device/rockchip/userdata
uboot.img:u-boot
boot.img:kernel
MiniLoaderAll.bin:u-boot
trust.img:u-boot

update ota update_ota.img 
./build.sh firmware
./build.sh updateimg
./build.sh recovery
make ARCH=arm savedefconfig

CONFIG_DM_VIDEO=y
CONFIG_OF_LIVE=y
make ARCH=arm rk3308_linux_aarch32_debug_defconfig
make ARCH=arm menuconfig
make ARCH=arm rk3308-voice-module-board-v10-aarch32.img -j8
/usr/bin/time -f "you take %E to build recovery" device/rockchip/common/mk-ramdisk.sh recovery.img rockchip_rk3308_recovery

update_config=1
network={
ssid="IoT-Test" 
psk="12345678" 
key_mgmt=WPA-PSK
}
wpa_supplicant -B -i wlan0 -c /data/cfg/wpa_supplicant.conf
wget http://192.168.1.199:8000/X50QML -O X50QML && chmod 777 X50QML
wget -c -r -np http://192.168.1.199:8000/x5/
python3 -m http.server
wget http://192.168.1.199:8000/x50.tar
wget http://192.168.1.199:8000/upgrade_1.0.0_x50.bin
cat /proc/sys/kernel/printk
echo 4 4 1 7 > /proc/sys/kernel/printk
export QT_QPA_PLATFORM=linuxfb:tty=/dev/fb0:rotation=270
killall X50app X50QML

pavucontrol
String params = "sample_rate=16000,data_type=audio"
Asia/Shanghai
export TZ='Asia/Shanghai'
echo 'update_config=1' >> /data/cfg/wpa_supplicant.conf
mount -o nolock,tcp -t nfs 192.168.1.199:/home/ppq/nfs /mnt
QT_QPA_FB_DRM=1

export QT_QPA_FB_DRM=0
export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt/plugins
export QT_PLUGIN_PATH=/usr/lib/qt/plugins
export QML2_IMPORT_PATH=/usr/qml
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:lib:/usr/lib:/usr/lib/qt/plugins/platforms
export QT_QPA_FONTDIR=/usr/share/fonts

export QT_QPA_FB_TSLIB=1
export QT_QPA_GENERIC_PLUGINS=tslib:/dev/input/event0
export POINTERCAL_FILE=/etc/pointercal

export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0
export TSLIB_TSDEVICE=/dev/input/event0
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_PLUGINDIR=/usr/lib/ts
export LD_PRELOAD=/usr/lib/libts.so
export TSLIB_CALIBFILE=/etc/pointercal

export QT_QPA_EGLFS_ROTATION=90
echo 1 > /sys/class/graphics/fb0/rotate
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS=/dev/input/event0:rotate=270:invertx
export QT_QPA_GENERIC_PLUGINS=evdevtouch:/dev/input/event0

e6 e6 00 00 03 00 02 05 01 00 00 6e 6e //系统开关打开
e6 e6 00 00 03 00 02 31 01 00 00 6e 6e //烟机风速1档
e6 e6 00 00 03 00 02 32 01 00 00 6e 6e //烟机照明打开
e6 e6 00 00 04 00 02 07 01 00 00 6e 6e //配网动作打开
e6 e6 00 00 04 00 02 07 00 00 00 6e 6e
e6 e6 00 00 04 00 02 f1 a1 00 00 6e 6e
e6 e6 00 00 04 00 02 f1 a4 00 00 6e 6e //恢复出厂
e6 e6 00 00 04 00 02 f1 03 00 00 6e 6e //产测开始
e6 e6 00 00 03 00 02 11 01 00 00 6e 6e //左灶状态
e6 e6 00 00 03 00 02 12 01 00 00 6e 6e //右灶状态
e6 e6 00 00 03 00 07 0a 00 00 00 09 0b 09 00 00 6e 6e
e6 e6 00 00 03 00 0a 42 04 43 26 45 00 66 47 00 74 00 00 6e 6e
e6 e6 00 00 03 00 0a 42 03 43 26 45 00 66 47 00 74 00 00 6e 6e //左腔
e6 e6 00 00 03 00 08 52 03 54 00 44 56 00 24 00 00 6e 6e //右腔
e6 e6 00 00 03 00 0d 42 01 43 26 45 00 66 47 00 24 49 00 72 00 00 6e 6e //左腔预约
e6 e6 00 00 03 00 0b 52 01 54 00 66 56 00 24 58 00 61 00 00 6e 6e //右腔预约
e6 e6 00 61 03 00 2a f6  00 05 01 13 01 14 00 31 01 32 00 34 0a 38 00 42  04 43 00 44 00 00 45 00 48 46 00 00 47 00 00 48  00 00 49 00 00 4a 00 4b 01 47 2f 6e 6e
HAS_LIBEGL
MALI_T76X
RPI_USERLAND
make qt5graphicaleffects-dirclean
make qt5graphicaleffects-rebuild

apt-cache madison ffmpeg