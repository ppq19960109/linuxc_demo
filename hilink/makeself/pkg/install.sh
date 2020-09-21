#!/bin/sh

echo "start install......"
#Add operation permission
chmod -R 777 ./
#dest path 
APP_PATH="/userdata/app"
HILINK_CONFIG_PATH="/userdata/hilink"
OEM_PATH="/oem"
#src file name
APP_NAME="hilinkapp"
HILINK_CONFIG_FILE="hilink.cfg"
HILINK_BAK_CONFIG_FILE="hilink_bak.cfg"
START_APP_FILE="start_app.sh"
#kill app
killall $APP_NAME
killall hydevapp
killall hy_server_iot
#copy files to path
if [ ! -d "$HILINK_CONFIG_PATH" ]; then
    mkdir "$HILINK_CONFIG_PATH"
fi
cp -rf $APP_NAME $APP_PATH/
cp -rf $HILINK_CONFIG_FILE $HILINK_CONFIG_PATH/
cp -rf $HILINK_BAK_CONFIG_FILE $HILINK_CONFIG_PATH/
cp -rf $START_APP_FILE $OEM_PATH/

HYAPP_PATH="hyapp"
IOTAPP_PATH="iotapp"
LIB_PATH="usrlibs"
TARGET_HYAPP_PATH="/userdata/hyapp"
TARGET_IOTAPP_PATH="/userdata/iotapp"
TARGET_LIB_PATH="/userdata/usrlibs"
cp -a $HYAPP_PATH/* $TARGET_HYAPP_PATH
cp -a $IOTAPP_PATH/* $TARGET_IOTAPP_PATH
mv $TARGET_IOTAPP_PATH/devices.json /userdata
cp -a $LIB_PATH/* $TARGET_LIB_PATH

#delete files
UPDATE_PATH="/userdata/update"
rm -rf *
rm -rf $UPDATE_PATH/*
sync
echo "Successfully installed"
#app reboot
cd $TARGET_IOTAPP_PATH
./hy_server_iot > /dev/null &
cd $TARGET_HYAPP_PATH
./hydevapp > /dev/null &
cd $APP_PATH
./$APP_NAME &
echo "App reboot......"