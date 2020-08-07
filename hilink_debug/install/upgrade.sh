#!/bin/sh

TOPDIR=`dirname $0`
cd $TOPDIR
echo $0
PKG_TAR_FILENAME=upgrade.tar
echo $PKG_TAR_FILENAME

# sed -n -e '1,/^exit 0$/!p' $0|sed -n -e '1!p'>$PKG_TAR_FILENAME
lines=`sed -n -e '1,/^exit 0$/p' $0| wc -l | awk '{printf $1}'`
lines=`expr $lines + 2`
echo $lines
line=`expr $lines - 1`
tail -n +$lines $0 > $PKG_TAR_FILENAME
# tail -n +$lines $0 | tar x -C ./
sync
# ENCRY1=`sed -n -e '1,/^exit 0$/!p' $0|sed -n -e '1p'`
ENCRY1=`head -n $line $0|tail -n 1`
echo $ENCRY1

ENCRY2=`md5sum $PKG_TAR_FILENAME | awk '{printf $1}'`
echo $ENCRY2

if [ $ENCRY1 != $ENCRY2 ]; then
    echo Verification error
    exit 1
fi
#---------------------------------------------------------------
tar -xvf $PKG_TAR_FILENAME
sync
#------------------------------------------
echo start install
UPDATE_PATH=pkg
#target path
APP_PATH=/app
HILINK_CONFIG_PATH=/userdata/hilink
#app name
APP_NAME=hilinkapp
HILINK_CONFIG_FILE=hilink.cfg
HILINK_BAK_CONFIG_FILE=hilink_bak.cfg

cp -rf $UPDATE_PATH/$APP_NAME $APP_PATH/
cp -rf $UPDATE_PATH/$HILINK_CONFIG_FILE $HILINK_CONFIG_PATH/
cp -rf $UPDATE_PATH/$HILINK_BAK_CONFIG_FILE $HILINK_CONFIG_PATH/

chmod -R 777 $APP_PATH/
sleep 1
rm -rf $PKG_TAR_FILENAME $UPDATE_PATH/
sync
echo Successfully installed
echo App reboot.......
killall $APP_NAME
$APP_PATH/$APP_NAME &
# reboot
exit 0
