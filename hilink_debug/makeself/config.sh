#!/bin/sh

#src directory
APP_PATH="$TOPDIR/demo"
HILINK_CONFIG_PATH="$TOPDIR/config"
#compress directory and compress file name
PKG_DIRNAME="pkg"
PKG_FINAL_FILENAME="upgrade.bin"

#upgrade file name
PKG_INSTALL_FILE="install.sh"
APP_FILE="$APP_PATH/hilinkapp"
HILINK_CONFIG_FILE="$HILINK_CONFIG_PATH/hilink.cfg"
HILINK_BAK_CONFIG_FILE="$HILINK_CONFIG_PATH/hilink_bak.cfg"

UPGRADE_FILE="$PKG_INSTALL_FILE $APP_FILE $HILINK_CONFIG_FILE $HILINK_BAK_CONFIG_FILE"
echo "UPGRADE_FILE:"$UPGRADE_FILE

#add compress directory
mkdir -p $PKG_DIRNAME
rm -rf $PKG_FINAL_FILENAME
rm -rf $PKG_DIRNAME/*
#copy src files to directory
cp -rf $UPGRADE_FILE $PKG_DIRNAME/