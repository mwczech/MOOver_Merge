#!/bin/sh

# (re)aktiviere PCM Speaker wieder (Bug 4657)
amixer set 'Master Mono' on
amixer set 'PCM Playback to Phone' on
alsactl store

# Add copyExport into crontab
crontab -l | grep copyExport > /dev/null
if [ $? == 1 ]
  then
    COPYEXPORT_CRON_LINE="00 01 * * * /root/copyExportToSD.sh | logger -t copyExportToSD.sh"
    ( echo "$COPYEXPORT_CRON_LINE"; crontab -l ) | crontab -
    echo "Add copyExportToSD to crontab"
fi

#mach shell-skripts ausf�hrbar
chmod 777 *.sh
dos2unix *.sh
dos2unix *.xdd
dos2unix *.cfg
dos2unix *.sql
dos2unix usbdev_automount
dos2unix usbdisk automount

# richte ftp access rights her
chmod g+s ftp/import ftp/export ftp/logplc
chown ftp ftp/import ftp/export ftp/PCSoftware
chgrp ftp ftp/import ftp/export ftp/PCSoftware

# Add checkIP into crontab
crontab -l | grep checkIP > /dev/null
if [ $? == 1 ]
  then
    CHECKIP_CRON_LINE="*/10 * * * * /root/checkIP.sh 2>&1 | logger -t checkIP.sh"
    ( echo "$CHECKIP_CRON_LINE"; crontab -l ) | crontab -
    echo "Add checkIP to crontab"
fi

# Starte PlcLogService
./PlcLogService -d /root/ftp/logplc -s &

# Starte IP_FinderServer
./IPFinderServer -t Butler & 

# Starte Datenbankserver
./dbserver --tcp --bind-addr=0.0.0.0 --threads=10 &> dbServer.log &

# Starte Can Message Manager
ip link show can1 > /dev/null
if [ $? != 0 ]
  then
    echo "CAN1 dont exists"
    ./canmsgmgr -P 5333 -L 2 -I 0 &> c1.log &
  else
    echo "CAN1 exists"
    ./canmsgmgr -P 5333 -L 2 -I 1 &> c1.log &
fi

# Start CAN logger
./CANLogger -f butlerCanLog.xdd &

if [ -d /root/canFlash ]; then
  cd /root/canFlash
  ./startCanFlash_GUI.sh
  exit
fi

if test -f /root/butler.db 
  then
      START_WAIT_TIME=35
  else
      START_WAIT_TIME=55
fi

./ButlerEngine &> e1.log &

sleep $START_WAIT_TIME

./startGUI.sh

./checkIP.sh

#./CanView -qws -display "VNC:1" -c canview.cfg >> canview.log
