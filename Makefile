CC=gcc
FLAGS=-Wall -pthread

all:	test
test: 
	mkdir output
	mkdir poutput
	mkdir pwoutput
	mkdir goutput
	$(CC) -o ctest test.c thread.c $(FLAGS)
	$(CC) -o ptest ptest.c $(FLAGS)
	$(CC) -o wtest pworkertest.c $(FLAGS)
	./ptest
	./ptest
	./wtest
	./wtest
	./ctest
	./ctest

clean: 
	rm *test
	rm -rf output
	rm -rf poutput
	rm -rf pwoutput
	rm -rf goutput
