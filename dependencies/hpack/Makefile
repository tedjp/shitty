.PHONY: all check clean

all: libhpack.a

libhpack.a: header.o number.o
	$(MAKE) -C huffy
	ar rcs $@ $^ \
		huffy/bitbuf.o \
		huffy/decode.o \
		huffy/encode.o \
		#

number.o: number.c number.h
	gcc -std=gnu99 -Wall -fPIC -c -o $@ $<

header.o: header.cpp header.h number.h huffy/decode.h
	g++ -std=gnu++11 -Wall -Wextra -fPIC -c -o $@ $<

test: test.c number.o
	gcc -std=gnu99 -Wall -o $@ $^

check: test
	./$<

clean:
	$(MAKE) -C huffy clean
	rm -f test libhpack.a *.o
