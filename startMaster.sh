#!/bin/bash
	echo 'starting master 1'	
	nohup ./master 0 50000 &> master1.out&
	sleep  .1
	echo 'starting master 2'	
	nohup ./master 50000 100000 &> master2.out&
	sleep  .1
	echo 'starting master 3'	
	nohup ./master 100000 150000 &> master3.out&
	sleep  .1
	echo 'starting master 4'	
	nohup ./master 150000 200000 &> master4.out&
