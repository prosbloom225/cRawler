CC=clang
CFLAGS=-I. -Wall  -std=c11 -D_BSD_SOURCE -Wno-unused-variable -fcolor-diagnostics -g -lcurl -lhiredis -O0
DEPS = regexlib.h -redisconnector.h
OBJ = master.o regexlib.o redisconnector.o

all: worker master

%.o: %.c $(DEPS)
	        $(CC) $(CFLAGS) -c -o $@ $<
master: $(OBJ)
	        $(CC) $(CFLAGS) -o $@ $^
worker: crawler.c
		$(CC) $(CFLAGS) -o $@ $^
clean:
	rm -rf *.o crawler master worker
install:
	echo "Test application, no install."

