#!/bin/sh
#folgendes ist dafuer in /var/spool/cron/crontabs/root einzutragen (ohne #)
#00 01 * * * /root/copyExportToSD.sh | logger -t copyExportToSD.sh

TARGET=/mnt/mmc/user/

cd /root/ftp/export/
FIRST=$(ls | tail -n1)

echo $FIRST

SECOND=$(echo $FIRST | tr 'Butler' 'auto')
cp $FIRST $TARGET$SECOND

#removes all but the last 10 files
cd $TARGET
ls -tp /mnt/mmc/user/ | grep -v '/$' | tail -n +11 | tr '*' ' ' | xargs rm -f

