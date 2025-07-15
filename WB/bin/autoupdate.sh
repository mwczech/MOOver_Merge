
chmod 777 ittiasql
chmod 777 canmsgmgr
chmod 777 updatemessage.sql


ittiasql <updatemessage.sql> /dev/null

sleep 30




nohup killall -9 crond
nohup killall -9 ButlerEvo
nohup killall -9 KMDB_QT_PlainGUI
nohup killall -9 HKDB_QT_GUI
nohup killall -9 HKDB_QT_Engine
nohup killall -9 ButlerEngine
nohup killall -9 KMDB_QT_Engine
nohup killall -9 canmsgmgr 
nohup killall -9 CANLogger 
nohup killall -9 dbserver
nohup killall -9 PlcLogService
nohup killall -9 IPFinderServer




nohup ./canmsgmgr -P 5333 -L 2 -I 1 &> c1.log &


#die Software draufspielen:


cd /root/update/canflash
chmod 777 canflash
./canflash -n1 -f magnetLineal.hex
cd ..
cd ..


#die parameters updaten (geht schnell)


cd /root/update/canflash
./canflash -n1 -x  magnetLineal.xdd
cd ..
cd ..

cd /root
nohup killall -9 canmsgmgr


rm -r /mnt/mmc/Update*

mkdir /mnt/mmc/Update_Sicherung

cp -f -r * /mnt/mmc/Update_Sicherung

cd update

cp -f -r * /root 


cd ..

chmod 777 KMDB_QT_Engine
chmod 777 KMDB_QT_Engine.xdd 
chmod 777 KMDB_QT_PlainGUI
chmod 777 HKDB_QT_Engine
chmod 777 HKDB_QT_Engine.xdd 
chmod 777 HKDB_QT_GUI
chmod 777 ButlerEvo
chmod 777 ButlerEngine
chmod 777 *.xdd
chmod 777 *.sh
chmod 777 DDMap.cfg
chmod 777 PlcLogService
chmod 777 DDMap.cfg
chmod 777 *.wav
chmod 777 canmsgmgr
chmod 777 dbserver
chmod 777 backup.sql
chmod 777 setting*
chmod 777 CAN*



nohup ./startall.sh

rm update

sleep 120 
crond

exit

