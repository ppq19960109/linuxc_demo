#!/bin/sh

USER_PATH="/userdata"
APP_PATH="/userdata/app"
HYAPP_PATH="/userdata/hyapp"
IOTAPP_PATH="/userdata/iotapp"

ALL_APP="hydaemon hy_server_iot hydevapp hilinkapp"
#kill app
killall $ALL_APP

#hilink start
HILINK_CONFIG_PATH="/userdata/hilink"
APP_HILINK_CONFIG_FILE="/userdata/app/hilink*.cfg"
rm -rf $HILINK_CONFIG_PATH/*
cp -r $APP_HILINK_CONFIG_FILE $HILINK_CONFIG_PATH
#hilink end

sleep 5
echo "Return to factory......"

rm -f $HYAPP_PATH/*.db 

rm -f $USER_PATH/*.txt $USER_PATH/*.db* $USER_PATH/crash/* $USER_PATH/logSeq/* $USER_PATH/zigbee*
sync
echo "reboot......"
#app reboot
reboot
