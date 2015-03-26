CC=clang
CFLAGS=-I. -Wall  -std=c11 -D_BSD_SOURCE -Wno-unused-variable -g -lcurl -lhiredis -lpthread -ltidy -rdynamic -pg -fprofile-arcs -ftest-coverage -pg
DEPS = regexlib.h redisconnector.h slavedriver.h 
OBJ = master.o regexlib.o redisconnector.o slavedriver.o  

all: master worker

%.o: %.c $(DEPS)
	        $(CC) $(CFLAGS) -c -o $@ $<
master: $(OBJ)
	        $(CC) $(CFLAGS) -o $@ $^
worker: redisconnector.o  regexlib.o
	        $(CC) $(CFLAGS) crawler.c -o $@ $^
clean:
	rm -rf *.o crawler master worker *.out *.gcno *.gcda
install:
	echo "Test application, no install."

