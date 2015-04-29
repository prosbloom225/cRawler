#!/bin/bash
for i in `seq 1 $1`; 
do
	echo 'starting worker '$i	
	nohup ./worker ftb.osieckim.com 6379 &>> /var/log/crawler/worker.out&
	sleep  .1
done	
