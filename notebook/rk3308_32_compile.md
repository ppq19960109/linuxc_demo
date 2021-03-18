uboot:
./make.sh rk3308-aarch32 
./make.sh evb-aarch32-rk3308
kernel:
make rk3308_linux_aarch32_defconfig
make rk3308_linux_aarch32_debug_defconfig

make rk3308-voice-module-board-v10-aarch32.img

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
