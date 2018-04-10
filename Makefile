CC=gcc

all: test

test: test.o thread.c 
	 $(CC) -o test test.c thread.c -pthread
	 ./test

clean: 
	rm *.o
