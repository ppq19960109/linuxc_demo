#!/bin/sh

HILINK_CONFIG_FILE="hilink.cfg"
HILINK_BAK_CONFIG_FILE="hilink_bak.cfg"

USER_PATH="/userdata"
HILINK_CONFIG_PATH="/userdata/hilink"

APP_PATH="/userdata/app"
TARGET_HYAPP_PATH="/userdata/hyapp"
TARGET_IOTAPP_PATH="/userdata/iotapp"

#kill app
killall hilinkapp
killall hydevapp
killall hy_server_iot
echo "Return to factory......"
rm -rf $HILINK_CONFIG_PATH/*
cp -rf $APP_PATH/$HILINK_CONFIG_FILE $APP_PATH/$HILINK_BAK_CONFIG_FILE $HILINK_CONFIG_PATH/

rm -f $TARGET_HYAPP_PATH/*.db 

rm -f $USER_PATH/*.txt $USER_PATH/*.db* $USER_PATH/crash/* $USER_PATH/logSeq/* $USER_PATH/zigbee*
sync
echo "reboot......"
#app reboot
# cd $TARGET_IOTAPP_PATH
# ./hy_server_iot > /dev/null &
# cd $TARGET_HYAPP_PATH
# ./hydevapp > /dev/null &
# cd $APP_PATH
# ./hilinkapp &
reboot
