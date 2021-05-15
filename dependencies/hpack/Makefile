.PHONY: all check clean

all: header.o number.o

number.o: number.c number.h
	gcc -std=gnu99 -Wall -c -o $@ $<

header.o: header.cpp header.h number.h huffy/decode.h
	g++ -std=gnu++11 -Wall -Wextra -c -o $@ $<

test: test.c number.o
	gcc -std=gnu99 -Wall -o $@ $^

check: test
	./$<

clean:
	rm -f test *.o
