#!/bin/sh

echo "start install......"

APP_PATH="app"
HYAPP_PATH="hyapp"
IOTAPP_PATH="iotapp"
LIB_PATH="usrlibs"
OEM_PATH="oem"
TARGET_APP_PATH="/userdata/app"
TARGET_HYAPP_PATH="/userdata/hyapp"
TARGET_IOTAPP_PATH="/userdata/iotapp"
TARGET_LIB_PATH="/userdata/usrlibs"
TARGET_OEM_PATH="/oem"
#Add operation permission
chmod -R 777 ./
#kill app
ALL_APP="hydaemon hy_server_iot hydevapp alinkapp hilinkapp"
killall $ALL_APP

USER_PATH="/userdata"
IOTAPP_DEVICE_JSON="/userdata/iotapp/devices.json"


cp -a $APP_PATH/* $TARGET_APP_PATH

#hilink start
HILINK_CONFIG_PATH="/userdata/hilink"
APP_HILINK_CONFIG_FILE="/userdata/app/hilink*.cfg"
if [ ! -d "$HILINK_CONFIG_PATH" ]; then
    mkdir "$HILINK_CONFIG_PATH"
fi
cp -r $APP_HILINK_CONFIG_FILE $HILINK_CONFIG_PATH
#hilink end

cp -a $HYAPP_PATH/* $TARGET_HYAPP_PATH
cp -a $IOTAPP_PATH/* $TARGET_IOTAPP_PATH
mv -f $IOTAPP_DEVICE_JSON $USER_PATH

cp -a $LIB_PATH/* $TARGET_LIB_PATH
cp -a $OEM_PATH/* $TARGET_OEM_PATH

#delete files
rm -rf *

UPDATE_FILE="../upgrade.bin"
UPDATE_BACKUP_FILE="/userdata/update/upgrade_backup.bin"

if [ -e "$UPDATE_FILE" ]; then
    mv -f $UPDATE_FILE $UPDATE_BACKUP_FILE
    rm -f $UPDATE_FILE
fi
sync
echo "Successfully installed"

echo "App reboot......"
reboot
