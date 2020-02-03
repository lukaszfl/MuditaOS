#!/bin/bash
# set -e

if [ -z $1 ]; then
  BUILD_PATH=./build
else
  BUILD_PATH=$1
fi

PURE_DEV=/dev/disk/by-id/usb-NXP_SEMI_NXP_MASS_STORAGE_0123456789ABCDEF-0:0
PURE_DISK=`readlink -f $PURE_DEV`
# check if already mouted

MOUNT_ENTRY=$(grep "$PURE_DISK" /etc/mtab)
if [ $? -ne 0 ]; then
	>&2 echo "PurePhone not mounted. Mount or retry with Ozone"2
	exit 2
fi
PURE_PATH=$(echo $MOUNT_ENTRY | awk -F " " '{print $2}')
# unespace
PURE_PATH=$(echo -e $PURE_PATH)


echo "PurePhone remove all files"
rm "$PURE_PATH"/* -r &>/dev/null || echo "PurePhone disk alread empty"
sudo sync $PURE_DEV

echo "PurePhone copy build files"
cp -v $BUILD_PATH/boot.bin "$PURE_PATH"/
cp -v $BUILD_PATH/sys/* "$PURE_PATH"/ -r

sudo sync $PURE_DEV
echo "PurePhone copied"

sudo umount "$PURE_PATH"
echo "PurePhone unmouted"
