for i in `seq 1 $1`; 
do
	echo 'starting worker '$i	
	nohup ./worker &> /var/log/crawler/worker$i.out
	sleep  .1
done	
