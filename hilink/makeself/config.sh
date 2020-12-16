#!/bin/sh

#src directory
SRC_APP_PATH="$TOPDIR/demo"
HILINK_CONFIG_PATH="$TOPDIR/config"
#compress directory and compress file name
PKG_DIRNAME="pkg"
PKG_FINAL_FILENAME="upgrade.bin"

#upgrade file name
PKG_INSTALL_FILE="install.sh"


HILINK_CONFIG_FILE="$HILINK_CONFIG_PATH/hilink.cfg $HILINK_CONFIG_PATH/hilink_bak.cfg"
APP_FILE="$SRC_APP_PATH/hilinkapp $SRC_APP_PATH/hyDaemon $HILINK_CONFIG_FILE"

echo "APP_FILE:"$APP_FILE

#add compress directory
mkdir -p $PKG_DIRNAME
rm -rf $PKG_FINAL_FILENAME
rm -rf $PKG_DIRNAME/*
#copy src files to directory
cp -r $PKG_INSTALL_FILE $PKG_DIRNAME/

APP_PATH=$INSTALL_PATH/"app"
cp -r $APP_FILE $APP_PATH

HYAPP_PATH=$INSTALL_PATH/"hyapp"
IOTAPP_PATH=$INSTALL_PATH/"iotapp"
LIB_PATH=$INSTALL_PATH/"usrlibs"
OEM_PATH=$INSTALL_PATH/"oem"
cp -a $APP_PATH $PKG_DIRNAME/
cp -a $HYAPP_PATH $PKG_DIRNAME/
cp -a $IOTAPP_PATH $PKG_DIRNAME/
cp -a $LIB_PATH $PKG_DIRNAME/
cp -a $OEM_PATH $PKG_DIRNAME/
