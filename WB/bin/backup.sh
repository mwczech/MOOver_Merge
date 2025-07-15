#!/bin/sh
logger -t backup.sh copy backup to micro sd card
find /mnt/mmc/root/b* -mtime +400 -exec rm -r {} \;
find /mnt/mmc/root/ftp/* -mtime +400 -exec rm -r {} \;
/root/ittiasql </root/backup.sql> /root/res.txt
cat res.txt
MMCPRESENT=$(find /mnt/mmc* | wc -l)
if [ $MMCPRESENT != 0 ]; then
	rsync -av /root/b2* /mnt/mmc/root
	rsync -av /root/ftp/logplc /mnt/mmc/root/ftp --no-perms
	rsync -av /root/settings /mnt/mmc/root
fi


#29.5.2020 Reiter:
#modified "find -exec" so that it removes ALL files whose modification date is older then 400 days.