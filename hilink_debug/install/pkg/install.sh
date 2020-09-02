#!/bin/sh

echo "start install..........."
APP_PATH="/app"
HILINK_CONFIG_PATH="/userdata/hilink"
#app name
APP_NAME="hilinkapp"
HILINK_CONFIG_FILE="hilink.cfg"
HILINK_BAK_CONFIG_FILE="hilink_bak.cfg"

cp -rf $APP_NAME $APP_PATH/
cp -rf $HILINK_CONFIG_FILE $HILINK_CONFIG_PATH/
cp -rf $HILINK_BAK_CONFIG_FILE $HILINK_CONFIG_PATH/

chmod -R +x $APP_PATH/

rm -rf *
sync
echo "Successfully installed"
killall $APP_NAME
$APP_PATH/$APP_NAME &
echo "App reboot......."