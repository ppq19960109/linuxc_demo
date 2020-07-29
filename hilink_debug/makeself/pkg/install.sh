#!/bin/sh

TOPDIR=`dirname $0`
echo $TOPDIR
echo "start"

#app path
APP_PATH=$TOPDIR$TOPDIR/ #/userdata/app
#app name
APP_NAME=hilinkapp

cp -rf $APP_NAME $APP_PATH/

chmod -R 777 $APP_PATH/
sync
# reboot
exit 0