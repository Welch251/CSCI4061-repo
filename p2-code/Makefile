# Authors: Evan Welch & Jakob Urnes

CFLAGS = -Wall -g
CC = gcc $(CFLAGS)

all : bl-server bl-client

bl-server : bl-server.o server.o simpio.o util.o blather.h
	$(CC) -o bl-server bl-server.o server.o simpio.o util.o

bl-server.o : bl-server.c blather.h
	$(CC) -c bl-server.c

server.o : server.c blather.h
	$(CC) -c server.c

util.o : util.c blather.h
	$(CC) -c util.c

simpio.o : simpio.c blather.h
	$(CC) -c simpio.c

bl-client : bl-client.o simpio.o util.o blather.h
	$(CC) -o bl-client bl-client.o simpio.o util.o -lpthread

bl-client.o : bl-client.c blather.h
	$(CC) -c bl-client.c

shell-tests : shell_tests.sh shell_tests_data.sh cat-sig.sh clean-tests
	chmod u+rx shell_tests.sh shell_tests_data.sh cat-sig.sh normalize.awk filter-semopen-bug.awk
	./shell_tests.sh

clean :
	rm -f *.o

clean-tests :
	rm -f test-*.{log,out,expect,diff,valgrindout}

real-clean :
	rm -f *.o *.fifo bl-server bl-client
