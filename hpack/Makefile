.PHONY: all check clean

all: number.o

number.o: number.c number.h
	gcc -Wall -c -o $@ $<

test: test.c number.o
	gcc -Wall -o $@ $^

check: test
	./$<

clean:
	rm -f test *.o
