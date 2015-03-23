#!/bin/bash
	echo 'starting master 1'	
	nohup time ./master 0 10000 &> master1.out&
	sleep  .1
	echo 'starting master 2'	
	nohup time ./master 10000 20000 &> master2.out&
	sleep  .1
	echo 'starting master 3'	
	nohup time ./master 30000 40000 &> master3.out&
	sleep  .1
	echo 'starting master 4'	
	nohup time ./master 40000 50000 &> master4.out&

	# echo 'starting master 1'	
	# nohup time ./master 50000 60000 &> master1.out&
	# sleep  .1
	# echo 'starting master 2'	
	# nohup time ./master 60000 70000 &> master2.out&
	# sleep  .1
	# echo 'starting master 3'	
	# nohup time ./master 70000 80000 &> master3.out&
	# sleep  .1
	# echo 'starting master 4'	
	# nohup time ./master 80000 90000 &> master4.out&
