redis-cli flushall
redis-cli -p 6380 flushall

./stopWorkers.sh
./stopMasters.sh

rm -rf /var/log/crawler/
