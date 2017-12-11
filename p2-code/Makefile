# Authors: Evan Welch & Jakob Urnes

CFLAGS = -Wall -g
CC = gcc $(CFLAGS)

bl-server : bl-server.o server.o simpio.o util.o blather.h
	$(CC) -o bl-server bl-server.o server.o simpio.o util.o

bl-server.o : bl-server.c blather.h
	$(CC) -lpthread -c bl-server.c

server.o : server.c blather.h
	$(CC) -c server.c

bl-client : bl-client.o simpio.o util.o blather.h
	$(CC) -o bl-client bl-client.o simpio.o util.o

bl-client.o : bl-client.c blather.h
	$(CC)  -lpthread -c bl-client.c

clean :
	rm -f *.o
