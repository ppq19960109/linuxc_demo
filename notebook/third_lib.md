openssl:

./config no-asm shared --prefix=$PWD/install --cross-compile-prefix=arm-none-linux-gnueabi-
./config no-asm no-async shared --prefix=$PWD/_install --cross-compile-prefix=arm-himix100-linux-
./config no-asm -shared --prefix=$(pwd)/_install
makefile中搜索-m64选项并删除，共两处
make CROSS_COMPILE=arm-rockchip-linux-gnueabihf-
make CROSS_COMPILE=/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-
sqlite3:
./configure --prefix=`pwd`/_install --host=arm-rockchip-linux-gnueabihf CC=arm-rockchip-linux-gnueabihf-gcc
./configure --prefix=`pwd`/_install --host=mipsel-openwrt-linux CC=/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-gcc

paho-mqtt-c:
cmake .. -DCMAKE_INSTALL_PREFIX=$(pwd)/_install -DPAHO_BUILD_STATIC=TRUE -DCMAKE_C_COMPILER=arm-rockchip-linux-gnueabihf-gcc -DPAHO_WITH_SSL=TRUE -DCMAKE_C_FLAGS+=/home/ppq/git/openssl/_install/include -DCMAKE_LIBRARY_PATH+=/home/ppq/git/openssl/_install/lib
cmake .. -DCMAKE_INSTALL_PREFIX=$(pwd)/_install -DPAHO_BUILD_STATIC=TRUE -DCMAKE_C_COMPILER=/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-gcc -DPAHO_WITH_SSL=TRUE -DOPENSSL_ROOT_DIR=/home/ppq/lib_u86/openssl-1.1.1i/_install 

curl:
--without-zlib --without-ssl
./configure --prefix=$(pwd)/_install --host=arm-rockchip-linux-gnueabihf CC=arm-rockchip-linux-gnueabihf-gcc --with-ssl=/home/ppq/git/openssl/_install
./configure --prefix=`pwd`/_install --host=mipsel-openwrt-linux CC=/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-gcc --without-zlib --without-ssl --with-ssl='/home/ppq/lib_u86/openssl-1.1.1i/_install' --without-zlib
python3:
./configure --prefix=`pwd`/_install --host=mipsel-openwrt-linux --build=mips CC=mipsel-openwrt-linux-gcc --enable-shared --disable-ipv6 ac_cv_file__dev_ptc=no ac_cv_file__dev_ptmx=no --enable-optimizations

ffmpeg:
pkg-config --exists librtmp||echo no

export PKG_CONFIG_PATH=/home/ppq/x265_3.2.1/build/_install/lib/pkgconfig:/home/ppq/git/rtmpdump-2.3/librtmp/_install/lib/pkgconfig:/home/ppq/openssl-1.0.1u/_install/lib/pkgconfig

./configure --prefix=`pwd`/_install --enable-shared --enable-nonfree --enable-gpl --disable-asm \
--enable-libx264 --enable-libx265 --enable-libmp3lame --enable-libxvid --enable-libfdk-aac --enable-openssl --enable-librtmp \
--extra-cflags='-I/home/ppq/git/x264-master/_install/include -I/home/ppq/lame-3.100/_install/include -I/home/ppq/xvidcore/build/generic/_install/include -I/home/ppq/fdk-aac-2.0.1/_install/include -I/home/ppq/x265_3.2.1/build/_install/include  -I/home/ppq/openssl-1.0.1u/_install/include  -I/home/ppq/git/rtmpdump-2.3/librtmp/_install/include' \
--extra-ldflags='-L/home/ppq/git/x264-master/_install/lib -L/home/ppq/lame-3.100/_install/lib -L/home/ppq/xvidcore/build/generic/_install/lib -L/home/ppq/fdk-aac-2.0.1/_install/lib -L/home/ppq/x265_3.2.1/build/_install/lib  -L/home/ppq/openssl-1.0.1u/_install/lib -L/home/ppq/git/rtmpdump-2.3/librtmp/_install/lib' \
--extra-libs=-ldl	

./configure --prefix=`pwd`/_install --enable-cross-compile --cpu=cortex-a7 --arch=armv7 --target-os=linux  \
--cross-prefix=arm-himix100-linux- --cc=arm-himix100-linux-gcc \
--enable-shared --enable-gpl --disable-asm --enable-pthreads --disable-programs \
--enable-libx264 \
--extra-cflags='-I../x264-master/_install/include' \
--extra-ldflags='-L../x264-master/_install/lib' \
--extra-libs=-ldl
-mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 

librtmp:
make prefix=`pwd`/_install XCFLAGS+='-I/home/ppq/openssl-1.0.1u/_install/include -I/home/ppq/git/zlib/_install/include' \
XLDFLAGS+='-L/home/ppq/openssl-1.0.1u/_install/lib -L/home/ppq/git/zlib/_install/lib'
libx264:
./configure --prefix=`pwd`/_install --host=arm-himix100-linux --cross-prefix=arm-himix100-linux- --disbale-asm --enable-shared --enable-static
nginx:
./configure --add-module=../nginx-rtmp-module --with-http_ssl_module --prefix=`pwd`/_install
./configure --add-module=../nginx-http-flv-module  --with-http_ssl_module --prefix=`pwd`/_install 

gsoap:
 
./configure --prefix=`pwd`/_install --with-openssl=/home/ppq/openssl-1.0.1u/_install --with-zlib=/home/ppq/git/zlib/_install --enable-samples
./configure --prefix=`pwd`/_install --disable-c-locale --without-openssl --disable-ssl --enable-samples

./wsdl2h -o onvif.h -P -x -c -s -t ./typemap.dat http://www.onvif.org/onvif/ver10/device/wsdl/devicemgmt.wsdl http://www.onvif.org/onvif/ver10/media/wsdl/media.wsdl http://www.onvif.org/onvif/ver10/event/wsdl/event.wsdl http://www.onvif.org/onvif/ver10/display.wsdl http://www.onvif.org/onvif/ver10/deviceio.wsdl http://www.onvif.org/onvif/ver20/imaging/wsdl/imaging.wsdl http://www.onvif.org/onvif/ver20/ptz/wsdl/ptz.wsdl http://www.onvif.org/onvif/ver10/receiver.wsdl http://www.onvif.org/onvif/ver10/recording.wsdl http://www.onvif.org/onvif/ver10/search.wsdl http://www.onvif.org/onvif/ver10/network/wsdl/remotediscovery.wsdl http://www.onvif.org/onvif/ver10/replay.wsdl http://www.onvif.org/onvif/ver20/analytics/wsdl/analytics.wsdl http://www.onvif.org/onvif/ver10/analyticsdevice.wsdl http://www.onvif.org/ver10/actionengine.wsdl http://www.onvif.org/ver10/pacs/accesscontrol.wsdl http://www.onvif.org/ver10/pacs/doorcontrol.wsdl


./wsdl2h -c -s -t typemap.dat -o onvif/onvif.h \
http://www.onvif.org/onvif/ver10/network/wsdl/remotediscovery.wsdl \
http://www.onvif.org/onvif/ver10/device/wsdl/devicemgmt.wsdl \
http://www.onvif.org/onvif/ver20/analytics/wsdl/analytics.wsdl \
http://www.onvif.org/onvif/ver10/analyticsdevice.wsdl \
http://www.onvif.org/onvif/ver10/media/wsdl/media.wsdl \
http://www.onvif.org/onvif/ver10/deviceio.wsdl \
http://www.onvif.org/onvif/ver10/display.wsdl \
http://www.onvif.org/onvif/ver10/event/wsdl/event.wsdl \
http://www.onvif.org/onvif/ver20/imaging/wsdl/imaging.wsdl \
http://www.onvif.org/onvif/ver10/recording.wsdl \
http://www.onvif.org/onvif/ver10/replay.wsdl \
http://www.onvif.org/onvif/ver10/search.wsdl \
http://www.onvif.org/onvif/ver10/receiver.wsdl \
http://www.onvif.org/onvif/ver20/ptz/wsdl/ptz.wsdl 

./wsdl2h -x -c -s -t typemap.dat -o onvif/onvif.h http://www.onvif.org/onvif/ver10/network/wsdl/remotediscovery.wsdl http://www.onvif.org/onvif/ver10/device/wsdl/devicemgmt.wsdl http://www.onvif.org/onvif/ver10/media/wsdl/media.wsdl http://www.onvif.org/onvif/ver10/event/wsdl/event.wsdl  http://www.onvif.org/onvif/ver20/imaging/wsdl/imaging.wsdl
./soapcpp2 -L -x -2 -c onvif.h -I../share/gsoap -I../share/gsoap/import
./soapcpp2 -x -2 -c onvif.h -I../share/gsoap -I../share/gsoap/import

.\wsdl2h.exe -x -c -s -t .\typemap.dat -o onvif/onvif.h http://www.onvif.org/onvif/ver10/network/wsdl/remotediscovery.wsdl http://www.onvif.org/onvif/ver10/device/wsdl/devicemgmt.wsdl http://www.onvif.org/onvif/ver10/media/wsdl/media.wsdl

.\soapcpp2.exe -L -x -2 -C -c onvif.h -I .. -I ..\import 

./soapcpp2 -L -x -C -2 -I../share/gsoap -I../share/gsoap/import -I../share/gsoap/plugin -I../share/gsoap/custom onvif.h

Valgrind：
./configure --host=arm-rockchip-linux-gnueabihf CC=arm-rockchip-linux-gnueabihf-gcc CPP=arm-rockchip-linux-gnueabihf-cpp CXX=arm-rockchip-linux-gnueabihf-g++ AR=arm-rockchip-linux-gnueabihf-ar --prefix=`pwd`/_install
./configure --host=mipsel-openwrt-linux CC=mipsel-openwrt-linux-gcc CPP=mipsel-openwrt-linux-cpp CXX=mipsel-openwrt-linux-g++ --prefix=`pwd`/_install
export VALGRIND_LIB=/data/nfs/valgrind_arm/libexec/valgrind /data/nfs/valgrind_arm/lib/valgrind
export PATH=$PATH:/data/nfs/valgrind_arm/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/nfs/valgrind_arm/libexec/valgrind
valgrind --tool=memcheck

cunit:
autoheader
automake --add-missing
libtoolize --automake --copy --debug --force

./configure --prefix=`pwd`/_install --host=arm-rockchip-linux-gnueabihf CC=arm-rockchip-linux-gnueabihf-gcc

cmake -DCMAKE_C_COMPILER=arm-rockchip-linux-gnueabihf-gcc -DCMAKE_INSTALL_PREFIX=`pwd`/_install ../

make install DESTDIR=../_install

opencv：BUILD_SHARED_LIBS
-DBUILD_JAVA=OFF -DBUILD_opencv_java=OFF

dlib: -DCMAKE_BUILD_TYPE=Debug Release   
cmake .. -DCMAKE_INSTALL_PREFIX=./_install -DCMAKE_BUILD_TYPE=Release -DDLIB_NO_GUI_SUPPORT=OFF -DCMAKE_C_COMPILER=arm-rockchip-linux-gnueabihf-gcc -DCMAKE_CXX_COMPILER=arm-rockchip-linux-gnueabihf-g++ 
cmake .. -DCMAKE_INSTALL_PREFIX=./_install -DCMAKE_BUILD_TYPE=Release -DDLIB_NO_GUI_SUPPORT=OFF -DCMAKE_C_COMPILER=arm-himix200-linux-gcc -DCMAKE_CXX_COMPILER=arm-himix200-linux-g++ 