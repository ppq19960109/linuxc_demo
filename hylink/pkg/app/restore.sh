#!/bin/sh

USER_PATH="/userdata"
HILINK_CONFIG_PATH="/userdata/hilink"

APP_PATH="/userdata/app"
HYAPP_PATH="/userdata/hyapp"
IOTAPP_PATH="/userdata/iotapp"

APP_HILINK_CONFIG_FILE="/userdata/app/hilink*.cfg"
ALL_APP="runing.sh hydaemon hilinkapp hy_server_iot hydevapp"
#kill app
killall $ALL_APP
sleep 5

echo "Return to factory......"
rm -rf $HILINK_CONFIG_PATH/*
cp -r $APP_HILINK_CONFIG_FILE $HILINK_CONFIG_PATH

rm -f $HYAPP_PATH/*.db 

rm -f $USER_PATH/*.txt $USER_PATH/*.db* $USER_PATH/crash/* $USER_PATH/logSeq/* $USER_PATH/zigbee*
sync
echo "reboot......"
#app reboot
reboot
