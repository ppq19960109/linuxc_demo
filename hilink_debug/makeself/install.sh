#!/bin/sh

echo "start install......"
#dest path 
APP_PATH="/app"
HILINK_CONFIG_PATH="/userdata/hilink"
#src file name
APP_NAME="hilinkapp"
HILINK_CONFIG_FILE="hilink.cfg"
HILINK_BAK_CONFIG_FILE="hilink_bak.cfg"
#copy files to path
cp -rf $APP_NAME $APP_PATH/
cp -rf $HILINK_CONFIG_FILE $HILINK_CONFIG_PATH/
cp -rf $HILINK_BAK_CONFIG_FILE $HILINK_CONFIG_PATH/
#Add operation permission
chmod -R +x $APP_PATH/
#delete files
rm -rf *
sync
echo "Successfully installed"
#app reboot
killall $APP_NAME
$APP_PATH/$APP_NAME &
echo "App reboot......"