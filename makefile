CC=clang
CFLAGS=-I. -Wall  -std=c11 -D_BSD_SOURCE -Wno-unused-variable -fcolor-diagnostics -g -lcurl -lhiredis -lpthread
DEPS = regexlib.h redisconnector.h slavedriver.h 
OBJ = master.o regexlib.o redisconnector.o slavedriver.o  

all: master worker

%.o: %.c $(DEPS)
	        $(CC) $(CFLAGS) -c -o $@ $<
master: $(OBJ)
	        $(CC) $(CFLAGS) -o $@ $^
worker: redisconnector.o 
	        $(CC) $(CFLAGS) crawler.c -o $@ $^
clean:
	rm -rf *.o crawler master worker
install:
	echo "Test application, no install."

