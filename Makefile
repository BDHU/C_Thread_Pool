CC=gcc
FLAGS=-Wall -pthread

all: test

run: test
	./test

test: test.o thread.c 
	$(CC) -o test test.c thread.c $(FLAGS)
	./test

clean: 
	rm *.o
