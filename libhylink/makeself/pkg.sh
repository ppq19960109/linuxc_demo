#!/bin/bash

if [ -z "$1" ];then
echo "input null"
exit 1
fi
echo $1

if [ "$1" == "alink" ];then
    echo "pkg alink"
    make -C daemon clean all 'CFLAGS+=-DALINK'
elif [ "$1" == "hilink" ];then
    echo "pkg hilink"
    make -C daemon clean all 'CFLAGS+=-DHLINK'
else
    echo "input error"
    exit 1
fi

cp daemon/hydaemon makeself/app

#pack directory
TOPDIR=`pwd`
INSTALL_PATH="$TOPDIR/makeself"
cd $INSTALL_PATH


#execute config file
CONFIG_FILE="config.sh"
source $CONFIG_FILE

if [ $? -ne 0 ]; then
    echo "config failed"
    exit 1;
fi
#makeself pack
MAKESELF_FILE="./makeself/makeself.sh"
$MAKESELF_FILE --notemp $PKG_DIRNAME $PKG_FINAL_FILENAME "Honyar link package v1.0.0" ./$PKG_INSTALL_FILE $1

./$PKG_FINAL_FILENAME --list
#makeself check
./$PKG_FINAL_FILENAME --check

if [ $? -ne 0 ]; then
    echo "Package failed"
    exit 1;
fi
echo "Package successful........"
