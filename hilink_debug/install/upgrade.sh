#!/bin/bash

TOPDIR=`dirname $0`
cd $TOPDIR
echo $0","$TOPDIR

PKG_TAR_FILENAME=upgrade.tar

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
UPDATE_PATH="pkg"
rm -rf $UPDATE_PATH

tar -xvf $PKG_TAR_FILENAME
sync
rm -rf $PKG_TAR_FILENAME
#------------------------------------------
UPGRADE_SCRIPT_FILE="install.sh"

cd $UPDATE_PATH
source $UPGRADE_SCRIPT_FILE

exit 0
