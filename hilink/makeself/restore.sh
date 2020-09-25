#!/bin/sh

HILINK_CONFIG_FILE="hilink.cfg"
HILINK_BAK_CONFIG_FILE="hilink_bak.cfg"

APP_PATH="/userdata/app"
HILINK_CONFIG_PATH="/userdata/hilink"

echo "Return to factory......"
rm -rf $HILINK_CONFIG_PATH/*
cp -rf $APP_PATH/$HILINK_CONFIG_FILE $APP_PATH/$HILINK_BAK_CONFIG_FILE $HILINK_CONFIG_PATH/

echo "reboot......"
#reboot
