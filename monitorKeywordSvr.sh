#!/bin/bash  
  

WORK_DIR="/home/ubuntu/google/src/build"
PNAME=keyword

while true  
do   
    ulimit -c unlimited
    procnum=` ps -ef|grep $PNAME |grep -v grep|wc -l`  
   if [ $procnum -eq 0 ]; then
	echo "$PNAME service was not started" 
	echo "Starting service ..." 
	$WORK_DIR/$PNAME
	echo "$PNAME service was exited!"
   else 	 
	echo "$PNAME service was already started by another way" 
	break;
   fi  
   sleep 10  
done 

echo "it's over to run the script"
