CFLAGS = -Wall -g
CC = gcc $(CFLAGS)

commando : commando.o, cmd.o, cmdctl.o commando.h
	$(CC) -o commando commando.o cmd.o cmdctl.o

commando.o : commando.c commando.h
	$(CC) -c commando.c

cmd.o : cmd.c commando.h
	$(CC) -c cmd.c

cmdctl.o : cmdctl.c commando.h
	$(CC) -c cmdctl.c

test-binary : binary_tests.c commando.h tests.h
	$(CC) -o test-binary binary_tests.c

clean :
	rm -f *.o
