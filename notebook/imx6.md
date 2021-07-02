#!/bin/sh
ifconfig eth0 up
udhcpc -b
telnetd&

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig

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

run findfdt;mmc dev ${mmcdev};mmc dev ${mmcdev}; if mmc rescan; then if run loadbootscript; then run bootscript; else if run loadimage; then run mmcboot; else run netboot; fi; fi; else run netboot; fi

setenv bootargs 'console=ttymxc0,115200 root=/dev/mmcblk1p2 rootwait rw'
setenv bootcmd 'mmc dev 1; fatload mmc 1:1 80800000 zImage; fatload mmc 1:1 83000000 imx6ull-14x14-emmc-4.3-480x272-c.dtb; bootz 80800000 - 83000000;'
saveenv
tar -vcjf rootfs.tar.bz2 *

mount -o nolock,tcp -t nfs 192.168.0.166:/home/ppq/nfs /mnt
tftp -p -r mic_in_config.sh 192.168.0.166
tftp -g -r mic_in_config.sh 192.168.0.166
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib:/usr/lib

export TSLIB_ROOT=/usr/lib/arm-tslib
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0
export TSLIB_TSDEVICE=/dev/input/event1
export TSLIB_CONFFILE=$TSLIB_ROOT/etc/ts.conf
export TSLIB_PLUGINDIR=$TSLIB_ROOT/lib/ts
export LD_PRELOAD=$TSLIB_ROOT/lib/libts.so
export TSLIB_CALIBFILE=/etc/pointercal

export QT_ROOT=/arm-qt
export QT_QPA_GENERIC_PLUGINS=tslib:/dev/input/event1
export QT_QPA_PLATFORM_PLUGIN_PATH=$QT_ROOT/plugins
export QT_QPA_PLATFORM=linuxfb:tty=/dev/fb0
export QT_PLUGIN_PATH=$QT_ROOT/plugins
export LD_LIBRARY_PATH=$QT_ROOT/lib:$QT_ROOT/plugins/platforms
export QML2_IMPORT_PATH=$QT_ROOT/qml
export QT_QPA_FB_TSLIB=1
export QT_QPA_FONTDIR=$QT_ROOT/lib/fonts

export QT_QPA_FONTDIR=/usr/share/fonts

./configure -prefix /home/ppq/qt/qt-everywhere-src-5.12.9/arm-qt \
-opensource \
-confirm-license \
-release \
-strip \
-shared \
-xplatform linux-arm-gnueabi-g++ \
-optimized-qmake \
-c++std c++11 \
--rpath=no \
-pch \
-skip qt3d \
-skip qtactiveqt \
-skip qtandroidextras \
-skip qtcanvas3d \
-skip qtconnectivity \
-skip qtdatavis3d \
-skip qtdoc \
-skip qtgamepad \
-skip qtlocation \
-skip qtmacextras \
-skip qtnetworkauth \
-skip qtpurchasing \
-skip qtremoteobjects \
-skip qtscript \
-skip qtscxml \
-skip qtsensors \
-skip qtspeech \
-skip qtsvg \
-skip qttools \
-skip qttranslations \
-skip qtwayland \
-skip qtwebengine \
-skip qtwebview \
-skip qtwinextras \
-skip qtx11extras \
-skip qtxmlpatterns \
-make libs \
-make examples \
-nomake tools -nomake tests \
-gui \
-widgets \
-dbus-runtime \
--glib=no \
--iconv=no \
--pcre=qt \
--zlib=qt \
-no-openssl \
--freetype=qt \
--harfbuzz=qt \
-no-opengl \
-linuxfb \
--xcb=no \
-tslib \
--libpng=qt \
--libjpeg=qt \
--sqlite=qt \
-plugin-sql-sqlite \
-I/home/ppq/qt/tslib-1.22/arm-tslib/include \
-L/home/ppq/qt/tslib-1.22/arm-tslib/lib \
-recheck-all
