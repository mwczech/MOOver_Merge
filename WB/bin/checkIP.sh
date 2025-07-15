#!/bin/bash

#protect to run process more times
sleep 1
PROCESSNAME=`basename $0`
COUNT=`ps | grep -v grep | grep -v logger | grep -wc ${PROCESSNAME}`

if [ ${COUNT} -gt 2 ] ; then
   echo $PROCESSNAME is runnig - exit actual instance
   exit 1
fi

# in case that cable is not connected and udhcpc is running - stop udhcpc
# it happend at start, iflpugd call interface_up function -> eth0 think that has connected cable for some seconds

ifconfig eth0 | grep RUNNING > /dev/null 2>&1
if [ $? == 1 ] ; then
   ps | grep udhcpc | grep eth0 > /dev/null 2>&1
   if [ $? == 0 ] ; then
      sleep 5
      ifconfig eth0 | grep RUNNING > /dev/null 2>&1
      if [ $? == 1 ] ; then
         echo cable not connected 2   
         ps | grep udhcpc | grep eth0 > /dev/null 2>&1
         if [ $? == 0 ] ; then
            UDHCPCPID=`cat /var/run/udhcpc.eth0.pid`
            echo "udhcpc eth0 kill" $UDHCPCPID
            kill $UDHCPCPID
         fi
      fi
   fi 
fi
 

# check openvpn connection
ping -c5 10.8.0.1 > /dev/null 2>&1
if [ $? == 1 ] ; then
   ping -c5 service.wasserbauer.at > /dev/null 2>&1
   if [ $? == 0 ] ; then
      echo "internet OK, openvpn ERR"
      sleep 10
      
      ping -c5 10.8.0.1 > /dev/null 2>&1
      if [ $? == 1 ] ; then 
         ping -c5 service.wasserbauer.at > /dev/null 2>&1
         if [ $? == 0 ] ; then
            echo "internet OK, openvpn ERR -> restart openvpn"
            /etc/init.d/S55openvpn restart
         fi
      fi
   fi
else
   ifconfig | grep tun0 > /dev/null 2>&1
   if [ $? == 1 ] ; then
      echo "ping to 10.8.0.1 - OK, tun not exist -> restart openvpn"
      /etc/init.d/S55openvpn restart
   fi
fi
