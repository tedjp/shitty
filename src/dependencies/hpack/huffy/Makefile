.PHONY: all check clean

CC = gcc -std=gnu99 -pipe -fPIC -Wall -g

all: bitbuf.o encode.o decode.o encode-table.o

encode-table.c: huffcodes.txt make-etable.sh
	./make-etable.sh < $< > $@

encode.c: encode-table.c encode-table.h

encode.o: encode.c encode.h
	$(CC) -c -o $@ $<

encode-test: encode-test.c encode.o ../number.c
	$(CC) -o $@ $^

bitbuf.o: bitbuf.c bitbuf.h
	$(CC) -c -o $@ $<

bitbuf-test: bitbuf-test.c bitbuf.o
	$(CC) -o $@ $^

decode.o: decode.c decode.h
	$(CC) -c -o $@ $<

decode-test: decode-test.c decode.o bitbuf.o ../number.o
	$(CC) -o $@ $^

random-test: random-test.c encode.o decode.o bitbuf.o ../number.o
	$(CC) -o $@ $^

TESTS = bitbuf-test encode-test decode-test random-test
check: $(TESTS)
	set -e; \
	for t in $(TESTS); do \
		echo Executing $$t; \
		./$$t; \
	done

clean:
	rm -f $(TESTS) *.o encode-table.c
