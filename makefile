CC=clang
CFLAGS=-I. -Wall  -std=c11 -D_BSD_SOURCE -Wno-unused-variable -fcolor-diagnostics -g -lcurl -lhiredis
DEPS = regexlib.h -redisconnector.h
OBJ = master.o regexlib.o redisconnector.o

%.o: %.c $(DEPS)
	        $(CC) $(CFLAGS) -c -o $@ $<

master: $(OBJ)
	        $(CC) $(CFLAGS) -o $@ $^
clean:
	rm -rf *.o crawler master
install:
	echo "Test application, no install."

