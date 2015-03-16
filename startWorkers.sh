for i in `seq 1 $1`; 
do
	echo 'starting worker '$i	
	nohup ./worker &
	sleep  .1
done	
