#!/bin/bash

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
MAKESELF_FILE="/home/ppq/git/makeself/makeself.sh"
$MAKESELF_FILE --notemp $PKG_DIRNAME $PKG_FINAL_FILENAME "Haryon hilink package v1.0.0" ./$PKG_INSTALL_FILE

./$PKG_FINAL_FILENAME --list
#makeself check
./$PKG_FINAL_FILENAME --check

if [ $? -ne 0 ]; then
    echo "Package failed"
    exit 1;
fi
echo "Package successful"