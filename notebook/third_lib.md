openssl:
./config no-asm -shared --prefix=$(pwd)/_install
makefile中搜索-m64选项并删除，共两处
make CROSS_COMPILE=arm-rockchip-linux-gnueabihf-

sqlite3:
./configure --prefix=`pwd`/_install --host=arm-rockchip-linux-gnueabihf CC=arm-rockchip-linux-gnueabihf-gcc
./configure --prefix=`pwd`/_install --host=mipsel-openwrt-linux CC=/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-gcc

paho-mqtt-c:
cmake .. -DCMAKE_INSTALL_PREFIX=$(pwd)/_install -DPAHO_BUILD_STATIC=TRUE -DCMAKE_C_COMPILER=arm-rockchip-linux-gnueabihf-gcc -DPAHO_WITH_SSL=TRUE -DCMAKE_C_FLAGS+=/home/ppq/git/openssl/_install/include -DCMAKE_LIBRARY_PATH+=/home/ppq/git/openssl/_install/lib
cmake .. -DCMAKE_INSTALL_PREFIX=$(pwd)/_install -DPAHO_BUILD_STATIC=TRUE -DCMAKE_C_COMPILER=/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-gcc

curl:
./configure --prefix=$(pwd)/_install --host=arm-rockchip-linux-gnueabihf CC=arm-rockchip-linux-gnueabihf-gcc --with-ssl=/home/ppq/git/openssl/_install
./configure --without-zlib --without-ssl --prefix=`pwd`/_install --host=mipsel-openwrt-linux CC=/home/ppq/lede-toolchain-ramips-mt7688_gcc-5.4.0_musl-1.1.16.Linux-x86_64/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16/bin/mipsel-openwrt-linux-gcc 

ffmpeg:
pkg-config --exists librtmp||echo no

export PKG_CONFIG_PATH=/home/ppq/x265_3.2.1/build/_install/lib/pkgconfig:/home/ppq/git/rtmpdump-2.3/librtmp/_install/lib/pkgconfig:/home/ppq/openssl-1.0.1u/_install/lib/pkgconfig

./configure --prefix=`pwd`/_install --enable-shared --enable-nonfree --enable-gpl --disable-asm \
--enable-libx264 --enable-libx265 --enable-libmp3lame --enable-libxvid --enable-libfdk-aac --enable-openssl --enable-librtmp \
--extra-cflags='-I/home/ppq/git/x264-master/_install/include -I/home/ppq/lame-3.100/_install/include -I/home/ppq/xvidcore/build/generic/_install/include -I/home/ppq/fdk-aac-2.0.1/_install/include -I/home/ppq/x265_3.2.1/build/_install/include  -I/home/ppq/openssl-1.0.1u/_install/include  -I/home/ppq/git/rtmpdump-2.3/librtmp/_install/include' \
--extra-ldflags='-L/home/ppq/git/x264-master/_install/lib -L/home/ppq/lame-3.100/_install/lib -L/home/ppq/xvidcore/build/generic/_install/lib -L/home/ppq/fdk-aac-2.0.1/_install/lib -L/home/ppq/x265_3.2.1/build/_install/lib  -L/home/ppq/openssl-1.0.1u/_install/lib -L/home/ppq/git/rtmpdump-2.3/librtmp/_install/lib' \
--extra-libs=-ldl	

librtmp:
make prefix=`pwd`/_install XCFLAGS+='-I/home/ppq/openssl-1.0.1u/_install/include -I/home/ppq/git/zlib/_install/include' \
XLDFLAGS+='-L/home/ppq/openssl-1.0.1u/_install/lib -L/home/ppq/git/zlib/_install/lib'

nginx:
./configure --add-module=../nginx-rtmp-module --with-http_ssl_module --prefix=`pwd`/_install
./configure --add-module=../nginx-http-flv-module  --with-http_ssl_module --prefix=`pwd`/_install 

gsoap:
--prefix=`pwd`/_install  
./configure --prefix=`pwd`/_install --with-openssl=/home/ppq/openssl-1.0.1u/_install --with-zlib=/home/ppq/git/zlib/_install --enable-samples

./wsdl2h -o onvif/onvif.h -c -s -t ./typemap.dat https://www.onvif.org/ver10/network/wsdl/remotediscovery.wsdl https://www.onvif.org/ver10/device/wsdl/devicemgmt.wsdl https://www.onvif.org/ver10/media/wsdl/media.wsdl

./soapcpp2 -2 -c onvif/onvif.h  -x -I . -I import -I custom -I plugin -d onvif 
soap_in_xsd__duration