CC=gcc
FLAGS=-Wall -pthread

all: test

run: test
	./ctest

test: test.o thread.c 
	$(CC) -o ctest test.c thread.c $(FLAGS)
	./ctest --mutex
	./ctest 

clean: 
	rm *.o
	rm ptest
	rm ctest
