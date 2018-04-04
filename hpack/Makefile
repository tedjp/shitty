.PHONY: all check clean

all: header.o number.o

number.o: number.c number.h
	gcc -Wall -c -o $@ $<

header.o: header.cpp header.h number.h huffy/decode.h
	g++ -Wall -Wextra -c -o $@ $<

test: test.c number.o
	gcc -Wall -o $@ $^

check: test
	./$<

clean:
	rm -f test *.o
