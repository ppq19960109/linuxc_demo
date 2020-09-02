#!/bin/sh

APP_PATH=$TOPDIR/demo
HILINK_CONFIG_PATH=$TOPDIR/config

PKG_DIRNAME="pkg"
PKG_TAR_FILENAME="upgrade.tar"
PKG_FINAL_FILENAME="upgrade.bin"
PKG_HASHI_FILENAME="hash.txt"

#start
PKG_INSTALL_FILE="install.sh"
APP_FILE="$APP_PATH/hilinkapp"
HILINK_CONFIG_FILE="$HILINK_CONFIG_PATH/hilink.cfg"
HILINK_BAK_CONFIG_FILE="$HILINK_CONFIG_PATH/hilink_bak.cfg"

UPGRADE_FILE="$PKG_INSTALL_FILE $APP_FILE $HILINK_CONFIG_FILE $HILINK_BAK_CONFIG_FILE"

echo "UPGRADE_FILE:"$UPGRADE_FILE

#end

mkdir -p $PKG_DIRNAME
rm -f $PKG_TAR_FILENAME $PKG_HASHI_FILENAME $PKG_FINAL_FILENAME
rm -rf $PKG_DIRNAME/*

cp -rf $UPGRADE_FILE $PKG_DIRNAME/