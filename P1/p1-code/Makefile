# Authors: Evan Welch & Jakob Urnes

CFLAGS = -Wall -g
CC = gcc $(CFLAGS)

commando : commando.o cmd.o cmdctl.o util.o commando.h
	$(CC) -o commando commando.o cmd.o cmdctl.o util.o

commando.o : commando.c commando.h
	$(CC) -c commando.c

cmd.o : cmd.c commando.h
	$(CC) -c cmd.c

cmdctl.o : cmdctl.c commando.h
	$(CC) -c cmdctl.c

util.o : util.c commando.h
	$(CC) -c util.c

binary_tests.o : binary_tests.c commando.h tests.h
	$(CC) -c $<

test_utils.o : test_utils.c tests.h
	$(CC) -c $<

binary_tests : binary_tests.o cmd.o cmdctl.o test_utils.o
	$(CC) -o $@ $^

test-binary : binary_tests
	valgrind ./binary_tests

test-shell : commando
	./shell_tests.sh

clean :
	rm -f *.o actual.txt expect.txt valgrind.txt

realclean :
	rm -f *.o actual.std binary_tests commando expect.std test_args
