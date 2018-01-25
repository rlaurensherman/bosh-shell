# Lauren Sherman
# Makefile

bosh: bosh.o
	gcc -o bosh bosh.o
bosh.o: bosh.c
	gcc -ansi -pedantic -Wall -D_POSIX_C_SOURCE=200809L -g -c bosh.c

clean:
	\rm -f *.o bosh
