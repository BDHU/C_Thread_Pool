CC=gcc
FLAGS=-Wall -pthread

all: test

run: test

test:  
	$(CC) -o ctest test.c thread.c $(FLAGS)
	./ctest
	./ctest
	$(CC) -o ptest ptest.c $(FLAGS)
	./ptest
	./ptest
	$(CC) -o wtest pworkertest.c $(FLAGS)
	./wtest
	./wtest
clean: rmo
	rm *test
rmo:
	rm output/tmp-*
	rm poutput/tmp-*
	rm pwoutput/tmp-*
       
