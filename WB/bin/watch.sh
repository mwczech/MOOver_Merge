#!/bin/sh
exit

#******************************
# Engine
#******************************
PID=$(pidof ButlerEngine) 
if [ "$PID" = "" ]; then
	cp e1.log ./ftp/logplc/e1.log
	echo "Engine not running - reboot!"
	#screenshot der GUI machen und im FTP-ordner abspeichern
	fbgrab /root/ftp/logplc/`date +'%Y-%m-%d-%H-%M-%S'`EngineFailure.png
	reboot
	exit 0
fi
#******************************
# GUI
#******************************
PID=$(pidof ButlerEvo)
if [ "$PID" = "" ]; then
        echo "GUI stopped - reboot!"
	#screenshot der GUI machen und im FTP-ordner abspeichern
	fbgrab /root/ftp/logplc/`date +'%Y-%m-%d-%H-%M-%S'`GuiFailure.png
	reboot
	exit 0
fi
#******************************
# canmsgmgr
#******************************
PID=$(pidof canmsgmgr)
if [ "$PID" = "" ]; then
        echo "canmsgmgr stopped - reboot!"
	#screenshot der GUI machen und im FTP-ordner abspeichern
	fbgrab /root/ftp/logplc/`date +'%Y-%m-%d-%H-%M-%S'`CanMsgMgrFailure.png
	reboot
	exit 0
fi
#******************************
# dbserver
#******************************
PID=$(pidof dbserver)
if [ "$PID" = "" ]; then
        echo "dbserver stopped - reboot!"
	#screenshot der GUI machen und im FTP-ordner abspeichern
	fbgrab /root/ftp/logplc/`date +'%Y-%m-%d-%H-%M-%S'`DbServerFailure.png
       	reboot
	exit 0
fi
#******************************
# PlcLogService
#******************************
PID=$(pidof PlcLogService)
if [ "$PID" = "" ]; then
        echo "PLCLogService stopped - reboot!"
	#screenshot der GUI machen und im FTP-ordner abspeichern
	fbgrab /root/ftp/logplc/`date +'%Y-%m-%d-%H-%M-%S'`PlcLogServiceFailure.png
	reboot
	exit 0
fi
