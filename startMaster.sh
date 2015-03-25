#!/bin/bash
	echo 'starting master 1'	
	nohup  ./master 0 40000 &> /var/log/crawler/master1.out&
	sleep  .1
	echo 'starting master 2'	
	nohup  ./master 40000 80000 &> /var/log/crawler/master2.out&
	sleep  .1
	echo 'starting master 3'	
	nohup  ./master 80000 120000 &> /var/log/crawler/master3.out&
	sleep  .1
	echo 'starting master 4'	
	nohup  ./master 120000 160000 &> /var/log/crawler/master4.out&
	sleep  .1
	echo 'starting master 5'	
	nohup  ./master 160000 200000 &> /var/log/crawler/master5.out&

