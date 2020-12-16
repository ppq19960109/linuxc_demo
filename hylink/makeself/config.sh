#!/bin/sh

#compress directory and compress file name
PKG_DIRNAME="pkg"
PKG_FINAL_FILENAME="upgrade.bin"

#upgrade file name
PKG_INSTALL_FILE="install.sh"

#add compress directory
mkdir -p $PKG_DIRNAME
rm -rf $PKG_FINAL_FILENAME
rm -rf $PKG_DIRNAME/*
#copy src files to directory
cp -r $PKG_INSTALL_FILE $PKG_DIRNAME/

APP_PATH=$INSTALL_PATH/"app"
HYAPP_PATH=$INSTALL_PATH/"hyapp"
IOTAPP_PATH=$INSTALL_PATH/"iotapp"
LIB_PATH=$INSTALL_PATH/"usrlibs"
OEM_PATH=$INSTALL_PATH/"oem"
cp -a $APP_PATH $PKG_DIRNAME/
cp -a $HYAPP_PATH $PKG_DIRNAME/
cp -a $IOTAPP_PATH $PKG_DIRNAME/
cp -a $LIB_PATH $PKG_DIRNAME/
cp -a $OEM_PATH $PKG_DIRNAME/
