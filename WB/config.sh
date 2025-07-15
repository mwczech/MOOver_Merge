#!/bin/sh

# Do an updated of the linux kernel with file linux.bin on the usbstick
DO_KERNEL=y

# Do an updated of the root filesystem with file rootfs.img on the usbstick
DO_ROOTFS=y

# Make a backup of 
#  -the database files 
#  -and the VPN-files
#  -the setting-file
#  -all log files
# on the board and copy them back to the board after the board has been flashed
DO_BACKUP=y

# Copy all Linux-Binaries in the folder 'bin' on the usbstick to the board.
DO_BINARIES=y

# Copy the folder VPN on the usbstick to the root-filesystem on the board
DO_VPN=n

# Copy the settings file on the usbstick on the root-filesystem on the board
DO_SETTINGS=n

# Copy everything from USBStick/bin/canFlash onto rootfs/canFlash
#	-> after booting, canFlash is executed
DO_UPDATE_CAN=n
